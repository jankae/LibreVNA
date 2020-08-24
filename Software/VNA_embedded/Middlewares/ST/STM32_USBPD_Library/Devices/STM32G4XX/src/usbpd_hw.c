/**
  ******************************************************************************
  * @file    usbpd_phy_hw_if.c
  * @author  MCD Application Team
  * @brief   This file contains phy interface control functions.
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

#define USBPD_HW_C
/* Includes ------------------------------------------------------------------*/
#include "usbpd_devices_conf.h"
#include "usbpd_hw.h"

/* Private typedef -----------------------------------------------------------*/
/* Variable containing ADC conversions results */
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

UCPD_TypeDef *USBPD_HW_GetUSPDInstance(uint8_t PortNum)
{
  return UCPD_INSTANCE0;
}

DMA_Channel_TypeDef *USBPD_HW_Init_DMARxInstance(uint8_t PortNum)
{
  LL_DMA_InitTypeDef DMA_InitStruct;

  /* Initialise the DMA */
  LL_DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  DMA_InitStruct.Mode = LL_DMA_MODE_NORMAL;
  DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
  DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
  DMA_InitStruct.NbData = 0;
  DMA_InitStruct.MemoryOrM2MDstAddress = 0x0;
  DMA_InitStruct.PeriphOrM2MSrcAddress = 0x0;
  DMA_InitStruct.Priority = LL_DMA_PRIORITY_HIGH;

  /* Enable the clock */
  UCPDDMA_INSTANCE0_CLOCKENABLE_RX;

  /* Initialise the DMA */
  DMA_InitStruct.PeriphRequest = UCPDDMA_INSTANCE0_REQUEST_RX;

  (void)LL_DMA_Init(UCPDDMA_INSTANCE0_DMA_RX, UCPDDMA_INSTANCE0_LL_CHANNEL_RX, &DMA_InitStruct);

  return UCPDDMA_INSTANCE0_CHANNEL_RX;
}

void USBPD_HW_DeInit_DMARxInstance(uint8_t PortNum)
{
  (void)PortNum;
}

DMA_Channel_TypeDef *USBPD_HW_Init_DMATxInstance(uint8_t PortNum)
{
  LL_DMA_InitTypeDef DMA_InitStruct;

  /* Initialise the DMA */
  LL_DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStruct.Mode = LL_DMA_MODE_NORMAL;
  DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
  DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
  DMA_InitStruct.NbData = 0;
  DMA_InitStruct.MemoryOrM2MDstAddress = 0x0;
  DMA_InitStruct.PeriphOrM2MSrcAddress = 0x0;

  /* Enable the clock */
  UCPDDMA_INSTANCE0_CLOCKENABLE_TX;

  DMA_InitStruct.PeriphRequest = UCPDDMA_INSTANCE0_REQUEST_TX;
  DMA_InitStruct.Priority = LL_DMA_PRIORITY_MEDIUM;
  (void)LL_DMA_Init(UCPDDMA_INSTANCE0_DMA_TX, UCPDDMA_INSTANCE0_LL_CHANNEL_TX, &DMA_InitStruct);

  return UCPDDMA_INSTANCE0_CHANNEL_TX;
}

void USBPD_HW_DeInit_DMATxInstance(uint8_t PortNum)
{
  (void)PortNum;
}

uint32_t USBPD_HW_GetRpResistorValue(uint8_t PortNum)
{
  (void)PortNum;
  return LL_UCPD_RESISTOR_3_0A;
}

void USBPD_HW_SetFRSSignalling(uint8_t PortNum, uint8_t cc)
{
  (void)PortNum;
  /* Configure the GPIO with the AF corresponding to UCPD */
  if (1u == cc)
  {
    /* FRS_TX1 PA2 (CC1) */
    UCPDFRS_INSTANCE0_FRSCC1;
  }
  else
  {
    /* FRS_TX2 PB0 (CC2) */
    UCPDFRS_INSTANCE0_FRSCC2;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

