#pragma once

#include "stm.hpp"
#include "RegisterDevice.hpp"

class STW81200 : public RegisterDevice {
	public:
		constexpr STW81200(const char *name, SPI_HandleTypeDef *spi, GPIO_TypeDef *LE,
				uint16_t LEpin, GPIO_TypeDef *HW_PD = nullptr,
				uint16_t HW_PDpin = 0, GPIO_TypeDef *PD_RF1 = nullptr,
				uint16_t PD_RF1pin = 0):
			RegisterDevice("STW81200", name),
			spi(spi),
			LE(LE),
			HW_PD(HW_PD),
			PD_RF1(PD_RF1),
			LEpin(LEpin),
			HW_PDpin(HW_PDpin),
			PD_RF1pin(PD_RF1pin)
			{};

		bool Init();

		void writeRegister(uint32_t address, uint64_t data) override;
		uint64_t readRegister(uint32_t address) override;

		enum class Reg : uint8_t {
			ST0 = 0x00,
			ST1 = 0x01,
			ST2 = 0x02,
			ST3 = 0x03,
			ST4 = 0x04,
			ST5 = 0x05,
			ST6 = 0x06,
			ST7 = 0x07,
			ST8 = 0x08,
			ST9 = 0x09,
			ST10 = 0x0A,
			ST11 = 0x0B,
		};
	private:
		void setLE(bool p);
		void setHW_PD(bool p);
		void setPD_RF1(bool p);
		void setReset(bool p);
		void updateRegisters();
		void writeRegister(Reg reg, uint64_t data);
		uint64_t readRegister(Reg reg);
		SPI_HandleTypeDef *spi;
		GPIO_TypeDef *LE, *HW_PD, *PD_RF1;
		uint16_t LEpin, HW_PDpin, PD_RF1pin;
};

