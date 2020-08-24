/**
  ******************************************************************************
  * @file    usbpd_cad_hw_if.c
  * @author  MCD Application Team
  * @brief   This file contains power hardware interface cad functions.
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

/* Includes ------------------------------------------------------------------*/
#define USBPD_CAD_HW_IF_C
#include "usbpd_devices_conf.h"
#include "usbpd_hw.h"
#include "usbpd_def.h"
#include "usbpd_cad_hw_if.h"
#include "usbpd_hw_if.h"
#include "usbpd_core.h"
#include "usbpd_trace.h"

/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_DEVICE
  * @{
  */

/** @addtogroup USBPD_DEVICE_CAD_HW_IF
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/**
  * @brief CAD State value @ref USBPD_DEVICE_CAD_HW_IF
  * @{
  */
#define USBPD_CAD_STATE_RESET                0u  /*!< USBPD CAD State Reset                              */
#define USBPD_CAD_STATE_DETACHED             1u  /*!< USBPD CAD State No cable detected                  */
#define USBPD_CAD_STATE_ATTACHED_WAIT        2u  /*!< USBPD CAD State Port partner detected              */
#define USBPD_CAD_STATE_ATTACHED             3u  /*!< USBPD CAD State Port partner attached              */
#define USBPD_CAD_STATE_EMC                  4u  /*!< USBPD CAD State Electronically Marked Cable detected */
#define USBPD_CAD_STATE_ATTEMC               5u  /*!< USBPD CAD State Port Partner detected throug EMC   */
#define USBPD_CAD_STATE_ACCESSORY            6u  /*!< USBPD CAD State Accessory detected                 */
#define USBPD_CAD_STATE_DEBUG                7u  /*!< USBPD CAD State Debug detected                     */
#define USBPD_CAD_STATE_SWITCH_TO_SRC        8u  /*!< USBPD CAD State switch to Source                   */
#define USBPD_CAD_STATE_SWITCH_TO_SNK        9u  /*!< USBPD CAD State switch to Sink                     */
#define USBPD_CAD_STATE_UNKNOW               10u /*!< USBPD CAD State unknow                             */
/*USBPD_CAD_STATE_ATTACHED_LEGACY      11u*/ /*!< USBPD CAD State Port partner attached to legacy cable */
#define USBPD_CAD_STATE_ERRORRECOVERY        12u /*!< USBPD CAD State error recovery                     */
#define USBPD_CAD_STATE_ERRORRECOVERY_EXIT   13u /*!< USBPD CAD State to exit error recovery             */

typedef uint32_t USBPD_CAD_STATE;
/**
  * @}
  */

/**
  * @brief USB PD CC lines HW condition
  */
#define HW_Detachment                          0x00UL    /*!< Nothing attached   */
#define HW_Attachment                          0x01UL    /*!< Sink attached   */
#define HW_PwrCable_NoSink_Attachment          0x02UL    /*!< Powered cable without Sink attached   */
#define HW_PwrCable_Sink_Attachment            0x03UL    /*!< Powered cable with Sink or VCONN-powered Accessory attached   */
#define HW_Debug_Attachment                    0x04UL    /*!< Debug Accessory Mode attached   */
#define HW_AudioAdapter_Attachment             0x05UL    /*!< Audio Adapter Accessory Mode attached   */

typedef uint32_t CAD_HW_Condition_TypeDef;

/**
  * @brief CAD State value @ref USBPD_DEVICE_CAD_HW_IF
  * @{
  */
typedef struct
{
  USBPD_CAD_STATE cstate                                    : 4; /* current state  */
  USBPD_CAD_STATE pstate                                    : 4; /* previous state */
  CCxPin_TypeDef  cc                                        : 2;
  CAD_HW_Condition_TypeDef    CurrentHWcondition            : 3;
  CAD_SNK_Source_Current_Adv_Typedef SNK_Source_Current_Adv : 2;
  uint32_t CAD_tDebounce_flag                               : 1;
  uint32_t CAD_ErrorRecoveryflag                            : 1;
#if defined(_DRP) || defined(_SRC)
  uint32_t CAD_ResistorUpdateflag                           : 1;
  uint32_t reserved                                         : 12;
#else
  uint32_t reserved                                         : 13;
#endif /* _SRC || _DRP */

#if defined(_DRP)
  uint32_t CAD_tToogle_start;
#endif /* _DRP */
  uint32_t CAD_tDebounce_start, CAD_tDebounce;      /* Variable used for attach or detach debounce timers */
} CAD_HW_HandleTypeDef;
/**
  * @}
  */

/* Private define ------------------------------------------------------------*/
#define CAD_TPDDEBOUCE_THRESHOLD         12u          /**< tPDDebounce threshold = 20ms                       */
#define CAD_TCCDEBOUCE_THRESHOLD         100u         /**< tCCDebounce threshold = 100ms                      */
#define CAD_TSRCDISCONNECT_THRESHOLD     2u           /**< tSRCDisconnect detach threshold between 0 to 20ms  */
#define CAD_INFINITE_TIME                0xFFFFFFFFu  /**< infinite time to wait a new interrupt event        */
#define CAD_TERROR_RECOVERY_TIME         26u          /**< tErrorRecovery min 25ms                            */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* handle to manage the detection state machine */
static CAD_HW_HandleTypeDef CAD_HW_Handles[USBPD_PORT_COUNT];

/* Private function prototypes -----------------------------------------------*/
/** @defgroup USBPD_DEVICE_CAD_HW_IF_Private_Functions USBPD DEVICE_CAD HW IF Private Functions
  * @{
  */
static void CAD_Check_HW(uint8_t PortNum);
#if defined(_DRP) || defined(_SNK)
static uint8_t CAD_Check_VBus(uint8_t PortNum);
#endif /* _DRP || _SNK */
static uint32_t ManageStateDetached(uint8_t PortNum);
static uint32_t ManageStateAttachedWait(uint8_t PortNum, USBPD_CAD_EVENT *pEvent, CCxPin_TypeDef *pCCXX);
static uint32_t ManageStateAttached(uint8_t PortNum, USBPD_CAD_EVENT *pEvent, CCxPin_TypeDef *pCCXX);
static uint32_t ManageStateEMC(uint8_t PortNum, USBPD_CAD_EVENT *pEvent, CCxPin_TypeDef *pCCXX);
/**
  * @}
  */

/** @defgroup USBPD_DEVICE_CAD_HW_IF_Exported_Functions USBPD DEVICE_CAD HW IF Exported Functions
  * @{
  */

/**
  * @brief  function to initialize the cable detection state machine
  * @param  PortNum       port
  * @param  pSettings     Pointer on PD settings based on @ref USBPD_SettingsTypeDef
  * @param  pParams       Pointer on PD params based on @ref USBPD_ParamsTypeDef
  * @param  WakeUp        Wake-up callback function used for waking up CAD
  * @retval None
  */
void CAD_Init(uint8_t PortNum, USBPD_SettingsTypeDef *pSettings, USBPD_ParamsTypeDef *pParams,  void (*WakeUp)(void))
{
  CAD_HW_HandleTypeDef *_handle = &CAD_HW_Handles[PortNum];
  LL_UCPD_InitTypeDef settings;

  Ports[PortNum].params = pParams;
#ifndef _VALID_RP
#define _VALID_RP vRp_3_0A
#endif
  Ports[PortNum].params->RpResistor = _VALID_RP;
  Ports[PortNum].settings = pSettings;
  _handle->cstate = USBPD_CAD_STATE_RESET;
  _handle->cc = CCNONE;
  _handle->CurrentHWcondition = HW_Detachment;
  _handle->SNK_Source_Current_Adv = vRd_Undefined;

  Ports[PortNum].USBPD_CAD_WakeUp = WakeUp;

  /* Initialise the USBPD_IP */
  Ports[PortNum].husbpd = USBPD_HW_GetUSPDInstance(PortNum);

  /* initialise usbpd */
  LL_UCPD_StructInit(&settings);
  (void)LL_UCPD_Init(Ports[PortNum].husbpd, &settings);
  LL_UCPD_SetRxOrderSet(Ports[PortNum].husbpd, LL_UCPD_ORDERSET_SOP | LL_UCPD_ORDERSET_SOP1 | LL_UCPD_ORDERSET_SOP2 | LL_UCPD_ORDERSET_CABLERST | LL_UCPD_ORDERSET_HARDRST);
#ifdef _LOW_POWER
  LL_UCPD_WakeUpEnable(Ports[PortNum].husbpd);
#endif

  LL_PWR_DisableUSBDeadBattery(); /* PWR->CR3 |= (1 << 14); */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB); /* GPIOB enable RCC->AHB2ENR |= 2; */
#if defined(MB1397) /* in case of eval G4 */
  /* Not needed on MB1397 */
#elif defined (USE_UNIBOARD) || defined(MB1428) || defined(MB1367) /* in case of Nucleo G4 */
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_ANALOG); /* PB4 mode = GP analog => CC2 */
#else
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT); /* PB4 mode = GP output => CC2 */
#endif
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_4, LL_GPIO_PULL_NO);

  /* Triming */
  /* Ports[PortNum].husbpd->RESERVED0 = 0x1D0F1D0F;*/

  /* Enable USBPD IP */
  LL_UCPD_Enable(Ports[PortNum].husbpd);

  /* Controls whether for pull-ups and pull-downs controls related to ANAMODE and ANASUBMODE should be applied to CC1 and CC2 analog PHYs */
  /* Should be done when UCPDEN is 1 */
  LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_CC1CC2);

#if defined(_SRC) || defined(_DRP)
  /* Intialise usbpd interrupt */
  if (USBPD_PORTPOWERROLE_SRC == Ports[PortNum].params->PE_PowerRole)
  {
    USBPDM1_AssertRp(PortNum);
  }
  else
#endif /* _SRC || _DRP */
  {
    USBPDM1_AssertRd(PortNum);
  }

}

/**
  * @brief  funtion to force CAD state machine into error recovery state
  * @param  PortNum Index of current used port
  * @retval None
  */
void CAD_Enter_ErrorRecovery(uint8_t PortNum)
{
  /* remove the ucpd resistor */
  USBPDM1_EnterErrorRecovery(PortNum);
  /* set the error recovery flag to allow the stack to switch into errorRecovery Flag */
  CAD_HW_Handles[PortNum].CAD_ErrorRecoveryflag = USBPD_TRUE;
  Ports[PortNum].USBPD_CAD_WakeUp();
}

/**
  * @brief  function to force the value of the RP resistor
  * @note   Must be called only if you want change the settings value
  * @param  PortNum Index of current used port
  * @param  RpValue RP value to set in devices based on @ref CAD_RP_Source_Current_Adv_Typedef
  * @retval 0 success else error
  */
uint32_t CAD_Set_ResistorRp(uint8_t PortNum, CAD_RP_Source_Current_Adv_Typedef RpValue)
{
#if defined(_DRP) || defined(_SRC)
  /* update the information about the default resitor value presented in detach mode */
  Ports[PortNum].params->RpResistor = RpValue;

  /* inform state machine about a resistor update */
  CAD_HW_Handles[PortNum].CAD_ResistorUpdateflag = USBPD_TRUE;
  Ports[PortNum].USBPD_CAD_WakeUp();
#endif /* _SRC || _DRP */
  return 0;
}

/**
  * @brief  CAD State machine
  * @param  PortNum Port
  * @param  pEvent  Pointer on CAD event based on @ref USBPD_CAD_EVENT
  * @param  pCCXX   Pointer on CC Pin based on @ref CCxPin_TypeDef
  * @retval Timeout value
  */
uint32_t CAD_StateMachine(uint8_t PortNum, USBPD_CAD_EVENT *pEvent, CCxPin_TypeDef *pCCXX)
{
  CAD_HW_HandleTypeDef *_handle = &CAD_HW_Handles[PortNum];
  uint32_t _timing = 2;

  /* set by default event to none */
  *pEvent = USBPD_CAD_EVENT_NONE;

  if (USBPD_TRUE == Ports[PortNum].params->PE_SwapOngoing)
  {
    return _timing;
  }

  if (_handle->CAD_ErrorRecoveryflag == USBPD_TRUE)
  {
    /* Force the state error recovery */
    _handle->CAD_ErrorRecoveryflag = USBPD_FALSE;
    _handle->cstate = USBPD_CAD_STATE_ERRORRECOVERY;
#if defined(__DEBUG_CAD) && defined(_TRACE)
    USBPD_TRACE_Add(USBPD_TRACE_CAD_LOW, PortNum, (uint8_t)_handle->cstate, NULL, 0);
#endif
  }

  /*Check CAD STATE*/
  switch (_handle->cstate)
  {
    case USBPD_CAD_STATE_SWITCH_TO_SRC:
    case USBPD_CAD_STATE_SWITCH_TO_SNK:
    {
#if defined(_DRP)
      if (Ports[PortNum].settings->CAD_RoleToggle == USBPD_TRUE)
      {
        if (USBPD_CAD_STATE_SWITCH_TO_SRC == _handle->cstate)
        {
          USBPDM1_AssertRp(PortNum);
          Ports[PortNum].params->PE_PowerRole = USBPD_PORTPOWERROLE_SRC;
          _timing = Ports[PortNum].settings->CAD_SRCToggleTime;
        }
        if (USBPD_CAD_STATE_SWITCH_TO_SNK == _handle->cstate)
        {
          USBPDM1_AssertRd(PortNum);
          Ports[PortNum].params->PE_PowerRole = USBPD_PORTPOWERROLE_SNK;
          _timing = Ports[PortNum].settings->CAD_SNKToggleTime;
        }
        _handle->CAD_tToogle_start = HAL_GetTick();
      }
#endif /* _DRP */
      /* execute detach to present the new resistor according the configuration selected */
      HW_SignalDetachment(PortNum);
      _handle->cstate = USBPD_CAD_STATE_DETACHED;
      break;
    }

    case USBPD_CAD_STATE_RESET:
    {
      /* on the first call of the usbpd state machine the interrupt and CC pin are enabled */
      LL_UCPD_EnableIT_TypeCEventCC2(Ports[PortNum].husbpd);
      LL_UCPD_EnableIT_TypeCEventCC1(Ports[PortNum].husbpd);
#if defined(_DRP)
      if (Ports[PortNum].settings->CAD_RoleToggle == USBPD_TRUE)
      {
        _handle->CAD_tToogle_start = HAL_GetTick();
      }
#endif /* _DRP */
      if (0 == PortNum)
      {
        UCPD_INSTANCE0_ENABLEIRQ;
      }
      _handle->cstate = USBPD_CAD_STATE_DETACHED;
      break;
    }

    case USBPD_CAD_STATE_DETACHED:
      _timing = ManageStateDetached(PortNum);
      break;

      /*CAD STATE ATTACHED WAIT*/
    case USBPD_CAD_STATE_ATTACHED_WAIT:
      _timing = ManageStateAttachedWait(PortNum, pEvent, pCCXX);
      break;

      /* CAD ELECTRONIC CABLE ATTACHED */
    case USBPD_CAD_STATE_EMC :
      _timing = ManageStateEMC(PortNum, pEvent, pCCXX);
      break;

      /*CAD electronic cable with Sink ATTACHED*/
    case USBPD_CAD_STATE_ATTEMC:
    case USBPD_CAD_STATE_ATTACHED:
      _timing = ManageStateAttached(PortNum, pEvent, pCCXX);
      break;

      /*CAD STATE AUDIO ACCESSORY ATTACHED*/
    case USBPD_CAD_STATE_ACCESSORY:
    {
      _timing = CAD_INFINITE_TIME;
      CAD_Check_HW(PortNum);
      if (_handle->CurrentHWcondition != HW_AudioAdapter_Attachment)
      {
        HW_SignalDetachment(PortNum);
        _handle->cstate = USBPD_CAD_STATE_SWITCH_TO_SNK;
        *pCCXX = CCNONE;
        _handle->cc = CCNONE;
        *pEvent = USBPD_CAD_EVENT_DETACHED;
      }
      break;
    }

    /*CAD STATE DEBUG ACCESSORY MODE ATTACHED*/
    case USBPD_CAD_STATE_DEBUG:
    {
      _timing = CAD_INFINITE_TIME;
      CAD_Check_HW(PortNum);
      if (_handle->CurrentHWcondition != HW_Debug_Attachment)
      {
        HW_SignalDetachment(PortNum);
        _handle->cstate = USBPD_CAD_STATE_SWITCH_TO_SNK;
        *pCCXX = CCNONE;
        _handle->cc = CCNONE;
        *pEvent = USBPD_CAD_EVENT_DETACHED;
      }
      break;
    }

    case USBPD_CAD_STATE_ERRORRECOVERY :
    {
      /* Remove the resistor */
      /* Enter recovery = Switch to SRC with no resistor */
      USBPDM1_EnterErrorRecovery(PortNum);

      /* forward detach event to DPM */
      *pCCXX = CCNONE;
      _handle->cc = CCNONE;
      *pEvent = USBPD_CAD_EVENT_DETACHED;

      /* start tErrorRecovery timeout */
      _handle->CAD_tDebounce_start = HAL_GetTick();
      _timing = CAD_TERROR_RECOVERY_TIME;
      _handle->cstate = USBPD_CAD_STATE_ERRORRECOVERY_EXIT;
      break;
    }

    case USBPD_CAD_STATE_ERRORRECOVERY_EXIT :
    {
      if ((HAL_GetTick() - _handle->CAD_tDebounce_start) >  CAD_TERROR_RECOVERY_TIME)
      {
        /* reconfigure the port
        port source  to src
        port snk     to snk
        port drp     to src   */
#if defined(_DRP) || defined(_SRC)
        if ((USBPD_PORTPOWERROLE_SRC == Ports[PortNum].settings->PE_DefaultRole) || (Ports[PortNum].settings->CAD_RoleToggle == USBPD_TRUE))
        {
          USBPDM1_AssertRp(PortNum);
          Ports[PortNum].params->PE_PowerRole = USBPD_PORTPOWERROLE_SRC;
        }
#if defined(_DRP)
        else
#endif /* _DRP */
#endif /* _DRP || _SRC */
#if defined(_DRP) || defined(_SNK)
        {
          USBPDM1_AssertRd(PortNum);
          Ports[PortNum].params->PE_PowerRole = USBPD_PORTPOWERROLE_SNK;
        }
#endif /* _DRP || _SNK */
        /* switch to state detach */
        _handle->CAD_tDebounce_start = HAL_GetTick();
        _handle->cstate = USBPD_CAD_STATE_DETACHED;
      }
      break;
    }

    default :
      break;
  }

#if defined(__DEBUG_CAD) && defined(_TRACE)
  if (_handle->cstate != _handle->pstate)
  {
    _handle->pstate = _handle->cstate;
    USBPD_TRACE_Add(USBPD_TRACE_CAD_LOW, PortNum, (uint8_t)_handle->cstate, NULL, 0);
  }
#endif /* defined(__DEBUG_CAD) && defined(_TRACE) */

#if !defined(_RTOS)
  /* Delay added for CC detection issue on NRTOS version */
  HAL_Delay(1);
#endif /*_RTOS*/
  return _timing;
}

/**
  * @}
  */

/** @addtogroup USBPD_DEVICE_CAD_HW_IF_Private_Functions
  * @{
  */

#if defined(_DRP) || defined(_SNK)
/**
  * @brief  Check if VBus is present or not
  * @param  PortNum  port
  * @retval Return 1 is VBUS is present (0 otherwise)
  */
uint8_t CAD_Check_VBus(uint8_t PortNum)
{
  return (HW_IF_PWR_GetVoltage(PortNum) > BSP_PWR_HIGH_VBUS_THRESHOLD) ? USBPD_TRUE : USBPD_FALSE;
}
#endif /* _DRP || _SNK */

/**
  * @brief  Check CCx HW condition
  * @param  PortNum                     port
  * @retval none
  */
void CAD_Check_HW(uint8_t PortNum)
{
  uint32_t CC1_value, CC2_value;
#if !defined(_RTOS)
  uint32_t CC1_value_temp, CC2_value_temp;
#endif /* !_RTOS */
  CAD_HW_HandleTypeDef *_handle = &CAD_HW_Handles[PortNum];
  CCxPin_TypeDef _CCx = CCNONE;
  /*
  ----------------------------------------------------------------------------
  | ANAMODE   |  ANASUBMODE[1:0]  |  Notes      |  TYPEC_VSTATE_CCx[1:0]      |
  |           |                   |             |  00  |  01  |  10  |  11    |
  ----------------------------------------------------------------------------
  | 0: Source | 00: Disabled      |Disabled N/A |         NA                  |
  |           | 01: Default USB Rp|             |vRaDef|vRdDef|vOPENDef|      |
  |           | 10: 1.5A Rp       |             |vRa1.5|vRd1.5|vOPEN1.5| NA   |
  |           | 11: 3.0A Rp       |             |vRa3.0|vRd3.0|vOPEN3.0| NA   |
  -----------------------------------------------------------------------------
  | 1: Sink   |                   |             |xx vRa|vRdUSB| vRd1.5 |vRd3.0|
  -----------------------------------------------------------------------------
  */
  CC1_value = Ports[PortNum].PIN_CC1;
  CC2_value = Ports[PortNum].PIN_CC2;

#if !defined(_RTOS)
  /* Workaround linked to issue with Ellisys test TD.PC.E5
     - it seems that in NRTOS version, we detect a glitch during DRP transition SNK to SRC */
  CC1_value_temp = Ports[PortNum].husbpd->SR & UCPD_SR_TYPEC_VSTATE_CC1; /*Ports[PortNum].PIN_CC1;*/
  CC2_value_temp = Ports[PortNum].husbpd->SR & UCPD_SR_TYPEC_VSTATE_CC2; /*Ports[PortNum].PIN_CC2;*/
  if ((CC1_value_temp != CC1_value) || (CC2_value_temp != CC2_value))
  {
    _handle->cc = _CCx;
    return;
  }
#endif /* !_RTOS */
  
  _handle->CurrentHWcondition     = HW_Detachment;
  _handle->SNK_Source_Current_Adv = vRd_Undefined;

#if defined(_DRP) || defined(_SRC)
  if (USBPD_PORTPOWERROLE_SRC == Ports[PortNum].params->PE_PowerRole)
  {
    uint8_t cc2_index = (uint8_t)(CC2_value >> UCPD_SR_TYPEC_VSTATE_CC2_Pos);

    switch (CC1_value)
    {
      case LL_UCPD_SRC_CC1_VRA :
      {
        CAD_HW_Condition_TypeDef _tab_hw1[] =
        {HW_AudioAdapter_Attachment, HW_PwrCable_Sink_Attachment, HW_PwrCable_NoSink_Attachment};
        _handle->CurrentHWcondition = _tab_hw1[cc2_index];
        _CCx = CC2;
        break;
      }
      case LL_UCPD_SRC_CC1_VRD :
      {
        CAD_HW_Condition_TypeDef _tab_hw2[] =
        {HW_PwrCable_Sink_Attachment, HW_Debug_Attachment, HW_Attachment};
        _handle->CurrentHWcondition = _tab_hw2[cc2_index];
        _CCx = CC1;
        break;
      }
      case LL_UCPD_SRC_CC1_OPEN:
      {
        CAD_HW_Condition_TypeDef _tab_hw3[] =
        {HW_PwrCable_NoSink_Attachment, HW_Attachment, HW_Detachment};
        _handle->CurrentHWcondition = _tab_hw3[cc2_index];
        if (HW_Detachment != _handle->CurrentHWcondition)
        {
          _CCx = CC2;
        }
        break;
      }
      default:
        break;
    }
  }
#if defined(_DRP)
  else /* USBPD_PORTPOWERROLE_SNK */
#endif /* _DRP */
#endif /*_DRP || _SRC */
#if defined(_DRP) || defined(_SNK)
  {
    if ((CC1_value != LL_UCPD_SNK_CC1_VOPEN) && (CC2_value == LL_UCPD_SNK_CC2_VOPEN))
    {
      _handle->CurrentHWcondition = HW_Attachment;
      _CCx = CC1;
      switch (CC1_value)
      {
        case LL_UCPD_SNK_CC1_VRP:
          _handle->SNK_Source_Current_Adv = vRd_USB;
          break;
        case LL_UCPD_SNK_CC1_VRP15A:
          _handle->SNK_Source_Current_Adv = vRd_1_5A;
          break;
        case LL_UCPD_SNK_CC1_VRP30A:
          _handle->SNK_Source_Current_Adv = vRd_3_0A;
          break;
        default:
          break;
      }
    }
    if ((CC2_value != LL_UCPD_SNK_CC2_VOPEN) && (CC1_value == LL_UCPD_SNK_CC1_VOPEN))
    {
      _handle->CurrentHWcondition = HW_Attachment;
      _CCx = CC2;
      switch (CC2_value)
      {
        case LL_UCPD_SNK_CC2_VRP:
          _handle->SNK_Source_Current_Adv = vRd_USB;
          break;
        case LL_UCPD_SNK_CC2_VRP15A:
          _handle->SNK_Source_Current_Adv = vRd_1_5A;
          break;
        case LL_UCPD_SNK_CC2_VRP30A:
          _handle->SNK_Source_Current_Adv = vRd_3_0A;
          break;
        default:
          break;
      }
    }
  }
#endif /*_DRP || _SNK */

  _handle->cc = _CCx;
}

static uint32_t ManageStateDetached(uint8_t PortNum)
{
  uint32_t _timing = 2;
  CAD_HW_HandleTypeDef *_handle = &CAD_HW_Handles[PortNum];

#if defined(_DRP) || defined(_SRC)
  if ((_handle->CAD_ResistorUpdateflag == USBPD_TRUE) && (USBPD_PORTPOWERROLE_SRC == Ports[PortNum].params->PE_PowerRole))
  {
    /* update the reistor value */
    USBPDM1_AssertRp(PortNum);
    _handle->CAD_ResistorUpdateflag = USBPD_FALSE;

    /* let time to internal state machine update */
    HAL_Delay(1);
  }
#endif /* _DRP || _SRC */

  CAD_Check_HW(PortNum);
  /* Change the status on the basis of the HW event given by CAD_Check_HW() */
  if (_handle->CurrentHWcondition == HW_Detachment)
  {
#if defined(_DRP)
    /* check if role switch must be perform and set the correct sleep time allowed */
    if (USBPD_TRUE == Ports[PortNum].settings->CAD_RoleToggle)
    {
      switch (Ports[PortNum].params->PE_PowerRole)
      {
        case USBPD_PORTPOWERROLE_SRC :
          if ((HAL_GetTick() - _handle->CAD_tToogle_start) > Ports[PortNum].settings->CAD_SRCToggleTime)
          {
            _handle->CAD_tToogle_start = HAL_GetTick();
            Ports[PortNum].params->PE_PowerRole = USBPD_PORTPOWERROLE_SNK;
            _timing = Ports[PortNum].settings->CAD_SNKToggleTime;
            USBPDM1_AssertRd(PortNum);
          }
          break;
        case USBPD_PORTPOWERROLE_SNK :
          if ((HAL_GetTick() - _handle->CAD_tToogle_start) > Ports[PortNum].settings->CAD_SNKToggleTime)
          {
            _handle->CAD_tToogle_start = HAL_GetTick();
            Ports[PortNum].params->PE_PowerRole = USBPD_PORTPOWERROLE_SRC;
            _timing = Ports[PortNum].settings->CAD_SRCToggleTime;
            USBPDM1_AssertRp(PortNum);
          }
          break;
        default:
          break;
      }
    }
    else
#endif /* _DRP */
    {
      _timing = CAD_INFINITE_TIME;
    }
  }
  else
  {
    if (_handle->CurrentHWcondition == HW_PwrCable_NoSink_Attachment)
    {
      _handle->cstate = USBPD_CAD_STATE_EMC;
    }
    else
    {
      BSP_PWR_VBUSInit(PortNum);
      _handle->cstate = USBPD_CAD_STATE_ATTACHED_WAIT;
      /* Get the time of this event */
      _handle->CAD_tDebounce_start = HAL_GetTick();
      _timing = CAD_TCCDEBOUCE_THRESHOLD + 1u;
    }
  }
  return _timing;
}

static uint32_t ManageStateAttachedWait(uint8_t PortNum, USBPD_CAD_EVENT *pEvent, CCxPin_TypeDef *pCCXX)
{
  uint32_t _timing = 2;
  CAD_HW_HandleTypeDef *_handle = &CAD_HW_Handles[PortNum];

  /* Evaluate elapsed time in Attach_Wait state */
  _handle->CAD_tDebounce = HAL_GetTick() - _handle->CAD_tDebounce_start;
  CAD_Check_HW(PortNum);

  if ((_handle->CurrentHWcondition != HW_Detachment) && (_handle->CurrentHWcondition != HW_PwrCable_NoSink_Attachment))
  {
#if defined(_DRP) || defined(_SRC)
    if ((BSP_PWR_VBUSGetVoltage(PortNum) > BSP_PWR_LOW_VBUS_THRESHOLD)
        && (USBPD_PORTPOWERROLE_SRC == Ports[PortNum].params->PE_PowerRole))
    {
      /* reset the timing because VBUS threshold not yet reached */
      _handle->CAD_tDebounce_start = HAL_GetTick();
      return CAD_TCCDEBOUCE_THRESHOLD;
    }
#endif /* _DRP || _SRC */

    /* Check tCCDebounce */
    if (_handle->CAD_tDebounce > CAD_TCCDEBOUCE_THRESHOLD)
    {
#if defined(_DRP) || defined(_SRC)
      /* if tCCDebounce has expired state must be changed*/
      if (USBPD_PORTPOWERROLE_SRC == Ports[PortNum].params->PE_PowerRole)
      {
        switch (_handle->CurrentHWcondition)
        {
          case HW_Attachment:
            HW_SignalAttachement(PortNum, _handle->cc);
            _handle->cstate = USBPD_CAD_STATE_ATTACHED;
            *pEvent = USBPD_CAD_EVENT_ATTACHED;
            *pCCXX = _handle->cc;
            break;

          case HW_PwrCable_NoSink_Attachment:
            _handle->cstate = USBPD_CAD_STATE_EMC;
            *pEvent = USBPD_CAD_EVENT_EMC;
            *pCCXX = _handle->cc;
            break;

          case HW_PwrCable_Sink_Attachment:
            HW_SignalAttachement(PortNum, _handle->cc);
            _handle->cstate = USBPD_CAD_STATE_ATTEMC;
            *pEvent = USBPD_CAD_EVENT_ATTEMC;
            break;

          case HW_Debug_Attachment:
            _handle->cstate = USBPD_CAD_STATE_DEBUG;
            *pEvent = USBPD_CAD_EVENT_DEBUG;
            break;

          case HW_AudioAdapter_Attachment:
            _handle->cstate = USBPD_CAD_STATE_ACCESSORY;
            *pEvent = USBPD_CAD_EVENT_ACCESSORY;
            break;

          case HW_Detachment:
          default:
            /* could not occurs */
            break;
        } /* end of switch */
        *pCCXX = _handle->cc;
        _timing = 2;
      }
#if defined(_DRP)
      else /* Check state transition for SNK role */
#endif /* _DRP */
#endif /* _DRP || _SRC */
#if defined(_DRP) || defined(_SNK)
      {
        _timing = 2;
        if (1u == CAD_Check_VBus(PortNum)) /* Check if Vbus is on */
        {
          _handle->cstate = USBPD_CAD_STATE_ATTACHED;
          *pEvent = USBPD_CAD_EVENT_ATTACHED;
          *pCCXX = _handle->cc;
          HW_SignalAttachement(PortNum, _handle->cc);
        }
      }
#endif /* _DRP || _SNK */
    }
    else
    {
      _timing = CAD_TCCDEBOUCE_THRESHOLD - _handle->CAD_tDebounce;
    }
    /* reset the flag for CAD_tDebounce */
    _handle->CAD_tDebounce_flag = USBPD_FALSE;
  }
  else /* CAD_HW_Condition[PortNum] = HW_Detachment */
  {
#if defined(_DRP) || defined(_SNK)
    if (USBPD_PORTPOWERROLE_SNK == Ports[PortNum].params->PE_PowerRole)
    {
      /* start counting of CAD_tDebounce */
      if (USBPD_FALSE == _handle->CAD_tDebounce_flag)
      {
        _handle->CAD_tDebounce_start = HAL_GetTick();
        _handle->CAD_tDebounce_flag = USBPD_TRUE;
        _timing = CAD_TPDDEBOUCE_THRESHOLD;
      }
      else /* CAD_tDebounce already running */
      {
        /* evaluate CAD_tDebounce */
        _handle->CAD_tDebounce = HAL_GetTick() - _handle->CAD_tDebounce_start;
        if ((_handle->CAD_tDebounce > CAD_TPDDEBOUCE_THRESHOLD))
        {
          _handle->CAD_tDebounce_flag = USBPD_FALSE;
          _handle->cstate             = USBPD_CAD_STATE_SWITCH_TO_SRC;
          HW_SignalDetachment(PortNum);
        }
      }
    }
#if defined(_DRP)
    else /* (hcad->PortPowerRole != USBPD_PORTPOWERROLE_SNK)*/
#endif /* _DRP */
#endif /* _DRP || _SNK */
#if defined(_DRP) || defined(_SRC)
    {
      /* start counting of CAD_tDebounce */
      if (USBPD_FALSE == _handle->CAD_tDebounce_flag)
      {
        _handle->CAD_tDebounce_start  = HAL_GetTick();
        _handle->CAD_tDebounce_flag   = USBPD_TRUE;
        _timing                       = CAD_TSRCDISCONNECT_THRESHOLD;
      }
      else /* CAD_tDebounce already running */
      {
        /* evaluate CAD_tDebounce */
        _handle->CAD_tDebounce = HAL_GetTick() - _handle->CAD_tDebounce_start;
        if (_handle->CAD_tDebounce > CAD_TSRCDISCONNECT_THRESHOLD)
        {
          _handle->CAD_tDebounce_flag = USBPD_FALSE;
          _handle->cstate             = USBPD_CAD_STATE_SWITCH_TO_SNK;
          HW_SignalDetachment(PortNum);
        }
      }
    }
#endif /* _DRP || _SRC */
  }

  return _timing;
}

static uint32_t ManageStateEMC(uint8_t PortNum, USBPD_CAD_EVENT *pEvent, CCxPin_TypeDef *pCCXX)
{
  uint32_t _timing = CAD_INFINITE_TIME;
  CAD_HW_HandleTypeDef *_handle = &CAD_HW_Handles[PortNum];

  CAD_Check_HW(PortNum);
  /* Change the status on the basis of the HW event given by CAD_Check_HW() */
  switch (_handle->CurrentHWcondition)
  {
    case HW_Detachment :
      _handle->cstate = USBPD_CAD_STATE_SWITCH_TO_SNK;
      *pEvent = USBPD_CAD_EVENT_DETACHED;
      *pCCXX = CCNONE;
      _timing = 1;
      break;
    case HW_PwrCable_Sink_Attachment:
    case HW_Attachment :
      _handle->cstate = USBPD_CAD_STATE_ATTACHED_WAIT;
      _handle->CAD_tDebounce_start = HAL_GetTick() - 5u;  /* this is only to check cable presence */
      _timing = 2;
      break;
    case HW_PwrCable_NoSink_Attachment:
    default :
      /* nothing to do still the same status */
#if defined(_DRP)
      if (USBPD_TRUE == Ports[PortNum].settings->CAD_RoleToggle)
      {
        if ((HAL_GetTick() - _handle->CAD_tToogle_start) > Ports[PortNum].settings->CAD_SRCToggleTime)
        {
          _handle->cstate = USBPD_CAD_STATE_SWITCH_TO_SNK;
        }
        _timing = 0;
      }
#endif /* _DRP */
      break;
  }
  return _timing;
}

static uint32_t ManageStateAttached(uint8_t PortNum, USBPD_CAD_EVENT *pEvent, CCxPin_TypeDef *pCCXX)
{
  uint32_t _timing = 2;
  CAD_HW_HandleTypeDef *_handle = &CAD_HW_Handles[PortNum];

#if defined(_DRP) || defined(_SRC)
  if (USBPD_PORTPOWERROLE_SRC == Ports[PortNum].params->PE_PowerRole)
  {
    uint32_t ccx  = (Ports[PortNum].CCx == CC1) ? (Ports[PortNum].husbpd->SR & UCPD_SR_TYPEC_VSTATE_CC1) : (Ports[PortNum].husbpd->SR & UCPD_SR_TYPEC_VSTATE_CC2);
    uint32_t comp = (Ports[PortNum].CCx == CC1) ? LL_UCPD_SRC_CC1_OPEN : LL_UCPD_SRC_CC2_OPEN;

    if (comp == ccx)
    {
      /* start counting of CAD_tDebounce */
      if (USBPD_FALSE == _handle->CAD_tDebounce_flag)
      {
        _handle->CAD_tDebounce_flag   = USBPD_TRUE;
        _handle->CAD_tDebounce_start  = HAL_GetTick();
        _timing                       = CAD_TSRCDISCONNECT_THRESHOLD;
      }
      else /* CAD_tDebounce already running */
      {
        /* evaluate CAD_tDebounce */
        _handle->CAD_tDebounce = HAL_GetTick() - _handle->CAD_tDebounce_start;
        if (_handle->CAD_tDebounce > CAD_TSRCDISCONNECT_THRESHOLD)
        {
          HW_SignalDetachment(PortNum);
#ifdef _DRP
          USBPDM1_AssertRd(PortNum);
#endif          
          _handle->CAD_tDebounce_flag = USBPD_FALSE;
          /* move inside state DETACH to avoid wrong VCONN level*/
          _handle->cstate             = USBPD_CAD_STATE_SWITCH_TO_SNK;
          *pEvent                     = USBPD_CAD_EVENT_DETACHED;
          *pCCXX                      = CCNONE;
          _timing                     = 0;
        }
      }
    }
    else
    {
      /* Reset tPDDebounce flag*/
      _handle->CAD_tDebounce_flag   = USBPD_FALSE;
      _timing = CAD_INFINITE_TIME;
    }
  }
#if defined(_DRP)
  else /* USBPD_PORTPOWERROLE_SNK case */
#endif /* _DRP */
#endif /*_DRP || _SRC */
#if defined(_DRP) || defined(_SNK)
  {
    if (CAD_Check_VBus(PortNum) == 0u) /* Check if Vbus is off */
    {
      _handle->CurrentHWcondition = HW_Detachment;
      HW_SignalDetachment(PortNum);
      /* restart the toggle time */
#if defined(_DRP)
      _handle->CAD_tToogle_start = HAL_GetTick();
#endif /* _DRP */
      _handle->cstate = USBPD_CAD_STATE_SWITCH_TO_SRC;
      *pEvent = USBPD_CAD_EVENT_DETACHED;
      *pCCXX = CCNONE;
      _timing = 0;
    }
    else
    {
      _timing = 10;
    }
  }
#endif /*_DRP || _SNK */

  return _timing;
}

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

