#pragma once

#include <cstdint>
#include "Protocol.hpp"
#include "FPGA/FPGA.hpp"

namespace VNA {

bool Setup(Protocol::SweepSettings s);
bool MeasurementDone(const FPGA::SamplingResult &result);
void Work();
void SweepHalted();
void Stop();

}

