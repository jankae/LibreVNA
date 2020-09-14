#pragma once

#include "stm.hpp"
#include "Si5351C.hpp"
#include "max2871.hpp"
#include "main.h"

extern I2C_HandleTypeDef hi2c2;
extern SPI_HandleTypeDef hspi1;

namespace VNAHAL {

static Si5351C Si5351 = Si5351C(&hi2c2, 26000000);
static MAX2871 Source = MAX2871(&hspi1, FPGA_CS_GPIO_Port, FPGA_CS_Pin, nullptr, 0, nullptr, 0, nullptr, 0, GPIOA, GPIO_PIN_6);
static MAX2871 LO1 = MAX2871(&hspi1, FPGA_CS_GPIO_Port, FPGA_CS_Pin, nullptr, 0, nullptr, 0, nullptr, 0, GPIOA, GPIO_PIN_6);

// Mapping of the Si5351 channels to PLLs/Mixers
namespace SiChannel {
	enum {
		Source = 3,
		LO1 = 5,
		Port2LO2 = 4,
		RefLO2 = 1,
		Port1LO2 = 2,
		LowbandSource = 0,
		ReferenceOut = 6,
		FPGA = 7,
	};
}

}
