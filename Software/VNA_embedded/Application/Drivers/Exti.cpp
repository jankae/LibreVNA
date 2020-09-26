#include "Exti.hpp"

using Entry  = struct {
	GPIO_TypeDef *gpio;
	Exti::Callback cb;
	void *ptr;
};

static constexpr uint8_t MaxEntries = 16;

static Entry entries[MaxEntries];

void Exti::Init() {
	HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
	HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 0);
	HAL_NVIC_SetPriority(EXTI2_IRQn, 1, 0);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 0);
	HAL_NVIC_SetPriority(EXTI4_IRQn, 1, 0);
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	for (uint8_t i = 0; i < MaxEntries; i++) {
		entries[i].gpio = NULL;
		entries[i].cb = NULL;
		entries[i].ptr = NULL;
	}
}

bool Exti::SetCallback(GPIO_TypeDef *gpio, uint16_t pin, EdgeType edge,
		Pull pull, Callback cb, void *ptr) {
	uint8_t index = 31 - __builtin_clz(pin);

	if (entries[index].gpio && entries[index].gpio != gpio) {
		return false;
	}

	GPIO_InitTypeDef g;
	g.Pin = pin;
	switch (edge) {
	case EdgeType::Falling:
		g.Mode = GPIO_MODE_IT_FALLING;
		break;
	case EdgeType::Rising:
		g.Mode = GPIO_MODE_IT_RISING;
		break;
	case EdgeType::Both:
		g.Mode = GPIO_MODE_IT_RISING_FALLING;
		break;
	}
	switch (pull) {
	case Pull::None:
		g.Pull = GPIO_NOPULL;
		break;
	case Pull::Up:
		g.Pull = GPIO_PULLUP;
		break;
	case Pull::Down:
		g.Pull = GPIO_PULLDOWN;
		break;
	}
	HAL_GPIO_Init(gpio, &g);

	entries[index].gpio = gpio;
	entries[index].cb = cb;
	entries[index].ptr = ptr;
	return true;
}

bool Exti::ClearCallback(GPIO_TypeDef *gpio, uint16_t pin) {
	uint8_t index = 31 - __builtin_clz(pin);
	if (entries[index].gpio) {
		if (gpio != entries[index].gpio) {
			return false;
		}
	}

	entries[index].gpio = NULL;
	entries[index].cb = NULL;
	entries[index].ptr = NULL;
	return true;
}

static inline void ExtiHandler(uint16_t source) {
	const uint32_t mask = 1 << source;
	if (__HAL_GPIO_EXTI_GET_IT(mask) != RESET) {
		__HAL_GPIO_EXTI_CLEAR_IT(mask);
		if (entries[source].cb) {
			entries[source].cb(entries[source].ptr);
		}
	}
}

extern "C" {
void EXTI0_IRQHandler(void) {
	ExtiHandler(0);
}

void EXTI1_IRQHandler(void) {
	ExtiHandler(1);
}

void EXTI2_IRQHandler(void) {
	ExtiHandler(2);
}

void EXTI3_IRQHandler(void) {
	ExtiHandler(3);
}

void EXTI4_IRQHandler(void) {
	ExtiHandler(4);
}

void EXTI9_5_IRQHandler(void) {
	ExtiHandler(5);
	ExtiHandler(6);
	ExtiHandler(7);
	ExtiHandler(8);
	ExtiHandler(9);
}

void EXTI15_10_IRQHandler(void) {
	ExtiHandler(10);
	ExtiHandler(11);
	ExtiHandler(12);
	ExtiHandler(13);
	ExtiHandler(14);
	ExtiHandler(15);
}
}

