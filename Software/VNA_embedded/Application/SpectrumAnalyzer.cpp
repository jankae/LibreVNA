#include "SpectrumAnalyzer.hpp"
#include "Hardware.hpp"
#include "HW_HAL.hpp"
#include <complex.h>
#include <limits>
#include "Communication.h"
#include "FreeRTOS.h"
#include "task.h"

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#define LOG_MODULE	"SA"
#include "Log.h"

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

static float port1Measurement, port2Measurement;

using namespace HWHAL;

static void StartNextSample() {
	uint64_t freq = s.f_start + (s.f_stop - s.f_start) * pointCnt / (points - 1);
	uint64_t LO1freq;
	uint32_t LO2freq;
	switch(signalIDstep) {
	case 0:
	default:
		// reset minimum amplitudes in first signal ID step
		port1Measurement = std::numeric_limits<float>::max();
		port2Measurement = std::numeric_limits<float>::max();
		// Use default LO frequencies
		LO1freq = freq + HW::IF1;
		LO2freq = HW::IF1 - HW::IF2;
		FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, 112);
		FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, 1120);
		break;
	case 1:
		LO2freq = HW::IF1 - HW::IF2;
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
		// Shift both LOs to other side
		LO1freq = freq + HW::IF1;
		LO2freq = HW::IF1 + HW::IF2;
		break;
	case 3:
		LO2freq = HW::IF1 + HW::IF2;
		// Shift second LO to other side
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
	case 4:
		// Use default frequencies with different ADC samplerate to remove images in final IF
		LO1freq = freq + HW::IF1;
		LO2freq = HW::IF1 - HW::IF2;
		FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, 120);
		FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, 1200);
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
	// Configure the sampling in the FPGA
	FPGA::WriteSweepConfig(0, 0, Source.GetRegisters(), LO1.GetRegisters(), 0,
			0, FPGA::SettlingTime::us20, FPGA::Samples::SPPRegister, 0,
			FPGA::LowpassFilter::M947);

	FPGA::StartSweep();
}

void SA::Setup(Protocol::SpectrumAnalyzerSettings settings) {
	LOG_DEBUG("Setting up...");
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
	FPGA::SetWindow((FPGA::Window) s.WindowType);
	FPGA::Enable(FPGA::Periphery::LO1Chip);
	FPGA::Enable(FPGA::Periphery::LO1RF);
	FPGA::Enable(FPGA::Periphery::ExcitePort1);
	FPGA::Enable(FPGA::Periphery::Port1Mixer);
	FPGA::Enable(FPGA::Periphery::Port2Mixer);
	lastLO2 = 0;
	active = true;
	StartNextSample();
}

bool SA::MeasurementDone(FPGA::SamplingResult result) {
	if(!active) {
		return false;
	}
	FPGA::AbortSweep();
	float port1 = abs(std::complex<float>(result.P1I, result.P1Q))/sampleNum;
	float port2 = abs(std::complex<float>(result.P2I, result.P2Q))/sampleNum;
	if(port1 < port1Measurement) {
		port1Measurement = port1;
	}
	if(port2 < port2Measurement) {
		port2Measurement = port2;
	}
	// trigger work function
	return true;
}

void SA::Work() {
	if(!active) {
		return;
	}
	if(!s.SignalID || signalIDstep >= 4) {
		// this measurement point is done, handle result according to detector
		uint16_t binIndex = pointCnt / binSize;
		uint32_t pointInBin = pointCnt % binSize;
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
			if(port1Measurement > p.spectrumResult.port1) {
				p.spectrumResult.port1 = port1Measurement;
			}
			if(port2Measurement > p.spectrumResult.port2) {
				p.spectrumResult.port2 = port2Measurement;
			}
			break;
		case Detector::NegPeak:
			if(pointInBin == 0) {
				p.spectrumResult.port1 = std::numeric_limits<float>::max();
				p.spectrumResult.port2 = std::numeric_limits<float>::max();
			}
			if(port1Measurement < p.spectrumResult.port1) {
				p.spectrumResult.port1 = port1Measurement;
			}
			if(port2Measurement < p.spectrumResult.port2) {
				p.spectrumResult.port2 = port2Measurement;
			}
			break;
		case Detector::Sample:
			if(pointInBin <= binSize / 2) {
				// still in first half of bin, simply overwrite
				p.spectrumResult.port1 = port1Measurement;
				p.spectrumResult.port2 = port2Measurement;
			}
			break;
		case Detector::Average:
			if(pointInBin == 0) {
				p.spectrumResult.port1 = 0;
				p.spectrumResult.port2 = 0;
			}
			p.spectrumResult.port1 += port1Measurement;
			p.spectrumResult.port2 += port2Measurement;
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
			p.spectrumResult.pointNum = binIndex;
			p.spectrumResult.frequency = s.f_start + (s.f_stop - s.f_start) * binIndex / (s.pointNum - 1);
			Communication::Send(p);
		}
		// setup for next step
		signalIDstep = 0;
		if(pointCnt < points - 1) {
			pointCnt++;
		} else {
			pointCnt = 0;
		}
	} else {
		// more measurements required for signal ID
		signalIDstep++;
	}
	StartNextSample();
}

void SA::Stop() {
	active = false;
	FPGA::AbortSweep();
}
