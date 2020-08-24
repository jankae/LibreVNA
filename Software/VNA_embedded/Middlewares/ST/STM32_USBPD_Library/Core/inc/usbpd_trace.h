/**
  ******************************************************************************
  * @file    usbpd_trace.h
  * @author  MCD Application Team
  * @brief   This file contains the headers of usbpd_cad.h for Cable Attach-Detach
  *          controls.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef __USBPD_TRACE_H_
#define __USBPD_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_CORE
  * @{
  */

/** @addtogroup USBPD_CORE_TRACE
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup USBPD_CORE_TRACE_Exported_Functions USBPD CORE TRACE Exported Functions
  * @{
  */
/**
  * @brief  Initialize the TRACE module
  * @retval None
  */
void            USBPD_TRACE_Init(void);

/**
  * @brief  Add information in debug trace buffer
  * @param  Type    Trace Type based on @ref TRACE_EVENT
  * @param  PortNum Port number value
  * @param  Sop     SOP type
  * @param  Ptr     Pointer on the data to send
  * @param  Size    Size of the data to send
  * @retval None.
  */
void            USBPD_TRACE_Add(TRACE_EVENT Type, uint8_t PortNum, uint8_t Sop, uint8_t *Ptr, uint32_t Size);

/**
  * @brief  Process to send the data
  * @retval None.
  */
uint32_t USBPD_TRACE_TX_Process(void);

/**
  * @}
  */

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

#endif /* __USBPD_CAD_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
