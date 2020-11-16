#pragma once

#include "stm.hpp"
#include "Si5351C.hpp"
#include "max2871.hpp"
#include "main.h"
#include "Flash.hpp"

extern I2C_HandleTypeDef hi2c2;
extern SPI_HandleTypeDef hspi1;

namespace HWHAL {

extern Si5351C Si5351;
extern MAX2871 Source;
extern MAX2871 LO1;
extern Flash flash;

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
