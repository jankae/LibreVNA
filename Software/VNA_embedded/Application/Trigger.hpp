#pragma once

#include <cstdint>

namespace Trigger {

enum class Mode : uint8_t {
	Off = 0,
	USB_GUI = 1,
	ExtRef = 2,
	Trigger = 3,
};

using CallbackISR = void(*)(void);

void Init(CallbackISR cb);

void SetMode(Mode m);
Mode GetMode();

void SetInput(bool high);
bool GetOutput();
bool GetInput();

}
