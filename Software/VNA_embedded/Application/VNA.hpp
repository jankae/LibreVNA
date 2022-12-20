#pragma once

#include <cstdint>
#include "Protocol.hpp"
#include "FPGA/FPGA.hpp"

namespace VNA {

bool Setup(Protocol::SweepSettings s);
void InitiateSweep();
bool GetStandbyMode();
bool IsWaitingInStandby();
bool MeasurementDone(const FPGA::SamplingResult &result);
void Work();
void SweepHalted();
void Stop();

void PrintStatus();

}

