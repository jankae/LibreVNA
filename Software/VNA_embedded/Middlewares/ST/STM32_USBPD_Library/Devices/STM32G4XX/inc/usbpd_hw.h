/**
  ******************************************************************************
  * @file    usbpd_hw.h
  * @author  MCD Application Team
  * @brief   This file contains interface hw control.
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

#ifndef USBPD_HW_H
#define USBPD_HW_H
/* Includes ------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Variable containing ADC conversions results */
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
UCPD_TypeDef *USBPD_HW_GetUSPDInstance(uint8_t PortNum);
DMA_Channel_TypeDef *USBPD_HW_Init_DMARxInstance(uint8_t PortNum);
void USBPD_HW_DeInit_DMARxInstance(uint8_t PortNum);
DMA_Channel_TypeDef *USBPD_HW_Init_DMATxInstance(uint8_t PortNum);
void USBPD_HW_DeInit_DMATxInstance(uint8_t PortNum);
uint32_t USBPD_HW_GetRpResistorValue(uint8_t Portnum);
void USBPD_HW_SetFRSSignalling(uint8_t Portnum, uint8_t cc);
#endif /* USBPD_BSP_HW_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

