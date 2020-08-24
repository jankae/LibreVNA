#pragma once

#include <cstdint>
#include "Protocol.hpp"
#include "FPGA.hpp"

namespace VNA {

using SweepCallback = void(*)(Protocol::Datapoint);
using StatusCallback = void(*)(FPGA::SamplingResult);

bool Init();
bool ConfigureSweep(Protocol::SweepSettings s, SweepCallback cb);
bool ConfigureManual(Protocol::ManualControl m, StatusCallback cb);
bool GetTemps(uint8_t *source, uint8_t *lo);

}

