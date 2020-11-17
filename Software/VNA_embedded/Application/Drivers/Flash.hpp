/*
 * Flash.hpp
 *
 *  Created on: Aug 26, 2020
 *      Author: jan
 */

#ifndef DRIVERS_FLASH_HPP_
#define DRIVERS_FLASH_HPP_

#include "stm.hpp"

class Flash {
public:
	constexpr Flash(SPI_HandleTypeDef *spi, GPIO_TypeDef *CS_gpio, uint16_t CS_pin)
	: spi(spi),CS_gpio(CS_gpio),CS_pin(CS_pin){};

	bool isPresent();
	void read(uint32_t address, uint16_t length, void *dest);
	bool write(uint32_t address, uint16_t length, void *src);
	bool eraseChip();
	bool eraseSector(uint32_t address);
	bool erase32Block(uint32_t address);
	bool erase64Block(uint32_t address);
	bool eraseRange(uint32_t start, uint32_t len);
	// Starts the reading process without actually reading any bytes
	void initiateRead(uint32_t address);
	const SPI_HandleTypeDef* const getSpi() const {
		return spi;
	}
	static constexpr uint32_t PageSize = 256;
	static constexpr uint32_t SectorSize = 4096;
	static constexpr uint32_t Block32Size = 32768;
	static constexpr uint32_t Block64Size = 65536;

private:
	void CS(bool high) {
		if(high) {
			CS_gpio->BSRR = CS_pin;
		} else {
			CS_gpio->BSRR = CS_pin << 16;
		}
	}
	void EnableWrite();
	bool WaitBusy(uint32_t timeout);
	SPI_HandleTypeDef * const spi;
	GPIO_TypeDef * const CS_gpio;
	const uint16_t CS_pin;
};


#endif /* DRIVERS_FLASH_HPP_ */
