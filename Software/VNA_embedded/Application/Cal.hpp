#pragma once

#include <stdint.h>
#include "Flash.hpp"
#include "Firmware.hpp"
#include "Protocol.hpp"

namespace Cal {

constexpr uint8_t maxPoints = 64;
constexpr uint32_t flash_address = Firmware::maxSize; // stored directly behind firmware in flash
constexpr uint32_t flash_size = 8192; // reserve two sectors for now

bool Load();
bool Save();
void SetDefault();

using Correction = struct _correction {
	int16_t port1;
	int16_t port2;
};

Correction SourceCorrection(uint64_t freq);
Correction ReceiverCorrection(uint64_t freq);
// converts a frequency as received from the GUI to a calibrated value that should be used for the PLLs etc.
uint64_t FrequencyCorrectionToDevice(uint64_t freq);
// corrects a measured frequency to its calibrated value for transfer to the GUI
uint64_t FrequencyCorrectionFromDevice(uint64_t freq);
void SendSource();
void SendReceiver();
void AddSourcePoint(const Protocol::AmplitudeCorrectionPoint& p);
void AddReceiverPoint(const Protocol::AmplitudeCorrectionPoint& p);

float getFrequencyCal();
bool setFrequencyCal(float ppm);

}
