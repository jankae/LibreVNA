#pragma once

#include "FPGA/FPGA.hpp"
#include "Protocol.hpp"

namespace Manual {

void Setup(Protocol::ManualControl m);
bool MeasurementDone(FPGA::SamplingResult result);
void Work();
void Stop();

}
