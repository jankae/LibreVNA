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
	bool write(uint32_t address, uint16_t length, uint8_t *src);
	bool eraseChip();
	// Starts the reading process without actually reading any bytes
	void initiateRead(uint32_t address);
	const SPI_HandleTypeDef* const getSpi() const {
		return spi;
	}

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
