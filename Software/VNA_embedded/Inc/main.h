/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

#include "stm32g4xx_ll_ucpd.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_dma.h"

#include "stm32g4xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FPGA_INIT_B_Pin GPIO_PIN_1
#define FPGA_INIT_B_GPIO_Port GPIOF
#define FPGA_AUX1_Pin GPIO_PIN_1
#define FPGA_AUX1_GPIO_Port GPIOA
#define FPGA_AUX3_Pin GPIO_PIN_2
#define FPGA_AUX3_GPIO_Port GPIOA
#define FPGA_AUX2_Pin GPIO_PIN_3
#define FPGA_AUX2_GPIO_Port GPIOA
#define FPGA_CS_Pin GPIO_PIN_4
#define FPGA_CS_GPIO_Port GPIOA
#define FLASH_CS_Pin GPIO_PIN_0
#define FLASH_CS_GPIO_Port GPIOB
#define FPGA_INTR_Pin GPIO_PIN_1
#define FPGA_INTR_GPIO_Port GPIOB
#define FPGA_PROGRAM_B_Pin GPIO_PIN_2
#define FPGA_PROGRAM_B_GPIO_Port GPIOB
#define EN_6V_Pin GPIO_PIN_12
#define EN_6V_GPIO_Port GPIOB
#define FPGA_RESET_Pin GPIO_PIN_5
#define FPGA_RESET_GPIO_Port GPIOB
#define FPGA_DONE_Pin GPIO_PIN_9
#define FPGA_DONE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
