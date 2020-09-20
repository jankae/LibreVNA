#pragma once

#include <cstdint>
#include "Protocol.hpp"

namespace HW {

static constexpr uint32_t ADCSamplerate = 914000;
static constexpr uint32_t IF1 = 60000000;
static constexpr uint32_t IF2 = 250000;
static constexpr uint32_t LO1_minFreq = 25000000;
static constexpr uint32_t MaxSamples = 130944;
static constexpr uint32_t PLLRef = 100000000;

enum class Mode {
	Idle,
	Manual,
	VNA,
	SA,
};

using WorkRequest = void (*)(void);

bool Init(WorkRequest wr);
void SetMode(Mode mode);
void SetIdle();
void Work();

bool GetTemps(uint8_t *source, uint8_t *lo);
void fillDeviceInfo(Protocol::DeviceInfo *info);
namespace Ref {
	bool available();
	// reference won't change until update is called
	void set(Protocol::ReferenceSettings s);
	void update();
}

}
