#include "HW_HAL.hpp"

Si5351C HWHAL::Si5351 = Si5351C(&hi2c2, 26000000);
MAX2871 HWHAL::Source = MAX2871(&hspi1, FPGA_CS_GPIO_Port, FPGA_CS_Pin, nullptr, 0, nullptr, 0, nullptr, 0, GPIOA, GPIO_PIN_6);
MAX2871 HWHAL::LO1 = MAX2871(&hspi1, FPGA_CS_GPIO_Port, FPGA_CS_Pin, nullptr, 0, nullptr, 0, nullptr, 0, GPIOA, GPIO_PIN_6);
extern SPI_HandleTypeDef hspi1;
Flash HWHAL::flash = Flash(&hspi1, FLASH_CS_GPIO_Port, FLASH_CS_Pin);
