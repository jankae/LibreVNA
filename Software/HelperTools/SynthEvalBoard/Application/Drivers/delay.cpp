#include "delay.hpp"

#include "stm.hpp"

void Delay::ms(uint32_t t) {
	while(t--) {
		us(1000);
	}
}
void Delay::us(uint32_t t) {
	TIM1->CNT = 0;
	TIM1->CR1 |= TIM_CR1_CEN;
	while (TIM1->CNT < t)
		;
	TIM1->CR1 &= ~TIM_CR1_CEN;
}
