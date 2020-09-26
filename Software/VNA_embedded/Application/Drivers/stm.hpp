#pragma once

#include "stm32g431xx.h"
#include "stm32g4xx_hal.h"

extern ADC_HandleTypeDef hadc1;

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

static inline int8_t getTemperature() {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	int16_t adc = HAL_ADC_GetValue(&hadc1);
	int16_t rangeFrom = *TEMPSENSOR_CAL2_ADDR - *TEMPSENSOR_CAL1_ADDR;
	int16_t rangeTo = TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP;
	adc -= *TEMPSENSOR_CAL1_ADDR;
	int16_t result = ((int32_t) adc * rangeTo) / rangeFrom + TEMPSENSOR_CAL1_TEMP;
	if(result < -128) {
		result = -128;
	} else if(result > 127) {
		result = 127;
	}
	return result;
}

}
