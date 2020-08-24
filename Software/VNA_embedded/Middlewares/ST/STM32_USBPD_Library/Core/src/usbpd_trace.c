/**
  ******************************************************************************
  * @file    usbpd_trace.c
  * @author  MCD Application Team
  * @brief   This file contains trace control functions.
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

/* Includes ------------------------------------------------------------------*/
#if defined(_TRACE)
#define USBPD_TRACE_C

#include "stdint.h"
#include "usbpd_def.h"
#include "usbpd_core.h"
#include "usbpd_trace.h"
#ifdef _TRACE
#include "tracer_emb.h"
#endif

/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_CORE
  * @{
  */

/** @addtogroup USBPD_CORE_TRACE
  * @{
  */

/* Private enums -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup USBPD_CORE_TRACE_Private_Defines USBPD TRACE Private Defines
  * @{
  */

#define TRACE_SIZE_HEADER_TRACE   9u      /* Type + Time x 2 + PortNum + Sop + Size */

#define TRACE_PORT_BIT_POSITION   5u      /* Bit position of port number in TAG id */

#define TLV_SOF                   (uint8_t)0xFDu
#define TLV_EOF                   (uint8_t)0xA5u
#define TLV_SIZE_MAX              256u
#define TLV_HEADER_SIZE           3u /* Size of TLV header (TAG(1) + LENGTH(2)  */
#define TLV_SOF_SIZE              4u /* TLV_SOF * 4                             */
#define TLV_EOF_SIZE              4u /* TLV_EOF * 4                             */

#define DEBUG_STACK_MESSAGE       0x12u
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/** @defgroup USBPD_CORE_TRACE_Private_Macros USBPD TRACE Private Macros
  * @{
  */
#define __TRACE_SET_TAG_ID(_PORT_, _TAG_)  (((_PORT_) << TRACE_PORT_BIT_POSITION) | (_TAG_))

#define TRACER_EMB_WRITE_DATA(_POSITION_,_DATA_)  TRACER_EMB_WriteData((_POSITION_),(_DATA_));\
                                                  (_POSITION_) = ((_POSITION_) + 1u);

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup USBPD_CORE_TRACE_Private_Variables USBPD TRACE Private Variables
  * @{
  */
extern uint32_t HAL_GetTick(void);

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/


/** @addtogroup USBPD_CORE_TRACE_Exported_Functions
  * @{
  */
void USBPD_TRACE_Init(void)
{
  /* initialize tracer module */
  TRACER_EMB_Init();

  /* Initialize PE trace */
  USBPD_PE_SetTrace(USBPD_TRACE_Add, 3u);
}

void USBPD_TRACE_Add(TRACE_EVENT Type, uint8_t PortNum, uint8_t Sop, uint8_t *Ptr, uint32_t Size)
{
  uint32_t _time;
  int32_t _writepos;
  uint16_t _writepos2;
  uint8_t *data_to_write;
  uint32_t index;
  uint32_t total_size;
  
#if !defined(_TRACE)
  /* Do not send debug traces */
  return;
#endif
  
  TRACER_EMB_Lock();
  
  /* Data are encapsulate inside a TLV string*/
  /* Allocate buffer Size */
  total_size = Size + TRACE_SIZE_HEADER_TRACE + TLV_HEADER_SIZE + TLV_SOF_SIZE + TLV_EOF_SIZE;
  _writepos = TRACER_EMB_AllocateBufer(total_size);
  
  /* Check allocation */
  if (_writepos  != -1)
  {
    _writepos2 = (uint16_t)_writepos;
    data_to_write = Ptr;
    
    /* Copy SOF bytes */
    for(index = 0u; index < TLV_SOF_SIZE; index++)
    {
      TRACER_EMB_WRITE_DATA(_writepos2, TLV_SOF);
    }
    /* Copy the TAG */
    TRACER_EMB_WRITE_DATA(_writepos2, __TRACE_SET_TAG_ID((PortNum + 1u), DEBUG_STACK_MESSAGE));
    /* Copy the LENGTH */
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)((total_size - TLV_HEADER_SIZE - TLV_SOF_SIZE - TLV_EOF_SIZE) >> 8u));
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)(total_size - TLV_HEADER_SIZE - TLV_SOF_SIZE - TLV_EOF_SIZE));
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)Type);
    
    _time = HAL_GetTick();
    
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)_time);
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)(_time >> 8u));
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)(_time >> 16u));
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)(_time >> 24u));
    
    TRACER_EMB_WRITE_DATA(_writepos2, PortNum);
    TRACER_EMB_WRITE_DATA(_writepos2, Sop);
    
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)(Size >> 8u));
    TRACER_EMB_WRITE_DATA(_writepos2, (uint8_t)Size);
    
    /* initialize the Ptr for Read/Write */
    for (index = 0u; index < Size; index++)
    {
      TRACER_EMB_WRITE_DATA(_writepos2, data_to_write[index]);
    }
    
    /* Copy EOF bytes */
    for(index = 0u; index < TLV_EOF_SIZE; index++)
    {
      TRACER_EMB_WRITE_DATA(_writepos2, TLV_EOF);
    }
  }
  
  TRACER_EMB_UnLock();
}

uint32_t USBPD_TRACE_TX_Process(void)
{
  return TRACER_EMB_TX_Process();
}

/**
  * @}
  */

/** @defgroup USBPD_CORE_TRACE_Private_Functions USBPD TRACE Private Functions
  * @{
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

/**
  * @}
  */
#endif /* _TRACE */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

