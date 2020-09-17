#pragma once

#include <cstdint>
#include "Protocol.hpp"
#include "FPGA/FPGA.hpp"

namespace VNA {

using SweepCallback = void(*)(Protocol::Datapoint);

bool Setup(Protocol::SweepSettings s, SweepCallback cb);
bool MeasurementDone(FPGA::SamplingResult result);
void Work();
void SweepHalted();
void Stop();

}

