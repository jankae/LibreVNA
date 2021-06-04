#pragma once

#include <cstdint>

#include "stm.hpp"

namespace Exti {

enum class EdgeType : uint8_t {
	Falling,
	Rising,
	Both,
};

enum class Pull : uint8_t {
	None,
	Up,
	Down,
};

using Callback = void(*)(void*);

void Init();
bool SetCallback(GPIO_TypeDef *gpio, uint16_t pin, EdgeType edge, Pull pull, Callback cb, void *ptr = nullptr);
bool ClearCallback(GPIO_TypeDef *gpio, uint16_t pin);

}
