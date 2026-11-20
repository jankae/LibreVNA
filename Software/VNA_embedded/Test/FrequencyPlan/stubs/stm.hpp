#pragma once
// Host stub for the STM32 HAL, enough to compile Si5351C.cpp, max2871.cpp and algorithm.cpp.
// The Si5351 I2C bus is an in-memory register file that the test reads back.
#include <cstdint>
#include <cstring>

typedef struct { int dummy; } I2C_HandleTypeDef;
typedef struct { int dummy; } SPI_HandleTypeDef;
typedef struct { volatile uint32_t BSRR, IDR; } GPIO_TypeDef;
typedef struct { int dummy; } GPIO_InitTypeDef;
typedef enum { HAL_OK = 0, HAL_ERROR = 1 } HAL_StatusTypeDef;
#define I2C_MEMADD_SIZE_8BIT 1

namespace hoststub {
extern uint8_t si5351_regs[256];
extern uint32_t tick;
}

static inline HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *, uint16_t, uint16_t reg, uint16_t,
		uint8_t *data, uint16_t len, uint32_t) {
	for (uint16_t i = 0; i < len; i++) {
		hoststub::si5351_regs[(reg + i) & 0xFF] = data[i];
	}
	return HAL_OK;
}
static inline HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *, uint16_t, uint16_t reg, uint16_t,
		uint8_t *data, uint16_t len, uint32_t) {
	for (uint16_t i = 0; i < len; i++) {
		// register 0 (device status): report both PLLs locked, CLKIN present
		data[i] = reg + i == 0 ? 0 : hoststub::si5351_regs[(reg + i) & 0xFF];
	}
	return HAL_OK;
}
static inline HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *, uint8_t *, uint16_t, uint32_t) { return HAL_OK; }
static inline HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *, uint8_t *, uint8_t *r, uint16_t n, uint32_t) {
	memset(r, 0, n);
	return HAL_OK;
}
static inline uint32_t HAL_GetTick() { return hoststub::tick++; }
static inline void HAL_Delay(uint32_t) {}
