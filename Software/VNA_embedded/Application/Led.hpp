#pragma once

#include <cstdint>

namespace LED {

void Init();
void Pulsating();
void Off();
void Error(uint8_t code);

void On();
void Toggle();

}
