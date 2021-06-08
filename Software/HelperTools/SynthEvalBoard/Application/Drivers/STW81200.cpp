#include "STW81200.hpp"
#include "FreeRTOS.h"
#include "task.h"

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#define LOG_MODULE	"STW81200"
#include "Log.h"

bool STW81200::Init() {
	setLE(false);
	setHW_PD(false);
	setPD_RF1(false);

	writeRegister(Reg::ST9, 0x00);

	return true;
}

void STW81200::writeRegister(uint32_t address, uint64_t data) {
	writeRegister((Reg) address, data);
}

uint64_t STW81200::readRegister(uint32_t address) {
	return readRegister((Reg) address);
}

void STW81200::writeRegister(Reg reg, uint64_t data) {
	uint8_t send[4];
	data &= 0x07FFFFFF;
	send[0] = (int) reg << 3;
	send[0] |= data >> 24;
	send[1] = (data >> 16) & 0xFF;
	send[2] = (data >> 8) & 0xFF;
	send[3] = (data >> 0) & 0xFF;
	HAL_SPI_Transmit(spi, send, sizeof(send), 100);
	setLE(true);
	setLE(false);
}

uint64_t STW81200::readRegister(Reg reg) {
	uint8_t send[4];
	uint8_t recv[4];
	send[0] = 0x80 | (int) reg << 3;
	send[1] = 0x00;
	send[2] = 0x00;
	send[3] = 0x00;
	HAL_SPI_TransmitReceive(spi, send, recv, sizeof(send), 100);
	setLE(true);
	setLE(false);
	uint64_t data = recv[0] & 0x07;
	data <<= 8;
	data |= recv[1];
	data <<= 8;
	data |= recv[2];
	data <<= 8;
	data |= recv[3];
	return data;
}

void STW81200::setLE(bool p) {
	if(!LE) {
		return;
	}
	if(p) {
		LE->BSRR = LEpin;
	} else {
		LE->BSRR = LEpin << 16;
	}
}

void STW81200::setHW_PD(bool p) {
	if(!HW_PD) {
		return;
	}
	if(p) {
		HW_PD->BSRR = HW_PDpin;
	} else {
		HW_PD->BSRR = HW_PDpin << 16;
	}
}

void STW81200::setPD_RF1(bool p) {
	if(!PD_RF1) {
		return;
	}
	if(p) {
		PD_RF1->BSRR = PD_RF1pin;
	} else {
		PD_RF1->BSRR = PD_RF1pin << 16;
	}
}



