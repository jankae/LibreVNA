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
	// Check against valid manufacturer IDs
	constexpr uint8_t valid_ids[] = {0xEF, 0x68, 0x9D};
	bool valid = false;
	for (uint8_t i = 0; i < sizeof(valid_ids); i++) {
		if (recv[1] == valid_ids[i]) {
			valid = true;
			break;
		}
	}
	return valid;
}

void Flash::read(uint32_t address, uint16_t length, void *dest) {
	initiateRead(address);
	// read data
	HAL_SPI_Receive(spi, (uint8_t*) dest, length, 1000);
	CS(true);
}

bool Flash::write(uint32_t address, uint16_t length, void *src) {
	if(address % PageSize != 0 || length%PageSize != 0) {
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
		// issue write command
		HAL_SPI_Transmit(spi, cmd, 4, 100);
		// write data
		HAL_SPI_Transmit(spi, (uint8_t*) src, 256, 1000);
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
	LOG_INFO("Erasing chip...");
	EnableWrite();
	CS(false);
	uint8_t chip_erase = 0x60;
	HAL_SPI_Transmit(spi, &chip_erase, 1, 100);
	CS(true);
	return WaitBusy(25000);
}

bool Flash::eraseSector(uint32_t address) {
	// align address with sector address
	address -= address % SectorSize;
	LOG_INFO("Erasing sector at %lu", address);
	EnableWrite();
	CS(false);
	uint8_t cmd[4] = {
		0x20,
		(uint8_t) (address >> 16) & 0xFF,
		(uint8_t) (address >> 8) & 0xFF,
		(uint8_t) (address & 0xFF),
	};
	HAL_SPI_Transmit(spi, cmd, 4, 100);
	CS(true);
	return WaitBusy(25000);
}

bool Flash::erase32Block(uint32_t address) {
	// align address with block address
	address -= address % Block32Size;
	LOG_INFO("Erasing 32kB block at %lu", address);
	EnableWrite();
	CS(false);
	uint8_t cmd[4] = {
		0x52,
		(uint8_t) (address >> 16) & 0xFF,
		(uint8_t) (address >> 8) & 0xFF,
		(uint8_t) (address & 0xFF),
	};
	HAL_SPI_Transmit(spi, cmd, 4, 100);
	CS(true);
	return WaitBusy(25000);
}

bool Flash::erase64Block(uint32_t address) {
	// align address with block address
	address -= address % Block64Size;
	LOG_INFO("Erasing 64kB block at %lu", address);
	EnableWrite();
	CS(false);
	uint8_t cmd[4] = {
		0xD8,
		(uint8_t) (address >> 16) & 0xFF,
		(uint8_t) (address >> 8) & 0xFF,
		(uint8_t) (address & 0xFF),
	};
	HAL_SPI_Transmit(spi, cmd, 4, 100);
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

bool Flash::eraseRange(uint32_t start, uint32_t len) {
	if(start % SectorSize != 0) {
		LOG_ERR("Start address of range has to be sector aligned (is %lu)", start);
		return false;
	}
	if(len % SectorSize != 0) {
		LOG_ERR("Length of range has to be multiple of sector size (is %lu)", len);
		return false;
	}
	uint32_t erased_len = 0;
	while(erased_len < len) {
		uint32_t remaining = len - erased_len;
		if(remaining >= Block64Size && start % Block64Size == 0) {
			erase64Block(start);
			erased_len += Block64Size;
			start += Block64Size;
			continue;
		}
		if(remaining >= Block32Size && start % Block32Size == 0) {
			erase32Block(start);
			erased_len += Block32Size;
			start += Block32Size;
			continue;
		}
		if(remaining >= SectorSize && start % SectorSize == 0) {
			eraseSector(start);
			erased_len += SectorSize;
			start += SectorSize;
			continue;
		}
		// Should never get here
	}
	return true;
}
