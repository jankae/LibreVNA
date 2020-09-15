#pragma once

#include <cstdint>
#include "Protocol.hpp"
#include "FPGA/FPGA.hpp"

namespace VNA {

using SweepCallback = void(*)(Protocol::Datapoint);
using StatusCallback = void(*)(FPGA::SamplingResult);

bool Init();
// returns whether the sweep is actually started
bool ConfigureSweep(Protocol::SweepSettings s, SweepCallback cb);
bool ConfigureManual(Protocol::ManualControl m, StatusCallback cb);
bool ConfigureGenerator(Protocol::GeneratorSettings g);
void SetIdle();

// Only call the following function when the sweep is inactive
bool GetTemps(uint8_t *source, uint8_t *lo);
void fillDeviceInfo(Protocol::DeviceInfo *info);
namespace Ref {
	bool available();
	bool applySettings(Protocol::ReferenceSettings s);
}

}

