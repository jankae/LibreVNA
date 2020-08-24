/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usbpd_dpm_core.c
  * @author  MCD Application Team
  * @brief   USBPD dpm core file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

#define __USBPD_DPM_CORE_C

/* Includes ------------------------------------------------------------------*/
#include "usbpd_core.h"
#include "usbpd_trace.h"
#include "usbpd_dpm_core.h"
#include "usbpd_dpm_user.h"
#include "usbpd_dpm_conf.h"
#include "cmsis_os.h"

/* Private enum */
enum
{
  USBPD_THREAD_PORT_0 = USBPD_PORT_0,
#if USBPD_PORT_COUNT == 2
  USBPD_THREAD_PORT_1 = USBPD_PORT_1,
#endif
  USBPD_THREAD_CAD
};

/* Generic STM32 prototypes */
extern uint32_t HAL_GetTick(void);

/* Private function prototypes -----------------------------------------------*/
void USBPD_DPM_CADCallback(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc);
void USBPD_PE_Task(void const *argument);
void USBPD_CAD_Task(void const *argument);

static void USBPD_PE_TaskWakeUp(uint8_t PortNum);
static void USBPD_DPM_CADTaskWakeUp(void);

/* Private typedef -----------------------------------------------------------*/
osThreadDef(PE_0, USBPD_PE_Task, osPriorityHigh, 0, 200);
osThreadDef(PE_1, USBPD_PE_Task, osPriorityHigh, 0, 200);

/* Private define ------------------------------------------------------------*/
#define MAX_TREAD_POWER   (USBPD_PORT_COUNT + 1)

#define OSTHREAD_PE(__VAL__) __VAL__==USBPD_PORT_0?osThread(PE_0):osThread(PE_1)

/* Private macro -------------------------------------------------------------*/
#define CHECK_PE_FUNCTION_CALL(_function_)  if(USBPD_OK != _function_) {return USBPD_ERROR;}
#define CHECK_CAD_FUNCTION_CALL(_function_) if(USBPD_CAD_OK != _function_) {return USBPD_ERROR;}

#if defined(_DEBUG_TRACE)
#define DPM_CORE_DEBUG_TRACE(_PORTNUM_, __MESSAGE__)  USBPD_TRACE_Add(USBPD_TRACE_DEBUG, _PORTNUM_, 0u, (uint8_t *)(__MESSAGE__), sizeof(__MESSAGE__) - 1u);
#else
#define DPM_CORE_DEBUG_TRACE(_PORTNUM_, __MESSAGE__)
#endif /* _DEBUG_TRACE */

/* Private variables ---------------------------------------------------------*/
static uint32_t DPM_Sleep_time[MAX_TREAD_POWER];
static osThreadId DPM_Thread_Table[MAX_TREAD_POWER];
osMessageQDef(queuePE, 2, uint16_t);
osMessageQDef(queueCAD, 1, uint16_t);
static osMessageQId PEQueueId[USBPD_PORT_COUNT], CADQueueId;

USBPD_ParamsTypeDef   DPM_Params[USBPD_PORT_COUNT];

/* Private functions ---------------------------------------------------------*/
static void DPM_ManageAttachedState(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc);

/**
  * @brief  Initialize the core stack (port power role, PWR_IF, CAD and PE Init procedures)
  * @retval USBPD status
  */
USBPD_StatusTypeDef USBPD_DPM_InitCore(void)
{
/* variable to get dynamique memory allocated by usbpd stack */
  uint32_t stack_dynamemsize;

static const USBPD_PE_Callbacks dpmCallbacks =
{
  NULL,
  NULL,
  USBPD_DPM_EvaluatePowerRoleSwap,
  USBPD_DPM_Notification,
  NULL,
  USBPD_DPM_GetDataInfo,
  USBPD_DPM_SetDataInfo,
  NULL,
  USBPD_DPM_SNK_EvaluateCapabilities,
  NULL,
  USBPD_PE_TaskWakeUp,
  NULL,
  NULL,
  NULL,
  USBPD_DPM_EvaluateDataRoleSwap,
  USBPD_DPM_IsPowerReady
};

  static const USBPD_CAD_Callbacks CAD_cbs = { USBPD_DPM_CADCallback, USBPD_DPM_CADTaskWakeUp };

  /* Check the lib selected */
  if( USBPD_TRUE != USBPD_PE_CheckLIB(_LIB_ID))
  {
    return USBPD_ERROR;
  }

  /* to get how much memory are dynamically allocated by the stack
     the memory return is corresponding to 2 ports so if the application
     managed only one port divide the value return by 2                   */
  stack_dynamemsize = USBPD_PE_GetMemoryConsumption();

  /* done to avoid warning */
  stack_dynamemsize--;

  /* Variable to be sure that DPM is correctly initialized */
  DPM_Params[USBPD_PORT_0].DPM_Initialized = USBPD_FALSE;
#if USBPD_PORT_COUNT == 2
  DPM_Params[USBPD_PORT_1].DPM_Initialized = USBPD_FALSE;
#endif /* USBPD_PORT_COUNT == 2 */

  /* check the stack settings */
  DPM_Params[USBPD_PORT_0].PE_SpecRevision  = DPM_Settings[USBPD_PORT_0].PE_SpecRevision;
  DPM_Params[USBPD_PORT_0].PE_PowerRole     = DPM_Settings[USBPD_PORT_0].PE_DefaultRole;
  DPM_Params[USBPD_PORT_0].PE_SwapOngoing   = USBPD_FALSE;
  DPM_Params[USBPD_PORT_0].ActiveCCIs       = CCNONE;
  DPM_Params[USBPD_PORT_0].VconnCCIs        = CCNONE;
  DPM_Params[USBPD_PORT_0].VconnStatus      = USBPD_FALSE;
#if USBPD_PORT_COUNT == 2
  DPM_Params[USBPD_PORT_1].PE_SpecRevision  = DPM_Settings[USBPD_PORT_1].PE_SpecRevision;
  DPM_Params[USBPD_PORT_1].PE_PowerRole     = DPM_Settings[USBPD_PORT_1].PE_DefaultRole;
  DPM_Params[USBPD_PORT_1].PE_SwapOngoing   = USBPD_FALSE;
  DPM_Params[USBPD_PORT_1].ActiveCCIs       = CCNONE;
  DPM_Params[USBPD_PORT_1].VconnCCIs        = CCNONE;
  DPM_Params[USBPD_PORT_1].VconnStatus      = USBPD_FALSE;
#endif /* USBPD_PORT_COUNT == 2 */

  /* CAD SET UP : Port 0 */
  CHECK_CAD_FUNCTION_CALL(USBPD_CAD_Init(USBPD_PORT_0, (USBPD_CAD_Callbacks *)&CAD_cbs, (USBPD_SettingsTypeDef *)&DPM_Settings[USBPD_PORT_0], &DPM_Params[USBPD_PORT_0]));
#if USBPD_PORT_COUNT == 2
  CHECK_CAD_FUNCTION_CALL(USBPD_CAD_Init(USBPD_PORT_1, (USBPD_CAD_Callbacks *)&CAD_cbs, (USBPD_SettingsTypeDef *)&DPM_Settings[USBPD_PORT_1], &DPM_Params[USBPD_PORT_1]));
#endif /* USBPD_PORT_COUNT == 2 */

  /* PE SET UP : Port 0 */
  CHECK_PE_FUNCTION_CALL(USBPD_PE_Init(USBPD_PORT_0, (USBPD_SettingsTypeDef *)&DPM_Settings[USBPD_PORT_0], &DPM_Params[USBPD_PORT_0], &dpmCallbacks));
#if USBPD_PORT_COUNT == 2
  CHECK_PE_FUNCTION_CALL(USBPD_PE_Init(USBPD_PORT_1, (USBPD_SettingsTypeDef *)&DPM_Settings[USBPD_PORT_1], &DPM_Params[USBPD_PORT_1], &dpmCallbacks));
#endif /* USBPD_PORT_COUNT == 2 */

  /* DPM is correctly initialized */
  DPM_Params[USBPD_PORT_0].DPM_Initialized = USBPD_TRUE;
#if USBPD_PORT_COUNT == 2
  DPM_Params[USBPD_PORT_1].DPM_Initialized = USBPD_TRUE;
#endif /* USBPD_PORT_COUNT == 2 */

  /* Enable CAD on Port 0 */
  USBPD_CAD_PortEnable(USBPD_PORT_0, USBPD_CAD_ENABLE);
#if USBPD_PORT_COUNT == 2
  USBPD_CAD_PortEnable(USBPD_PORT_1, USBPD_CAD_ENABLE);
#endif /* USBPD_PORT_COUNT == 2 */

  return USBPD_OK;
}

/**
  * @brief  Initialize the OS parts (task, queue,... )
  * @retval USBPD status
  */
USBPD_StatusTypeDef USBPD_DPM_InitOS(void)
{
  osThreadDef(CAD, USBPD_CAD_Task, osPriorityRealtime, 0, 300);
  if((DPM_Thread_Table[USBPD_THREAD_CAD] = osThreadCreate(osThread(CAD), NULL)) == NULL)
  {
    return USBPD_ERROR;
  }
  CADQueueId = osMessageCreate(osMessageQ(queueCAD), NULL);

  /* Create the queue corresponding to PE task */
  PEQueueId[0] = osMessageCreate(osMessageQ(queuePE), NULL);
#if USBPD_PORT_COUNT == 2
  PEQueueId[1] = osMessageCreate(osMessageQ(queuePE), NULL);
#endif /* USBPD_PORT_COUNT == 2 */

  /* PE task to be created on attachment */
  DPM_Thread_Table[USBPD_THREAD_PORT_0] = NULL;
#if USBPD_PORT_COUNT == 2
  DPM_Thread_Table[USBPD_THREAD_PORT_1] = NULL;
#endif /* USBPD_PORT_COUNT == 2 */

  return USBPD_OK;
}

/**
  * @brief  Initialize the OS parts (port power role, PWR_IF, CAD and PE Init procedures)
  * @retval None
  */
void USBPD_DPM_Run(void)
{
  osKernelStart();
}

/**
  * @brief  Initialize DPM (port power role, PWR_IF, CAD and PE Init procedures)
  * @retval USBPD status
  */
void USBPD_DPM_TimerCounter(void)
{
  /* Call PE/PRL timers functions only if DPM is initialized */
  if (USBPD_TRUE == DPM_Params[USBPD_PORT_0].DPM_Initialized)
  {
    USBPD_DPM_UserTimerCounter(USBPD_PORT_0);
    USBPD_PE_TimerCounter(USBPD_PORT_0);
    USBPD_PRL_TimerCounter(USBPD_PORT_0);
  }
#if USBPD_PORT_COUNT==2
  if (USBPD_TRUE == DPM_Params[USBPD_PORT_1].DPM_Initialized)
  {
    USBPD_DPM_UserTimerCounter(USBPD_PORT_1);
    USBPD_PE_TimerCounter(USBPD_PORT_1);
    USBPD_PRL_TimerCounter(USBPD_PORT_1);
  }
#endif /* USBPD_PORT_COUNT == 2 */

}

/**
  * @brief  WakeUp PE task
  * @param  PortNum port number
  * @retval None
  */
static void USBPD_PE_TaskWakeUp(uint8_t PortNum)
{
  osMessagePut(PEQueueId[PortNum], 0xFFFF, 0);
}

/**
  * @brief  WakeUp CAD task
  * @retval None
  */
static void USBPD_DPM_CADTaskWakeUp(void)
{
  osMessagePut(CADQueueId, 0xFFFF, 0);
}

/**
  * @brief  Main task for PE layer
  * @param  argument Not used
  * @retval None
  */
void USBPD_PE_Task(void const *argument)
{
  uint8_t _port = (uint32_t)argument;

  for(;;)
  {
    DPM_Sleep_time[_port] =
    USBPD_PE_StateMachine_SNK(_port);
    osMessageGet(PEQueueId[_port], DPM_Sleep_time[_port]);

  }
}

/**
  * @brief  Main task for CAD layer
  * @param  argument Not used
  * @retval None
  */
void USBPD_CAD_Task(void const *argument)
{
  for(;;)
  {
    DPM_Sleep_time[USBPD_THREAD_CAD] = USBPD_CAD_Process();
    osMessageGet(CADQueueId, DPM_Sleep_time[USBPD_THREAD_CAD]);
  }
}

/**
  * @brief  CallBack reporting events on a specified port from CAD layer.
  * @param  PortNum   The handle of the port
  * @param  State     CAD state
  * @param  Cc        The Communication Channel for the USBPD communication
  * @retval None
  */
void USBPD_DPM_CADCallback(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc)
{

  switch(State)
  {
  case USBPD_CAD_EVENT_ATTEMC :
    {
      DPM_ManageAttachedState(PortNum, State, Cc);
      break;
    }
  case USBPD_CAD_EVENT_ATTACHED :
    DPM_ManageAttachedState(PortNum, State, Cc);
    break;
  case USBPD_CAD_EVENT_DETACHED :
  case USBPD_CAD_EVENT_EMC :
    {
    /* The ufp is detached */
      (void)USBPD_PE_IsCableConnected(PortNum, 0);
    /* Terminate PE task */
    if (DPM_Thread_Table[PortNum] != NULL)
    {
      osThreadTerminate(DPM_Thread_Table[PortNum]);
      DPM_Thread_Table[PortNum] = NULL;
    }
    USBPD_DPM_UserCableDetection(PortNum, State);
    DPM_Params[PortNum].ActiveCCIs = CCNONE;
    DPM_Params[PortNum].PE_Power   = USBPD_POWER_NO;
    break;
    }
  default :
    /* nothing to do */
    break;
  }
}

static void DPM_ManageAttachedState(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc)
{
  DPM_Params[PortNum].ActiveCCIs = Cc;
  (void)USBPD_PE_IsCableConnected(PortNum, 1);

  USBPD_DPM_UserCableDetection(PortNum, State);

  /* Create PE task */
  if (DPM_Thread_Table[PortNum] == NULL)
  {
    DPM_Thread_Table[PortNum] = osThreadCreate(OSTHREAD_PE(PortNum), (void *)((uint32_t)PortNum));
    if (DPM_Thread_Table[PortNum] == NULL)
    {
      /* should not occurr. May be an issue with FreeRTOS heap size too small */
      while(1);
    }
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
