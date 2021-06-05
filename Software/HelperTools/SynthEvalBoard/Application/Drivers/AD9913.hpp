#pragma once

#include "stm.hpp"
#include "RegisterDevice.hpp"

class AD9913 : public RegisterDevice {
	public:
		constexpr AD9913(const char *name, SPI_HandleTypeDef *spi, GPIO_TypeDef *CS,
				uint16_t CSpin, GPIO_TypeDef *IO_UPDATE,
				uint16_t IO_UPDATEpin, GPIO_TypeDef *MRESET,
				uint16_t MRESETpin, GPIO_TypeDef *PWR_DWN = nullptr,
				uint16_t PWR_DWNpin = 0):
			RegisterDevice("AD9913", name),
			spi(spi),
			CS(CS),
			IO_UPDATE(IO_UPDATE),
			PWR_DWN(PWR_DWN),
			MRESET(MRESET),
			CSpin(CSpin),
			IO_UPDATEpin(IO_UPDATEpin),
			PWR_DWNpin(PWR_DWNpin),
			MRESETpin(MRESETpin)
			{};

		bool Init();

		void writeRegister(uint32_t address, uint64_t data) override;
		uint64_t readRegister(uint32_t address) override;

		enum class Reg : uint8_t {
			CFR1 = 0x00,
			CFR2 = 0x01,
			DACControl = 0x02,
			FTW = 0x03,
			POW = 0x04,
			LINSWEEPPARAM = 0x06,
			LINSWEEPDELTA = 0x07,
			LINSWEEPRAMP = 0x08,
			PROFILE0 = 0x09,
			PROFILE1 = 0x0A,
			PROFILE2 = 0x0B,
			PROFILE3 = 0x0C,
			PROFILE4 = 0x0D,
			PROFILE5 = 0x0E,
			PROFILE6 = 0x0F,
			PROFILE7 = 0x10,
		};
	private:
		void setCS(bool p);
		void setPWR_DWN(bool p);
		void setIO_UPDATE(bool p);
		void setReset(bool p);
		void updateRegisters();
		void writeRegister(Reg reg, uint64_t data);
		uint64_t readRegister(Reg reg);
		static uint8_t getRegisterSize(Reg reg);
		SPI_HandleTypeDef *spi;
		GPIO_TypeDef *CS, *IO_UPDATE, *PWR_DWN, *MRESET;
		uint16_t CSpin, IO_UPDATEpin, PWR_DWNpin, MRESETpin;
};

