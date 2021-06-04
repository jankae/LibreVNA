/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32l4xx_hal.h"

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
#define STW_LE_Pin GPIO_PIN_4
#define STW_LE_GPIO_Port GPIOA
#define STW_HW_PD_Pin GPIO_PIN_0
#define STW_HW_PD_GPIO_Port GPIOB
#define STW_PD_RF1_Pin GPIO_PIN_1
#define STW_PD_RF1_GPIO_Port GPIOB
#define AD9913_PWR_DWN_Pin GPIO_PIN_10
#define AD9913_PWR_DWN_GPIO_Port GPIOB
#define AD9913_IO_UPDATE_Pin GPIO_PIN_11
#define AD9913_IO_UPDATE_GPIO_Port GPIOB
#define AD9913_CS_Pin GPIO_PIN_12
#define AD9913_CS_GPIO_Port GPIOB
#define SI5332_IN2_Pin GPIO_PIN_8
#define SI5332_IN2_GPIO_Port GPIOA
#define SI5332_IN1_Pin GPIO_PIN_15
#define SI5332_IN1_GPIO_Port GPIOA
#define SI5332_IN3_Pin GPIO_PIN_3
#define SI5332_IN3_GPIO_Port GPIOB
#define SI5332_IN4_Pin GPIO_PIN_4
#define SI5332_IN4_GPIO_Port GPIOB
#define SI5332_IN5_Pin GPIO_PIN_5
#define SI5332_IN5_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
