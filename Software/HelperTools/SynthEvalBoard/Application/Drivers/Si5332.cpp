#include "Si5332.hpp"

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"Si5332"
#include "Log.h"

bool Si5332::Init() {
	uint8_t dummy;
	if(HAL_I2C_Mem_Read(i2c, this->address, 0, I2C_MEMADD_SIZE_8BIT, &dummy, 1, 100) != HAL_OK) {
		LOG_ERR("Failed to initialize");
		return false;
	}

	LOG_INFO("Initialized");
	return true;
}

void Si5332::writeRegister(uint32_t address, uint64_t data) {
	uint8_t write = data;
	HAL_I2C_Mem_Write(i2c, this->address, address, I2C_MEMADD_SIZE_8BIT, &write, 1, 100);
}

uint64_t Si5332::readRegister(uint32_t address) {
	uint8_t read;
	HAL_I2C_Mem_Read(i2c, this->address, address, I2C_MEMADD_SIZE_8BIT, &read, 1, 100);
	return read;
}
