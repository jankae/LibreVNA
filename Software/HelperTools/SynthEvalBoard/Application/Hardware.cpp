#include "Hardware.hpp"

#include "Protocol.hpp"

#include "AD9913.hpp"
#include "Si5332.hpp"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1, hspi2;

static Si5332 si5332 = Si5332("Si5332", &hi2c1, 0xD4);

static AD9913 ad9913 = AD9913("AD9913", &hspi2, AD9913_CS_GPIO_Port,
	AD9913_CS_Pin, AD9913_IO_UPDATE_GPIO_Port, AD9913_IO_UPDATE_Pin,
	AD9913_MRESET_GPIO_Port, AD9913_MRESET_Pin, AD9913_PWR_DWN_GPIO_Port,
	AD9913_PWR_DWN_Pin);

bool HW::Init() {
	if(!ad9913.Init()) {
		return false;
	}
	if(!si5332.Init()) {
		return false;
	}
	return true;
}

void HW::fillDeviceInfo(Protocol::DeviceInfo *info) {
	*info = HW::Info;
}
