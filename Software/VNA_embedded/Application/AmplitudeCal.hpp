#pragma once

#include <stdint.h>
#include "Flash.hpp"
#include "Firmware.hpp"
#include "Protocol.hpp"

namespace AmplitudeCal {

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
void SendSource();
void SendReceiver();
void AddSourcePoint(const Protocol::AmplitudeCorrectionPoint& p);
void AddReceiverPoint(const Protocol::AmplitudeCorrectionPoint& p);

}
