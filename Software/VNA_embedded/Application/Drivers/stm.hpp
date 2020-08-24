#pragma once

#include "stm32g431xx.h"
#include "stm32g4xx_hal.h"

namespace STM {

static inline bool InInterrupt() {
	return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

}
