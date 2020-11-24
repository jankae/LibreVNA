/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usbpd_pdo_defs.h
  * @author  MCD Application Team
  * @brief   Header file for definition of PDO/APDO values for 2 ports(DRP/SNK) configuration
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

#ifndef __USBPD_PDO_DEF_H_
#define __USBPD_PDO_DEF_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Define   ------------------------------------------------------------------*/

/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Exported typedef ----------------------------------------------------------*/

/* USER CODE BEGIN typedef */

/**
  * @brief  USBPD Port PDO Structure definition
  *
  */

/* USER CODE END typedef */

/* Exported define -----------------------------------------------------------*/

/* USER CODE BEGIN Exported_Define */

#define USBPD_CORE_PDO_SRC_FIXED_MAX_CURRENT 3
#define USBPD_CORE_PDO_SNK_FIXED_MAX_CURRENT 1500

/* USER CODE END Exported_Define */

/* Exported constants --------------------------------------------------------*/

/* USER CODE BEGIN constants */

/* USER CODE END constants */

/* Exported macro ------------------------------------------------------------*/

/* USER CODE BEGIN macro */

/* USER CODE END macro */

/* Exported variables --------------------------------------------------------*/

/* USER CODE BEGIN variables */

/* USER CODE END variables */

#ifndef __USBPD_PWR_IF_C
extern const uint32_t PORT0_PDO_ListSRC[USBPD_MAX_NB_PDO];
extern const uint32_t PORT0_PDO_ListSNK[USBPD_MAX_NB_PDO];
extern const uint32_t PORT1_PDO_ListSRC[USBPD_MAX_NB_PDO];
extern const uint32_t PORT1_PDO_ListSNK[USBPD_MAX_NB_PDO];
#else
/* Definition of Source PDO for Port 0 */
const uint32_t PORT0_PDO_ListSRC[USBPD_MAX_NB_PDO] =
{
  /* PDO 1 */
        (0x00000000U),
  /* PDO 2 */
        (0x00000000U),
  /* PDO 3 */
        (0x00000000U),
  /* PDO 4 */
        (0x00000000U),
  /* PDO 5 */
        (0x00000000U),
  /* PDO 6 */
        (0x00000000U),
  /* PDO 7 */
        (0x00000000U)
};

/* Definition of Sink PDO for Port 0 */
const uint32_t PORT0_PDO_ListSNK[USBPD_MAX_NB_PDO] =
{
  /* PDO 1 */
        (0x00000000U),
  /* PDO 2 */
        (0x00000000U),
  /* PDO 3 */
        (0x00000000U),
  /* PDO 4 */
        (0x00000000U),
  /* PDO 5 */
        (0x00000000U),
  /* PDO 6 */
        (0x00000000U),
  /* PDO 7 */
        (0x00000000U)
};

#endif

/* Exported functions --------------------------------------------------------*/

/* USER CODE BEGIN functions */

/* USER CODE END functions */

#ifdef __cplusplus
}
#endif

#endif /* __USBPD_PDO_DEF_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
