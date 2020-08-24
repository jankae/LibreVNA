/**
  ******************************************************************************
  * @file    usbpd_cad_hw_if.h
  * @author  MCD Application Team
  * @brief   This file contains the headers of usbpd_cad.h for Cable Attach-Detach
  *          controls.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef __USBPD_CAD_HW_IF_H_
#define __USBPD_CAD_HW_IF_H_

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

/** @addtogroup USBPD_DEVICE_CAD_HW_IF
  * @{
  */


/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup USBPD_DEVICE_CAD_HW_IF_Exported_Functions
  * @{
  */
void CAD_Init(uint8_t PortNum, USBPD_SettingsTypeDef *Settings, USBPD_ParamsTypeDef *Params, void (*PtrWakeUp)(void));
uint32_t CAD_StateMachine(uint8_t PortNum, USBPD_CAD_EVENT *Event, CCxPin_TypeDef *CCXX);
void CAD_Enter_ErrorRecovery(uint8_t PortNum);
uint32_t CAD_Set_ResistorRp(uint8_t PortNum, CAD_RP_Source_Current_Adv_Typedef RpValue);

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

#endif /* __USBPD_CAD_HW_IF_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
