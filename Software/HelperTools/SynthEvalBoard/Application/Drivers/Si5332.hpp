#pragma once

#include "stm.hpp"
#include "RegisterDevice.hpp"

class Si5332 : public RegisterDevice {
	public:
		constexpr Si5332(const char *name, I2C_HandleTypeDef *i2c, uint8_t address):
			RegisterDevice("Si5332", name),
			i2c(i2c),
			address(address){};

		bool Init();

		void writeRegister(uint32_t address, uint64_t data) override;
		uint64_t readRegister(uint32_t address) override;
	private:
		I2C_HandleTypeDef *i2c;
		uint8_t address;
};

