#pragma once

#include <cstdint>
#include "Protocol.hpp"

namespace HW {

enum class Mode {
	Idle,
	Manual,
	VNA,
	SA,
};

bool Init();
void SetMode(Mode mode);
void SetIdle();

bool GetTemps(uint8_t *source, uint8_t *lo);
void fillDeviceInfo(Protocol::DeviceInfo *info);
namespace Ref {
	bool available();
	// reference won't change until update is called
	void set(Protocol::ReferenceSettings s);
	void update();
}

}
