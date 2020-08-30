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

using Info = struct info {
	uint32_t FPGA_bitstream_address;
	uint32_t FPGA_bitstream_size;
	uint32_t CPU_image_address;
	uint32_t CPU_image_size;
	bool valid;
	bool CPU_need_update;
};

Info GetFlashContentInfo(Flash *f);
void PerformUpdate(Flash *f);

}


#endif /* COMMUNICATION_FIRMWARE_HPP_ */
