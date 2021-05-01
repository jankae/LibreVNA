#include "SpectrumAnalyzer.hpp"
#include "Hardware.hpp"
#include "HW_HAL.hpp"
#include <complex.h>
#include <limits>
#include "Communication.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Util.hpp"
#include <array>

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#define LOG_MODULE	"SA"
#include "Log.h"

using namespace HWHAL;

static Protocol::SpectrumAnalyzerSettings s;
static uint32_t pointCnt;
static uint32_t points;
static uint32_t binSize;
static uint8_t signalIDstep;
static uint32_t sampleNum;
static Protocol::PacketInfo p;
static bool active = false;
static uint32_t lastLO2;
static uint32_t actualRBW;
static uint16_t DFTpoints;
static bool negativeDFT; // if true, a positive frequency shift at input results in a negative shift at the 2.IF. Handle DFT accordingly

static float port1Measurement[FPGA::DFTbins], port2Measurement[FPGA::DFTbins];

static uint8_t signalIDsteps;
static std::array<uint8_t, 4> signalIDprescalers;
static uint8_t attenuator;
static int64_t trackingFreq;
static bool trackingLowband;

static void StartNextSample() {
	uint64_t freq = s.f_start + (s.f_stop - s.f_start) * pointCnt / (points - 1);
	freq = Cal::FrequencyCorrectionToDevice(freq);
	uint64_t LO1freq;
	uint32_t LO2freq;
	switch(signalIDstep) {
	case 0:
		// reset minimum amplitudes in first signal ID step
		for (uint16_t i = 0; i < DFTpoints; i++) {
			port1Measurement[i] = std::numeric_limits<float>::max();
			port2Measurement[i] = std::numeric_limits<float>::max();
		}
		// Use default LO frequencies
		LO1freq = freq + HW::IF1;
		LO2freq = HW::IF1 - HW::IF2;
		FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, 112);
		FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, 1120);
		negativeDFT = true;

		// set tracking generator to default values
		trackingFreq = 0;
		trackingLowband = false;
		attenuator = 0;
		// this is the first step in each point, update tracking generator if enabled
		if (s.trackingGenerator) {
			trackingFreq = freq + s.trackingGeneratorOffset;
			if(trackingFreq > 0 && trackingFreq <= (int64_t) HW::Info.limits_maxFreq) {
				// tracking frequency is valid, calculate required settings and select band
				auto amplitude = HW::GetAmplitudeSettings(s.trackingPower, trackingFreq, s.applySourceCorrection, s.trackingGeneratorPort);
				// only set the flag here, it is reset at the beginning of each sweep (this makes sure it is set if any of the points are not reached by the TG)
				if(amplitude.unlevel) {
					HW::SetOutputUnlevel(true);
				}
				attenuator = amplitude.attenuator;
				if(trackingFreq < HW::BandSwitchFrequency) {
					Si5351.SetCLK(SiChannel::LowbandSource, trackingFreq, Si5351C::PLL::B, amplitude.lowBandPower);
					FPGA::Disable(FPGA::Periphery::SourceChip);
					FPGA::Disable(FPGA::Periphery::SourceRF);
					trackingLowband = true;
				} else {
					Source.SetFrequency(trackingFreq);
					Source.SetPowerOutA(amplitude.highBandPower, true);
					FPGA::Enable(FPGA::Periphery::SourceChip);
					FPGA::Enable(FPGA::Periphery::SourceRF);
					trackingLowband = false;
					// selected power potentially changed, update default registers
					FPGA::WriteMAX2871Default(Source.GetRegisters());
				}
			}
		}
		break;
	case 1:
		LO2freq = HW::IF1 - HW::IF2;
		negativeDFT = false;
		// Shift first LO to other side
		// depending on the measurement frequency this is not possible or additive mixing has to be used
		if(freq >= HW::IF1 + HW::LO1_minFreq) {
			// frequency is high enough to shift 1.LO below measurement frequency
			LO1freq = freq - HW::IF1;
			break;
		} else if(freq <= HW::IF1 - HW::LO1_minFreq) {
			// frequency is low enough to add 1.LO to measurement frequency
			LO1freq = HW::IF1 - freq;
			break;
		}
		// unable to reach required frequency with 1.LO, skip this signal ID step
		signalIDstep++;
		/* no break */
	case 2:
		// Shift second LOs to other side
		LO1freq = freq + HW::IF1;
		LO2freq = HW::IF1 + HW::IF2;
		negativeDFT = false;
		break;
	case 3:
		// Shift both LO to other side
		LO2freq = HW::IF1 + HW::IF2;
		negativeDFT = true;
		// depending on the measurement frequency this is not possible or additive mixing has to be used
		if(freq >= HW::IF1 + HW::LO1_minFreq) {
			// frequency is high enough to shift 1.LO below measurement frequency
			LO1freq = freq - HW::IF1;
			break;
		} else if(freq <= HW::IF1 - HW::LO1_minFreq) {
			// frequency is low enough to add 1.LO to measurement frequency
			LO1freq = HW::IF1 - freq;
			break;
		}
		// unable to reach required frequency with 1.LO, skip this signal ID step
		signalIDstep++;
		/* no break */
	default:
		// Use default frequencies with different ADC samplerate to remove images in final IF
		negativeDFT = true;
		LO1freq = freq + HW::IF1;
		LO2freq = HW::IF1 - HW::IF2;
		FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, signalIDprescalers[signalIDstep-4]);
		FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, (uint16_t) signalIDprescalers[signalIDstep-4] * 10);
	}
	LO1.SetFrequency(LO1freq);
	// LO1 is not able to reach all frequencies with the required precision, adjust LO2 to account for deviation
	int32_t LO1deviation = (int64_t) LO1.GetActualFrequency() - LO1freq;
	LO2freq += LO1deviation;
	// only adjust LO2 PLL if necessary (if the deviation is significantly less than the RBW it does not matter)
	if((uint32_t) abs(LO2freq - lastLO2) > actualRBW / 2) {
		Si5351.SetCLK(SiChannel::Port1LO2, LO2freq, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
		Si5351.SetCLK(SiChannel::Port2LO2, LO2freq, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
		lastLO2 = LO2freq;
	}
	if (s.UseDFT) {
		uint32_t spacing = (s.f_stop - s.f_start) / (points - 1);
		uint32_t start = HW::IF2;
		if(negativeDFT) {
			// needs to look below the start frequency, shift start
			start -= spacing * (DFTpoints - 1);
		}
		FPGA::SetupDFT(start, spacing);
		FPGA::StartDFT();
	}

	// Configure the sampling in the FPGA
	FPGA::WriteSweepConfig(0, trackingLowband, Source.GetRegisters(), LO1.GetRegisters(), attenuator,
			trackingFreq, FPGA::SettlingTime::us20, FPGA::Samples::SPPRegister, 0,
			FPGA::LowpassFilter::Auto);

	FPGA::StartSweep();
}

void SA::Setup(Protocol::SpectrumAnalyzerSettings settings) {
	LOG_DEBUG("Setting up...");
	HW::SetOutputUnlevel(false);
	SA::Stop();
	vTaskDelay(5);
	s = settings;
	HW::SetMode(HW::Mode::SA);
	FPGA::SetMode(FPGA::Mode::FPGA);
	// in almost all cases a full sweep requires more points than the FPGA can handle at a time
	// individually start each point and do the sweep in the uC
	FPGA::SetNumberOfPoints(1);
	// calculate required samples per measurement for requested RBW
	// see https://www.tek.com/blog/window-functions-spectrum-analyzers for window factors
	constexpr float window_factors[4] = {0.89f, 2.23f, 1.44f, 3.77f};
	sampleNum = HW::ADCSamplerate * window_factors[s.WindowType] / s.RBW;
	// round up to next multiple of 16
	if(sampleNum%16) {
		sampleNum += 16 - sampleNum%16;
	}
	if(sampleNum >= HW::MaxSamples) {
		sampleNum = HW::MaxSamples;
	}
	actualRBW = HW::ADCSamplerate * window_factors[s.WindowType] / sampleNum;
	FPGA::SetSamplesPerPoint(sampleNum);
	// calculate amount of required points
	points = 2 * (s.f_stop - s.f_start) / actualRBW;
	// adjust to integer multiple of requested result points (in order to have the same amount of measurements in each bin)
	points += s.pointNum - points % s.pointNum;
	binSize = points / s.pointNum;
	LOG_DEBUG("%u displayed points, resulting in %lu points and bins of size %u", s.pointNum, points, binSize);
	// set initial state
	pointCnt = 0;
	// enable the required hardware resources
	Si5351.Enable(SiChannel::Port1LO2);
	Si5351.Enable(SiChannel::Port2LO2);
	if (s.trackingGenerator) {
		Si5351.Enable(SiChannel::LowbandSource);
	} else {
		Si5351.Disable(SiChannel::LowbandSource);
	}
	FPGA::SetWindow((FPGA::Window) s.WindowType);
	FPGA::Enable(FPGA::Periphery::LO1Chip);
	FPGA::Enable(FPGA::Periphery::LO1RF);
	FPGA::Enable(FPGA::Periphery::ExcitePort1, s.trackingGeneratorPort == 0);
	FPGA::Enable(FPGA::Periphery::ExcitePort2, s.trackingGeneratorPort == 1);
	FPGA::Enable(FPGA::Periphery::PortSwitch, s.trackingGenerator);
	FPGA::Enable(FPGA::Periphery::Amplifier, s.trackingGenerator);
	FPGA::Enable(FPGA::Periphery::Port1Mixer);
	FPGA::Enable(FPGA::Periphery::Port2Mixer);

	if(s.SignalID) {
		// use different ADC prescalers depending on RBW: For small RBWs, images with the shifted ADC samplerate can be closer to the IF
		// because they get suppressed by the RBW filter. For larger RBWs multiple different ADC samplerates are required to move all
		// aliased images far enough away from the IF. This only works up to about 40kHz RBW. Above that even with signal ID some images
		// will be present in the processed data
		if(actualRBW <= 10000) {
			signalIDsteps = 6;
			signalIDprescalers = {132, 156};
		} else {
			signalIDsteps = 8;
			signalIDprescalers = {126, 130, 144, 176};
		}
	}

	if (s.UseDFT) {
		uint32_t spacing = (s.f_stop - s.f_start) / (points - 1);
		// The DFT can only look at a small bandwidth otherwise the passband of the final ADC filter is visible in the data
		// Limit to about 30kHz
		uint32_t maxDFTpoints = 30000 / spacing;
		// Limit to actual supported number of bins
		if(maxDFTpoints > FPGA::DFTbins) {
			maxDFTpoints = FPGA::DFTbins;
		}
		DFTpoints = maxDFTpoints;
		FPGA::DisableInterrupt(FPGA::Interrupt::NewData);
	} else {
		DFTpoints = 1; // can only measure one point at a time
		FPGA::StopDFT();
		FPGA::EnableInterrupt(FPGA::Interrupt::NewData);
	}

	lastLO2 = 0;
	active = true;
	StartNextSample();
}

bool SA::MeasurementDone(const FPGA::SamplingResult &result) {
	if(!active) {
		return false;
	}
	FPGA::AbortSweep();

	for(uint16_t i=0;i<DFTpoints;i++) {
		float port1, port2;
		if (s.UseDFT) {
			// use DFT result
			auto dft = FPGA::ReadDFTResult();
			port1 = dft.P1;
			port2 = dft.P2;
		} else {
			port1 = fabs(std::complex<float>(result.P1I, result.P1Q));
			port2 = fabs(std::complex<float>(result.P2I, result.P2Q));
		}
		port1 /= sampleNum;
		port2 /= sampleNum;

		uint16_t index = i;
		if (negativeDFT) {
			// bin order is reversed
			index = DFTpoints - i - 1;
		}

		if(port1 < port1Measurement[index]) {
			port1Measurement[index] = port1;
		}
		if(port2 < port2Measurement[index]) {
			port2Measurement[index] = port2;
		}
	}

	if (s.UseDFT) {
		FPGA::StopDFT();
		// will be started again in StartNextSample
	}

	// trigger work function
	return true;
}

void SA::Work() {
	if(!active) {
		return;
	}
	if(!s.SignalID || signalIDstep >= signalIDsteps - 1) {
		// this measurement point is done, handle result according to detector
		for(uint16_t i=0;i<DFTpoints;i++) {
			uint16_t binIndex = (pointCnt + i) / binSize;
			uint32_t pointInBin = (pointCnt + i) % binSize;
			bool lastPointInBin = pointInBin >= binSize - 1;
			auto det = (Detector) s.Detector;
			if(det == Detector::Normal) {
				det = binIndex & 0x01 ? Detector::PosPeak : Detector::NegPeak;
			}
			switch(det) {
			case Detector::PosPeak:
				if(pointInBin == 0) {
					p.spectrumResult.port1 = std::numeric_limits<float>::min();
					p.spectrumResult.port2 = std::numeric_limits<float>::min();
				}
				if(port1Measurement[i] > p.spectrumResult.port1) {
					p.spectrumResult.port1 = port1Measurement[i];
				}
				if(port2Measurement[i] > p.spectrumResult.port2) {
					p.spectrumResult.port2 = port2Measurement[i];
				}
				break;
			case Detector::NegPeak:
				if(pointInBin == 0) {
					p.spectrumResult.port1 = std::numeric_limits<float>::max();
					p.spectrumResult.port2 = std::numeric_limits<float>::max();
				}
				if(port1Measurement[i] < p.spectrumResult.port1) {
					p.spectrumResult.port1 = port1Measurement[i];
				}
				if(port2Measurement[i] < p.spectrumResult.port2) {
					p.spectrumResult.port2 = port2Measurement[i];
				}
				break;
			case Detector::Sample:
				if(pointInBin <= binSize / 2) {
					// still in first half of bin, simply overwrite
					p.spectrumResult.port1 = port1Measurement[i];
					p.spectrumResult.port2 = port2Measurement[i];
				}
				break;
			case Detector::Average:
				if(pointInBin == 0) {
					p.spectrumResult.port1 = 0;
					p.spectrumResult.port2 = 0;
				}
				p.spectrumResult.port1 += port1Measurement[i];
				p.spectrumResult.port2 += port2Measurement[i];
				if(lastPointInBin) {
					// calculate average
					p.spectrumResult.port1 /= binSize;
					p.spectrumResult.port2 /= binSize;
				}
				break;
			case Detector::Normal:
				// nothing to do, normal detector handled by PosPeak or NegPeak in each sample
				break;
			}
			if(lastPointInBin) {
				// Send result to application
				p.type = Protocol::PacketType::SpectrumAnalyzerResult;
				// measurements are already up to date, fill remaining fields
				p.spectrumResult.frequency = s.f_start + (s.f_stop - s.f_start) * binIndex / (s.pointNum - 1);
				// scale approximately (constant determined empirically)
				p.spectrumResult.port1 /= 253000000.0;
				p.spectrumResult.port2 /= 253000000.0;
				if (s.applyReceiverCorrection) {
					auto correction = Cal::ReceiverCorrection(p.spectrumResult.frequency);
					p.spectrumResult.port1 *= powf(10.0f, (float) correction.port1 / 100.0f / 20.0f);
					p.spectrumResult.port2 *= powf(10.0f, (float) correction.port2 / 100.0f / 20.0f);
				}
				p.spectrumResult.pointNum = binIndex;
				Communication::Send(p);
			}
		}
		// setup for next step
		signalIDstep = 0;

		if(pointCnt % 10 == 0) {
			// send device info every nth point
			FPGA::Enable(FPGA::Periphery::SourceChip); // needs to enable the chip to get a valid temperature reading
			Protocol::PacketInfo packet;
			packet.type = Protocol::PacketType::DeviceInfo;
			HW::fillDeviceInfo(&packet.info, true);
			FPGA::Disable(FPGA::Periphery::SourceChip);
			Communication::Send(packet);
		}

		if(pointCnt < points - DFTpoints) {
			pointCnt += DFTpoints;
		} else {
			pointCnt = 0;
			// reset possibly active unlevel flag before next sweep
			HW::SetOutputUnlevel(false);
		}
	} else {
		// more measurements required for signal ID
		signalIDstep++;
	}
	HW::Ref::update();
	StartNextSample();
}

void SA::Stop() {
	active = false;
	FPGA::AbortSweep();
}
