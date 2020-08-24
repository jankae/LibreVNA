/**
  ******************************************************************************
  * @file    usbpd_timersserver.h
  * @author  MCD Application Team
  * @brief   This file contains the headers of usbpd_timerserver.h.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __USBPD_TIMERSSERVER_H_
#define __USBPD_TIMERSSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_DEVICE
  * @{
  */

/** @addtogroup USBPD_DEVICE_TIMESERVER
  * @{
  */

/* Exported constants --------------------------------------------------------*/
typedef enum {
  TIM_PORT0_CRC,
  TIM_PORT0_RETRY,
  TIM_PORT1_CRC,
  TIM_PORT1_RETRY,
  TIM_MAX
}
TIM_identifier;

#define TIM_MAX_TIME 10000u /*time in us, means 10 ms */
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void     USBPD_TIM_Init(void);
void     USBPD_TIM_DeInit(void);
void     USBPD_TIM_Start(TIM_identifier Id, uint32_t TimeUs);
uint32_t USBPD_TIM_IsExpired(TIM_identifier Id);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBPD_TIMERSSERVER_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

