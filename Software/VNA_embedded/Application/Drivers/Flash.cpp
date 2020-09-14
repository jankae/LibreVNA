#include "Flash.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include <cstring>

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"Flash"
#include "Log.h"

bool Flash::isPresent() {
	CS(false);
	// read JEDEC ID
	uint8_t send[4] = {0x9F};
	uint8_t recv[4];
	HAL_SPI_TransmitReceive(spi, send, recv, 4, 100);
	CS(true);
	if(recv[1] != 0xEF) {
		// wrong manufacturer ID, communication with flash not working
		return false;
	}
	return true;
}

void Flash::read(uint32_t address, uint16_t length, void *dest) {
	initiateRead(address);
	// read data
	HAL_SPI_Receive(spi, (uint8_t*) dest, length, 1000);
	CS(true);
}

bool Flash::write(uint32_t address, uint16_t length, uint8_t *src) {
	if((address & 0xFF) != 0 || length%256 != 0) {
		// only writes to complete pages allowed
		LOG_ERR("Invalid write address/size: %lu/%u", address, length);
		return false;
	}
	address &= 0x00FFFFFF;
	LOG_DEBUG("Writing %u bytes to address %lu", length, address);
	while(length > 0) {
		EnableWrite();
		CS(false);
		uint8_t cmd[4] = {
			0x02,
			(uint8_t) (address >> 16) & 0xFF,
			(uint8_t) (address >> 8) & 0xFF,
			(uint8_t) (address & 0xFF),
		};
		// issue read command
		HAL_SPI_Transmit(spi, cmd, 4, 100);
		// write data
		HAL_SPI_Transmit(spi, src, 256, 1000);
		CS(true);
		if(!WaitBusy(20)) {
			LOG_ERR("Write timed out");
			return false;
		}
		// Verify
		uint8_t buf[256];
		read(address, 256, buf);
		if(memcmp(src, buf, 256)) {
			LOG_ERR("Verification error");
			return false;
		}
		address += 256;
		length -= 256;
		src += 256;
	}
	return true;
}

void Flash::EnableWrite() {
	CS(false);
	// enable write latch
	uint8_t wel = 0x06;
	HAL_SPI_Transmit(spi, &wel, 1, 100);
	CS(true);
}

bool Flash::eraseChip() {
	LOG_INFO("Erasing...");
	EnableWrite();
	CS(false);
	// enable write latch
	uint8_t chip_erase = 0x60;
	HAL_SPI_Transmit(spi, &chip_erase, 1, 100);
	CS(true);
	return WaitBusy(25000);
}

void Flash::initiateRead(uint32_t address) {
	address &= 0x00FFFFFF;
	CS(false);
	uint8_t cmd[4] = {
		0x03,
		(uint8_t) (address >> 16) & 0xFF,
		(uint8_t) (address >> 8) & 0xFF,
		(uint8_t) (address & 0xFF),
	};
	// issue read command
	HAL_SPI_Transmit(spi, cmd, 4, 100);
}

bool Flash::WaitBusy(uint32_t timeout) {
	uint32_t starttime = HAL_GetTick();
	CS(false);
	uint8_t readStatus1 = 0x05;
	HAL_SPI_Transmit(spi, &readStatus1, 1, 100);
	do {
		vTaskDelay(1);
		uint8_t status1;
		HAL_SPI_Receive(spi, &status1, 1, 100);
		if (!(status1 & 0x01)) {
			CS(true);
			return true;
		}
	} while (HAL_GetTick() - starttime < timeout);
	// timed out
	CS(true);
	LOG_ERR("Timeout occured");
	return false;
}
