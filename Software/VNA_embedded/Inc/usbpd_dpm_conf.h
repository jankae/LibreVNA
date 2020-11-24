/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usbpd_dpm_conf.h
  * @author  MCD Application Team
  * @brief   Header file for stack/application settings file
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

#ifndef __USBPD_DPM_CONF_H_
#define __USBPD_DPM_CONF_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbpd_pdo_defs.h"
#include "usbpd_vdm_user.h"
#include "usbpd_dpm_user.h"

/* USER CODE BEGIN Includes */
/* Section where include file can be added */

/* USER CODE END Includes */

/* Define   ------------------------------------------------------------------*/
/* Define VID, PID,... manufacturer parameters */
#define USBPD_VID (0x0483u)     /*!< Vendor ID (assigned by the USB-IF)                     */
#define USBPD_PID (0x0002u)     /*!< Product ID (assigned by the manufacturer)              */
#define USBPD_XID (0xF0000003u) /*!< Value provided by the USB-IF assigned to the product   */

/* USER CODE BEGIN Define */
/* Section where Define can be added */

/* USER CODE END Define */

/* Exported typedef ----------------------------------------------------------*/
/* USER CODE BEGIN Typedef */
/* Section where Typedef can be added */

/* USER CODE END Typedef */

/* Private variables ---------------------------------------------------------*/
#ifndef __USBPD_DPM_CORE_C
extern USBPD_SettingsTypeDef            DPM_Settings[USBPD_PORT_COUNT];
#else /* __USBPD_DPM_CORE_C */
USBPD_SettingsTypeDef       DPM_Settings[USBPD_PORT_COUNT] =
{
  {
    .PE_SupportedSOP = USBPD_SUPPORTED_SOP_SOP    , /* Supported SOP : SOP, SOP' SOP" SOP'Debug SOP"Debug */
    .PE_SpecRevision = USBPD_SPECIFICATION_REV2,/* spec revision value                                     */
    .PE_DefaultRole = USBPD_PORTPOWERROLE_SNK,  /* Default port role                                       */
    .PE_RoleSwap = USBPD_FALSE,                  /* support port role swap                                  */
    .PE_VDMSupport = USBPD_FALSE,
    .PE_RespondsToDiscovSOP = USBPD_FALSE,      /*!< Can respond successfully to a Discover Identity */
    .PE_AttemptsDiscovSOP = USBPD_FALSE,        /*!< Can send a Discover Identity */
    .PE_PingSupport = USBPD_FALSE,              /* support Ping (only for PD3.0)                                            */
    .PE_CapscounterSupport = USBPD_FALSE,       /* support caps counter                                    */
    .CAD_RoleToggle = USBPD_FALSE,               /* CAD role toggle                                         */
    .CAD_TryFeature = 0,              /* CAD try feature                                         */
    .CAD_AccesorySupport = USBPD_FALSE,         /* CAD accessory support                                   */
    .PE_PD3_Support.d =                           /*!< PD3 SUPPORT FEATURE                                              */
    {
      .PE_UnchunkSupport                = USBPD_FALSE,  /* support Unchunked mode (valid only spec revision 3.0)   */
      .PE_FastRoleSwapSupport           = USBPD_FALSE,   /* support fast role swap only spec revsion 3.0            */
      .Is_GetPPSStatus_Supported        = USBPD_FALSE,  /*!< PPS message NOT supported by PE stack */
      .Is_SrcCapaExt_Supported          = USBPD_FALSE,  /*!< Source_Capabilities_Extended message supported or not by DPM */
      .Is_Alert_Supported               = USBPD_FALSE,   /*!< Alert message supported or not by DPM */
      .Is_GetStatus_Supported           = USBPD_FALSE,   /*!< Status message supported or not by DPM (Is_Alert_Supported should be enabled) */
      .Is_GetManufacturerInfo_Supported = USBPD_FALSE,  /*!< Manufacturer_Info message supported or not by DPM */
      .Is_GetCountryCodes_Supported     = USBPD_FALSE,  /*!< Country_Codes message supported or not by DPM */
      .Is_GetCountryInfo_Supported      = USBPD_FALSE,  /*!< Country_Info message supported or not by DPM */
      .Is_SecurityRequest_Supported     = USBPD_FALSE,  /*!< Security_Response message supported or not by DPM */
      .Is_FirmUpdateRequest_Supported   = USBPD_FALSE,  /*!< Firmware update response message supported by PE */
      .Is_SnkCapaExt_Supported          = USBPD_FALSE,  /*!< Sink_Capabilities_Extended message supported by PE */
    },

    .CAD_SRCToggleTime = 0,                    /* uint8_t CAD_SRCToggleTime; */
    .CAD_SNKToggleTime = 0,                    /* uint8_t CAD_SNKToggleTime; */
  }
};

#endif

/* USER CODE BEGIN Variable */
/* Section where Variable can be added */

/* USER CODE END Variable */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN Constant */
/* Section where Constant can be added */

/* USER CODE END Constant */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN Macro */
/* Section where Macro can be added */

/* USER CODE END Macro */

#ifdef __cplusplus
}
#endif

#endif /* __USBPD_DPM_CONF_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
