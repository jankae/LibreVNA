#include "stm.hpp"

using Callback = void(*)(void);
static constexpr uint8_t numCallbacks = 10;
static Callback callbacks[numCallbacks];
uint8_t read_index, write_index;

static void increment(uint8_t &index) {
	if(index < numCallbacks - 1) {
		index++;
	} else {
		index = 0;
	}
}
static uint8_t callbackFifoLevel() {
	int8_t level = (int8_t) write_index - (int8_t) read_index;
	if(level < 0) {
		level += numCallbacks;
	}
	return (uint8_t) level;
}

void STM::Init() {
	read_index = write_index = 0;
	HAL_NVIC_SetPriority(COMP_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(COMP_IRQn);
}

bool STM::DispatchToInterrupt(void (*cb)(void)) {
	if(callbackFifoLevel() < numCallbacks - 1) {
		callbacks[write_index] = cb;
		increment(write_index);
		HAL_NVIC_SetPendingIRQ(COMP_IRQn);
		return true;
	} else {
		// already at limit
		return false;
	}
}

extern "C" {
void COMP_IRQHandler() {
	while(callbackFifoLevel() > 0) {
		if (callbacks[read_index]) {
			callbacks[read_index]();
		}
		increment(read_index);
	}
}
}
