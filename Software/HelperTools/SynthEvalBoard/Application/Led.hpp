#pragma once

#include <cstdint>

namespace LED {

void Init();
void Pulsating(uint8_t led);
void Off(uint8_t led);
void On(uint8_t led);
void Error(uint8_t led, uint8_t code);

}
