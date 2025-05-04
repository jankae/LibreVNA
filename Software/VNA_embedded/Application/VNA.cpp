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
static constexpr uint16_t full2portDatapointSize = 66;// See Protocol::VNADatapoint::
static constexpr uint32_t reservedUSBbuffer = (maxPointsBetweenHalts + 2 /*additional buffer*/) * (full2portDatapointSize + 8 /*USB packet overhead*/);

static uint32_t PLLRefFreqs[] = {HW::PLLRef, HW::PLLRef - 1000000};
static constexpr uint8_t PLLRefFreqsNum = sizeof(PLLRefFreqs)/sizeof(PLLRefFreqs[0]);
static uint8_t sourceRefIndex, LO1RefIndex;

using namespace HWHAL;

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

static bool setPLLFrequencies(uint64_t f) {
	uint64_t sourceFreq = 0;
	uint64_t LOFreq = 0;
	if(f > HW::Info.limits_maxFreq) {
		sourceFreq = f / sourceHarmonic;
		LOFreq = (f + HW::getIF1()) / LOHarmonic;
	} else {
		if(f >= HW::BandSwitchFrequency) {
			sourceFreq = f;
		}
		LOFreq = f + HW::getIF1();
	}
	if(sourceFreq > HW::BandSwitchFrequency) {
		Source.SetFrequency(sourceFreq);
	}
	LO1.SetFrequency(LOFreq);
	bool needsRefSwitch = false;
	if(settings.suppressPeaks) {
		// Integer spurs can cause a small peak.
		if(sourceFreq > HW::BandSwitchFrequency) {
			uint32_t sourceDist = Source.DistanceToIntegerSpur();
			if((sourceDist > 0) && (sourceDist < 3 * HW::getIF2())) {
				LOG_DEBUG("Source spur at %lu: %lu", (uint32_t) f, sourceDist);
				sourceRefIndex = !sourceRefIndex;
				Source.SetReference(PLLRefFreqs[sourceRefIndex], false, 1, false);
				Source.SetFrequency(sourceFreq);
				needsRefSwitch = true;
			}
		}
		uint32_t LODist = LO1.DistanceToIntegerSpur();
		if((LODist > 0) && (LODist  < 3 * HW::getIF2())) {
			LOG_DEBUG("LO spur at %lu", (uint32_t) f);
			LO1RefIndex = !LO1RefIndex;
			LO1.SetReference(PLLRefFreqs[LO1RefIndex], false, 1, false);
			LO1.SetFrequency(LOFreq);
			needsRefSwitch = true;
		}
	}
	return needsRefSwitch;
}

static bool needs2LOshift(uint64_t f, uint32_t current2LO, uint32_t IFBW, uint32_t *new2LO) {
	// Check if 2.LO needs to be shifted
	uint64_t actualSource, actual1LO;
	actualSource = Source.GetActualFrequency();
	actual1LO = LO1.GetActualFrequency();
	if(f > HW::Info.limits_maxFreq) {
		actualSource *= sourceHarmonic;
		actual1LO *= LOHarmonic;
	} else if(f < HW::BandSwitchFrequency) {
		// can use the lowband PLL with high frequency resolution, assume perfect frequency match
		actualSource = f;
	}
	uint32_t actualFirstIF = actual1LO - actualSource;
	uint32_t actualFinalIF = actualFirstIF - current2LO;
	uint32_t IFdeviation = abs(actualFinalIF - HW::getIF2());
	if(IFdeviation > IFBW / 2) {
		*new2LO = actualFirstIF - HW::getIF2();
		return true;
	} else {
		// no shift required
		return false;
	}
}

bool VNA::Setup(Protocol::SweepSettings s) {
	// Abort possible active sweep first
	VNA::Stop();
	vTaskDelay(5);
	data.clear();
	HW::SetMode(HW::Mode::VNA);
	Trigger::SetInput(false);

	sourceRefIndex = 0;
	LO1RefIndex = 0;
	Source.SetReference(PLLRefFreqs[sourceRefIndex], false, 1, false);
	LO1.SetReference(PLLRefFreqs[LO1RefIndex], false, 1, false);

	FPGA::SetMode(FPGA::Mode::FPGA);
	FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, HW::getADCPrescaler());
	FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, HW::getDFTPhaseInc());
	if(settings.points > FPGA::MaxPoints) {
		settings.points = FPGA::MaxPoints;
	}
	if(settings.dwell_time > HW::Info.limits_maxDwellTime) {
		settings.dwell_time = HW::Info.limits_maxDwellTime;
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

	FPGA::SetSettlingTime(s.dwell_time + HW::getPLLSettlingDelay());

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
	Si5351.Enable(SiChannel::Port1LO2);
	Si5351.Enable(SiChannel::Port2LO2);
	Si5351.Enable(SiChannel::RefLO2);
	Si5351.SetPLL(Si5351C::PLL::B, last_LO2*HW::LO2Multiplier, HW::Ref::getSource());
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
		if(setPLLFrequencies(freq)) {
			needs_halt = true;
		}
		uint32_t new_LO2;
		auto needs_shift = needs2LOshift(freq, last_LO2, actualBandwidth, &new_LO2);
		if(needs_shift) {
			if(s.suppressPeaks) {
				needs_halt = true;
				last_LO2 = new_LO2;
			} else {
				LOG_WARN("Point at f=%lu%06lu needs an LO shift but the feature is disabled. This will cause a peak.", (uint32_t) (freq/1000000), (uint32_t) (freq%1000000));
			}
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
				pointsWithoutHalt = 0;
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

		FPGA::WriteSweepConfig(i, lowband, Source.GetRegisters(),
				LO1.GetRegisters(), attenuator, freq,
				FPGA::Samples::SPPRegister, needs_halt);
		last_lowband = lowband;
	}
	// reset a possibly changed PLL reference index
	sourceRefIndex = 0;
	LO1RefIndex = 0;
	Source.SetReference(PLLRefFreqs[sourceRefIndex], false, 1, false);
	LO1.SetReference(PLLRefFreqs[LO1RefIndex], false, 1, false);
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
	if(!Communication::Send(info)) {
		LOG_ERR("Failed to transmit VNADatapoint");
	}
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
	// Stop the sweep. The source may not have been active for the last phase but the PLL
	// must be enabled for reading the temperature. Stopping the sweep implicitly enables
	// the source PLL
	FPGA::StopSweep();
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
		bool adcShiftRequired = false;
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

			uint32_t LO_mixing = (HW::getIF1() + frequency) - (HW::getIF1() - HW::getIF2());
			if(abs(Util::Alias(LO_mixing, HW::getADCRate()) - HW::getIF2()) <= actualBandwidth * 2) {
				// the image is in or near the IF bandwidth and would cause a peak
				// Use a slightly different ADC sample rate if possible
				if(HW::getIF2() == HW::DefaultIF2) {
					adcShiftRequired = true;
				}
			}
		} else if(!FPGA::IsEnabled(FPGA::Periphery::SourceRF)){
			// first sweep point in highband is also halted, disable lowband source
			Si5351.Disable(SiChannel::LowbandSource);
			FPGA::Enable(FPGA::Periphery::SourceRF);
		}

		if (pointCnt == 0 && settings.suppressPeaks) {
			sourceRefIndex = 0;
			LO1RefIndex = 0;
			Source.SetReference(PLLRefFreqs[sourceRefIndex], false, 1, false);
			LO1.SetReference(PLLRefFreqs[LO1RefIndex], false, 1, false);
			// update PLL reference frequencies
			Si5351.SetCLK(SiChannel::Source, PLLRefFreqs[sourceRefIndex], Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
			Si5351.SetCLK(SiChannel::LO1, PLLRefFreqs[LO1RefIndex], Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
			last_LO2 = HW::getIF1() - HW::getIF2();
			Si5351.SetPLL(Si5351C::PLL::B, last_LO2*HW::LO2Multiplier, HW::Ref::getSource());
			Si5351.ResetPLL(Si5351C::PLL::B);
			Si5351.WaitForLock(Si5351C::PLL::B, 10);
			HAL_Delay(2);
		}

		if(settings.suppressPeaks) {
			// does not actually change PLL settings, just calculates the register values and
			// is required to determine the need for a 2.LO shift
			if(setPLLFrequencies(frequency)) {
				// update PLL reference frequencies
				Si5351.SetCLK(SiChannel::Source, PLLRefFreqs[sourceRefIndex], Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
				Si5351.SetCLK(SiChannel::LO1, PLLRefFreqs[LO1RefIndex], Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
			}
			if(needs2LOshift(frequency, last_LO2, actualBandwidth, &last_LO2)) {
				Si5351.SetPLL(Si5351C::PLL::B, last_LO2*HW::LO2Multiplier, HW::Ref::getSource());
				Si5351.ResetPLL(Si5351C::PLL::B);
				Si5351.WaitForLock(Si5351C::PLL::B, 10);
				// PLL reset causes the 2.LO to turn off briefly and then ramp on back, needs delay before next point
				Delay::us(1500);
			}
		}

		if(adcShiftRequired) {
			FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, alternativePrescaler);
			FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, alternativePhaseInc);
			adcShifted = true;
		} else if(adcShifted) {
			// reset to default value
			FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, HW::getADCPrescaler());
			FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, HW::getDFTPhaseInc());
			adcShifted = false;
		}

		if(usb_available_buffer() >= reservedUSBbuffer) {
			// enough space available, can resume immediately
			FPGA::ResumeHaltedSweep();
		} else {
			// USB buffer could potentially overflow before next halted point, wait until more space is available.
			uint32_t start = HAL_GetTick();
			while(usb_available_buffer() < reservedUSBbuffer) {
				if(HAL_GetTick() - start > 1000) {
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
