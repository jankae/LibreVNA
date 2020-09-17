#pragma once

#include "Protocol.hpp"
#include "FPGA/FPGA.hpp"

namespace SA {

enum class Detector {
	PosPeak = 0x00,
	NegPeak = 0x01,
	Sample = 0x02,
	Normal = 0x03,
	Average = 0x04,
};

void Setup(Protocol::SpectrumAnalyzerSettings settings);
bool MeasurementDone(FPGA::SamplingResult result);
void Work();
void Stop();

}
