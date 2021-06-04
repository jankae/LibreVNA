#pragma once

#include "stm32l443xx.h"
#include "stm32l4xx_hal.h"

namespace STM {

void Init();
// No FreeRTOS function calls are allowed from interrupts with higher priorities than 5.
// Certain parts of the data acquisition need higher priorities (so they don't get interrupted by FreeRTOS)
// but they also need to trigger FreeRTOS functions. This can be achieved by dispatching a function-pointer
// to a lower priority interrupt. The passed function can then handle the FreeRTOS function call
bool DispatchToInterrupt(void (*cb)(void));

static inline bool InInterrupt() {
	return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

}
