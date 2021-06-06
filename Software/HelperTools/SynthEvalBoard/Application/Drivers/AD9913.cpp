#include "AD9913.hpp"
#include "FreeRTOS.h"
#include "task.h"

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#define LOG_MODULE	"AD9913"
#include "Log.h"

bool AD9913::Init() {
	setCS(true);
	setPWR_DWN(false);
	setIO_UPDATE(false);

	setReset(true);
	vTaskDelay(1);
	setReset(false);
//
//	while(1) {
//	uint64_t dac = readRegister(Reg::LINSWEEPPARAM);
//	LOG_DEBUG("DAC Control register: 0x%08x", dac);
//	vTaskDelay(100);
//	}
	return true;
}

void AD9913::writeRegister(uint32_t address, uint64_t data) {
	writeRegister((Reg) address, data);
	updateRegisters();
}

uint64_t AD9913::readRegister(uint32_t address) {
	return readRegister((Reg) address);
}

void AD9913::writeRegister(Reg reg, uint64_t data) {
	uint8_t regsize = getRegisterSize(reg);
	if(!regsize) {
		// invalid register
		return;
	}
	uint8_t send[1 + regsize];
	send[0] = (uint8_t) reg;
	for(uint8_t i=regsize;i>0;i--) {
		send[i] = data & 0xFF;
		data >>= 8;
	}
	setCS(false);
	HAL_SPI_Transmit(spi, send, sizeof(send), 100);
	setCS(true);
}

uint64_t AD9913::readRegister(Reg reg) {
	uint8_t regsize = getRegisterSize(reg);
	if(!regsize) {
		// invalid register
		return 0;
	}
	uint8_t send[1 + regsize];
	uint8_t recv[1 + regsize];
	send[0] = 0x80 | (uint8_t) reg;
	for(uint8_t i=regsize;i>0;i--) {
		send[i] = 0xFF; // use open drain with pull up
	}
	setCS(false);
	HAL_SPI_TransmitReceive(spi, send, recv, sizeof(send), 100);
	setCS(true);
	uint64_t ret = 0;
	for(uint8_t i=1;i<=regsize;i++) {
		ret <<= 8;
		ret |= recv[i];
	}
	return ret;
}

void AD9913::setCS(bool p) {
	if(!CS) {
		return;
	}
	if(p) {
		CS->BSRR = CSpin;
	} else {
		CS->BSRR = CSpin << 16;
	}
}

void AD9913::setPWR_DWN(bool p) {
	if(!PWR_DWN) {
		return;
	}
	if(p) {
		PWR_DWN->BSRR = PWR_DWNpin;
	} else {
		PWR_DWN->BSRR = PWR_DWNpin << 16;
	}
}

void AD9913::setIO_UPDATE(bool p) {
	if(!IO_UPDATE) {
		return;
	}
	if(p) {
		IO_UPDATE->BSRR = IO_UPDATEpin;
	} else {
		IO_UPDATE->BSRR = IO_UPDATEpin << 16;
	}
}

void AD9913::setReset(bool p) {
	if(!MRESET) {
		return;
	}
	if(p) {
		MRESET->BSRR = MRESETpin;
	} else {
		MRESET->BSRR = MRESETpin << 16;
	}
}

void AD9913::updateRegisters() {
	setIO_UPDATE(true);
	vTaskDelay(1);
	setIO_UPDATE(false);
}

uint8_t AD9913::getRegisterSize(Reg reg) {
	switch(reg) {
	case Reg::CFR2:
	case Reg::POW:
		return 2; // 16 bit registers
	case Reg::CFR1:
	case Reg::DACControl:
	case Reg::FTW:
	case Reg::LINSWEEPRAMP:
		return 4; // 32 bit registers
	case Reg::LINSWEEPPARAM:
	case Reg::LINSWEEPDELTA:
		return 8; // 64 bit registers
	case Reg::PROFILE0:
	case Reg::PROFILE1:
	case Reg::PROFILE2:
	case Reg::PROFILE3:
	case Reg::PROFILE4:
	case Reg::PROFILE5:
	case Reg::PROFILE6:
	case Reg::PROFILE7:
		return 6; // 48 bit registers
	default:
		return 0; // invalid register
	}
}


