#include <HW_HAL.hpp>
#include <VNA.hpp>
#include "Si5351C.hpp"
#include "max2871.hpp"
#include "main.h"
#include "delay.hpp"
#include "FPGA/FPGA.hpp"
#include <complex>
#include "Exti.hpp"
#include "Hardware.hpp"
#include "Communication.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Util.hpp"
#include "usb.h"
#include "Trigger.hpp"
#include <cmath>
#include <array>

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"VNA"
#include "Log.h"

static Protocol::SweepSettings settings;
static uint16_t pointCnt;
static uint8_t stageCnt;
static uint32_t last_LO2;
static double logMultiplier, logFrequency;
static Protocol::VNADatapoint<32> data;
static bool active = false;
static bool waitingInStandby = false;
static Si5351C::DriveStrength fixedPowerLowband;
static bool adcShifted;
static uint32_t actualBandwidth;

static uint64_t firstPointTime;
static bool firstPoint;
static bool zerospan;

static constexpr uint8_t sourceHarmonic = 5;
static constexpr uint8_t LOHarmonic = 3;

static constexpr float alternativeSamplerate = 914285.7143f;
static constexpr uint8_t alternativePrescaler = 102400000UL / alternativeSamplerate;
static_assert(alternativePrescaler * alternativeSamplerate == 102400000UL, "alternative ADCSamplerate can not be reached exactly");
static constexpr uint16_t alternativePhaseInc = 4096 * HW::DefaultIF2 / alternativeSamplerate;
static_assert(alternativePhaseInc * alternativeSamplerate == 4096 * HW::DefaultIF2, "DFT can not be computed for 2.IF when using alternative samplerate");

// Constants for USB buffer overflow prevention
static constexpr uint16_t maxPointsBetweenHalts = 40;
static constexpr uint32_t reservedUSBbuffer = maxPointsBetweenHalts * (sizeof(Protocol::Datapoint) + 8 /*USB packet overhead*/);

using namespace HWHAL;

static constexpr uint8_t alternativePrescalers[] = {112, 113, 114, 115};

static uint8_t LO2_adjustment[FPGA::MaxPoints * 5 / 2];

static void setLO2Adjustment(unsigned int point, int32_t value) {
	if(value >= 1L<<20) {
		value = (1L<<20)-1;
	} else if(value <-(1L<<20)) {
		value = -(1L<<20);
	}
	uint16_t base = point / 2 * 5;
	if(point & 0x01) {
		LO2_adjustment[base+2] = (LO2_adjustment[base+2] & 0x0F) | ((value<<4) & 0xF0);
		LO2_adjustment[base+3] = (value >> 4) & 0xFF;
		LO2_adjustment[base+4] = (value >> 12) & 0xFF;
	} else {
		LO2_adjustment[base+0] = value & 0xFF;
		LO2_adjustment[base+1] = (value >> 8) & 0xFF;
		LO2_adjustment[base+2] = ((value >> 16) & 0x0F) | (LO2_adjustment[base+2] & 0xF0);
	}
}
static int32_t getLO2Adjustment(unsigned int point) {
	uint16_t base = point / 2 * 5;
	uint32_t ret = 0;
	if(point & 0x01) {
		ret |= (LO2_adjustment[base+2] & 0xF0) >> 4;
		ret |= (uint16_t) LO2_adjustment[base+3] << 4;
		ret |= (uint32_t) LO2_adjustment[base+4] << 12;
	} else {
		ret |= LO2_adjustment[base+0];
		ret |= (uint16_t) LO2_adjustment[base+1] << 8;
		ret |= (uint32_t) (LO2_adjustment[base+2]&0x0F) << 16;
	}
	// sign extend
	constexpr uint32_t m = 1U << (20 - 1);
	ret = (ret ^ m) - m;
	return ret;
}

static uint32_t defaultLO2;

static uint64_t getPointFrequency(uint16_t pointNum) {
	if(!settings.logSweep) {
		return settings.f_start + (settings.f_stop - settings.f_start) * pointNum / (settings.points - 1);
	} else {
		static uint16_t lastPointNum = 0;
		if (pointNum == 0) {
			logFrequency = settings.f_start;
		} else if(pointNum == lastPointNum) {
			// nothing to do
		} else if(pointNum == lastPointNum + 1) {
			logFrequency *= logMultiplier;
		} else {
			logFrequency = settings.f_start * pow(10.0, pointNum * log10((double)settings.f_stop / settings.f_start) / (settings.points - 1));
		}
		lastPointNum = pointNum;
		return logFrequency;
	}
}

static uint32_t closestLOAlias(uint64_t LO1, uint64_t LO2, uint32_t IFBW, uint32_t ADC_Samplerate) {
	constexpr uint64_t max_LO_harmonic = 2000000000;
	constexpr uint32_t max_ADC_alias = 5000000;

	uint32_t closestAlias = std::numeric_limits<uint32_t>::max();

	for(int64_t lo1 = LO1; lo1 <= (int64_t) max_LO_harmonic; lo1 += LO1) {
		// figure out which 2.LO harmonics we have to check
		uint64_t lo2_min = lo1 - max_ADC_alias;
		uint64_t lo2_max = lo1 + max_ADC_alias;

		uint16_t lo2_min_harm = ((lo2_min + LO2 - 1) / LO2);
		uint16_t lo2_max_harm = lo2_max / LO2;

		if(lo2_max_harm * LO2 > max_LO_harmonic) {
			lo2_max_harm = max_LO_harmonic / LO2;
		}

		if(lo2_min_harm > lo2_max_harm) {
			// no aliasing possible, skip 2.LO loop
			continue;
		}

		for(int64_t lo2 = LO2 * lo2_min_harm; lo2 <= (int64_t) LO2 * lo2_max_harm; lo2 += LO2) {
			uint32_t mixing = llabs(lo1 - lo2);
			if(mixing > max_ADC_alias) {
				continue;
			}
			int32_t alias = Util::Alias(mixing, ADC_Samplerate);
			uint32_t alias_dist = labs((int32_t) HW::getIF2() - alias);
			if(alias_dist < closestAlias) {
				closestAlias = alias_dist;
			}
//			if(abs(HW::getIF2() - alias) <= IFBW*3) {
//				// we do have LO mixing products aliasing into the 2.IF
//				return false;
//			}
		}
	}
	return closestAlias;
}

//static bool noLOAliasing(uint64_t LO1, uint64_t LO2, uint32_t IFBW) {
//	constexpr uint64_t max_LO_harmonic = 2000000000;
//	constexpr uint32_t max_ADC_alias = 5000000;
//
//	for(int64_t lo1 = LO1; lo1 <= (int64_t) max_LO_harmonic; lo1 += LO1) {
//		// figure out which 2.LO harmonics we have to check
//		uint64_t lo2_min = lo1 - max_ADC_alias;
//		uint64_t lo2_max = lo1 + max_ADC_alias;
//
//		uint16_t lo2_min_harm = ((lo2_min + LO2 - 1) / LO2);
//		uint16_t lo2_max_harm = lo2_max / LO2;
//
//		if(lo2_max_harm * LO2 > max_LO_harmonic) {
//			lo2_max_harm = max_LO_harmonic / LO2;
//		}
//
//		if(lo2_min_harm > lo2_max_harm) {
//			// no aliasing possible, skip 2.LO loop
//			continue;
//		}
//
//		for(int64_t lo2 = LO2 * lo2_min_harm; lo2 <= (int64_t) LO2 * lo2_max_harm; lo2 += LO2) {
//			uint32_t mixing = llabs(lo1 - lo2);
//			if(mixing > max_ADC_alias) {
//				continue;
//			}
//			int32_t alias = Util::Alias(mixing, HW::getADCRate());
//			if(abs(HW::getIF2() - alias) <= IFBW*3) {
//				// we do have LO mixing products aliasing into the 2.IF
//				return false;
//			}
//		}
//	}
//	// all good, no aliasing
//	return true;
//}

static void setPLLFrequencies(uint64_t f, uint32_t current2LO, uint32_t IFBW, uint32_t *new2LO) {
	uint64_t actualSource;
	// set the source, this will never change
	if (f > HW::Info.limits_maxFreq) {
		Source.SetFrequency(f / sourceHarmonic);
		actualSource = Source.GetActualFrequency() * sourceHarmonic;
	} else if (f >= HW::BandSwitchFrequency) {
		Source.SetFrequency(f);
		actualSource = Source.GetActualFrequency();
	} else {
		// source will be set in sweep halted interrupt
		actualSource = f;
	}

	// set the 1.LO
	uint64_t actual1LO;
	if(f > HW::Info.limits_maxFreq) {
		LO1.SetFrequency((f + HW::getIF1()) / LOHarmonic);
		actual1LO = LO1.GetActualFrequency() * LOHarmonic;
	} else {
		LO1.SetFrequency(f + HW::getIF1());
		actual1LO = LO1.GetActualFrequency();
	}
	// adjust 2.LO if necessary
	*new2LO = current2LO;
	uint32_t actualFirstIF = actual1LO - actualSource;
	uint32_t actualFinalIF = actualFirstIF - *new2LO;
	uint32_t IFdeviation = abs(actualFinalIF - HW::getIF2());
	if(IFdeviation > IFBW / 2) {
		*new2LO = actualFirstIF - HW::getIF2();
	}
}

static bool findBestADCRate(uint64_t LO1, uint32_t LO2, uint32_t IFBW, FPGA::ADCSamplerate &bestRate) {

	const std::array<uint8_t, 5> ADC_prescalers = { HW::DefaultADCprescaler,
			alternativePrescalers[0], alternativePrescalers[1],
			alternativePrescalers[2], alternativePrescalers[3] };
	const std::array<FPGA::ADCSamplerate, 5> returnArray = {
			FPGA::ADCSamplerate::Default, FPGA::ADCSamplerate::Alt1,
			FPGA::ADCSamplerate::Alt2, FPGA::ADCSamplerate::Alt3,
			FPGA::ADCSamplerate::Alt4 };

	uint8_t maxIndex = ADC_prescalers.size();

	if(!settings.suppressPeaks) {
		maxIndex = 1;
	}

	uint8_t bestIndex = 0;
	uint32_t furthestAliasDistance = 0;

	for(uint8_t i = 0;i<maxIndex;i++) {
//		LOG_ERR("Checking F=%lu, SRC=%lu, LO1=%lu", (uint32_t) f, (uint32_t) actualSource, (uint32_t) actual1LO);

		uint32_t rate = FPGA::Clockrate / ADC_prescalers[i];

		auto closest_alias = closestLOAlias(LO1, LO2, IFBW, rate);
		if(closest_alias > IFBW * 3) {
			// no need to look further, chose this option
			bestRate = returnArray[i];
			return true;
		} else if(closest_alias > furthestAliasDistance) {
			bestIndex = i;
			furthestAliasDistance = closest_alias;
		}

//		// check if LO mixing product aliases into the ADC
//		if(noLOAliasing(actual1LO, *new2LO, IFBW)) {
//			// found an IF that can be used without problems
//			if(shift != 0) {
////				LOG_WARN("Shifting IF for f=%lu, LO1=%lu, LO2= %lu", (uint32_t) f, (uint32_t) actual1LO, *new2LO);
//			}
//			return true;
//		}
	}
	// all available IF shifts result in aliasing in the ADC
//	LOG_ERR("Failed to shift IF for f=%lu", (uint32_t) f);
	// no perfect option, use best shift
//	auto shift = IF_shifts[bestIndex];
//	// set the 1.LO
//	uint64_t actual1LO;
//	if(f > HW::Info.limits_maxFreq) {
//		LO1.SetFrequency((f + HW::getIF1() + shift) / LOHarmonic);
//		actual1LO = LO1.GetActualFrequency() * LOHarmonic;
//	} else {
//		LO1.SetFrequency(f + HW::getIF1() + shift);
//		actual1LO = LO1.GetActualFrequency();
//	}
//	// adjust 2.LO if necessary
//	*new2LO = current2LO;
//	uint32_t actualFirstIF = actual1LO - actualSource;
//	uint32_t actualFinalIF = actualFirstIF - *new2LO;
//	uint32_t IFdeviation = abs(actualFinalIF - HW::getIF2());
//	if(IFdeviation > IFBW / 2) {
//		*new2LO = actualFirstIF - HW::getIF2();
//	}
	bestRate = returnArray[bestIndex];
	return false;
}

static bool shift1IF(uint64_t f, uint32_t current2LO, uint32_t IFBW, uint32_t *new2LO, uint32_t ADC_rate) {
	const std::array<uint32_t, 21> IF_shifts = {0, IFBW * 2, IFBW * 3,
			IFBW * 5, IFBW * 7, IFBW * 7 / 10, IFBW * 11 / 10, IFBW * 13
					/ 10, IFBW * 17 / 10, IFBW * 19 / 10, IFBW, IFBW * 23 / 10,
			IFBW * 29 / 10, IFBW * 31 / 10, IFBW * 37 / 10, IFBW * 41 / 10, IFBW
					* 43 / 10, IFBW * 47 / 10, IFBW * 53 / 10, IFBW * 59 / 10,
			IFBW * 61 / 10 };

//	const std::array<uint32_t, 8> IF_shifts = {0, 10, 33, 100, 330, 1000, 3300, 10000};

	uint64_t actualSource;
	// set the source, this will never change
	if (f > HW::Info.limits_maxFreq) {
		actualSource = Source.GetActualFrequency() * sourceHarmonic;
	} else if (f >= HW::BandSwitchFrequency) {
		actualSource = Source.GetActualFrequency();
	} else {
		// source will be set in sweep halted interrupt
		actualSource = f;
	}

	uint8_t maxIndex = IF_shifts.size();
	if(!settings.suppressPeaks) {
		maxIndex = 1;
	}

	uint8_t bestIndex = 0;
	uint32_t furthestAliasDistance = 0;

	for(uint8_t i = 0;i<maxIndex;i++) {
		auto shift = IF_shifts[i];
		// set the 1.LO
		uint64_t actual1LO;
		if(f > HW::Info.limits_maxFreq) {
			LO1.SetFrequency((f + HW::getIF1() + shift) / LOHarmonic);
			actual1LO = LO1.GetActualFrequency() * LOHarmonic;
		} else {
			LO1.SetFrequency(f + HW::getIF1() + shift);
			actual1LO = LO1.GetActualFrequency();
		}
		// adjust 2.LO if necessary
		*new2LO = current2LO;
		uint32_t actualFirstIF = actual1LO - actualSource;
		uint32_t actualFinalIF = actualFirstIF - *new2LO;
		uint32_t IFdeviation = abs(actualFinalIF - HW::getIF2());
		if(IFdeviation > IFBW / 2) {
			*new2LO = actualFirstIF - HW::getIF2();
		}

//		LOG_ERR("Checking F=%lu, SRC=%lu, LO1=%lu", (uint32_t) f, (uint32_t) actualSource, (uint32_t) actual1LO);

		auto closest_alias = closestLOAlias(actual1LO, *new2LO, IFBW, ADC_rate);
		if(closest_alias > IFBW * 3) {
			// no need to look further, chose this option
			return true;
		} else if(closest_alias > furthestAliasDistance) {
			bestIndex = i;
			furthestAliasDistance = closest_alias;
		}

//		// check if LO mixing product aliases into the ADC
//		if(noLOAliasing(actual1LO, *new2LO, IFBW)) {
//			// found an IF that can be used without problems
//			if(shift != 0) {
////				LOG_WARN("Shifting IF for f=%lu, LO1=%lu, LO2= %lu", (uint32_t) f, (uint32_t) actual1LO, *new2LO);
//			}
//			return true;
//		}
	}
	// all available IF shifts result in aliasing in the ADC
//	LOG_ERR("Failed to shift IF for f=%lu", (uint32_t) f);
	// no perfect option, use best shift
	auto shift = IF_shifts[bestIndex];
	// set the 1.LO
	uint64_t actual1LO;
	if(f > HW::Info.limits_maxFreq) {
		LO1.SetFrequency((f + HW::getIF1() + shift) / LOHarmonic);
		actual1LO = LO1.GetActualFrequency() * LOHarmonic;
	} else {
		LO1.SetFrequency(f + HW::getIF1() + shift);
		actual1LO = LO1.GetActualFrequency();
	}
	// adjust 2.LO if necessary
	*new2LO = current2LO;
	uint32_t actualFirstIF = actual1LO - actualSource;
	uint32_t actualFinalIF = actualFirstIF - *new2LO;
	uint32_t IFdeviation = abs(actualFinalIF - HW::getIF2());
	if(IFdeviation > IFBW / 2) {
		*new2LO = actualFirstIF - HW::getIF2();
	}
	return false;
}

//static bool needs2LOshift(uint64_t f, uint32_t current2LO, uint32_t IFBW, uint32_t *new2LO) {
//	// Check if 2.LO needs to be shifted
//	uint64_t actualSource, actual1LO;
//	actualSource = Source.GetActualFrequency();
//	actual1LO = LO1.GetActualFrequency();
//	if(f > HW::Info.limits_maxFreq) {
//		actualSource *= sourceHarmonic;
//		actual1LO *= LOHarmonic;
//	} else if(f < HW::BandSwitchFrequency) {
//		// can use the lowband PLL with high frequency resolution, assume perfect frequency match
//		actualSource = f;
//	}
//	uint32_t actualFirstIF = actual1LO - actualSource;
//	uint32_t actualFinalIF = actualFirstIF - current2LO;
//	uint32_t IFdeviation = abs(actualFinalIF - HW::getIF2());
//	if(IFdeviation > IFBW / 2) {
//		*new2LO = actualFirstIF - HW::getIF2();
//		return true;
//	} else {
//		// no shift required
//		return false;
//	}
//}

bool VNA::Setup(Protocol::SweepSettings s) {
	// Abort possible active sweep first
	VNA::Stop();
	vTaskDelay(5);
	data.clear();
	HW::SetMode(HW::Mode::VNA);
	FPGA::SetMode(FPGA::Mode::FPGA);
	// Configure the ADC prescalers
	FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, HW::getADCPrescaler());
	FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, HW::getDFTPhaseInc());
	FPGA::WriteRegister(FPGA::Reg::ADCPrescalerAlt1, alternativePrescalers[0]);
	FPGA::WriteRegister(FPGA::Reg::PhaseIncrementAlt1, alternativePrescalers[0]*10);
	FPGA::WriteRegister(FPGA::Reg::ADCPrescalerAlt2, alternativePrescalers[1]);
	FPGA::WriteRegister(FPGA::Reg::PhaseIncrementAlt2, alternativePrescalers[1]*10);
	FPGA::WriteRegister(FPGA::Reg::ADCPrescalerAlt3, alternativePrescalers[2]);
	FPGA::WriteRegister(FPGA::Reg::PhaseIncrementAlt3, alternativePrescalers[2]*10);
	FPGA::WriteRegister(FPGA::Reg::ADCPrescalerAlt4, alternativePrescalers[3]);
	FPGA::WriteRegister(FPGA::Reg::PhaseIncrementAlt4, alternativePrescalers[3]*10);
	if(settings.points > FPGA::MaxPoints) {
		settings.points = FPGA::MaxPoints;
	}
	settings = s;
	// calculate factor between adjacent points for log sweep for faster calculation when sweeping
	logMultiplier = pow((double) settings.f_stop / settings.f_start, 1.0 / (settings.points-1));
	// Configure sweep
	FPGA::SetNumberOfPoints(settings.points);
	uint32_t samplesPerPoint = (HW::getADCRate() / s.if_bandwidth);
	// round up to next multiple of 16 (16 samples are spread across 5 IF2 periods)
	if(samplesPerPoint%16) {
		samplesPerPoint += 16 - samplesPerPoint%16;
	}
	actualBandwidth = HW::getADCRate() / samplesPerPoint;
	// has to be one less than actual number of samples
	FPGA::SetSamplesPerPoint(samplesPerPoint);

	// reset unlevel flag if it was set from a previous sweep/mode
	HW::SetOutputUnlevel(false);
	// Start with average level
	auto cdbm = (s.cdbm_excitation_start + s.cdbm_excitation_stop) / 2;
	// correct for port 1, assumes port 2 is identical
	auto centerFreq = (s.f_start + s.f_stop) / 2;
	// force calculation of amplitude setting for PLL, even with lower frequencies
	if(centerFreq < HW::BandSwitchFrequency) {
		centerFreq = HW::BandSwitchFrequency;
	}
	auto amplitude = HW::GetAmplitudeSettings(cdbm, centerFreq, true, false);
	if(amplitude.unlevel) {
		HW::SetOutputUnlevel(true);
	}

	uint8_t fixedAttenuatorHighband = amplitude.attenuator;
	Source.SetPowerOutA(amplitude.highBandPower, true);

	// amplitude calculation for lowband
	amplitude = HW::GetAmplitudeSettings(cdbm, HW::BandSwitchFrequency / 2, true, false);
	if(amplitude.unlevel) {
		HW::SetOutputUnlevel(true);
	}
	uint8_t fixedAttenuatorLowband = amplitude.attenuator;
	fixedPowerLowband = amplitude.lowBandPower;

	FPGA::WriteMAX2871Default(Source.GetRegisters());

	last_LO2 = HW::getIF1() - HW::getIF2();
	defaultLO2 = last_LO2;
	Si5351.SetCLK(SiChannel::Port1LO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.SetCLK(SiChannel::Port2LO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.SetCLK(SiChannel::RefLO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.ResetPLL(Si5351C::PLL::B);
	Si5351.WaitForLock(Si5351C::PLL::B, 10);

	zerospan = (s.f_start == s.f_stop) && (s.cdbm_excitation_start == s.cdbm_excitation_stop);

	bool last_lowband = false;

	uint16_t pointsWithoutHalt = 0;

	// Transfer PLL configuration to FPGA
	for (uint16_t i = 0; i < settings.points; i++) {
		uint64_t freq = getPointFrequency(i);
		int16_t power = s.cdbm_excitation_start + (s.cdbm_excitation_stop - s.cdbm_excitation_start) * i / (settings.points - 1);
		freq = Cal::FrequencyCorrectionToDevice(freq);

		bool needs_halt = false;
		bool lowband = false;
		if (freq < HW::BandSwitchFrequency) {
			needs_halt = true;
			lowband = true;
		}
		if(i == 0) {
			// halt before first point (makes sure that the 2.LO gets configured correctly if it was shifted for the last point in
			// the previous sweep
			needs_halt = true;
		}
		// SetFrequency only manipulates the register content in RAM, no SPI communication is done.
		// No mode-switch of FPGA necessary here.
		uint32_t new2LO;
		setPLLFrequencies(freq, last_LO2, actualBandwidth, &new2LO);
		FPGA::ADCSamplerate ADC_rate;
		if(!findBestADCRate(LO1.GetActualFrequency(), new2LO, actualBandwidth, ADC_rate)) {
			// all available ADC rates result in a spike
			uint8_t presc;
			switch(ADC_rate) {
			case FPGA::ADCSamplerate::Default: presc = HW::DefaultADCprescaler; break;
			case FPGA::ADCSamplerate::Alt1: presc = alternativePrescalers[0]; break;
			case FPGA::ADCSamplerate::Alt2: presc = alternativePrescalers[1]; break;
			case FPGA::ADCSamplerate::Alt3: presc = alternativePrescalers[2]; break;
			case FPGA::ADCSamplerate::Alt4: presc = alternativePrescalers[3]; break;
			}
			uint32_t samplerate = FPGA::Clockrate / presc;
			shift1IF(freq, new2LO, actualBandwidth, &new2LO, samplerate);
		}
//		LOG_WARN("F %lu 1.LO: %lu 2.LO: %lu ADC: %d", (uint32_t) freq, (uint32_t) LO1.GetActualFrequency(), new2LO, (int) ADC_rate);
//		vTaskDelay(10);
		if(new2LO != last_LO2 && s.suppressPeaks) {
			last_LO2 = new2LO;
			needs_halt = true;
		}

		if (last_lowband && !lowband) {
			// additional halt before first highband point to enable highband source
			needs_halt = true;
		}

		// halt on regular intervals to prevent USB buffer overflow
		if(!needs_halt) {
			pointsWithoutHalt++;
			if(pointsWithoutHalt > maxPointsBetweenHalts) {
				needs_halt = true;
			}
		} else {
			pointsWithoutHalt = 0;
		}

		uint8_t attenuator = freq >= HW::BandSwitchFrequency ? fixedAttenuatorHighband : fixedAttenuatorLowband;
		if(!s.fixedPowerSetting) {
			// adapt power level throughout the sweep
			amplitude = HW::GetAmplitudeSettings(power, freq, true, false);
			if(freq >= HW::BandSwitchFrequency) {
				Source.SetPowerOutA(amplitude.highBandPower, true);
			}
			if(amplitude.unlevel) {
				HW::SetOutputUnlevel(true);
			}
			attenuator = amplitude.attenuator;
		}

		// needs halt before first point to allow PLLs to settle
		if (i == 0) {
			needs_halt = true;
		}

		setLO2Adjustment(i, (int32_t) last_LO2 - defaultLO2);
		FPGA::WriteSweepConfig(i, lowband, Source.GetRegisters(),
				LO1.GetRegisters(), attenuator, freq, FPGA::SettlingTime::us60,
				ADC_rate, needs_halt);
		last_lowband = lowband;
	}
	// revert clk configuration to previous value (might have been changed in sweep calculation)
	Si5351.SetCLK(SiChannel::RefLO2, HW::getIF1() - HW::getIF2(), Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.ResetPLL(Si5351C::PLL::B);
	Si5351.WaitForLock(Si5351C::PLL::B, 10);
	// Enable mixers/amplifier/PLLs
	FPGA::SetWindow(FPGA::Window::Kaiser);
	FPGA::Enable(FPGA::Periphery::Port1Mixer);
	FPGA::Enable(FPGA::Periphery::Port2Mixer);
	FPGA::Enable(FPGA::Periphery::RefMixer);
	FPGA::Enable(FPGA::Periphery::Amplifier);
	FPGA::Enable(FPGA::Periphery::SourceChip);
	FPGA::Enable(FPGA::Periphery::SourceRF);
	FPGA::Enable(FPGA::Periphery::LO1Chip);
	FPGA::Enable(FPGA::Periphery::LO1RF);
	FPGA::SetupSweep(s.stages, s.port1Stage, s.port2Stage, s.syncMode != 0, s.syncMaster);
	Trigger::SetMode((Trigger::Mode) s.syncMode);
	FPGA::Enable(FPGA::Periphery::PortSwitch);
	pointCnt = 0;
	stageCnt = 0;
	adcShifted = false;
	active = true;
	// Enable new data and sweep halt interrupt
	FPGA::EnableInterrupt(FPGA::Interrupt::NewData);
	FPGA::EnableInterrupt(FPGA::Interrupt::SweepHalted);
	// Start the sweep if not configured for standby
	last_LO2 = HW::getIF1() - HW::getIF2();
	firstPoint = true;
	if (settings.standby) {
		waitingInStandby = true;
	}
	else {
		FPGA::StartSweep();
	}
	return true;
}

void VNA::InitiateSweep() {
	// Invoked by a host via InitiateSweep packet
	if(waitingInStandby){
		// make sure that SweepSettings have been configured for standby operation
		FPGA::StartSweep();
		waitingInStandby = false;
	}
}

bool VNA::GetStandbyMode() {
	return settings.standby;
}

bool VNA::IsWaitingInStandby() {
	return waitingInStandby;
}

void VNA::SetWaitingInStandby(bool waiting) {
	waitingInStandby = waiting;
}

static void PassOnData() {
	Protocol::PacketInfo info;
	info.type = Protocol::PacketType::VNADatapoint;
	info.VNAdatapoint = &data;
	Communication::Send(info);
	data.clear();
}

bool VNA::MeasurementDone(const FPGA::SamplingResult &result) {
	if(!active) {
		return false;
	}
	if(result.pointNum != pointCnt || result.stageNum != stageCnt) {
		LOG_WARN("Indicated point does not match (%u != %u, %d != %d)", result.pointNum, pointCnt, result.stageNum, stageCnt);
		FPGA::AbortSweep();
		return false;
	}
	// normal sweep mode
	data.addValue(result.P1I, result.P1Q, stageCnt, (int) Protocol::Source::Port1);
	data.addValue(result.P2I, result.P2Q, stageCnt, (int) Protocol::Source::Port2);
	data.addValue(result.RefI, result.RefQ, stageCnt, (int) Protocol::Source::Port1 | (int) Protocol::Source::Port2 | (int) Protocol::Source::Reference);
	data.pointNum = pointCnt;
	if(zerospan) {
		uint64_t timestamp = HW::getLastISRTimestamp();
		if(firstPoint) {
			data.us = 0;
			firstPointTime = timestamp;
			firstPoint = false;
		} else {
			data.us = timestamp - firstPointTime;
		}
	} else {
		// non-zero span, set frequency/power
		data.frequency = getPointFrequency(pointCnt);
		data.cdBm = settings.cdbm_excitation_start + (settings.cdbm_excitation_stop - settings.cdbm_excitation_start) * pointCnt / (settings.points - 1);
	}
	// figure out whether this sweep point is complete
	stageCnt++;
	if(stageCnt > settings.stages) {
		// point is complete
		stageCnt = 0;
		STM::DispatchToInterrupt(PassOnData);
		pointCnt++;
		if (pointCnt >= settings.points) {
			// reached end of sweep, start again
			pointCnt = 0;
			// request to trigger work function
			return true;
		}
	}
	return false;
}

void VNA::Work() {
	// end of sweep
	if(Trigger::GetMode() != Trigger::Mode::ExtRef) {
		HW::Ref::update();
	}
	// Compile info packet
	Protocol::PacketInfo packet;
	packet.type = Protocol::PacketType::DeviceStatus;
	if(HW::getStatusUpdateFlag()) {
		HW::getDeviceStatus(&packet.status, true);
		Communication::Send(packet);
	}
	// do not reset unlevel flag here, as it is calculated only once at the setup of the sweep
	// Start next sweep if not configured for standby
	if (settings.standby) {
		waitingInStandby = true;
	}
	else {
		FPGA::StartSweep();
	}

}

void VNA::SweepHalted() {
	if(!active) {
		return;
	}
	// Resuming the halted sweep requires I2C bus operations to the Si5355. When trigger synchronization is enabled
	// in the external reference mode, this might collide with the trigger input check. Instead both these actions
	// are handled through the STM::DispatchToInterrupt functionality, ensuring that they do not interrupt each other
	STM::DispatchToInterrupt([](){
		LOG_DEBUG("Halted before point %d", pointCnt);
//		bool adcShiftRequired = false;
		uint64_t frequency = getPointFrequency(pointCnt);
		frequency = Cal::FrequencyCorrectionToDevice(frequency);
		int16_t power = settings.cdbm_excitation_start
				+ (settings.cdbm_excitation_stop - settings.cdbm_excitation_start)
						* pointCnt / (settings.points - 1);
		if (frequency < HW::BandSwitchFrequency) {
			auto driveStrength = fixedPowerLowband;
			if(!settings.fixedPowerSetting) {
				auto amplitude = HW::GetAmplitudeSettings(power, frequency, true, false);
				// attenuator value has already been set in sweep setup
				driveStrength = amplitude.lowBandPower;
			}

			// need the Si5351 as Source
			bool freqSuccess = Si5351.SetCLK(SiChannel::LowbandSource, frequency, Si5351C::PLL::B, driveStrength);
			static bool lowbandDisabled = false;
			if (pointCnt == 0) {
				// First point in sweep, switch to correct source
				FPGA::Disable(FPGA::Periphery::SourceRF);
				lowbandDisabled = true;
			}
			if(lowbandDisabled && freqSuccess) {
				// frequency is valid, can enable lowband source now
				Si5351.Enable(SiChannel::LowbandSource);
				Delay::ms(10);
				lowbandDisabled = false;
			}

			// At low frequencies the 1.LO feedthrough mixes with the 2.LO in the second mixer.
			// Depending on the stimulus frequency, the resulting mixing product might alias to the 2.IF
			// in the ADC which causes a spike. Check for this and shift the ADC sampling frequency if necessary

//			uint32_t LO_mixing = (HW::getIF1() + frequency) - (HW::getIF1() - HW::getIF2());
//			if(abs(Util::Alias(LO_mixing, HW::getADCRate()) - HW::getIF2()) <= actualBandwidth * 2) {
//				// the image is in or near the IF bandwidth and would cause a peak
//				// Use a slightly different ADC sample rate if possible
//				if(HW::getIF2() == HW::DefaultIF2) {
//					adcShiftRequired = true;
//				}
//			}
		} else if(!FPGA::IsEnabled(FPGA::Periphery::SourceRF)){
			// first sweep point in highband is also halted, disable lowband source
			Si5351.Disable(SiChannel::LowbandSource);
			FPGA::Enable(FPGA::Periphery::SourceRF);
		}
		if(settings.suppressPeaks) {
			// does not actually change PLL settings, just calculates the register values and
			// is required to determine the need for a 2.LO shift
			uint32_t new2LO = defaultLO2 + getLO2Adjustment(pointCnt);
//			setPLLFrequencies(frequency, last_LO2, actualBandwidth, &new2LO);
			if(new2LO != last_LO2) {
				last_LO2 = new2LO;
				Si5351.SetCLK(SiChannel::Port1LO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
				Si5351.SetCLK(SiChannel::Port2LO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
				Si5351.SetCLK(SiChannel::RefLO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
				Si5351.ResetPLL(Si5351C::PLL::B);
				Si5351.WaitForLock(Si5351C::PLL::B, 10);
				// PLL reset causes the 2.LO to turn off briefly and then ramp on back, needs delay before next point
				Delay::us(1500);
			}
		}

		if (pointCnt == 0) {
			HAL_Delay(2);
		}

//		if(adcShiftRequired) {
//			FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, alternativePrescaler);
//			FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, alternativePhaseInc);
//			adcShifted = true;
//		} else if(adcShifted) {
//			// reset to default value
//			FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, HW::getADCPrescaler());
//			FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, HW::getDFTPhaseInc());
//			adcShifted = false;
//		}

		if(usb_available_buffer() >= reservedUSBbuffer) {
			// enough space available, can resume immediately
			FPGA::ResumeHaltedSweep();
		} else {
			// USB buffer could potentially overflow before next halted point, wait until more space is available.
			uint32_t start = HAL_GetTick();
			while(usb_available_buffer() < reservedUSBbuffer) {
				if(HAL_GetTick() - start > 100) {
					// still no buffer space after some time, something more serious must have gone wrong
					// -> abort sweep and return to idle
					usb_clear_buffer();
					FPGA::AbortSweep();
					HW::SetIdle();
					return;
				}
			}
			FPGA::ResumeHaltedSweep();
		}
	});
}

void VNA::Stop() {
	active = false;
	FPGA::AbortSweep();
}

void VNA::PrintStatus() {
	HAL_Delay(10);
	LOG_INFO("VNA status:");
	HAL_Delay(10);
	LOG_INFO("Active: %d", active);
	HAL_Delay(10);
	LOG_INFO("Points: %d/%d", pointCnt, settings.points);
	HAL_Delay(10);
	LOG_INFO("Stages: %d/%d", stageCnt, settings.stages);
	HAL_Delay(10);
	LOG_INFO("FPGA status: 0x%04x", FPGA::GetStatus());
}
