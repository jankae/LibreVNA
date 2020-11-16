#include "Firmware.hpp"

#include "Protocol.hpp"
#include <cstring>
#include "HW_HAL.hpp"

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"FW"
#include "Log.h"

#define FPGA_MAXSIZE	512000
#define CPU_MAXSIZE		131072

using Header = struct {
	char magic[4];
	uint32_t FPGA_start;
	uint32_t FPGA_size;
	uint32_t CPU_start;
	uint32_t CPU_size;
	uint32_t crc;
} __attribute__((packed));

Firmware::Info Firmware::GetFlashContentInfo() {
	Info ret;
	memset(&ret, 0, sizeof(ret));
	Header h;
	HWHAL::flash.read(0, sizeof(h), &h);
	// sanity check values
	if (memcmp(&h.magic, "VNA!",
			4) || h.FPGA_start == UINT32_MAX || h.FPGA_size > FPGA_MAXSIZE
			|| h.CPU_start == UINT32_MAX || h.CPU_size > CPU_MAXSIZE) {
		LOG_WARN("Invalid content, probably empty FLASH");
		return ret;
	}
	LOG_DEBUG("Checking FPGA bitstream...");
	uint32_t crc = UINT32_MAX;
	uint8_t buf[128];
	uint32_t checked_size = 0;
	while (checked_size < h.FPGA_size + h.CPU_size) {
		uint16_t read_size = sizeof(buf);
		if (h.FPGA_size + h.CPU_size - checked_size < read_size) {
			read_size = h.FPGA_size + h.CPU_size - checked_size;
		}
		HWHAL::flash.read(h.FPGA_start + checked_size, read_size, buf);
		crc = Protocol::CRC32(crc, buf, read_size);
		checked_size += read_size;
	}
	if (crc != h.crc) {
		LOG_ERR("CRC mismatch, invalid FPGA bitstream/CPU firmware");
		return ret;
	}
	// Compare CPU firmware in external Flash to the one currently running in the MCU
	checked_size = 0;
	while (checked_size < h.CPU_size) {
		uint16_t read_size = sizeof(buf);
		if (h.CPU_size - checked_size < read_size) {
			read_size = h.CPU_size - checked_size;
		}
		HWHAL::flash.read(h.CPU_start + checked_size, read_size, buf);
		if(memcmp(buf, (void*)(0x8000000+checked_size), read_size)) {
			LOG_INFO("Difference to CPU firmware in external FLASH detected, update required");
			ret.CPU_need_update = true;
			break;
		}
		checked_size += read_size;
	}
	ret.valid = true;
	ret.FPGA_bitstream_address = h.FPGA_start;
	ret.FPGA_bitstream_size = h.FPGA_size;
	ret.CPU_image_address = h.CPU_start;
	ret.CPU_image_size = h.CPU_size;

	return ret;
}

static void copy_flash(uint32_t size, SPI_TypeDef *spi) __attribute__ ((noinline, section (".data")));

/* This function is executed from RAM as it possibly overwrites the whole FLASH.
 *
 * It assumes that the flash has already be unlocked and the SPI interface to the
 * external flash has already initiated a read command. At the end of the copy
 * process it initiates a software reset.
 *
 * !NO FUNCTION CALLS AT ALL ARE ALLOWED IN HERE!
 */
static void copy_flash(uint32_t size, SPI_TypeDef *spi) {
	/* First, erase internal flash */
	/* disable caches */
	__HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
	__HAL_FLASH_DATA_CACHE_DISABLE();
	/* Erase FLASH */
	// Erase the only flash bank
	SET_BIT(FLASH->CR, FLASH_CR_MER1);
	// Start erase process
	SET_BIT(FLASH->CR, FLASH_CR_STRT);
	// Wait for operation to finish
	while (FLASH->SR & FLASH_SR_BSY)
	;
	// Clear bank1 erase request
	CLEAR_BIT(FLASH->CR, (FLASH_CR_MER1));

	/* The complete FLASH has been erased. Copy the external FLASH memory
	 * content into the internal MCU flash */
	uint32_t written = 0;

	// Enable FIFO notifications for 8 bit
	SET_BIT(spi->CR2, SPI_RXFIFO_THRESHOLD);

	/* Enable FLASH write */
	FLASH->CR |= FLASH_CR_PG;
	uint32_t *to = (uint32_t*) 0x8000000;
	while (written < size) {
		uint8_t buf[8];
		// Get 64bit from external flash
		for(uint8_t i=0;i<8;i++) {
			// wait for SPI ready to transmit dummy data
			while(!(spi->SR & SPI_FLAG_TXE));
			// send dummy byte
			*(__IO uint8_t *)&spi->DR = 0x00;
			// wait for received byte to be ready
			while(!(spi->SR & SPI_FLAG_RXNE));
			// get received byte
			buf[i] = *(__IO uint8_t *)&spi->DR;
		}
		// program received data into flash
		*(__IO uint32_t*) to++ = *(uint32_t*)&buf[0];
		__ISB();
		*(__IO uint32_t*) to++ = *(uint32_t*)&buf[4];
		/* Wait for it to finish */
		while (FLASH->SR & FLASH_SR_BSY)
			;
		// clear possible error flags (there is no way to respond to errors at this point)
		uint32_t error = (FLASH->SR & FLASH_FLAG_SR_ERRORS);
		error |= (FLASH->ECCR & FLASH_FLAG_ECCD);
		if (error != 0u) {
			/* Clear error programming flags */
			__HAL_FLASH_CLEAR_FLAG(error);
		}
		/* Check FLASH End of Operation flag  */
		if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP)) {
			/* Clear FLASH End of Operation pending bit */
			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
		}
		written += 8;
	}
	/* Write operation completed, disable the PG Bit */
	FLASH->CR &= (~FLASH_CR_PG);

	/* The new firmware is in place. This function can not return as the return
	 * address might be anywhere in the new firmware. Instead perform a software reset
	 * here */
	__DSB();
	SCB->AIRCR = ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos) |
	SCB_AIRCR_SYSRESETREQ_Msk);
	__DSB();

	for (;;) {
		__NOP();
	}
}

void Firmware::PerformUpdate(Info info) {
	if(!info.valid) {
		LOG_ERR("Invalid firmware data, not performing update");
		return;
	}

	LOG_INFO("Loading new firmware...");
	Log_Flush();

	__disable_irq();

	/* Flash must be unlocked */
	HAL_FLASH_Unlock();
	FLASH_WaitForLastOperation(50000);

	// Initiate readback from flash at CPU firmware start address
	HWHAL::flash.initiateRead(info.CPU_image_address);

	copy_flash(info.CPU_image_size, HWHAL::flash.getSpi()->Instance);
	__builtin_unreachable();
}
