/*
 * Firmware.hpp
 *
 *  Created on: Aug 26, 2020
 *      Author: jan
 */

#ifndef COMMUNICATION_FIRMWARE_HPP_
#define COMMUNICATION_FIRMWARE_HPP_

#include "Flash.hpp"

namespace Firmware {

#define FPGA_MAX_IMAGES	4

static constexpr uint32_t FPGA_MAXSIZE = 400000;
static constexpr uint32_t CPU_MAXSIZE = 131072;

using Info = struct info {
	uint32_t CPU_image_address;
	uint32_t CPU_image_size;
	uint8_t num_FPGA_images;
	bool valid;
	bool CPU_need_update;
};

using FPGAImageInfo = struct fpgaimageinfo {
	uint32_t start_address;
	uint32_t size;
};

using Header = struct {
	char magic[4];
	uint32_t FPGA_start;
	uint32_t FPGA_size;
	uint32_t CPU_start;
	uint32_t CPU_size;
	uint32_t crc;
} __attribute__((packed));

using AdditionalFPGAImageHeader = struct {
	char magic[3];
	uint8_t additionalImages;
} __attribute__((packed));

using FPGAImageHeader = struct {
	uint32_t start;
	uint32_t size;
	uint32_t crc;
} __attribute__((packed));

static constexpr uint32_t calcMaxSize() {
	uint32_t max = sizeof(Header)
				+ sizeof(AdditionalFPGAImageHeader)
				+ sizeof(FPGAImageHeader)*FPGA_MAX_IMAGES
				+ FPGA_MAXSIZE*FPGA_MAX_IMAGES
				+ CPU_MAXSIZE;
	// round up to next flash sector
	if(max % Flash::SectorSize) {
		max += Flash::SectorSize - max % Flash::SectorSize;
	}
	return max;
}

static constexpr uint32_t maxSize = calcMaxSize();

static_assert(maxSize < Flash::FlashSize);

FPGAImageInfo GetFPGAImageInfo(uint8_t image_num);

Info GetFlashContentInfo();
void PerformUpdate(Info info);

}


#endif /* COMMUNICATION_FIRMWARE_HPP_ */
