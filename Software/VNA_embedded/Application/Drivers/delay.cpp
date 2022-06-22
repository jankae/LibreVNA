#include "delay.hpp"

#include "stm.hpp"

static volatile uint64_t t_us = 0;

void Delay::Init() {
	TIM1->CNT = 0;

	// enable update interrupt
	TIM1->DIER |= TIM_DIER_UIE;
    HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

	TIM1->CR1 |= TIM_CR1_CEN;
}

uint64_t Delay::get_us() {
    HAL_NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);
	uint32_t buf = TIM1->CNT;
	uint16_t timer_value = buf & 0xFFFF;
	uint64_t ret;
	if(buf & 0x80000000) {
		// UIF bit set, timer overflow not handled yet
		ret = t_us + UINT16_MAX + timer_value;
	} else {
		ret = t_us + timer_value;
	}
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
    return ret;
}

void Delay::ms(uint32_t t) {
	while(t--) {
		us(1000);
	}
}
void Delay::us(uint32_t t) {
	uint64_t start = TIM1->CNT;
	while(TIM1->CNT - start < t);
}

extern "C" {

void TIM1_UP_TIM16_IRQHandler() {
	// clear bit
	TIM1->SR &= ~TIM_SR_UIF;
	// update count
	t_us += UINT16_MAX;
}

}
