#include "Firmware.hpp"

#include "Protocol.hpp"
#include <cstring>
#include "HW_HAL.hpp"

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"FW"
#include "Log.h"

static Firmware::FPGAImageInfo fpga_image_infos[FPGA_MAX_IMAGES];
static uint8_t num_fpga_images;

static uint32_t calcCRCoverFlashMemory(uint32_t start, uint32_t size) {
	uint32_t crc = UINT32_MAX;
	uint8_t buf[128];
	uint32_t checked_size = 0;
	while (checked_size < size) {
		uint16_t read_size = sizeof(buf);
		if (size - checked_size < read_size) {
			read_size = size - checked_size;
		}
		HWHAL::flash.read(start + checked_size, read_size, buf);
		crc = Protocol::CRC32(crc, buf, read_size);
		checked_size += read_size;
	}
	return crc;
}

Firmware::Info Firmware::GetFlashContentInfo() {
	Info ret;
	memset(&ret, 0, sizeof(ret));
	ret.num_FPGA_images = 1;
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
	if (calcCRCoverFlashMemory(h.FPGA_start, h.FPGA_size + h.CPU_size) != h.crc) {
		LOG_ERR("CRC mismatch, invalid FPGA bitstream/CPU firmware");
		return ret;
	}
	// Compare CPU firmware in external Flash to the one currently running in the MCU
	uint8_t buf[128];
	uint32_t checked_size = 0;
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
	fpga_image_infos[0].start_address = h.FPGA_start;
	fpga_image_infos[0].size = h.FPGA_size;
	ret.CPU_image_address = h.CPU_start;
	ret.CPU_image_size = h.CPU_size;

	// check if additional FPGA images are present
	if(h.FPGA_start >= sizeof(Header) + sizeof(AdditionalFPGAImageHeader)
			&& h.CPU_start >= sizeof(Header) + sizeof(AdditionalFPGAImageHeader)) {
		// there is enough room before the images for the additional image header
		AdditionalFPGAImageHeader ah;
		HWHAL::flash.read(sizeof(Header), sizeof(ah), &ah);
		uint8_t additionalImages = 0;
		// sanity check values
		if (memcmp(&ah.magic, "VNA", 3) == 0) {
			additionalImages = ah.additionalImages;
		}
		const uint32_t FPGAHeaderStart = sizeof(Header) + sizeof(AdditionalFPGAImageHeader);
		for(auto i=0;i<additionalImages;i++) {
			FPGAImageHeader fh;
			HWHAL::flash.read(FPGAHeaderStart + i*sizeof(fh), sizeof(fh), &fh);
			// Sanity check values
			if (fh.start >= UINT32_MAX || fh.size > FPGA_MAXSIZE) {
				LOG_WARN("Ignoring additional FPGA image %d, implausible start/size", i);
				continue;
			}
			// check CRC
			if (calcCRCoverFlashMemory(fh.start, fh.size) != fh.crc) {
				LOG_WARN("Ignoring additional FPGA image %d, CRC mismatch", i);
				continue;
			}
			// add to available FPGA images
			fpga_image_infos[ret.num_FPGA_images].start_address = fh.start;
			fpga_image_infos[ret.num_FPGA_images].size = fh.size;
			ret.num_FPGA_images++;
		}
	}
	num_fpga_images = ret.num_FPGA_images;
	LOG_INFO("FLASH evaluation finished, got %d FPGA images", ret.num_FPGA_images);

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

Firmware::FPGAImageInfo Firmware::GetFPGAImageInfo(uint8_t image_num) {
	if(image_num < num_fpga_images) {
		return fpga_image_infos[image_num];
	} else {
		FPGAImageInfo ret = {};
		return ret;
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
