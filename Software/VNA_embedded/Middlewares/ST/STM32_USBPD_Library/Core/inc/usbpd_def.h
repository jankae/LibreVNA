/**
  ******************************************************************************
  * @file    usbpd_def.h
  * @author  MCD Application Team
  * @brief   Global defines for USB-PD libarary
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
#ifndef __USBPD_DEF_H_
#define __USBPD_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#if   defined ( __CC_ARM )
#define __ASM            __asm                                      /*!< asm keyword for ARM Compiler          */
#define __INLINE         __inline                                   /*!< inline keyword for ARM Compiler       */
#define __STATIC_INLINE  static __inline
#define __PACKEDSTRUCTBEGIN __packed struct
#define __PACKEDSTRUCTEND
#elif defined ( __ICCARM__ )
#define __ASM            __asm                                      /*!< asm keyword for IAR Compiler          */
#define __INLINE         inline                                     /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
#define __STATIC_INLINE  static inline
#define __PACKEDSTRUCTBEGIN __packed struct
#define __PACKEDSTRUCTEND
#elif defined ( __GNUC__ )
#define __ASM            __asm                                      /*!< asm keyword for GNU Compiler          */
#define __INLINE         inline                                     /*!< inline keyword for GNU Compiler       */
#define __STATIC_INLINE  static inline
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#define __PACKEDSTRUCTBEGIN struct
#define __PACKEDSTRUCTEND   __packed
#endif

#include <stdint.h>
#include <stddef.h>

/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_CORE
  * @{
  */

/** @addtogroup USBPD_CORE_DEF
  * @{
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup USBPD_CORE_DEF_Exported_Macros USBPD CORE DEF Exported Macros
  * @{
  */
/**
  * @brief  Compare two variables and return the smallest
  * @param  __VAR1__ First variable to be compared
  * @param  __VAR2__ Second variable to be compared
  * @retval Returns the smallest variable
  */
#define USBPD_MIN(__VAR1__, __VAR2__) (((__VAR1__) > (__VAR2__))?(__VAR2__):(__VAR1__))

/**
  * @brief  Compare two variables and return the biggest
  * @param  __VAR1__ First variable to be compared
  * @param  __VAR2__ Second variable to be compared
  * @retval Returns the biggest variable
  */
#define USBPD_MAX(__VAR1__, __VAR2__) (((__VAR1__) < (__VAR2__))?(__VAR2__):(__VAR1__))

/**
  * @brief  Check if the requested voltage is valid
  * @param  __MV__    Requested voltage in mV units
  * @param  __MAXMV__ Max Requested voltage in mV units
  * @param  __MINMV__ Min Requested voltage in mV units
  * @retval 1 if valid voltage else 0
  */
#define USBPD_IS_VALID_VOLTAGE(__MV__, __MAXMV__, __MINMV__) ((((__MV__) <= (__MAXMV__)) && ((__MV__) >= (__MINMV__)))? 1u: 0u)

#define DIV_ROUND_UP(x, y) (((x) + ((y) - 1u)) / (y))
#define MV2ADC(__X__)           ( (__X__*4095) / 3300 )
#define ADC2MV(__X__)           ( (__X__*3300) / 4095 )

/* Macros for integer division with various rounding variants default integer
   division rounds down. */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#define USPBPD_WRITE32(addr,data)   do {                                                                       \
                                         uint8_t bindex;                                                       \
                                         for(bindex = 0u; bindex < 4u; bindex++)                                 \
                                         {                                                                     \
                                           ((uint8_t *)addr)[bindex] = ((uint8_t)(data >> (8u * bindex)) & 0x000000FFu);  \
                                         }                                                                     \
                                       } while(0u);

#define LE16(addr) (((uint16_t)(*((uint8_t *)(addr))))\
                                             + (((uint16_t)(*(((uint8_t *)(addr)) + 1u))) << 8u))

#define LE32(addr) ((((uint32_t)(*(((uint8_t *)(addr)) + 0u))) + \
                                              (((uint32_t)(*(((uint8_t *)(addr)) + 1u))) << 8u) + \
                                              (((uint32_t)(*(((uint8_t *)(addr)) + 2u))) << 16u) + \
                                              (((uint32_t)(*(((uint8_t *)(addr)) + 3u))) << 24)))

/**
  * @}
  */

/* Exported defines   --------------------------------------------------------*/
/** @defgroup USBPD_CORE_DEF_Exported_Defines USBPD CORE DEF Exported Defines
  * @{
  */
#define USBPD_TRUE  1u
#define USBPD_FALSE 0u

#define __DEBUG_PESTATEMACHINE

/** @defgroup USBPD_CORE_DEF_Exported_Defines_Swiches USBPD Compilations switches
  * @brief List of compilation switches which can be used to reduce size of the CORE library
  * @{
  */

/* Default Switch */
#define USBPDCORE_GOTOMIN
#define USBPDCORE_BIST
#define USBPDCORE_GETSNKCAP
#define USBPDCORE_GETSRCCAP
#define USBPDCORE_GIVESNKCAP

#if defined(USBPDCORE_LIB_PD3_FULL) || defined(USBPDCORE_LIB_PD3_CONFIG_1) || defined(USBPD_TCPM_LIB_PD3_FULL) || defined(USBPD_TCPM_LIB_PD3_CONFIG_1)
/*
   USBPDCORE_LIB_PD3_FULL
   USBPDCORE_LIB_PD3_CONFIG_1  : NO VDM
   USBPD_TCPM_LIB_PD3_FULL     : PD3.0 TCPM FULL
   USBPD_TCPM_LIB_PD3_CONFIG_1 : PD3.0 TCPM FULL without VDM
*/
#define USBPDCORE_SRC
#define USBPDCORE_SNK
#define USBPDCORE_DRP
#define USBPDCORE_SNK_CAPA_EXT

#if defined(USBPDCORE_LIB_PD3_FULL) || defined(USBPD_TCPM_LIB_PD3_FULL)
#define USBPDCORE_SVDM
#endif /* USBPDCORE_LIB_PD3_FULL || USBPD_TCPM_LIB_PD3_FULL */

#define USBPDCORE_UVDM
#define USBPD_REV30_SUPPORT
#define USBPDCORE_CABLE
#define USBPDCORE_FASTROLESWAP
#define USBPDCORE_PPS
#define USBPDCORE_ALERT
#define USBPDCORE_SRC_CAPA_EXT
#define USBPDCORE_STATUS
#define USBPDCORE_BATTERY
#define USBPDCORE_MANU_INFO
#define USBPDCORE_SECURITY_MSG
#define USBPDCORE_FWUPD
#define USBPDCORE_COUNTRY_MSG
#define USBPDCORE_VCONN_SUPPORT
#define USBPDCORE_DATA_SWAP
#define USBPDCORE_ERROR_RECOVERY
#define USBPDCORE_UNCHUNCKED_MODE
#define USBPDCORE_PING_SUPPORT

#if defined(USBPD_TCPM_LIB_PD3_FULL) || defined(USBPD_TCPM_LIB_PD3_CONFIG_1)
#define USBPDCORE_TCPM_SUPPORT
#endif /* TCPM */

#endif /* PD3.0 */

/* List of compilation switches which can be used to reduce size of the CORE librairy */
#if defined(USBPDCORE_LIB_PD2_FULL) || defined(USBPDCORE_LIB_PD2_CONFIG_1) || defined(USBPDCORE_LIB_PD2_CONFIG_MINSRC) || defined(USBPDCORE_LIB_PD2_CONFIG_MINSNK)  || defined(USBPD_TCPM_LIB_PD2_FULL) || defined(USBPD_TCPM_LIB_PD2_CONFIG_1) || defined(USBPD_TCPM_LIB_PD2_MINSRC) || defined(USBPD_TCPM_LIB_PD2_MINSNK)
/*
   USBPDCORE_LIB_PD2_FULL
   USBPDCORE_LIB_PD2_CONFIG_1 : NO VDM
   USBPDCORE_LIB_PD2_CONFIG_MINSRC : ONLY SRC and  NO option
   USBPDCORE_LIB_PD2_CONFIG_MINSNK : ONLY SNK, and NO option
   USBPD_TCPM_LIB_PD2_FULL     : PD2.0 TCPM FULL
   USBPD_TCPM_LIB_PD2_CONFIG_1 : PD2.0 TCPM FULL without VDM
   USBPD_TCPM_LIB_PD2_MINSRC : PD2.0 TCPM Only SRC
   USBPD_TCPM_LIB_PD2_MINSNK : PD2.0 TCPM Only SNK
*/
#define USBPD_REV20_SUPPORT

#if !defined(USBPDCORE_LIB_PD2_CONFIG_MINSNK) && !defined(USBPD_TCPM_LIB_PD2_MINSNK)
#define USBPDCORE_SRC
#endif

#if !defined(USBPDCORE_LIB_PD2_CONFIG_MINSRC) && !defined(USBPD_TCPM_LIB_PD2_MINSRC)
#define USBPDCORE_SNK
#endif

#if defined(USBPDCORE_LIB_PD2_CONFIG_MINSRC) || defined(USBPDCORE_LIB_PD2_CONFIG_MINSNK) || defined(USBPD_TCPM_LIB_PD2_MINSRC) || defined(USBPD_TCPM_LIB_PD2_MINSNK)
#else
#define USBPDCORE_DRP
#define USBPDCORE_VCONN_SUPPORT
#define USBPDCORE_DATA_SWAP
#define USBPDCORE_CABLE
#define USBPDCORE_ERROR_RECOVERY
#define USBPDCORE_UVDM
#endif

#if defined(USBPDCORE_LIB_PD2_FULL) || defined(USBPD_TCPM_LIB_PD2_FULL)
#define USBPDCORE_SVDM
#endif /* USBPDCORE_LIB_PD3_FULL || USBPD_TCPM_LIB_PD2_FULL */

#if defined(USBPD_TCPM_LIB_PD2_FULL) || defined(USBPD_TCPM_LIB_PD2_CONFIG_1) || defined(USBPD_TCPM_LIB_PD2_MINSRC) || defined(USBPD_TCPM_LIB_PD2_MINSNK)
#define USBPDCORE_TCPM_SUPPORT
#endif /* TCPM */

#endif /* PD2.0 */

#if defined(USBPDCORE_LIB_PD3_CONFIG_2)
#undef USBPDCORE_GOTOMIN
#undef USBPDCORE_BIST
#undef USBPDCORE_GETSNKCAP
#undef USBPDCORE_GETSRCCAP
#undef USBPDCORE_GIVESNKCAP
#undef USBPDCORE_SNK_CAPA_EXT

#define USBPDCORE_SNK
#define USBPD_REV30_SUPPORT
#define USBPDCORE_FWUPD
#define USBPDCORE_UVDM
#define USBPDCORE_UNCHUNCKED_MODE
#endif /* USBPDCORE_LIB_PD3_CONFIG_2 */

/* _LIB_ID definition */
/*
  _LIB_ID constructs like this: 0xXYVVVWWW
  * X: 3 (PD3.0) or 2 (PD2.0)
  * Y: 0 (CORE) or 1 (TCPM)
  * VVV: Stack version (ex 200 for Stack 2.0.0)
  * WWW: 0 (FULL VERSION) or config_x
*/
/* Defines for PD revision */
#define LIB_PD_VERSION_POS  28u
#define LIB_PD_VERSION_MSK  (0xFu << LIB_PD_VERSION_POS)
#define LIB_PD2             (2u   << LIB_PD_VERSION_POS)
#define LIB_PD3             (3u   << LIB_PD_VERSION_POS)
/* Defines for CORE or TCPM */
#define LIB_CORE_TCPM_POS   24u
#define LIB_CORE_TCPM_MSK   (0xFu << LIB_CORE_TCPM_POS)
#define LIB_CORE            (0u   << LIB_CORE_TCPM_POS)
#define LIB_TCPM            (1u   << LIB_CORE_TCPM_POS)
/* Defines for STACK version */
#define LIB_STACK_VER_POS   12u
#define LIB_STACK_VER_MSK   (0xFFFu << LIB_STACK_VER_POS)
#define LIB_STACK_VER       (0x250u  << LIB_STACK_VER_POS)
/* Defines for configuration */
#define LIB_CONFIG_MSK      0xFFFu
#define LIB_FULL            0x000u
#define LIB_CONFIG_1        0x001u
#define LIB_CONFIG_MINSRC   0x002u
#define LIB_CONFIG_MINSNK   0x004u
#define LIB_CONFIG_2        0x010u

#ifdef USBPDCORE_LIB_PD3_FULL
#define _LIB_ID   (LIB_PD3 | LIB_CORE | LIB_STACK_VER | LIB_FULL)
#endif
#ifdef USBPDCORE_LIB_PD3_CONFIG_1
#define _LIB_ID   (LIB_PD3 | LIB_CORE | LIB_STACK_VER | LIB_CONFIG_1)
#endif
#ifdef USBPDCORE_LIB_PD2_FULL
#define _LIB_ID   (LIB_PD2 | LIB_CORE | LIB_STACK_VER | LIB_FULL)
#endif
#ifdef USBPDCORE_LIB_PD2_CONFIG_1
#define _LIB_ID   (LIB_PD2 | LIB_CORE | LIB_STACK_VER | LIB_CONFIG_1)
#endif
#ifdef USBPDCORE_LIB_PD2_CONFIG_MINSRC
#define _LIB_ID   (LIB_PD2 | LIB_CORE | LIB_STACK_VER | LIB_CONFIG_MINSRC)
#endif
#ifdef USBPDCORE_LIB_PD2_CONFIG_MINSNK
#define _LIB_ID   (LIB_PD2 | LIB_CORE | LIB_STACK_VER | LIB_CONFIG_MINSNK)
#endif
#ifdef USBPD_TCPM_LIB_PD2_FULL
#define _LIB_ID   (LIB_PD2 | LIB_TCPM | LIB_STACK_VER | LIB_FULL)
#endif
#ifdef USBPD_TCPM_LIB_PD2_MINSRC
#define _LIB_ID   (LIB_PD2 | LIB_TCPM | LIB_STACK_VER | LIB_CONFIG_MINSRC)
#endif
#ifdef USBPD_TCPM_LIB_PD2_MINSNK
#define _LIB_ID   (LIB_PD2 | LIB_TCPM | LIB_STACK_VER | LIB_CONFIG_MINSNK)
#endif
#ifdef USBPD_TCPM_LIB_PD2_CONFIG_1
#define _LIB_ID   (LIB_PD2 | LIB_TCPM | LIB_STACK_VER | LIB_CONFIG_1)
#endif
#ifdef USBPD_TCPM_LIB_PD3_CONFIG_1
#define _LIB_ID   (LIB_PD3 | LIB_TCPM | LIB_STACK_VER | LIB_CONFIG_1)
#endif
#ifdef USBPD_TCPM_LIB_PD3_FULL
#define _LIB_ID   (LIB_PD3 | LIB_TCPM | LIB_STACK_VER | LIB_FULL)
#endif
#if defined(USBPDCORE_LIB_PD3_CONFIG_2)
#define _LIB_ID   (LIB_PD3 | LIB_TCPM | LIB_STACK_VER | LIB_CONFIG_2)
#endif
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
#define USBPD_PORT_0                     (0U)              /*!< Port 0 identifier */
#define USBPD_PORT_1                     (1U)              /*!< Port 1 identifier */

#define USBPD_MAX_NB_PDO                 (7U)              /*!< Maximum number of supported Power Data Objects: fix by the Specification */
#define BIST_CARRIER_MODE_MS             (50U)             /*!< Time in ms of the BIST signal*/

/*
 * Maximum size of a Power Delivery packet (in bits on the wire) :
 *    16-bit header + 0..7 32-bit data objects  (+ 4b5b encoding)
 *    64-bit preamble + SOP (4x 5b) + header (16-bit) + message in 4b5b + 32-bit CRC  + EOP (1x 5b)
 * =  64bit           + 4*5bit      + 16bit * 5/4 + 7 * 32bit * 5/4 + 32bit * 5/4 + 5
 */
#define PHY_BIT_LEN             ((uint16_t)429u)
#define PHY_MAX_RAW_SIZE        ((uint16_t)((PHY_BIT_LEN*2u) + 3u))
#define PHY_MAX_RAW_BYTE_SIZE   ((uint8_t)60u) /*!<PHY_BIT_LEN / 8 + SAFE Bytes */

/** @defgroup USBPD_PDO_Index_And_Mask_Constants Index and Mask constants used in PDO bits handling
  * @{
  */
#define USBPD_PDO_TYPE_Pos                           (30U)                                                     /*!< PDO Type bits position                          */
#define USBPD_PDO_TYPE_Msk                           (0x3U << USBPD_PDO_TYPE_Pos)                              /*!< PDO Type bits mask : 0xC0000000                 */
#define USBPD_PDO_TYPE_FIXED                         (uint32_t)(USBPD_CORE_PDO_TYPE_FIXED << USBPD_PDO_TYPE_Pos)         /*!< PDO Type = FIXED                                */
#define USBPD_PDO_TYPE_BATTERY                       (uint32_t)(USBPD_CORE_PDO_TYPE_BATTERY << USBPD_PDO_TYPE_Pos)       /*!< PDO Type = BATTERY                              */
#define USBPD_PDO_TYPE_VARIABLE                      (uint32_t)(USBPD_CORE_PDO_TYPE_VARIABLE << USBPD_PDO_TYPE_Pos)      /*!< PDO Type = VARIABLE                             */
#if defined(USBPD_REV30_SUPPORT)
#define USBPD_PDO_TYPE_APDO                          (uint32_t)(USBPD_CORE_PDO_TYPE_APDO<< USBPD_PDO_TYPE_Pos)           /*!< PDO Type = APDO                                 */
#endif /* USBPD_REV30_SUPPORT */

/* Source Fixed type PDO elments */
#define USBPD_PDO_SRC_FIXED_DRP_SUPPORT_Pos          (29U)                                                     /*!< DRP Support bit position                        */
#define USBPD_PDO_SRC_FIXED_DRP_SUPPORT_Msk          (0x1U << USBPD_PDO_SRC_FIXED_DRP_SUPPORT_Pos)             /*!< DRP Support bit mask : 0x20000000               */
#define USBPD_PDO_SRC_FIXED_DRP_NOT_SUPPORTED        (0U)                                                      /*!< DRP not supported                               */
#define USBPD_PDO_SRC_FIXED_DRP_SUPPORTED            USBPD_PDO_SRC_FIXED_DRP_SUPPORT_Msk                       /*!< DRP supported                                   */

#define USBPD_PDO_SRC_FIXED_USBSUSPEND_Pos           (28U)                                                     /*!< USB Suspend Support bit position                */
#define USBPD_PDO_SRC_FIXED_USBSUSPEND_Msk           (0x1U << USBPD_PDO_SRC_FIXED_USBSUSPEND_Pos)              /*!< USB Suspend Support bit mask : 0x10000000       */
#define USBPD_PDO_SRC_FIXED_USBSUSPEND_NOT_SUPPORTED (0U)                                                      /*!< USB Suspend not supported                       */
#define USBPD_PDO_SRC_FIXED_USBSUSPEND_SUPPORTED     USBPD_PDO_SRC_FIXED_USBSUSPEND_Msk                        /*!< USB Suspend supported                           */

#define USBPD_PDO_SRC_FIXED_EXT_POWER_Pos            (27U)                                                     /*!< External Power available bit position           */
#define USBPD_PDO_SRC_FIXED_EXT_POWER_Msk            (0x1U << USBPD_PDO_SRC_FIXED_EXT_POWER_Pos)               /*!< External Power available bit mask : 0x08000000  */
#define USBPD_PDO_SRC_FIXED_EXT_POWER_NOT_AVAILABLE  (0U)                                                      /*!< External Power not available                    */
#define USBPD_PDO_SRC_FIXED_EXT_POWER_AVAILABLE      USBPD_PDO_SRC_FIXED_EXT_POWER_Msk                         /*!< External Power available                        */

#define USBPD_PDO_SRC_FIXED_USBCOMM_Pos              (26U)                                                     /*!< USB Communication Support bit position          */
#define USBPD_PDO_SRC_FIXED_USBCOMM_Msk              (0x1U << USBPD_PDO_SRC_FIXED_USBCOMM_Pos)                 /*!< USB Communication Support bit mask : 0x04000000 */
#define USBPD_PDO_SRC_FIXED_USBCOMM_NOT_SUPPORTED    (0U)                                                      /*!< USB Communication not supported                 */
#define USBPD_PDO_SRC_FIXED_USBCOMM_SUPPORTED        USBPD_PDO_SRC_FIXED_USBCOMM_Msk                           /*!< USB Communication supported                     */

#define USBPD_PDO_SRC_FIXED_DRD_SUPPORT_Pos          (25U)                                                     /*!< Dual Role Data Support bit position             */
#define USBPD_PDO_SRC_FIXED_DRD_SUPPORT_Msk          (0x1U << USBPD_PDO_SRC_FIXED_DRD_SUPPORT_Pos)             /*!< Dual Role Data Support bit mask : 0x02000000    */
#define USBPD_PDO_SRC_FIXED_DRD_NOT_SUPPORTED        (0U)                                                      /*!< Dual Role Data not supported                    */
#define USBPD_PDO_SRC_FIXED_DRD_SUPPORTED            USBPD_PDO_SRC_FIXED_DRD_SUPPORT_Msk                       /*!< Dual Role Data supported                        */

#if defined(USBPD_REV30_SUPPORT)
#define USBPD_PDO_SRC_FIXED_UNCHUNK_SUPPORT_Pos      (24U)                                                     /*!< Unchunked Extended Messages Support bit position             */
#define USBPD_PDO_SRC_FIXED_UNCHUNK_SUPPORT_Msk      (0x1U << USBPD_PDO_SRC_FIXED_UNCHUNK_SUPPORT_Pos)         /*!< Unchunked Extended Messages Support bit mask : 0x01000000    */
#define USBPD_PDO_SRC_FIXED_UNCHUNK_NOT_SUPPORTED    (0U)                                                      /*!< Unchunked Extended Messages not supported                    */
#define USBPD_PDO_SRC_FIXED_UNCHUNK_SUPPORTED        USBPD_PDO_SRC_FIXED_UNCHUNK_SUPPORT_Msk                   /*!< Unchunked Extended Messages supported                        */
#endif /* USBPD_REV30_SUPPORT */

#define USBPD_PDO_SRC_FIXED_PEAKCURRENT_Pos          (20U)                                                             /*!< Peak Current info bits position            */
#define USBPD_PDO_SRC_FIXED_PEAKCURRENT_Msk          (0x3U << USBPD_PDO_SRC_FIXED_PEAKCURRENT_Pos)                     /*!< Peak Current info bits mask : 0x00300000   */
#define USBPD_PDO_SRC_FIXED_PEAKCURRENT_EQUAL        (USBPD_CORE_PDO_PEAKEQUAL << USBPD_PDO_SRC_FIXED_PEAKCURRENT_Pos) /*!< Peak Current info : Equal to Ioc           */
#define USBPD_PDO_SRC_FIXED_PEAKCURRENT_OVER1        (USBPD_CORE_PDO_PEAKOVER1 << USBPD_PDO_SRC_FIXED_PEAKCURRENT_Pos) /*!< Peak Current info : Overload Cap 1         */
#define USBPD_PDO_SRC_FIXED_PEAKCURRENT_OVER2        (USBPD_CORE_PDO_PEAKOVER2 << USBPD_PDO_SRC_FIXED_PEAKCURRENT_Pos) /*!< Peak Current info : Overload Cap 2         */
#define USBPD_PDO_SRC_FIXED_PEAKCURRENT_OVER3        (USBPD_CORE_PDO_PEAKOVER3 << USBPD_PDO_SRC_FIXED_PEAKCURRENT_Pos) /*!< Peak Current info : Overload Cap 3         */

#define USBPD_PDO_SRC_FIXED_VOLTAGE_Pos              (10U)                                                     /*!< Voltage in 50 mV units bits position               */
#define USBPD_PDO_SRC_FIXED_VOLTAGE_Msk              (0x3FFU << USBPD_PDO_SRC_FIXED_VOLTAGE_Pos)               /*!< Voltage in 50 mV units bits mask : 0x000FFC00      */

#define USBPD_PDO_SRC_FIXED_MAX_CURRENT_Pos          (0U)                                                      /*!< Max current in 10 mA units bits position           */
#define USBPD_PDO_SRC_FIXED_MAX_CURRENT_Msk          (0x3FFU << USBPD_PDO_SRC_FIXED_MAX_CURRENT_Pos)           /*!< Max current in 10 mA units bits mask : 0x000003FF  */

/* Source Variable type PDO elments */
#define USBPD_PDO_SRC_VARIABLE_MAX_VOLTAGE_Pos       (20U)                                                     /*!< Max Voltage in 50 mV units bits position           */
#define USBPD_PDO_SRC_VARIABLE_MAX_VOLTAGE_Msk       (0x3FFU << USBPD_PDO_SRC_VARIABLE_MAX_VOLTAGE_Pos)        /*!< Max Voltage in 50 mV units bits mask : 0x3FF00000  */

#define USBPD_PDO_SRC_VARIABLE_MIN_VOLTAGE_Pos       (10U)                                                     /*!< Max Voltage in 50 mV units bits position           */
#define USBPD_PDO_SRC_VARIABLE_MIN_VOLTAGE_Msk       (0x3FFU << USBPD_PDO_SRC_VARIABLE_MIN_VOLTAGE_Pos)        /*!< Max Voltage in 50 mV units bits mask : 0x000FFC00  */

#define USBPD_PDO_SRC_VARIABLE_MAX_CURRENT_Pos       (0U)                                                      /*!< Max current in 10 mA units bits position           */
#define USBPD_PDO_SRC_VARIABLE_MAX_CURRENT_Msk       (0x3FFU << USBPD_PDO_SRC_VARIABLE_MAX_CURRENT_Pos)        /*!< Max current in 10 mA units bits mask : 0x000003FF  */

/* Source Battery type PDO elments */
#define USBPD_PDO_SRC_BATTERY_MAX_VOLTAGE_Pos        (20U)                                                     /*!< Max Voltage in 50 mV units bits position           */
#define USBPD_PDO_SRC_BATTERY_MAX_VOLTAGE_Msk        (0x3FFU << USBPD_PDO_SRC_BATTERY_MAX_VOLTAGE_Pos)         /*!< Max Voltage in 50 mV units bits mask : 0x3FF00000  */

#define USBPD_PDO_SRC_BATTERY_MIN_VOLTAGE_Pos        (10U)                                                     /*!< Max Voltage in 50 mV units bits position           */
#define USBPD_PDO_SRC_BATTERY_MIN_VOLTAGE_Msk        (0x3FFU << USBPD_PDO_SRC_BATTERY_MIN_VOLTAGE_Pos)         /*!< Max Voltage in 50 mV units bits mask : 0x000FFC00  */

#define USBPD_PDO_SRC_BATTERY_MAX_POWER_Pos          (0U)                                                      /*!< Max allowable power in 250mW units bits position          */
#define USBPD_PDO_SRC_BATTERY_MAX_POWER_Msk          (0x3FFU << USBPD_PDO_SRC_BATTERY_MAX_POWER_Pos)           /*!< Max allowable power in 250mW units bits mask : 0x000003FF */

/* Sink Fixed type PDO elments */
#define USBPD_PDO_SNK_FIXED_DRP_SUPPORT_Pos          (29U)                                                     /*!< DRP Support bit position                        */
#define USBPD_PDO_SNK_FIXED_DRP_SUPPORT_Msk          (0x1U << USBPD_PDO_SNK_FIXED_DRP_SUPPORT_Pos)             /*!< DRP Support bit mask : 0x20000000               */
#define USBPD_PDO_SNK_FIXED_DRP_NOT_SUPPORTED        (0U)                                                      /*!< DRP not supported                               */
#define USBPD_PDO_SNK_FIXED_DRP_SUPPORTED            USBPD_PDO_SNK_FIXED_DRP_SUPPORT_Msk                       /*!< DRP supported                                   */

#define USBPD_PDO_SNK_FIXED_HIGHERCAPAB_Pos           (28U)                                                    /*!< Higher capability support bit position          */
#define USBPD_PDO_SNK_FIXED_HIGHERCAPAB_Msk           (0x1U << USBPD_PDO_SNK_FIXED_HIGHERCAPAB_Pos)            /*!< Higher capability support bit mask : 0x10000000 */
#define USBPD_PDO_SNK_FIXED_HIGHERCAPAB_NOT_SUPPORTED (0U)                                                     /*!< Higher capability not supported                 */
#define USBPD_PDO_SNK_FIXED_HIGHERCAPAB_SUPPORTED     USBPD_PDO_SNK_FIXED_HIGHERCAPAB_Msk                      /*!< Higher capability supported                     */

#define USBPD_PDO_SNK_FIXED_EXT_POWER_Pos            (27U)                                                     /*!< External Power available bit position           */
#define USBPD_PDO_SNK_FIXED_EXT_POWER_Msk            (0x1U << USBPD_PDO_SNK_FIXED_EXT_POWER_Pos)               /*!< External Power available bit mask : 0x08000000  */
#define USBPD_PDO_SNK_FIXED_EXT_POWER_NOT_AVAILABLE  (0U)                                                      /*!< External Power not available                    */
#define USBPD_PDO_SNK_FIXED_EXT_POWER_AVAILABLE      USBPD_PDO_SNK_FIXED_EXT_POWER_Msk                         /*!< External Power available                        */

#define USBPD_PDO_SNK_FIXED_USBCOMM_Pos              (26U)                                                     /*!< USB Communication Support bit position          */
#define USBPD_PDO_SNK_FIXED_USBCOMM_Msk              (0x1U << USBPD_PDO_SNK_FIXED_USBCOMM_Pos)                 /*!< USB Communication Support bit mask : 0x04000000 */
#define USBPD_PDO_SNK_FIXED_USBCOMM_NOT_SUPPORTED    (0U)                                                      /*!< USB Communication not supported                 */
#define USBPD_PDO_SNK_FIXED_USBCOMM_SUPPORTED        USBPD_PDO_SNK_FIXED_USBCOMM_Msk                           /*!< USB Communication supported                     */

#define USBPD_PDO_SNK_FIXED_DRD_SUPPORT_Pos          (25U)                                                     /*!< Dual Role Data Support bit position             */
#define USBPD_PDO_SNK_FIXED_DRD_SUPPORT_Msk          (0x1U << USBPD_PDO_SNK_FIXED_DRD_SUPPORT_Pos)             /*!< Dual Role Data Support bit mask : 0x02000000    */
#define USBPD_PDO_SNK_FIXED_DRD_NOT_SUPPORTED        (0U)                                                      /*!< Dual Role Data not supported                    */
#define USBPD_PDO_SNK_FIXED_DRD_SUPPORTED            USBPD_PDO_SNK_FIXED_DRD_SUPPORT_Msk                       /*!< Dual Role Data supported                        */

#if defined(USBPD_REV30_SUPPORT)
#define USBPD_PDO_SNK_FIXED_FRS_SUPPORT_Pos          (23U)                                                     /*!< Fast Role Swap required Current bit position             */
#define USBPD_PDO_SNK_FIXED_FRS_SUPPORT_Msk          (0x3U << USBPD_PDO_SNK_FIXED_FRS_SUPPORT_Pos)             /*!< Fast Role Swap required Current bit mask : 0x01800000    */
#define USBPD_PDO_SNK_FIXED_FRS_NOT_SUPPORTED        (0U)                                                      /*!< Fast Role Swap not supported                             */
#define USBPD_PDO_SNK_FIXED_FRS_DEFAULT              (USBPD_CORE_PDO_FRS_DEFAULT_USB_POWER << USBPD_PDO_SNK_FIXED_FRS_SUPPORT_Pos)  /*!< Fast Role Swap required default USB power  */
#define USBPD_PDO_SNK_FIXED_FRS_1_5A                 (USBPD_CORE_PDO_FRS_1_5A_5V << USBPD_PDO_SNK_FIXED_FRS_SUPPORT_Pos)            /*!< Fast Role Swap 1.5A at 5V                  */
#define USBPD_PDO_SNK_FIXED_FRS_3A                   (USBPD_CORE_PDO_FRS_3A_5V << USBPD_PDO_SNK_FIXED_FRS_SUPPORT_Pos)              /*!< Fast Role Swap 3A at 5V                    */
#endif /* USBPD_REV30_SUPPORT */

#define USBPD_PDO_SNK_FIXED_VOLTAGE_Pos              (10U)                                                     /*!< Voltage in 50 mV units bits position               */
#define USBPD_PDO_SNK_FIXED_VOLTAGE_Msk              (0x3FFU << USBPD_PDO_SNK_FIXED_VOLTAGE_Pos)               /*!< Voltage in 50 mV units bits mask : 0x000FFC00      */

#define USBPD_PDO_SNK_FIXED_OP_CURRENT_Pos           (0U)                                                      /*!< Operational current in 10 mA units bits position           */
#define USBPD_PDO_SNK_FIXED_OP_CURRENT_Msk           (0x3FFU << USBPD_PDO_SNK_FIXED_OP_CURRENT_Pos)            /*!< Operational current in 10 mA units bits mask : 0x000003FF  */

/* Sink Variable type PDO elments */
#define USBPD_PDO_SNK_VARIABLE_MAX_VOLTAGE_Pos       (20U)                                                     /*!< Max Voltage in 50 mV units bits position           */
#define USBPD_PDO_SNK_VARIABLE_MAX_VOLTAGE_Msk       (0x3FFU << USBPD_PDO_SNK_VARIABLE_MAX_VOLTAGE_Pos)        /*!< Max Voltage in 50 mV units bits mask : 0x3FF00000  */

#define USBPD_PDO_SNK_VARIABLE_MIN_VOLTAGE_Pos       (10U)                                                     /*!< Max Voltage in 50 mV units bits position           */
#define USBPD_PDO_SNK_VARIABLE_MIN_VOLTAGE_Msk       (0x3FFU << USBPD_PDO_SNK_VARIABLE_MIN_VOLTAGE_Pos)        /*!< Max Voltage in 50 mV units bits mask : 0x000FFC00  */

#define USBPD_PDO_SNK_VARIABLE_OP_CURRENT_Pos        (0U)                                                      /*!< Operational current in 10 mA units bits position           */
#define USBPD_PDO_SNK_VARIABLE_OP_CURRENT_Msk        (0x3FFU << USBPD_PDO_SNK_VARIABLE_OP_CURRENT_Pos)         /*!< Operational current in 10 mA units bits mask : 0x000003FF  */

/* Sink Battery type PDO elments */
#define USBPD_PDO_SNK_BATTERY_MAX_VOLTAGE_Pos        (20U)                                                     /*!< Max Voltage in 50 mV units bits position           */
#define USBPD_PDO_SNK_BATTERY_MAX_VOLTAGE_Msk        (0x3FFU << USBPD_PDO_SNK_BATTERY_MAX_VOLTAGE_Pos)         /*!< Max Voltage in 50 mV units bits mask : 0x3FF00000  */

#define USBPD_PDO_SNK_BATTERY_MIN_VOLTAGE_Pos        (10U)                                                     /*!< Max Voltage in 50 mV units bits position           */
#define USBPD_PDO_SNK_BATTERY_MIN_VOLTAGE_Msk        (0x3FFU << USBPD_PDO_SNK_BATTERY_MIN_VOLTAGE_Pos)         /*!< Max Voltage in 50 mV units bits mask : 0x000FFC00  */

#define USBPD_PDO_SNK_BATTERY_OP_POWER_Pos           (0U)                                                      /*!< Operational power in 250mW units bits position          */
#define USBPD_PDO_SNK_BATTERY_OP_POWER_Msk           (0x3FFU << USBPD_PDO_SNK_BATTERY_OP_POWER_Pos)            /*!< Operational power in 250mW units bits mask : 0x000003FF */

#if defined(USBPD_REV30_SUPPORT)
#ifdef USBPDCORE_PPS
/* Source APDO type PDO elments */
#define USBPD_PDO_SRC_APDO_PPS_Pos                   (28U)                                                     /*!< Programmable Power Supply bit position           */
#define USBPD_PDO_SRC_APDO_PPS_Msk                   (0x3U << USBPD_PDO_SRC_APDO_PPS_Pos)                      /*!< Programmable Power Supply bit mask : 0x300000000 */
#define USBPD_PDO_SRC_APDO_PPS                       (0U)                                                      /*!< Programmable Power Supply field value 00         */

#define USBPD_PDO_SRC_APDO_MAX_VOLTAGE_Pos           (17U)                                                     /*!< APDO Max Voltage in 100 mV increments bits position  */
#define USBPD_PDO_SRC_APDO_MAX_VOLTAGE_Msk           (0xFFU << USBPD_PDO_SRC_APDO_MAX_VOLTAGE_Pos)             /*!< APDO Max Voltage in 100 mV increments bits mask : 0x01FE0000 */

#define USBPD_PDO_SRC_APDO_MIN_VOLTAGE_Pos           (8U)                                                      /*!< APDO Min Voltage in 100 mV increments bits position  */
#define USBPD_PDO_SRC_APDO_MIN_VOLTAGE_Msk           (0xFFU << USBPD_PDO_SRC_APDO_MIN_VOLTAGE_Pos)             /*!< APDO Min Voltage in 100 mV increments bits mask : 0x0000FF00 */

#define USBPD_PDO_SRC_APDO_MAX_CURRENT_Pos           (0U)                                                      /*!< APDO Max Current in 50 mA increments bits position  */
#define USBPD_PDO_SRC_APDO_MAX_CURRENT_Msk           (0x7FU << USBPD_PDO_SRC_APDO_MAX_CURRENT_Pos)             /*!< APDO Max Current in 50 mA increments bits mask : 0x0000007F */

/* Sink APDO type PDO elments */
#define USBPD_PDO_SNK_APDO_PPS_Pos                   (28U)                                                     /*!< Programmable Power Supply bit position           */
#define USBPD_PDO_SNK_APDO_PPS_Msk                   (0x3U << USBPD_PDO_SNK_APDO_PPS_Pos)                      /*!< Programmable Power Supply bit mask : 0x300000000 */
#define USBPD_PDO_SNK_APDO_PPS                       (0U)                                                      /*!< Programmable Power Supply field value 00         */

#define USBPD_PDO_SNK_APDO_MAX_VOLTAGE_Pos           (17U)                                                     /*!< APDO Max Voltage in 100 mV increments bits position  */
#define USBPD_PDO_SNK_APDO_MAX_VOLTAGE_Msk           (0xFFU << USBPD_PDO_SNK_APDO_MAX_VOLTAGE_Pos)             /*!< APDO Max Voltage in 100 mV increments bits mask : 0x01FE0000 */

#define USBPD_PDO_SNK_APDO_MIN_VOLTAGE_Pos           (8U)                                                      /*!< APDO Min Voltage in 100 mV increments bits position  */
#define USBPD_PDO_SNK_APDO_MIN_VOLTAGE_Msk           (0xFFU << USBPD_PDO_SNK_APDO_MIN_VOLTAGE_Pos)             /*!< APDO Min Voltage in 100 mV increments bits mask : 0x0000FF00 */

#define USBPD_PDO_SNK_APDO_MAX_CURRENT_Pos           (0U)                                                      /*!< APDO Max Current in 50 mA increments bits position  */
#define USBPD_PDO_SNK_APDO_MAX_CURRENT_Msk           (0x7FU << USBPD_PDO_SNK_APDO_MAX_CURRENT_Pos)             /*!< APDO Max Current in 50 mA increments bits mask : 0x0000007F */
#endif /* USBPDCORE_PPS */
#endif /* USBPD_REV30_SUPPORT */

#define USBPD_EXTENDED_MESSAGE                       (0x80U)                                                   /*!< Flag to indicate that it is a extended message     */

#if defined(USBPD_REV30_SUPPORT)
/** @defgroup USBPD_ADO_TYPE_ALERT USB-PD Type alert definition used for Alert Data Object
  * @{
  */
#define USBPD_ADO_TYPE_ALERT_BATTERY_STATUS (1u << 1u) /*!< Battery Status Change Event(Attach/Detach/charging/discharging/idle) */
#define USBPD_ADO_TYPE_ALERT_OCP            (1u << 2u) /*!< Over-Current Protection event when set (Source only, for Sink Reserved and Shall be set to zero) */
#define USBPD_ADO_TYPE_ALERT_OTP            (1u << 3u) /*!< Over-Temperature Protection event when set  */
#define USBPD_ADO_TYPE_ALERT_OPERATING_COND (1u << 4u) /*!< Operating Condition Change when set */
#define USBPD_ADO_TYPE_ALERT_SRC_INPUT      (1u << 5u) /*!< Source Input Change Event when set */
#define USBPD_ADO_TYPE_ALERT_OVP            (1u << 6u) /*!< Over-Voltage Protection event when set (Sink only, for Source Reserved and Shall be set to zero) */
/**
  * @}
  */

/** @defgroup USBPD_ADO_FIXED_BATT USB-PD Fixed Batteries definition used for Alert Data Object
  * @{
  */
#define USBPD_ADO_FIXED_BATT_BATTERY_0 (1u << 0u) /*!< Fixed Batterie 0 had a status change */
#define USBPD_ADO_FIXED_BATT_BATTERY_1 (1u << 1u) /*!< Fixed Batterie 1 had a status change */
#define USBPD_ADO_FIXED_BATT_BATTERY_2 (1u << 2u) /*!< Fixed Batterie 2 had a status change */
#define USBPD_ADO_FIXED_BATT_BATTERY_3 (1u << 3u) /*!< Fixed Batterie 3 had a status change */
/**
  * @}
  */

/** @defgroup USBPD_ADO_HOT_SWAP_BATT USB-PD Hot Swappable Batteries definition used for Alert Data Object
  * @{
  */
#define USBPD_ADO_HOT_SWAP_BATT_BATTERY_4 (1u << 0u) /*!< Hot Swappable Batterie 4 had a status change */
#define USBPD_ADO_HOT_SWAP_BATT_BATTERY_5 (1u << 1u) /*!< Hot Swappable Batterie 5 had a status change */
#define USBPD_ADO_HOT_SWAP_BATT_BATTERY_6 (1u << 2u) /*!< Hot Swappable Batterie 6 had a status change */
#define USBPD_ADO_HOT_SWAP_BATT_BATTERY_7 (1u << 3u) /*!< Hot Swappable Batterie 7 had a status change */
/**
  * @}
  */

/** @defgroup USBPD_SDB_POWER_STATUS USB-PD Status Data Block - Power Status
  * @{
  */
#define USBPD_SDB_POWER_STATUS_CABLE                  (1u << 1u) /*!< Source power limited due to cable supported current */
#define USBPD_SDB_POWER_STATUS_INSUFFICIENT_POWER     (1u << 2u) /*!< Source power limited due to insufficient power available while sourcing other ports */
#define USBPD_SDB_POWER_STATUS_INSUFFICIENT_EXT_POWER (1u << 3u) /*!< Source power limited due to insufficient external power */
#define USBPD_SDB_POWER_STATUS_EVENT_FLAGS            (1u << 4u) /*!< Source power limited due to Event Flags in place (Event Flags must also be set) */
#define USBPD_SDB_POWER_STATUS_TEMPERATURE            (1u << 4u) /*!< Source power limited due to temperature */
/**
  * @}
  */

/** @defgroup USBPD_MANUFINFO_TARGET USB-PD Manufacturer Info Target
  * @{
  */
#define USBPD_MANUFINFO_TARGET_PORT_CABLE_PLUG 0u /*!< Manufacturer Info Target Port/Cable Plug */
#define USBPD_MANUFINFO_TARGET_BATTERY 1u         /*!< Manufacturer Info Target  Battery */
/**
  * @}
  */

/** @defgroup USBPD_MANUFINFO_REF USB-PD Manufacturer Info Ref
  * @{
  */
#define USBPD_MANUFINFO_REF_MAX_VALUES 7u /*!< Manufacturer Info Ref 0..3:Fixed Batteries and 4..7: Hot Swappable Batteries*/
/**
  * @}
  */

/** @defgroup USBPD_BSDO_BATT_INFO USB-PD Battery Status - Info
  * @{
  */
#define USBPD_BSDO_BATT_INFO_INVALID_REF              (1u << 0u) /*!<  Battery Status - Info: Invalid Battery reference */
#define USBPD_BSDO_BATT_INFO_BATT_PRESENT             (1u << 1u) /*!<  Battery Status - Info: Battery is present when set*/
#define USBPD_BSDO_BATT_INFO_BATT_ISCHARGING          (0u << 2u) /*!<  Battery Status - Info: Battery is Charging*/
#define USBPD_BSDO_BATT_INFO_BATT_ISDISCHARGING       (1u << 2u) /*!<  Battery Status - Info: Battery is Discharging*/
#define USBPD_BSDO_BATT_INFO_BATT_ISIDLE              (2u << 2u) /*!<  Battery Status - Info: Battery is Idle*/
/**
  * @}
  */

#ifdef USBPDCORE_PPS
/** @defgroup USBPD_CORE_DEF_REAL_TIME_FLAGS USBPD CORE DEF Real Time Flags
  * @{
  */
#define USBPD_PPS_REALTIMEFLAGS_PTF_NOT_SUPPORTED     (00u << 1u) /*!< PTF: 00 - Not Supported                              */
#define USBPD_PPS_REALTIMEFLAGS_PTF_NORMAL            (01u << 1u) /*!< PTF: 01 - Normal                                     */
#define USBPD_PPS_REALTIMEFLAGS_PTF_WARNING           (10u << 1u) /*!< PTF: 10 - Warning                                    */
#define USBPD_PPS_REALTIMEFLAGS_PTF_OVER_TEMPERATURE  (11u << 1u) /*!< PTF: 11 - Over temperature                           */
#define USBPD_PPS_REALTIMEFLAGS_OMF_ENABLED           (1u  << 3u) /*!< OMF set when operating in Current Foldback mode      */
#define USBPD_PPS_REALTIMEFLAGS_OMF_DISABLED          (0u  << 3u) /*!< OMF set when operating in Current Foldback mode      */
/**
  * @}
  */
#endif /* USBPDCORE_PPS */

#if defined(USBPDCORE_SNK_CAPA_EXT)
/** @defgroup USBPD_SKEDB_VERSION USB-PD Sink Capabilities Extended - SKEDB version
  * @{
  */
#define USBPD_SKEDB_VERSION_1P0                  1u /*!< Version 1.0 */
/**
  * @}
  */

/** @defgroup USBPD_SKEDB_LOADSTEP USB-PD Sink Capabilities Extended - Load Step field
  * @{
  */
#define USBPD_SKEDB_LOADSTEP_150MA               (0u << 0u) /*!< 150mA/ìs Load Step (default) */
#define USBPD_SKEDB_LOADSTEP_500MA               (1u << 0u) /*!< 500mA/ìs Load Step */
/**
  * @}
  */

/** @defgroup USBPD_SKEDB_COMPLIANCE USB-PD Sink Capabilities Extended - Compliance field
  * @{
  */
#define USBPD_SKEDB_COMPLIANCE_LPS               (1u << 0u) /*!< Requires LPS Source when set */
#define USBPD_SKEDB_COMPLIANCE_PS1               (1u << 1u) /*!< Requires PS1 Source when set */
#define USBPD_SKEDB_COMPLIANCE_PS2               (1u << 2u) /*!< Requires PS2 Source when set */
/**
  * @}
  */

/** @defgroup USBPD_SKEDB_TOUCHTEMP USB-PD Sink Capabilities Extended - Touch Temperature
  * @{
  */
#define USBPD_SKEDB_TOUCHTEMP_NA                  (0u) /*!< Temperature conforms to Not applicable          */
#define USBPD_SKEDB_TOUCHTEMP_DEFAULT             (1u) /*!< Temperature conforms to [IEC 60950-1] (default) */
#define USBPD_SKEDB_TOUCHTEMP_TS1                 (2u) /*!< Temperature conforms to [IEC 62368-1] TS1       */
#define USBPD_SKEDB_TOUCHTEMP_TS2                 (2u) /*!< Temperature conforms to [IEC 62368-1] TS2       */
/**
  * @}
  */

/** @defgroup USBPD_SKEDB_SINKMODES USB-PD Sink Capabilities Extended - Sink Modes
  * @{
  */
#define USBPD_SKEDB_SINKMODES_PPS                 (1u << 0u) /*!< 1: PPS charging supported         */
#define USBPD_SKEDB_SINKMODES_VBUS                (1u << 1u) /*!< 1: VBUS powered                   */
#define USBPD_SKEDB_SINKMODES_MAINS               (1u << 2u) /*!< 1: Mains powered                  */
#define USBPD_SKEDB_SINKMODES_BATPOW              (1u << 3u) /*!< 1: Battery powered                */
#define USBPD_SKEDB_SINKMODES_BATUNL              (1u << 4u) /*!< 1: Battery essentially unlimited  */
/**
  * @}
  */
#endif /* USBPDCORE_SNK_CAPA_EXT */

#if defined(USBPDCORE_FWUPD)

/** @defgroup USBPD_FWUPD_MSGTYPE USB-PD Firmware Update Message Request and Responses Defines
  * @{
  */

typedef enum
{
  USBPD_FWUPD_MSGTYPE_NONE                = 0x00u, /*!< Reserved value                                                 */
  USBPD_FWUPD_MSGTYPE_RSP_GET_FW_ID       = 0x01u, /*!< Response is used to respond to a GET_FW_ID Request             */
  USBPD_FWUPD_MSGTYPE_RSP_PDFU_INITIATE   = 0x02u, /*!< Response is used to respond to a PDFU_INITIATE Request         */
  USBPD_FWUPD_MSGTYPE_RSP_PDFU_DATA       = 0x03u, /*!< Response is used to respond to a PDFU_DATA Request             */
  USBPD_FWUPD_MSGTYPE_RSP_PDFU_VALIDATE   = 0x05u, /*!< Response is used to respond to a PDFU_VALIDATE Request         */
  USBPD_FWUPD_MSGTYPE_RSP_PDFU_DATA_PAUSE = 0x07u, /*!< Response is used to respond to a PDFU_DATA_PAUSE Request       */
  USBPD_FWUPD_MSGTYPE_RSP_VENDOR_SPECIFIC = 0x7Fu, /*!< Response is used to respond to a VENDOR_SPECIFIC Request       */
  USBPD_FWUPD_MSGTYPE_REQ_GET_FW_ID       = 0x81u, /*!< Request is used to retrieve information about a PDFU Responder
                                                        and determine if a firmware update is necessary                */
  USBPD_FWUPD_MSGTYPE_REQ_PDFU_INITIATE   = 0x82u, /*!< Request is used to initiate firmware update                    */
  USBPD_FWUPD_MSGTYPE_REQ_PDFU_DATA       = 0x83u, /*!< Request is used to transfer a data block from a firmware image
                                                        (response required)                                            */
  USBPD_FWUPD_MSGTYPE_REQ_PDFU_DATA_NR    = 0x84u, /*!< Request is used to transfer a data block from a firmware image
                                                        (response not required)                                        */
  USBPD_FWUPD_MSGTYPE_REQ_PDFU_VALIDATE   = 0x85u, /*!< Request is used to request validation of a firmware image      */
  USBPD_FWUPD_MSGTYPE_REQ_PDFU_ABORT      = 0x86u, /*!< Request is used to end firmware image update prematurely       */
  USBPD_FWUPD_MSGTYPE_REQ_PDFU_DATA_PAUSE = 0x87u, /*!< Request is used to pause a firmware image update               */
  USBPD_FWUPD_MSGTYPE_REQ_VENDOR_SPECIFIC = 0xFFu, /*!< Request is for vendor-specific use                             */
}
USBPD_FWUPD_MsgType_TypeDef;

/**
  * @}
  */

/** @defgroup USBPD_FWUPD_PROT_VER USB-PD Firmware Update Message Protocol version Defines
  * @{
  */

#define  USBPD_FWUPD_PROT_VER_V1P0                0x01u /*!< USB PD Firmware Update Protocol Version 1.0 */

/**
  * @}
  */

/** @defgroup USBPD_FWUPD_SIZE_PAYLOAD USB-PD Firmware Update Size Payload Defines
  * @{
  */

#define  USBPD_FWUPD_SIZE_PAYLOAD_RSP_GET_FW_ID        (sizeof(USBPD_FWUPD_GetFwIDRspPayload_TypeDef)) /*!< Payload size of Response is used to respond to a GET_FW_ID Request             */
#define  USBPD_FWUPD_SIZE_PAYLOAD_RSP_PDFU_INITIATE    (sizeof(USBPD_FWUPD_PdfuInitRspPayload_TypeDef)) /*!< Payload size of Response is used to respond to a PDFU_INITIATE Request         */
#define  USBPD_FWUPD_SIZE_PAYLOAD_RSP_PDFU_DATA        (sizeof(USBPD_FWUPD_PdfuDataRspPayload_TypeDef)) /*!< Payload size of Response is used to respond to a PDFU_DATA Request             */
#define  USBPD_FWUPD_SIZE_PAYLOAD_RSP_PDFU_VALIDATE    (sizeof(USBPD_FWUPD_PdfuValidateRspPayload_TypeDef)) /*!< Payload size of Response is used to respond to a PDFU_VALIDATE Request         */
#define  USBPD_FWUPD_SIZE_PAYLOAD_RSP_PDFU_DATA_PAUSE  (sizeof(USBPD_FWUPD_PdfuDataPauseRspPayload_TypeDef)) /*!< Payload size of Response is used to respond to a PDFU_DATA_PAUSE Request       */
#define  USBPD_FWUPD_SIZE_PAYLOAD_RSP_VENDOR_SPECIFIC  (sizeof(USBPD_FWUPD_VendorSpecificRspPayload_TypeDef)) /*!< Payload size of Response is used to respond to a VENDOR_SPECIFIC Request       */
#define  USBPD_FWUPD_SIZE_PAYLOAD_REQ_GET_FW_ID        0u /*!< Payload size of Request is used to retrieve information */
#define  USBPD_FWUPD_SIZE_PAYLOAD_REQ_PDFU_INITIATE    8u /*!< Payload size of Request is used to initiate firmware update                    */
#define  USBPD_FWUPD_SIZE_PAYLOAD_REQ_PDFU_DATA_MAX    (2u + 256u) /*!< Payload size of Request is used to transfer a data block from a firmware image
                                                             (response required)                                            */
#define  USBPD_FWUPD_SIZE_PAYLOAD_REQ_PDFU_DATA_NR_MAX (2u + 256u) /*!< Payload size of Request is used to transfer a data block from a firmware image
                                                             (response not required)                                        */
#define  USBPD_FWUPD_SIZE_PAYLOAD_REQ_PDFU_VALIDATE    0u /*!< Payload size of Request is used to request validation of a firmware image      */
#define  USBPD_FWUPD_SIZE_PAYLOAD_REQ_PDFU_ABORT       0u /*!< Payload size of Request is used to end firmware image update prematurely       */
#define  USBPD_FWUPD_SIZE_PAYLOAD_REQ_PDFU_DATA_PAUSE  0u /*!< Payload size of Request is used to pause a firmware image update               */
#define  USBPD_FWUPD_SIZE_PAYLOAD_REQ_VENDOR_SPECIFIC_MAX  (2u + 256u) /*!< Payload size of Request is for vendor-specific use                */

/**
  * @}
  */
#endif /* USBPDCORE_FWUPD */
#endif /* USBPD_REV30_SUPPORT */
/**
  * @}
  */

/** @defgroup USBPD_SupportedSOP_TypeDef USB PD Supported SOP Types structure definition
  * @{
  */
#define USBPD_SUPPORTED_SOP_NONE       (     0u) /*<! Not supported */
#define USBPD_SUPPORTED_SOP_SOP        (1u << 0u) /*<! SOP           */
#define USBPD_SUPPORTED_SOP_SOP1       (1u << 1u) /*<! SOP '         */
#define USBPD_SUPPORTED_SOP_SOP2       (1u << 2u) /*<! SOP ''        */
#define USBPD_SUPPORTED_SOP_SOP1_DEBUG (1u << 3u) /*<! SOP' Debug    */
#define USBPD_SUPPORTED_SOP_SOP2_DEBUG (1u << 4u) /*<! SOP '' Debug  */

typedef uint32_t USBPD_SupportedSOP_TypeDef;

/**
  * @}
  */

/** @defgroup USBPD_CORE_PE_ET_HR_STATUS USBPD CORE PE Hard Reset Status
  * @{
  */
typedef enum
{
  USBPD_HR_STATUS_START_ACK,
  USBPD_HR_STATUS_START_REQ,
  USBPD_HR_STATUS_MSG_SENT,
  USBPD_HR_STATUS_WAIT_VBUS_VSAFE0V,
  USBPD_HR_STATUS_COMPLETED,
}
USBPD_HR_Status_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PE_ET_PRS_STATUS USBPD CORE PE Power Role Swap status
  * @{
  */
typedef enum
{
  USBPD_PRS_STATUS_NA,
  USBPD_PRS_STATUS_START_ACK,
  USBPD_PRS_STATUS_START_REQ,
  USBPD_PRS_STATUS_ACCEPTED,
  USBPD_PRS_STATUS_REJECTED,
  USBPD_PRS_STATUS_WAIT,
  USBPD_PRS_STATUS_VBUS_OFF,
  USBPD_PRS_STATUS_SRC_RP2RD,
  USBPD_PRS_STATUS_SRC_PS_READY_SENT,
  USBPD_PRS_STATUS_SNK_PS_READY_RECEIVED,
  USBPD_PRS_STATUS_SNK_RD2RP,
  USBPD_PRS_STATUS_VBUS_ON,
  USBPD_PRS_STATUS_SNK_PS_READY_SENT,
  USBPD_PRS_STATUS_SRC_PS_READY_RECEIVED,
  USBPD_PRS_STATUS_COMPLETED,
  USBPD_PRS_STATUS_FAILED,
  USBPD_PRS_STATUS_ABORTED,
} USBPD_PRS_Status_TypeDef;
/**
  * @}
  */

/**
  * @brief Status of VSafe
  * @{
  */
typedef enum
{
  USBPD_VSAFE_0V          , /*!< USBPD VSAFE0V   */
  USBPD_VSAFE_5V          , /*!< USBPD VSAFE5V   */
}
USBPD_VSAFE_StatusTypeDef;
/**
  * @}
  */

/**
  * @brief USB Power Delivery Status structures definition
  */
typedef enum
{
  USBPD_OK,
  USBPD_NOTSUPPORTED,
  USBPD_ERROR,
  USBPD_BUSY,
  USBPD_TIMEOUT,

  /* PRL status */
  USBPD_PRL_GOODCRC,
  USBPD_PRL_DISCARDED,
  USBPD_PRL_SOFTRESET,
  USBPD_PRL_CABLERESET,
#if defined(USBPD_REV30_SUPPORT)
  USBPD_PRL_SNKTX,
#endif /* USBPD_REV30_SUPPORT */

  /* Message repply */
  USBPD_ACCEPT,
  USBPD_GOTOMIN,
  USBPD_REJECT,
  USBPD_WAIT,
  USBPD_NAK,
  USBPD_ACK,

  USBPD_FAIL,
  USBPD_RXEVENT_SOP,
  USBPD_RXEVENT_SOP1,
  USBPD_RXEVENT_SOP2,
  USBPD_NOEVENT,
  USBPD_DISCARDRX,

  USBPD_MALLOCERROR,
#if defined(USBPDCORE_FWUPD)
  USBPD_PDFU_NODATA,
  USBPD_PDFU_PAUSE,
  USBPD_PDFU_RESUME,
#endif /* USBPDCORE_FWUPD */
}
USBPD_StatusTypeDef;

/**
  * @brief USB PD CC lines structures definition
  */
#define CCNONE                          0x00u
#define CC1                             0x01u
#define CC2                             0x02u

typedef uint32_t CCxPin_TypeDef;


/** @defgroup USBPD_SpecRev_TypeDef USB PD Specification Revision structure definition
  * @brief  USB PD Specification Revision structure definition
  * @{
  */
#define USBPD_SPECIFICATION_REV1        0x00u  /*!< Revision 1.0      */
#define USBPD_SPECIFICATION_REV2        0x01u  /*!< Revision 2.0      */
#define USBPD_SPECIFICATION_REV3        0x02u  /*!< Revision 3.0      */

typedef uint32_t USBPD_SpecRev_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_SpecRev_TypeDef USB PD Specification Revision structure definition
  * @brief  USB PD Specification Revision structure definition
  * @{
  */
#define USBPD_VDMVERSION_REV1           0x00u  /*!< Revision 1.0      */
#define USBPD_VDMVERSION_REV2           0x01u  /*!< Revision 2.0 only used if USBPD_SPECIFICATION_REV3 */

typedef uint32_t USBPD_VDMVersion_TypeDef;
/**
  * @}
  */

/**
  * @brief CAD event value
  * @{
  */
typedef enum
{
  USBPD_CAD_EVENT_NONE      = 0u  ,         /*!< USBPD CAD event None                                */
  USBPD_CAD_EVENT_DETACHED  = 1u  ,         /*!< USBPD CAD event No cable detected                   */
  USBPD_CAD_EVENT_ATTACHED  = 2u  ,         /*!< USBPD CAD event Port partner attached               */
  USBPD_CAD_EVENT_EMC       = 3u  ,         /*!< USBPD CAD event Electronically Marked Cable detected*/
  USBPD_CAD_EVENT_ATTEMC    = 4u  ,         /*!< USBPD CAD event Port Partner detected throug EMC    */
  USBPD_CAD_EVENT_ACCESSORY = 5u  ,         /*!< USBPD CAD event Accessory detected                  */
  USBPD_CAD_EVENT_DEBUG     = 6u  ,         /*!< USBPD CAD event Debug detected                      */
/*  USBPD_CAD_EVENT_LEGACY    = 7u  ,*/         /*!< USBPD CAD event legacy cables detected              */
  USPPD_CAD_EVENT_UNKNOW    = 8u            /*!< USBPD CAD event unknow                              */
} USBPD_CAD_EVENT;
/**
  * @}
  */
/** @defgroup USBPD_PortDataRole_TypeDef USB PD Port Data Role Types structure definition
  * @brief  USB PD Port Data Role Types structure definition
  * @{
  */
#define USBPD_PORTDATAROLE_UFP                  0x00u  /*!< UFP        */
#define USBPD_PORTDATAROLE_SOP1_SOP2            USBPD_PORTDATAROLE_UFP  /*!<  For all other SOP* Packets the Port Data Role
                                                                          field is Reserved and shall be set to zero.  */
#define USBPD_PORTDATAROLE_DFP                  0x01u   /*!< DFP        */

typedef uint32_t USBPD_PortDataRole_TypeDef;
/**
  * @}
  */

/**
  * @brief  USB PD Control Message Types structure definition
  *
  */
typedef enum
{
  USBPD_CONTROLMSG_GOODCRC               = 0x01u,  /*!< GoodCRC Control Message         */
  USBPD_CONTROLMSG_GOTOMIN               = 0x02u,  /*!< GotoMin Control Message         */
  USBPD_CONTROLMSG_ACCEPT                = 0x03u,  /*!< Accept Control Message          */
  USBPD_CONTROLMSG_REJECT                = 0x04u,  /*!< Reject Control Message          */
  USBPD_CONTROLMSG_PING                  = 0x05u,  /*!< Ping Control Message            */
  USBPD_CONTROLMSG_PS_RDY                = 0x06u,  /*!< PS_RDY Control Message          */
  USBPD_CONTROLMSG_GET_SRC_CAP           = 0x07u,  /*!< Get_Source_Cap Control Message  */
  USBPD_CONTROLMSG_GET_SNK_CAP           = 0x08u,  /*!< Get_Sink_Cap Control Message    */
  USBPD_CONTROLMSG_DR_SWAP               = 0x09u,  /*!< DR_Swap Control Message         */
  USBPD_CONTROLMSG_PR_SWAP               = 0x0Au,  /*!< PR_Swap Control Message         */
  USBPD_CONTROLMSG_VCONN_SWAP            = 0x0Bu,  /*!< VCONN_Swap Control Message      */
  USBPD_CONTROLMSG_WAIT                  = 0x0Cu,  /*!< Wait Control Message            */
  USBPD_CONTROLMSG_SOFT_RESET            = 0x0Du,  /*!< Soft_Reset Control Message      */
#if defined(USBPD_REV30_SUPPORT)
  USBPD_CONTROLMSG_NOT_SUPPORTED         = 0x10u,  /*!< Not supported                   */
  USBPD_CONTROLMSG_GET_SRC_CAPEXT        = 0x11u,  /*!< Get source capability extended  */
  USBPD_CONTROLMSG_GET_STATUS            = 0x12u,  /*!< Get status                      */
  USBPD_CONTROLMSG_FR_SWAP               = 0x13u,  /*!< Fast role swap                  */
  USBPD_CONTROLMSG_GET_PPS_STATUS        = 0x14u,  /*!< Get PPS Status                  */
  USBPD_CONTROLMSG_GET_COUNTRY_CODES     = 0x15u,  /*!< Get Country codes               */
#if defined(USBPDCORE_SNK_CAPA_EXT)
  USBPD_CONTROLMSG_GET_SNK_CAPEXT        = 0x16u,  /*!< Get Sink Capability extended    */
#endif /* USBPDCORE_SNK_CAPA_EXT */
#endif /* USBPD_REV30_SUPPORT */
}USBPD_ControlMsg_TypeDef;

/**
  * @brief  USB PD Data Message Types structure definition
  *
  */
typedef enum
{
  USBPD_DATAMSG_SRC_CAPABILITIES         = 0x01u,  /*!< Source Capabilities Data Message  */
  USBPD_DATAMSG_REQUEST                  = 0x02u,  /*!< Request Data Message              */
  USBPD_DATAMSG_BIST                     = 0x03u,  /*!< BIST Data Message                 */
  USBPD_DATAMSG_SNK_CAPABILITIES         = 0x04u,  /*!< Sink_Capabilities Data Message    */
#if defined(USBPD_REV30_SUPPORT)
  USBPD_DATAMSG_BATTERY_STATUS           = 0x05u,  /*!< Battery status                    */
  USBPD_DATAMSG_ALERT                    = 0x06u,  /*!< Alert                             */
  USBPD_DATAMSG_GET_COUNTRY_INFO         = 0x07u,  /*!< Get country info                  */
#endif /* USBPD_REV30_SUPPORT */
  USBPD_DATAMSG_VENDOR_DEFINED           = 0x0Fu   /*!< Vendor_Defined Data Message       */
}USBPD_DataMsg_TypeDef;

/**
  * @brief Sink CC pins Multiple Source Current Advertisements
  */
#define vRd_Undefined     0x00u    /*!< Port Power Role Source   */
#define vRd_USB           0x01u    /*!< Default USB Power   */
#define vRd_1_5A          0x02u    /*!< USB Type-C Current @ 1.5 A   */
#define vRd_3_0A          0x03u    /*!< USB Type-C Current @ 3 A   */

typedef uint32_t CAD_SNK_Source_Current_Adv_Typedef;


/**
  * @brief Sink CC pins Multiple Source Current Advertisements
  */
#define vRp_Default             0x00u    /*!< Default USB Power   */
#define vRp_1_5A                0x01u    /*!< USB Type-C Current @ 1.5 A   */
#define vRp_3_0A                0x02u    /*!< USB Type-C Current @ 3 A   */

typedef uint32_t CAD_RP_Source_Current_Adv_Typedef;

/**
  * @brief USB PD SOP Message Types Structure definition
  */
typedef enum
{
  USBPD_SOPTYPE_SOP            = 0u,     /**< SOP*  MESSAGES               */
  USBPD_SOPTYPE_SOP1           = 1u,     /**< SOP'  MESSAGES               */
  USBPD_SOPTYPE_SOP2           = 2u,     /**< SOP'' MESSAGES               */
  USBPD_SOPTYPE_SOP1_DEBUG     = 3u,     /**< SOP'  DEBUG_MESSAGES         */
  USBPD_SOPTYPE_SOP2_DEBUG     = 4u,     /**< SOP'' DEBUG_MESSAGES         */
  USBPD_SOPTYPE_HARD_RESET     = 5u,     /**< HARD RESET MESSAGE           */
  USBPD_SOPTYPE_CABLE_RESET    = 6u,     /**< CABLE RESET MESSAGE          */
  USBPD_SOPTYPE_BIST_MODE_2    = 7u,     /**< BIST_MODE2 MESSAGE           */
  USBPD_SOPTYPE_INVALID        = 0xFFu,  /**< Invalid type                 */
} USBPD_SOPType_TypeDef;

/**
  * @brief USB funtionnal state Types enum definition
  *
  */
typedef enum
{
  USBPD_DISABLE = 0u,
  USBPD_ENABLE = !USBPD_DISABLE
} USBPD_FunctionalState;


/**
  * @brief USB PD Port Power Role Types structure definition
  *
  */
#define USBPD_CABLEPLUG_FROMDFPUFP      0x00u                           /*!< Message originated from a DFP or UFP    */
#define USBPD_PORTPOWERROLE_SNK         USBPD_CABLEPLUG_FROMDFPUFP      /*!< Sink                                    */
#define USBPD_CABLEPLUG_FROMCABLEPLUG   0x01u                           /*!< Message originated from a Cable Plug    */
#define USBPD_PORTPOWERROLE_SRC         USBPD_CABLEPLUG_FROMCABLEPLUG   /*!< Source                                  */

typedef uint32_t USBPD_PortPowerRole_TypeDef;

/**
  * @brief  USB PD Extended Message Types structure definition
  *
  */
#define USBPD_EXT_NONE                  0x00u
#define USBPD_EXT_SOURCE_CAPABILITIES   0x01u  /*!< sent by Source or Dual-Role Power    - SOP only  */
#define USBPD_EXT_STATUS                0x02u  /*!< sent by Source                       - SOP only  */
#define USBPD_EXT_GET_BATTERY_CAP       0x03u  /*!< sent by Source or Sink               - SOP only  */
#define USBPD_EXT_GET_BATTERY_STATUS    0x04u  /*!< sent by Source or Sink               - SOP only  */
#define USBPD_EXT_BATTERY_CAPABILITIES  0x05u  /*!< sent by Source or Sink               - SOP only  */
#define USBPD_EXT_GET_MANUFACTURER_INFO 0x06u  /*!< sent by Source or Sink or Cable Plug - SOP*      */
#define USBPD_EXT_MANUFACTURER_INFO     0x07u  /*!< sent by Source or Sink or Cable Plug - SOP*      */
#define USBPD_EXT_SECURITY_REQUEST      0x08u  /*!< sent by Source or Sink               - SOP*      */
#define USBPD_EXT_SECURITY_RESPONSE     0x09u  /*!< sent by Source or Sink or Cable Plug - SOP*      */
#define USBPD_EXT_FIRM_UPDATE_REQUEST   0x0Au  /*!< sent by Source or Sink               - SOP*      */
#define USBPD_EXT_FIRM_UPDATE_RESPONSE  0x0Bu  /*!< sent by Source or Sink or Cable Plug - SOP*      */
#define USBPD_EXT_PPS_STATUS            0x0Cu  /*!< sent by Source                       - SOP only  */
#define USBPD_EXT_COUNTRY_INFO          0x0Du  /*!< sent by Source or Sink               - SOP only  */
#define USBPD_EXT_COUNTRY_CODES         0x0Eu  /*!< sent by Source or Sink               - SOP only  */
#if defined(USBPDCORE_SNK_CAPA_EXT)
#define USBPD_EXT_SINK_CAPABILITIES     0x0Fu  /*!< sent by Sink or Dual-Role Power      - SOP only  */
#endif /* USBPDCORE_SNK_CAPA_EXT */

typedef uint8_t USBPD_ExtendedMsg_TypeDef;

/**
  * @brief  USB PD BIST Mode Types structure definition
  *
  */
typedef enum
{
  USBPD_BIST_RECEIVER_MODE               = 0x00u,  /*!< BIST Receiver Mode      */
  USBPD_BIST_TRANSMIT_MODE               = 0x01u,  /*!< BIST Transmit Mode      */
  USBPD_RETURNED_BIST_COUNTERS           = 0x02u,  /*!< Returned BIST Counters  */
  USBPD_BIST_CARRIER_MODE0               = 0x03u,  /*!< BIST Carrier Mode 0     */
  USBPD_BIST_CARRIER_MODE1               = 0x04u,  /*!< BIST Carrier Mode 1     */
  USBPD_BIST_CARRIER_MODE2               = 0x05u,  /*!< BIST Carrier Mode 2     */
  USBPD_BIST_CARRIER_MODE3               = 0x06u,  /*!< BIST Carrier Mode 3     */
  USBPD_BIST_EYE_PATTERN                 = 0x07u,  /*!< BIST Eye Pattern        */
  USBPD_BIST_TEST_DATA                   = 0x08u   /*!< BIST Test Data          */
} USBPD_BISTMsg_TypeDef;

/** @defgroup USBPD_CORE_PDO_Type_TypeDef PDO type definition
  * @brief  PDO type values in PDO definition
  * @{
  */
#define USBPD_CORE_PDO_TYPE_FIXED       0x00u            /*!< Fixed Supply PDO                             */
#define USBPD_CORE_PDO_TYPE_BATTERY     0x01u            /*!< Battery Supply PDO                           */
#define USBPD_CORE_PDO_TYPE_VARIABLE    0x02u            /*!< Variable Supply (non-battery) PDO            */
#if defined(USBPD_REV30_SUPPORT) && defined(USBPDCORE_PPS)
#define USBPD_CORE_PDO_TYPE_APDO        0x03u            /*!< Augmented Power Data Object (APDO)           */
#endif /*_USBPD_REV30_SUPPORT && PPS*/

typedef uint32_t USBPD_CORE_PDO_Type_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_POWER_Type_TypeDef USB-PD power state
  * @brief  USB Power state
  * @{
  */
#define USBPD_POWER_NO                  0x0u /*!< No power contract                      */
#define USBPD_POWER_DEFAULT5V           0x1u /*!< Default 5V                             */
#define USBPD_POWER_IMPLICITCONTRACT    0x2u /*!< Implicit contract                      */
#define USBPD_POWER_EXPLICITCONTRACT    0x3u /*!< Explicit contract                      */
#define USBPD_POWER_TRANSITION          0x4u /*!< Power transition                       */

typedef uint32_t USBPD_POWER_StateTypedef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PDO_DRPowerSupport_TypeDef DRP Support type
  * @brief  DRP support values in PDO definition (Source or Sink)
  * @{
  */
#define USBPD_CORE_PDO_DRP_NOT_SUPPORTED        0x00u          /*!< Dual Role Power not supported                */
#define USBPD_CORE_PDO_DRP_SUPPORTED            0x01u          /*!< Dual Role Power supported                    */

typedef uint32_t USBPD_CORE_PDO_DRPowerSupport_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PDO_USBSuspendSupport_TypeDef USB Suspend type
  * @brief  USB Suspend support values in PDO definition (Source)
  * @{
  */
#define USBPD_CORE_PDO_USBSUSP_NOT_SUPPORTED    0x00u      /*!< USB Suspend not supported                    */
#define USBPD_CORE_PDO_USBSUSP_SUPPORTED        0x01u      /*!< USB Suspend supported                        */

typedef uint32_t USBPD_CORE_PDO_USBSuspendSupport_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PDO_ExtPowered_TypeDef Externally Powered type
  * @brief  Fixed Power Source Externally Powered indication values in PDO definition (Source or Sink)
  * @{
  */
#define USBPD_CORE_PDO_NOT_EXT_POWERED  0x00u            /*!< No external power source is available        */
#define USBPD_CORE_PDO_EXT_POWERED      0x01u            /*!< External power source is available           */

typedef uint32_t USBPD_CORE_PDO_ExtPowered_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PDO_USBCommCapable_TypeDef USB Communication capability type
  * @brief  USB Communication capability over USB Data lines indication values in PDO definition (Source or Sink)
  * @{
  */
#define USBPD_CORE_PDO_USBCOMM_NOT_CAPABLE      0x00u        /*!< Device not capable of communication over USB Data lines */
#define USBPD_CORE_PDO_USBCOMM_CAPABLE          0x01u        /*!< Device capable of communication over USB Data lines     */

typedef uint32_t USBPD_CORE_PDO_USBCommCapable_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PDO_DRDataSupport_TypeDef Dual Role Data Support type
  * @brief  Dual Role Data support values in PDO definition (Source or Sink)
  * @{
  */
#define USBPD_CORE_PDO_DRD_NOT_SUPPORTED        0x00u          /*!< Dual Role Data not supported                 */
#define USBPD_CORE_PDO_DRD_SUPPORTED            0x01u          /*!< Dual Role Data supported                     */

typedef uint32_t USBPD_CORE_PDO_DRDataSupport_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PDO_PeakCurr_TypeDef Peak Current Capability type
  * @brief  Fixed Power Source Peak Current Capability type structure definition (Source)
  * @{
  */
#define   USBPD_CORE_PDO_PEAKEQUAL 0x00u                  /*!< Peak current equals                          */
#define   USBPD_CORE_PDO_PEAKOVER1 0x01u                  /*!< Overload Capabilities:
  1. Peak current equals 150% IOC for 1ms @ 5% duty cycle (low current equals 97% IOC for 19ms)
  2. Peak current equals 125% IOC for 2ms @ 10% duty cycle (low current equals 97% IOC for 18ms)
  3. Peak current equals 110% IOC for 10ms @ 50% duty cycle (low current equals 90% IOC for 10ms */
#define   USBPD_CORE_PDO_PEAKOVER2 0x02u,                  /*!< Overload Capabilities:
  1. Peak current equals 200% IOC for 1ms @ 5% duty cycle (low current equals 95% IOC for 19ms)
  2. Peak current equals 150% IOC for 2ms @ 10% duty cycle (low current equals 94% IOC for 18ms)
  3. Peak current equals 125% IOC for 10ms @ 50% duty cycle (low current equals 75% IOC for 10ms)*/
#define   USBPD_CORE_PDO_PEAKOVER3 0x03u                  /*!< Overload Capabilities:
  1. Peak current equals 200% IOC for 1ms @ 5% duty cycle (low current equals 95% IOC for 19ms)
  2. Peak current equals 175% IOC for 2ms @ 10% duty cycle (low current equals 92% IOC for 18ms)
  3. Peak current equals 150% IOC for 10ms @ 50% duty cycle (low current equals 50% IOC for 10ms)*/

typedef uint32_t USBPD_CORE_PDO_PeakCurr_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PDO_HigherCapability_TypeDef USB Higher Capability type
  * @brief  Values in PDO definition (Sink) indicating if Sink needs more than vSafe5V to provide full functionality
  * @{
  */
typedef enum
{
  USBPD_CORE_PDO_NO_HIGHER_CAPABILITY  = 0x00u,      /*!< No need for more than vSafe5V to provide full functionality */
  USBPD_CORE_PDO_HIGHER_CAPABILITY     = 0x01u,      /*!< Sink needs more than vSafe5V to provide full functionality  */
} USBPD_CORE_PDO_HigherCapability_TypeDef;
/**
  * @}
  */

#if defined(USBPD_REV30_SUPPORT)
/** @defgroup USBPD_CORE_PDO_UnchunkSupport_TypeDef Unchunked Extended Messages Support type
  * @brief  Unchunked Extended Messages support values in PDO definition (Source)
  * @{
  */
typedef enum
{
  USBPD_CORE_PDO_UNCHUNK_NOT_SUPPORTED = 0x00u,      /*!< Unchunked Extended Messages not supported    */
  USBPD_CORE_PDO_UNCHUNK_SUPPORTED     = 0x01u,      /*!< Unchunked Extended Messages supported        */
} USBPD_CORE_PDO_UnchunkSupport_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_PDO_FastRoleSwapRequiredCurrent_TypeDef Fast Role Swap Required current type
  * @brief  Fast Role Swap Required Current values in PDO definition (Sink)
  * @{
  */
typedef enum
{
  USBPD_CORE_PDO_FRS_NOT_SUPPORTED      = 0x00u,     /*!< Fast Role Swap not supported    */
  USBPD_CORE_PDO_FRS_DEFAULT_USB_POWER  = 0x01u,     /*!< Default USB Power               */
  USBPD_CORE_PDO_FRS_1_5A_5V            = 0x02u,     /*!< 1_5A at 5V supported            */
  USBPD_CORE_PDO_FRS_3A_5V              = 0x03u,     /*!< 3A at 5V supported              */
} USBPD_CORE_PDO_FastRoleSwapRequiredCurrent_TypeDef;
/**
  * @}
  */
#endif /*_USBPD_REV30_SUPPORT*/

/** @defgroup USBPD_NotifyEventValue_TypeDef USBPD notification event type value
  * @brief notification envent used inside PE callbacks (USBPD_PE_NotifyDPM) to inform DPM
  * @{
  */
typedef enum
{
  USBPD_NOTIFY_REQUEST_ACCEPTED        = 1u,
  USBPD_NOTIFY_REQUEST_REJECTED        = 2u,
  USBPD_NOTIFY_REQUEST_WAIT            = 3u,
  USBPD_NOTIFY_REQUEST_GOTOMIN         = 4u,
  USBPD_NOTIFY_GETSNKCAP_SENT          = 5u,
  USBPD_NOTIFY_GETSNKCAP_RECEIVED      = 6u,
  USBPD_NOTIFY_GETSNKCAP_ACCEPTED      = 7u,
  USBPD_NOTIFY_GETSNKCAP_REJECTED      = 8u,
  USBPD_NOTIFY_GETSNKCAP_TIMEOUT       = 9u,
  USBPD_NOTIFY_SNKCAP_SENT             =10u,
  USBPD_NOTIFY_GETSRCCAP_SENT          =11u,
  USBPD_NOTIFY_GETSRCCAP_RECEIVED      =12u,
  USBPD_NOTIFY_GETSRCCAP_ACCEPTED      =13u,
  USBPD_NOTIFY_GETSRCCAP_REJECTED      =14u,
  USBPD_NOTIFY_SRCCAP_SENT             =15u,
  USBPD_NOTIFY_POWER_EXPLICIT_CONTRACT =16u,
  USBPD_NOTIFY_POWER_SRC_READY         =17u,
  USBPD_NOTIFY_POWER_SNK_READY         =18u,
  USBPD_NOTIFY_POWER_SNK_STOP          =19u,
  USBPD_NOTIFY_POWER_SWAP_TO_SNK_DONE  =20u,
  USBPD_NOTIFY_POWER_SWAP_TO_SRC_DONE  =21u,
  USBPD_NOTIFY_POWER_SWAP_REJ          =22u,
  USBPD_NOTIFY_POWER_SWAP_NOT_SUPPORTED=23u,
  USBPD_NOTIFY_RESISTOR_ASSERT_RP      =24u,
  USBPD_NOTIFY_RESISTOR_ASSERT_RD      =25u,
  USBPD_NOTIFY_SVDM_ACK                =26u,
  USBPD_NOTIFY_SVDM_NACK               =27u,
  USBPD_NOTIFY_SVDM_BUSY               =28u,
  USBPD_NOTIFY_SVDM_TIMEOUT            =29u,
  USBPD_NOTIFY_HARDRESET_RX            =30u,
  USBPD_NOTIFY_HARDRESET_TX            =31u,
  USBPD_NOTIFY_STATE_SNK_READY         =32u,
  USBPD_NOTIFY_STATE_SRC_DISABLED      =33u,
  USBPD_NOTIFY_DATAROLESWAP_SENT       =34u,
  USBPD_NOTIFY_DATAROLESWAP_RECEIVED   =35u,
  USBPD_NOTIFY_DATAROLESWAP_UFP        =36u,
  USBPD_NOTIFY_DATAROLESWAP_DFP        =37u,
  USBPD_NOTIFY_DATAROLESWAP_WAIT       =38u,
  USBPD_NOTIFY_DATAROLESWAP_REJECTED   =39u,
  USBPD_NOTIFY_DATAROLESWAP_NOT_SUPPORTED=40u,
  USBPD_NOTIFY_GOTOMIN_SENT            =41u,
  USBPD_NOTIFY_GOTOMIN_POWERREADY      =42u,
  USBPD_NOTIFY_SNK_GOTOMIN             =43u,
  USBPD_NOTIFY_SNK_GOTOMIN_READY       =44u,
  USBPD_NOTIFY_REQUEST_ERROR           =45u,
  USBPD_NOTIFY_REQUEST_COMPLETE        =46u,
  USBPD_NOTIFY_REQUEST_CANCELED        =47u,
  USBPD_NOTIFY_SOFTRESET_SENT          =48u,
  USBPD_NOTIFY_SOFTRESET_ACCEPTED      =49u,
  USBPD_NOTIFY_SOFTRESET_RECEIVED      =50u,
  USBPD_NOTIFY_PING_RECEIVED           =51u,
  USBPD_NOTIFY_REQUEST_ENTER_MODE      =52u,
  USBPD_NOTIFY_REQUEST_ENTER_MODE_ACK  =53u,
  USBPD_NOTIFY_REQUEST_ENTER_MODE_NAK  =54u,
  USBPD_NOTIFY_REQUEST_ENTER_MODE_BUSY =55u,
  USBPD_NOTIFY_PD_SPECIFICATION_CHANGE =56u,
  USBPD_NOTIFY_POWER_SWAP_SENT         =57u,
  USBPD_NOTIFY_POWER_SWAP_ACCEPTED     =58u,
  USBPD_NOTIFY_POWER_SWAP_WAIT         =59u,
  USBPD_NOTIFY_POWER_SWAP_RECEIVED     =60u,
  USBPD_NOTIFY_VCONN_SWAP_RECEIVED     =61u,
  USBPD_NOTIFY_VCONN_SWAP_SENT         =62u,
  USBPD_NOTIFY_VCONN_SWAP_ACCEPTED     =63u,
  USBPD_NOTIFY_VCONN_SWAP_WAIT         =64u,
  USBPD_NOTIFY_VCONN_SWAP_REJECTED     =65u,
  USBPD_NOTIFY_VCONN_SWAP_COMPLETE     =66u,
  USBPD_NOTIFY_VCONN_SWAP_NOT_SUPPORTED=67u,
  USBPD_NOTIFY_CTRL_MSG_SENT           =68u,
  USBPD_NOTIFY_DATA_MSG_SENT           =69u,
  USBPD_NOTIFY_GET_SRC_CAP_EXT_RECEIVED=70u,
  USBPD_NOTIFY_SRC_CAP_EXT_RECEIVED    =71u,
  USBPD_NOTIFY_SRC_CAP_EXT_SENT        =72u,
  USBPD_NOTIFY_GET_PPS_STATUS_RECEIVED =73u,
  USBPD_NOTIFY_GET_PPS_STATUS_SENT     =74u,
  USBPD_NOTIFY_PPS_STATUS_RECEIVED     =75u,
  USBPD_NOTIFY_PPS_STATUS_SENT         =76u,
  USBPD_NOTIFY_GET_STATUS_RECEIVED     =77u,
  USBPD_NOTIFY_STATUS_RECEIVED         =78u,
  USBPD_NOTIFY_STATUS_SENT             =79u,
  USBPD_NOTIFY_ALERT_RECEIVED          =80u,
  USBPD_NOTIFY_VDM_IDENTIFY_RECEIVED   =81u,
  USBPD_NOTIFY_VDM_CABLE_IDENT_RECEIVED=82u,
  USBPD_NOTIFY_VDM_SVID_RECEIVED       =83u,
  USBPD_NOTIFY_VDM_MODE_RECEIVED       =84u,
  USBPD_NOTIFY_REQUEST_EXIT_MODE       =85u,
  USBPD_NOTIFY_REQUEST_EXIT_MODE_ACK   =86u,
  USBPD_NOTIFY_REQUEST_EXIT_MODE_NAK   =87u,
  USBPD_NOTIFY_REQUEST_EXIT_MODE_BUSY  =88u,
  USBPD_NOTIFY_MSG_NOT_SUPPORTED       =89u,
  USBPD_NOTIFY_POWER_STATE_CHANGE      =90u,
  USBPD_NOTIFY_REQUEST_DISCARDED       =91u,
  USBPD_NOTIFY_AMS_INTERRUPTED         =92u,
  USBPD_NOTIFY_ALERT_SENT              =93u,
  USBPD_NOTIFY_CABLERESET_TX           =94u,
  USBPD_NOTIFY_PE_DISABLED             =95u,
  USBPD_NOTIFY_GET_SNK_CAP_EXT_RECEIVED=96u,
  USBPD_NOTIFY_SNK_CAP_EXT_SENT        =97u,
  USBPD_NOTIFY_SNK_CAP_EXT_RECEIVED    =98u,
  USBPD_NOTIFY_ALL                     = USBPD_NOTIFY_SNK_CAP_EXT_RECEIVED + 1u,
} USBPD_NotifyEventValue_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_CORE_VDM_Exported_Defines USBPD CORE VDM Exported Defines
  * @{
  */
#define MAX_MODES_PER_SVID  6u

/**
  * @brief Product Type field in ID Header
  * @{
  */
#define PRODUCT_TYPE_UNDEFINED          0u /*!< Undefined                              */
#define PRODUCT_TYPE_HUB                1u /*!< PDUSB Hub (UFP)                        */
#define PRODUCT_TYPE_PERIPHERAL         2u /*!< PDUSB Host (UFP)                       */
#if defined(USBPD_REV30_SUPPORT)
#define PRODUCT_TYPE_HOST               2u /*!< PDUSB Host  (DFP)                      */
#define PRODUCT_TYPE_POWER_BRICK        3u /*!< Power Brick (DFP)                      */
#endif /* USBPD_REV30_SUPPORT */
#define PRODUCT_TYPE_PASSIVE_CABLE      3u /*!< Passive Cable (Cable Plug)             */
#define PRODUCT_TYPE_ACTIVE_CABLE       4u /*!< Active Cable (Cable Plug)              */
#if defined(USBPD_REV30_SUPPORT)
#define PRODUCT_TYPE_AMC                4u /*!<  Alternate Mode Controller (AMC) (DFP) */
#endif /* USBPD_REV30_SUPPORT */
#define PRODUCT_TYPE_AMA                5u /*!< Alternate Mode Adapter (AMA) (UFP)     */

typedef uint32_t USBPD_ProductType_TypeDef;

/**
  * @}
  */

/**
  * @brief USB Host or Device Capability field in ID Header
  * @{
  */
#define USB_NOTCAPABLE                  0u /*!< Not USB capable                                     */
#define USB_CAPABLE                     1u /*!< Capable of being enumerated as a USB host or device */

typedef uint32_t USBPD_USBCapa_TypeDef;

/**
  * @}
  */

/**
  * @brief Modal operation field in ID Header
  * @{
  */
#define MODAL_OPERATION_NONSUPP         0u /*!< Product not supports Modal Operation. */
#define MODAL_OPERATION_SUPPORTED       1u /*!< Product supports Modal Operation.     */

typedef uint32_t USBPD_ModalOp_TypeDef;

/**
  * @}
  */

/**
  * @brief Cable to USB field in Active/Passive cable
  * @{
  */
#define CABLE_TO_TYPE_A         0u /*!< USB Type-A (PD 2.0 only)  */
#define CABLE_TO_TYPE_B         1u /*!< USB Type-B (PD 2.0 only)  */
#define CABLE_TO_TYPE_C         2u /*!< USB Type-C                */
#define CABLE_CAPTIVE           3u /*!< Captive                   */

typedef uint32_t USBPD_CableToType;

/**
  * @}
  */

/**
  * @brief  cable latency values in nanoseconds (max) in Active/Passive cable
  * @{
  */
typedef enum
{
  CABLE_LATENCY_10NS      = 1u,  /*!< <10ns (~1m)        */
  CABLE_LATENCY_20NS      = 2u,  /*!< 10ns to 20ns (~2m) */
  CABLE_LATENCY_30NS      = 3u,  /*!< 20ns to 30ns (~3m) */
  CABLE_LATENCY_40NS      = 4u,  /*!< 30ns to 40ns (~4m) */
  CABLE_LATENCY_50NS      = 5u,  /*!< 40ns to 50ns (~5m) */
  CABLE_LATENCY_60NS      = 6u,  /*!< 50ns to 60ns (~6m) */
  CABLE_LATENCY_70NS      = 7u,  /*!< 60ns to 70ns (~7m) */
  CABLE_LATENCY_1000NS    = 8u   /*!< > 70ns (>~7m) for P2.0 or 1000ns  (~100m) for P3.0    */
#if defined(USBPD_REV30_SUPPORT)
 ,CABLE_LATENCY_2000NS    = 9u,  /*!< 2000ns (~200m)     */
  CABLE_LATENCY_3000NS    = 10u  /*!< 3000ns (~300m)     */
#endif /* USBPD_REV30_SUPPORT */
} USBPD_CableLatency;

/**
  * @}
  */

/**
  * @brief  Cable maximun VBUS voltage
  * @{
  */
#define VBUS_MAX_20V                    0u
#define VBUS_MAX_30V                    1u
#define VBUS_MAX_40V                    2u
#define VBUS_MAX_50V                    3u

typedef uint32_t USBPD_CableMaxVoltage;

/**
  * @}
  */

/**
  * @brief  Cable Termination Type in Active/Passive cable
  * @{
  */
#define CABLE_TERM_BOTH_PASSIVE_NO_VCONN        0u  /*!< VCONN not required (PD2.0 only) */
#define CABLE_TERM_BOTH_PASSIVE_VCONN           1u   /*!< VCONN required (PD2.0 only)     */
#if defined(USBPD_REV30_SUPPORT)
#define CABLE_TERM_ONE_EACH_VCONN               2u  /*!< One end Active, one end passive, VCONN required */
#define CABLE_TERM_BOTH_ACTIVE_VCONN            3u   /*!< Both ends Active, VCONN required  */
#endif /* USBPD_REV30_SUPPORT */

typedef uint32_t USBPD_CableTermType;

/**
  * @}
  */

#if defined(USBPD_REV30_SUPPORT)
/**
  * @brief  Maximum Cable VBUS Voltage in Active/Passive cable
  * @{
  */
#define VBUS_20V                0u /*!< Maximum Cable VBUS Voltage 20V */
#define VBUS_30V                1u /*!< Maximum Cable VBUS Voltage 30V */
#define VBUS_40V                2u /*!< Maximum Cable VBUS Voltage 40V */
#define VBUS_50V                3u /*!< Maximum Cable VBUS Voltage 50V */

typedef uint32_t USBPD_VBUSMaxVoltage;

/**
  * @}
  */
#endif /* USBPD_REV30_SUPPORT */

/**
  * @brief  configurability of SS Directionality in Active/Passive cable and AMA VDO (PD2.0 only)
  * @{
  */
#define SS_DIR_FIXED            0u /*!< SSTX Directionality Support Fixed        */
#define SS_DIR_CONFIGURABLE     1u /*!< SSTX Directionality Support Configurable */

typedef uint32_t USBPD_SsDirectionality;

/**
  * @}
  */

/**
  * @brief SVDM commands definition
  */
#define SVDM_RESERVEDCOMMAND    0x00u
#define SVDM_DISCOVER_IDENTITY  0x01u
#define SVDM_DISCOVER_SVIDS     0x02u
#define SVDM_DISCOVER_MODES     0x03u
#define SVDM_ENTER_MODE         0x04u
#define SVDM_EXIT_MODE          0x05u
#define SVDM_ATTENTION          0x06u
#define SVDM_SPECIFIC_1         0x10u
#define SVDM_SPECIFIC_2         0x11u
#define SVDM_SPECIFIC_3         0x12u
#define SVDM_SPECIFIC_4         0x13u
#define SVDM_SPECIFIC_5         0x14u
#define SVDM_SPECIFIC_6         0x15u
#define SVDM_SPECIFIC_7         0x16u
#define SVDM_SPECIFIC_8         0x17u
#define SVDM_SPECIFIC_9         0x18u
#define SVDM_SPECIFIC_10        0x19u
#define SVDM_SPECIFIC_11        0x1Au
#define SVDM_SPECIFIC_12        0x1Bu
#define SVDM_SPECIFIC_13        0x1Cu
#define SVDM_SPECIFIC_14        0x1Du
#define SVDM_SPECIFIC_15        0x1Eu
#define SVDM_SPECIFIC_16        0x1Fu

typedef uint32_t USBPD_VDM_Command_Typedef;

/**
  * @brief  VBUS Current Handling Capability in Active/Passive cable VDO
  * @{
  */
#define VBUS_3A                 1u /*!< VBUS  Current Handling Capability 3A */
#define VBUS_5A                 2u /*!< VBUS  Current Handling Capability 5A */

typedef uint32_t USBPD_VBUSCurrentHandCap;

/**
  * @}
  */

/**
  * @brief  USB Superspeed Signaling Support in Active/Passive cable VDO
  * @{
  */
#define USB2P0_ONLY             0u /*!< USB2.0 only*/
#define USB3P1_GEN1             1u /*!< USB3.1 Gen1 and USB2.0 */
#define USB3P1_GEN1N2           2u /*!< USB3.1 Gen1, Gen2 and USB2.0*/

typedef uint32_t USBPD_UsbSsSupport;
/**
  * @}
  */

/**
  * @brief  Power needed by adapter for full functionality in AMA VDO header
  * @{
  */
typedef enum
{
  VCONN_1W    = 0u, /*!< VCONN  power 1W   */
  VCONN_1P5W  = 1u, /*!< VCONN  power 1.5W */
  VCONN_2W    = 2u, /*!< VCONN  power 2W   */
  VCONN_3W    = 3u, /*!< VCONN  power 3W   */
  VCONN_4W    = 4u, /*!< VCONN  power 4W   */
  VCONN_5W    = 5u, /*!< VCONN  power 5W   */
  VCONN_6W    = 6u, /*!< VCONN  power 6W   */
} USBPD_VConnPower;

/**
  * @}
  */

/**
  * @brief  VCONN being required by an adapter in AMA VDO header
  * @{
  */
typedef enum
{
  VCONN_NOT_REQUIRED =  0u, /*!< VCONN not required  */
  VCONN_REQUIRED     =  1u, /*!< VCONN required      */
} USBPD_VConnRequirement;

/**
  * @}
  */

/**
  * @brief  VBUS being required by an adapter in AMA VDO header
  * @{
  */
typedef enum
{
  VBUS_NOT_REQUIRED = 0u, /*!< VBUS not required */
  VBUS_REQUIRED     = 1u, /*!< VBUS required     */
} USBPD_VBusRequirement;

/**
  * @}
  */

#define SVDM_INITIATOR          0x00u
#define SVDM_RESPONDER_ACK      0x01u
#define SVDM_RESPONDER_NAK      0x02u
#define SVDM_RESPONDER_BUSY     0x03u

typedef uint32_t USBPD_VDM_CommandType_Typedef;

/**
  * @brief  AMA USB Superspeed Signaling Support in AMA VDO header
  * @{
  */
typedef enum
{
  AMA_USB2P0_ONLY       = 0u, /*!< USB2.0 only                   */
  AMA_USB3P1_GEN1       = 1u, /*!< USB3.1 Gen1 and USB2.0        */
  AMA_USB3P1_GEN1N2     = 2u, /*!< USB3.1 Gen1, Gen2 and USB2.0  */
  AMA_USB2P0_BILLBOARD  = 3u, /*!< USB2.0 billboard only         */
} USBPD_AmaUsbSsSupport;

/**
  * @}
  */

/**
  * @}
  */

#if defined(USBPDCORE_FWUPD)
/**
  * @brief  USBPD Firmware Update Status Information
  * @{
  */
typedef enum
{
  USBPD_FWUPD_STATUS_OK                         = 0x00u,     /*!< Request completed successfully or delayed                             */
  USBPD_FWUPD_STATUS_ERR_TARGET                 = 0x01u,     /*!< FW not targeted for this device                                       */
  USBPD_FWUPD_STATUS_ERR_FILE                   = 0x02u,     /*!< Fails vendor-specific verification test                               */
  USBPD_FWUPD_STATUS_ERR_WRITE                  = 0x03u,     /*!< Unable to write memory                                                */
  USBPD_FWUPD_STATUS_ERR_ERASE                  = 0x04u,     /*!< Memory erase function failed                                          */
  USBPD_FWUPD_STATUS_ERR_CHECK_ERASED           = 0x05u,     /*!< Memory erase check failed                                             */
  USBPD_FWUPD_STATUS_ERR_PROG                   = 0x06u,     /*!< Program memory function failed                                        */
  USBPD_FWUPD_STATUS_ERR_VERIFY                 = 0x07u,     /*!< Program memory failed verification                                    */
  USBPD_FWUPD_STATUS_ERR_ADDRESS                = 0x08u,     /*!< Received address is out of range                                      */
  USBPD_FWUPD_STATUS_ERR_NOTDONE                = 0x09u,     /*!< Received PDFU_DATA Request with a zero length Data Block, but the
                                                                  PDFU Responder expects more data                                      */
  USBPD_FWUPD_STATUS_ERR_FIRMWARE               = 0x0Au,     /*!< Device's firmware is corrupt. It cannot return to normal operations.  */
  USBPD_FWUPD_STATUS_ERR_POR                    = 0x0Du,     /*!< Unexpected power on reset                                             */
  USBPD_FWUPD_STATUS_ERR_UNKNOWN                = 0x0Eu,     /*!< Something went wrong                                                  */
  USBPD_FWUPD_STATUS_ERR_UNEXPECTED_HARD_RESET  = 0x80u,     /*!< Used when firmware update starts after a hard reset (enumeration, etc.)
                                                                  that occurred in the middle of firmware update */
  USBPD_FWUPD_STATUS_ERR_UNEXPECTED_SOFT_RESET  = 0x81u,     /*!< Used when firmware update starts after soft reset (new power contract, etc.)
                                                                  that occurred in the middle of firmware update */
  USBPD_FWUPD_STATUS_ERR_UNEXPECTED_REQUEST     = 0x82u,     /*!< PDFU Responder received a request that is not appropriate for the current Phase */
  USBPD_FWUPD_STATUS_ERR_REJECT_PAUSE           = 0x83u,     /*!< PDFU Responder is unable or unwilling to pause a firmware image transfer */
} USBPD_FWUPD_Status_TypeDef;
/**
  * @}
  */
#endif /* USBPDCORE_FWUPD */

/** @defgroup USBPD_CORE_DataInfoType_TypeDef USB CORE Data information type
  * @brief Data Infor types used in PE callbacks (USBPD_PE_GetDataInfo and USBPD_PE_SetDataInfo)
  * @{
  */
typedef enum
{
  USBPD_CORE_DATATYPE_SRC_PDO          = 0x00u,      /*!< Handling of port Source PDO                  */
  USBPD_CORE_DATATYPE_SNK_PDO          = 0x01u,      /*!< Handling of port Sink PDO                    */
  USBPD_CORE_DATATYPE_RDO_POSITION     = 0x02u,      /*!< Storage of requested DO position in PDO list */
  USBPD_CORE_DATATYPE_REQ_VOLTAGE      = 0x03u,      /*!< Storage of requested voltage value */
  USBPD_CORE_DATATYPE_RCV_SRC_PDO      = 0x04u,      /*!< Storage of Received Source PDO values        */
  USBPD_CORE_DATATYPE_RCV_SNK_PDO      = 0x05u,      /*!< Storage of Received Sink PDO values          */
  USBPD_CORE_DATATYPE_RCV_REQ_PDO      = 0x06u,      /*!< Storage of Received Sink Request PDO value   */
  USBPD_CORE_DATATYPE_REQUEST_DO       = 0x07u,      /*!< Storage of DO to be used in request message (from Sink to Source) */
  USBPD_CORE_EXTENDED_CAPA             = 0x08u,      /*!< Extended capability                          */
  USBPD_CORE_INFO_STATUS               = 0x09u,      /*!< Information status                           */
  USBPD_CORE_PPS_STATUS                = 0x0Au,      /*!< PPS Status data                              */
  USBPD_CORE_ALERT                           ,       /*!< Alert                                        */
  USBPD_CORE_GET_MANUFACTURER_INFO           ,       /*!< Get Manufacturer Info                        */
  USBPD_CORE_MANUFACTURER_INFO               ,       /*!< Manufacturer Info                            */
  USBPD_CORE_GET_BATTERY_STATUS              ,       /*!< Get Battery Status                           */
  USBPD_CORE_BATTERY_STATUS                  ,       /*!< Battery Status                               */
  USBPD_CORE_GET_BATTERY_CAPABILITY          ,       /*!< Get Battery Capability                       */
  USBPD_CORE_BATTERY_CAPABILITY              ,       /*!< Battery Capability                           */
  USBPD_CORE_UNSTRUCTURED_VDM                ,       /*!< Unstructured VDM messag                      */
#if defined(USBPDCORE_SNK_CAPA_EXT)
  USBPD_CORE_SNK_EXTENDED_CAPA               ,       /*!< Sink Extended capability                   */
#endif /* USBPDCORE_SNK_CAPA_EXT */
} USBPD_CORE_DataInfoType_TypeDef;
/**
  * @}
  */

/**
  * @}
  */

/* Exported typedef ----------------------------------------------------------*/

/** @defgroup USBPD_CORE_DEF_Exported_TypeDef USBPD CORE DEF Exported TypeDef
  * @{
  */

#if defined(USBPD_REV30_SUPPORT) && defined(USBPDCORE_PPS)
/**
  * @brief  USB PD Programmable Power Supply APDO Structure definition (same for SRC and SNK)
  *
  */
typedef struct
{
  uint32_t MaxCurrentIn50mAunits  : 7u;  /*!< Maximum Current in 50mA increments       */
  uint32_t Reserved1              : 1u;  /*!< Reserved  - Shall be set to zero         */
  uint32_t MinVoltageIn100mV      : 8u;  /*!< Minimum Voltage in 100mV increments      */
  uint32_t Reserved2              : 1u;  /*!< Reserved  - Shall be set to zero         */
  uint32_t MaxVoltageIn100mV      : 8u;  /*!< Maximum Voltage in 100mV increments      */
  uint32_t Reserved3              : 3u;  /*!< Reserved  - Shall be set to zero         */
  uint32_t ProgrammablePowerSupply: 2u;  /*!< 00b - Programmable Power Supply          */
  uint32_t PPS_APDO               : 2u;  /*!< 11b - Augmented Power Data Object (APDO) */
} USBPD_ProgrammablePowerSupplyAPDO_TypeDef;

#endif /*_USBPD_REV30_SUPPORT && PPS*/

/**
  * @brief  USB PD Source Fixed Supply Power Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t MaxCurrentIn10mAunits :                                  10u;
  uint32_t VoltageIn50mVunits :                                     10u;
  USBPD_CORE_PDO_PeakCurr_TypeDef PeakCurrent :                     2u;
#if defined(USBPD_REV30_SUPPORT)
  uint32_t Reserved22_23 :                                          2u;
  uint32_t UnchunkedExtendedMessage :                               1u;
#else
  uint32_t Reserved22_24 :                                          3u;
#endif /* USBPD_REV30_SUPPORT */
  USBPD_CORE_PDO_DRDataSupport_TypeDef DataRoleSwap :               1u;
  USBPD_CORE_PDO_USBCommCapable_TypeDef USBCommunicationsCapable :  1u;
  USBPD_CORE_PDO_ExtPowered_TypeDef ExternallyPowered :             1u;
  USBPD_CORE_PDO_USBSuspendSupport_TypeDef USBSuspendSupported :    1u;
  USBPD_CORE_PDO_DRPowerSupport_TypeDef DualRolePower :             1u;
  USBPD_CORE_PDO_Type_TypeDef FixedSupply :                         2u;

} USBPD_SRCFixedSupplyPDO_TypeDef;

/**
  * @brief  USB PD Source Variable Supply Power Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t MaxCurrentIn10mAunits :
      10u;
  uint32_t MinVoltageIn50mVunits :
      10u;
  uint32_t MaxVoltageIn50mVunits :
      10u;
  uint32_t VariableSupply :
      2u;
} USBPD_SRCVariableSupplyPDO_TypeDef;

/**
  * @brief  USB PD Source Battery Supply Power Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t MaxAllowablePowerIn250mWunits :
      10u;
  uint32_t MinVoltageIn50mVunits :
      10u;
  uint32_t MaxVoltageIn50mVunits :
      10u;
  uint32_t Battery :
      2u;
} USBPD_SRCBatterySupplyPDO_TypeDef;

/**
  * @brief  USB PD Sink Fixed Supply Power Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t OperationalCurrentIn10mAunits :
      10u;
  uint32_t VoltageIn50mVunits :
      10u;
#if defined(USBPD_REV30_SUPPORT)
  uint32_t FastRoleSwapRequiredCurrent :
      2u;
  uint32_t Reserved20_22 :
      3u;
#else
  uint32_t Reserved20_24 :
      5u;
#endif /* USBPD_REV30_SUPPORT */
  uint32_t DataRoleSwap :
      1u;
  uint32_t USBCommunicationsCapable :
      1;
  uint32_t ExternallyPowered :
      1u;
  uint32_t HigherCapability :
      1u;
  uint32_t DualRolePower :
      1u;
  uint32_t FixedSupply :
      2u;
} USBPD_SNKFixedSupplyPDO_TypeDef;

/**
  * @brief  USB PD Sink Variable Supply Power Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t OperationalCurrentIn10mAunits :
      10u;
  uint32_t MinVoltageIn50mVunits :
      10u;
  uint32_t MaxVoltageIn50mVunits :
      10u;
  uint32_t VariableSupply :
      2u;
} USBPD_SNKVariableSupplyPDO_TypeDef;

/**
  * @brief  USB PD Sink Battery Supply Power Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t OperationalPowerIn250mWunits :
      10u;
  uint32_t MinVoltageIn50mVunits :
      10u;
  uint32_t MaxVoltageIn50mVunits :
      10u;
  uint32_t Battery :
      2u;
} USBPD_SNKBatterySupplyPDO_TypeDef;

/**
  * @brief  USB PD Sink Generic Power Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t Bits_0_10                      : 10u; /*!< Specific Power Capabilities are described by the (A)PDOs in the following sections. */
  uint32_t VoltageIn50mVunits             : 10u; /*!< Maximum Voltage in 50mV units valid for all PDO (not APDO) */
  uint32_t Bits_20_29                     : 10u; /*!< Specific Power Capabilities are described by the (A)PDOs in the following sections. */
  USBPD_CORE_PDO_Type_TypeDef PowerObject : 2u;  /*!< (A) Power Data Object  */
} USBPD_GenericPDO_TypeDef;

/**
  * @brief  USB PD Power Data Object Structure definition
  *
  */
typedef union
{
  uint32_t d32;

  USBPD_GenericPDO_TypeDef            GenericPDO;       /*!< Generic Power Data Object Structure            */

  USBPD_SRCFixedSupplyPDO_TypeDef     SRCFixedPDO;      /*!< Fixed Supply PDO - Source                      */

  USBPD_SRCVariableSupplyPDO_TypeDef  SRCVariablePDO;   /*!< Variable Supply (non-Battery) PDO - Source     */

  USBPD_SRCBatterySupplyPDO_TypeDef   SRCBatteryPDO;    /*!< Battery Supply PDO - Source                    */

  USBPD_SNKFixedSupplyPDO_TypeDef     SNKFixedPDO;      /*!< Fixed Supply PDO - Sink                        */

  USBPD_SNKVariableSupplyPDO_TypeDef  SNKVariablePDO;   /*!< Variable Supply (non-Battery) PDO - Sink       */

  USBPD_SNKBatterySupplyPDO_TypeDef   SNKBatteryPDO;    /*!< Battery Supply PDO - Sink                      */

#if defined(USBPD_REV30_SUPPORT)
#ifdef USBPDCORE_PPS
  USBPD_ProgrammablePowerSupplyAPDO_TypeDef  SRCSNKAPDO;/*!< Programmable Power Supply APDO - Source / Sink */
#endif /* USBPDCORE_PPS */
#endif /* USBPD_REV30_SUPPORT */

} USBPD_PDO_TypeDef;

/**
  * @brief  USB PD Sink Fixed and Variable Request Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t MaxOperatingCurrent10mAunits : /*!< Corresponding to min if GiveBackFlag = 1 */
      10u;
  uint32_t OperatingCurrentIn10mAunits :
      10u;
#if defined(USBPD_REV30_SUPPORT)
  uint32_t Reserved20_22 :
      3u;
  uint32_t UnchunkedExtendedMessage :
      1u;
#else
  uint32_t Reserved20_23 :
      4u;
#endif /* USBPD_REV30_SUPPORT */
  uint32_t NoUSBSuspend :
      1u;
  uint32_t USBCommunicationsCapable :
      1u;
  uint32_t CapabilityMismatch :
      1u;
  uint32_t GiveBackFlag :
      1u;
  uint32_t ObjectPosition :
      3u;
  uint32_t Reserved31 :
      1u;
} USBPD_SNKFixedVariableRDO_TypeDef;

/**
  * @brief  USB PD Sink Battery Request Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t MaxOperatingPowerIn250mWunits :
      10u;
  uint32_t OperatingPowerIn250mWunits :
      10u;
#if defined(USBPD_REV30_SUPPORT)
  uint32_t Reserved20_22 :
      3u;
  uint32_t UnchunkedExtendedMessage :
      1u;                                      /*!< Unchunked Extended Messages Supported                    */
#else
  uint32_t Reserved20_23 :
      4u;
#endif /* USBPD_REV30_SUPPORT */
  uint32_t NoUSBSuspend :
      1u;
  uint32_t USBCommunicationsCapable :
      1u;
  uint32_t CapabilityMismatch :
      1u;
  uint32_t GiveBackFlag :
      1u;
  uint32_t ObjectPosition :
      3u;
  uint32_t Reserved31 :
      1u;
} USBPD_SNKBatteryRDO_TypeDef;

#if defined(USBPD_REV30_SUPPORT)
/**
  * @brief  USB PD Sink Programmable Request Data Object Structure definition
  *
  */
typedef struct
{
  uint32_t OperatingCurrentIn50mAunits  : 7u;  /*!< Operating Current 50mA units                             */
  uint32_t Reserved1                    : 2u;  /*!< Reserved  - Shall be set to zero                         */
  uint32_t OutputVoltageIn20mV          : 11;  /*!< Output Voltage in 20mV units                             */
  uint32_t Reserved2                    : 3u;  /*!< Reserved  - Shall be set to zero                         */
  uint32_t UnchunkedExtendedMessage     : 1u;  /*!< Unchunked Extended Messages Supported                    */
  uint32_t NoUSBSuspend                 : 1u;  /*!< No USB Suspend                                           */
  uint32_t USBCommunicationsCapable     : 1u;  /*!< USB Communications Capable                               */
  uint32_t CapabilityMismatch           : 1u;  /*!< Capability Mismatch                                      */
  uint32_t Reserved3                    : 1u;  /*!< Reserved  - Shall be set to zero                         */
  uint32_t ObjectPosition               : 3u;  /*!< Object position (000b is Reserved and Shall Not be used) */
  uint32_t Reserved4                    : 1u;  /*!< USB Communications Capable                               */
} USBPD_SNKProgrammableRDO_TypeDef;
#endif /* USBPD_REV30_SUPPORT */


/**
  * @brief  USB PD Sink Generic Request Data Object Structure definition
  *
  */
typedef struct
{
#if defined(USBPD_REV30_SUPPORT)
  uint32_t Bits_0_22                    : 23; /*!< Bits 0 to 22 of RDO                                      */
  uint32_t UnchunkedExtendedMessage     : 1u;  /*!< Unchunked Extended Messages Supported                    */
#else
  uint32_t Bits_0_23                    : 24u;  /*!< Bits 0 to 23 of RDO                                     */
#endif /* USBPD_REV30_SUPPORT */
  uint32_t NoUSBSuspend                 : 1u;  /*!< No USB Suspend                                           */
  uint32_t USBCommunicationsCapable     : 1u;  /*!< USB Communications Capable                               */
  uint32_t CapabilityMismatch           : 1u;  /*!< Capability Mismatch                                      */
  uint32_t Bit_27                       : 1u;  /*!< Reserved  - Shall be set to zero                         */
  uint32_t ObjectPosition               : 3u;  /*!< Object position (000b is Reserved and Shall Not be used) */
  uint32_t Bit_31                       : 1u;  /*!< USB Communications Capable                               */
} USBPD_SNKGenericRDO_TypeDef;

/**
  * @brief  USB PD Sink Request Data Object Structure definition
  *
  */
typedef union
{
  uint32_t d32;

  USBPD_SNKGenericRDO_TypeDef       GenericRDO;       /*!<  Generic Request Data Object Structure           */

  USBPD_SNKFixedVariableRDO_TypeDef FixedVariableRDO; /*!< Fixed and Variable Request Data Object Structure */

  USBPD_SNKBatteryRDO_TypeDef       BatteryRDO;       /*!< Battery Request Data Object Structure            */

#if defined(USBPD_REV30_SUPPORT)
  USBPD_SNKProgrammableRDO_TypeDef  ProgRDO;          /*!< Programmable Request Data Object Structure       */
#endif /* USBPD_REV30_SUPPORT */

} USBPD_SNKRDO_TypeDef;

#if defined(USBPD_REV30_SUPPORT) && defined(USBPDCORE_PPS)
/**
  * @brief  USBPD Port APDO Structure definition
  *
  */
typedef struct
{
  uint32_t *ListOfAPDO;                          /*!< Pointer on Augmented Power Data Objects list, defining
                                                     port capabilities */
  uint8_t  NumberOfAPDO;                         /*!< Number of Augmented Power Data Objects defined in ListOfAPDO
                                                     This parameter must be set at max to @ref USBPD_MAX_NB_PDO value */
}USBPD_PortAPDO_TypeDef;
#endif /* USBPD_REV30_SUPPORT && USBPDCORE_PPS */

/**
  * @brief  USB PD BIST Data Object Structure definition
  *
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t BistErrorCounter :
      16u;
    uint32_t Reserved16_27 :
      12u;
    uint32_t BistMode :
      4u;
  }
  b;
} USBPD_BISTDataObject_TypeDef;

/** @brief  Sink requested power profile Structure definition
  *
  */
typedef struct
{
  uint32_t MaxOperatingCurrentInmAunits;           /*!< Sink board Max operating current in mA units   */
  uint32_t OperatingVoltageInmVunits;              /*!< Sink board operating voltage in mV units       */
  uint32_t MaxOperatingVoltageInmVunits;           /*!< Sink board Max operating voltage in mV units   */
  uint32_t MinOperatingVoltageInmVunits;           /*!< Sink board Min operating voltage in mV units   */
  uint32_t OperatingPowerInmWunits;                /*!< Sink board operating power in mW units         */
  uint32_t MaxOperatingPowerInmWunits;             /*!< Sink board Max operating power in mW units     */
} USBPD_SNKPowerRequest_TypeDef;


/** @defgroup USBPD_CORE_VDM_Exported_Structures USBPD CORE VDM Exported Structures
  * @{
  */

/** @defgroup USBPD_ProductVdo_TypeDef USB PD VDM Product VDO
  * @brief USB PD Product VDO Structure definition
  * @{
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t bcdDevice :      /*!< Device version             */
    16u;
    uint32_t USBProductId :   /*!< USB Product ID             */
    16u;
  }
  b;
}USBPD_ProductVdo_TypeDef;

/**
  * @}
  */

/** @defgroup USBPD_IDHeaderVDOStructure_definition USB SVDM ID header VDO Structure definition
  * @brief USB SVDM ID header VDO Structure definition
  * @{
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t VID : 16u;                                 /*!< SVDM Header's SVDM Version                 */
    uint32_t Reserved : 10u;                            /*!< Reserved                                   */
    USBPD_ModalOp_TypeDef ModalOperation : 1u;          /*!< Modal Operation Supported based on
                                                            @ref USBPD_ModalOp_TypeDef                 */
    USBPD_ProductType_TypeDef ProductTypeUFPorCP : 3u;  /*!< Product Type (UFP or Cable Plug)based on
                                                            @ref USBPD_ProductType_TypeDef             */
    USBPD_USBCapa_TypeDef USBDevCapability : 1u;        /*!< USB Communications Capable as a USB Device based on
                                                            @ref USBPD_USBCapa_TypeDef                 */
    USBPD_USBCapa_TypeDef USBHostCapability : 1u;       /*!< USB Communications Capable as USB Host based on
                                                            @ref USBPD_USBCapa_TypeDef                 */
  }b20;
#if defined(USBPD_REV30_SUPPORT)
  struct
  {
    uint32_t VID : 16u;                                 /*!< SVDM Header's SVDM Version                 */
    uint32_t Reserved : 7u;                             /*!< Reserved                                   */
    uint32_t ProductTypeDFP : 3u;                       /*!< Product Type (DFP) based on
                                                            @ref USBPD_ProductType_TypeDef             */
    USBPD_ModalOp_TypeDef ModalOperation : 1u;          /*!< Modal Operation Supported based on
                                                            @ref USBPD_ModalOp_TypeDef                 */
    USBPD_ProductType_TypeDef ProductTypeUFPorCP : 3u;  /*!< Product Type (UFP or Cable Plug)based on
                                                            @ref USBPD_ProductType_TypeDef             */
    USBPD_USBCapa_TypeDef USBDevCapability : 1u;        /*!< USB Communications Capable as a USB Device based on
                                                            @ref USBPD_USBCapa_TypeDef                 */
    USBPD_USBCapa_TypeDef USBHostCapability : 1u;       /*!< USB Communications Capable as USB Host based on
                                                            @ref USBPD_USBCapa_TypeDef                 */
  }b30;
#endif /* USBPD_REV30_SUPPORT */
}USBPD_IDHeaderVDO_TypeDef;
/**
  * @}
  */

typedef union
{
  struct /* PD 2.0*/
  {
    USBPD_SsDirectionality    SSRX2_DirSupport    : 1u;  /*!< SSRX2 Directionality Support (PD2.0)     */
    USBPD_SsDirectionality    SSRX1_DirSupport    : 1u;  /*!< SSRX1 Directionality Support (PD2.0)     */
    USBPD_SsDirectionality    SSTX2_DirSupport    : 1u;  /*!< SSTX2 Directionality Support (PD2.0)     */
    USBPD_SsDirectionality    SSTX1_DirSupport    : 1u;  /*!< SSTX1 Directionality Support (PD2.0)     */
  }
  pd_v20;
#if defined(USBPD_REV30_SUPPORT)
  struct /* PD 3.0*/
  {
    uint8_t                   Reserved            : 2u;  /*!< Reserved                                 */
    USBPD_VBUSMaxVoltage      MaxVBUS_Voltage     : 2u;  /*!< Maximum Cable VBUS Voltage               */
  }
  pd_v30;
#endif /* USBPD_REV30_SUPPORT */
} USBPD_CableVdo_Field1TypeDef;

/** @defgroup USBPD_AttentionInfo_TypeDef USB PD Attention Info object Structure definition
  * @brief USB PD Attention Info object Structure definition
  * @{
  */
typedef struct
{
  uint32_t  VDO;
  uint16_t  SVID;
  USBPD_VDM_Command_Typedef Command;
  uint8_t   ModeIndex;
}USBPD_AttentionInfo_TypeDef;

/**
  * @}
  */

typedef union
{
  struct /* PD 2.0*/
  {
    uint8_t                   Reserved            : 4u;  /*!< Reserved                               */
  }
  pd_v20;
#if defined(USBPD_REV30_SUPPORT)
  struct /* PD 3.0*/
  {
    uint8_t                   VDOVersion          : 3u;  /*!< Version Number of the VDO              */
    uint8_t                   Reserved            : 1u;  /*!< Reserved                               */
  }
  pd_v30;
#endif /* USBPD_REV30_SUPPORT */
}USBPD_CableVdo_Field2TypeDef;

#define VDM_UNSTRUCTUREDVDM_TYPE        0x0u
#define VDM_STRUCTUREDVDM_TYPE          0x1u

typedef uint32_t USBPD_VDM_VDMType_Typedef;

/** @defgroup USBPD_SVDMHeaderStructure_definition USB SVDM Message header Structure definition
  * @brief USB SVDM Message header Structure definition
  * @{
  */
typedef union
{
  uint32_t d32;
  struct
  {
    USBPD_VDM_Command_Typedef Command :         /*!< SVDM Header's Command          */
    5u;
    uint32_t Reserved5 :                        /*!< Reserved                       */
    1u;
    USBPD_VDM_CommandType_Typedef CommandType : /*!< SVDM Header's Command Type     */
    2u;
    uint32_t ObjectPosition :                   /*!< SVDM Header's Object Position  */
    3u;
    uint32_t Reserved11 :                       /*!< Reserved                       */
    2u;
    uint32_t SVDMVersion :                      /*!< SVDM Header's SVDM Version     */
    2u;
    USBPD_VDM_VDMType_Typedef VDMType :         /*!< SVDM Header's VDM Type         */
    1u;
    uint32_t SVID :                             /*!< SVDM Header's SVID             */
    16u;
  }
  b;
}USBPD_SVDMHeader_TypeDef;

/**
  * @}
  */

#ifdef USBPDCORE_UVDM
/** @defgroup USBPD_UVDMHeaderStructure_definition USB UVDM Message header Structure definition
  * @brief USB UVDM Message header Structure definition
  * @{
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t VendorUse                : 15u;  /*!< Content of this field is defined by the vendor.  */
    USBPD_VDM_VDMType_Typedef VDMType : 1u;   /*!< VDM Type                                         */
    uint32_t VID                      : 16u;  /*!< Vendor ID (VID)                                  */
  }
  b;
}USBPD_UVDMHeader_TypeDef;

/**
  * @}
  */
#endif /* USBPDCORE_UVDM */

/** @defgroup USBPD_CableVdo_TypeDef USB PD VDM Cable VDO
  * @brief USB PD Cable VDO Structure definition
  * @{
  */
typedef union
{
  uint32_t d32;
  struct
  {
    USBPD_UsbSsSupport        USB_SS_Support      : 3u;  /*!< USB SuperSpeed Signaling Support           */
    uint32_t                  reserved            : 2u;
    USBPD_VBUSCurrentHandCap  VBUS_CurrentHandCap : 2u;  /*!< VBUS Current Handling Capability           */
    uint32_t                  Fields1             : 2u;  /*!< Based on @ref USBPD_CableVdo_Field1TypeDef */
    USBPD_CableMaxVoltage     CableMaxVoltage     : 2u;  /*!< Cable maximun voltage                      */
    USBPD_CableTermType       CableTermType       : 2u;  /*!< Cable Termination Type                     */
    uint32_t                  CableLatency        : 4u;  /*!< Cable Latency                              */
    uint32_t                  Reserved            : 1u;  /*!< Reserved                                   */
    USBPD_CableToType         CableToType         : 2u;  /*!< USB Type-C plug to USB Type-A/B/C/Captive (PD 2.0)
                                                             USB Type-C plug to USB Type-C/Captive (PD 3.0) */
    uint32_t                  Fields2             : 1u;  /*!< Based on @ref USBPD_CableVdo_Field2TypeDef */
    uint32_t                  VDO_Version         : 3u;  /*!< Version number  of the VDO                 */
    uint32_t                  CableFWVersion      : 4u;  /*!< Cable FW version number (vendor defined)   */
    uint32_t                  CableHWVersion      : 4u;  /*!< Cable HW version number (vendor defined)   */
  }
  b;
}USBPD_CableVdo_TypeDef;

/**
  * @}
  */

/** @defgroup USBPD_CertStatVdo_TypeDef USB PD VDM Cert stat VDO
  * @brief USB PD Cert stat VDO Structure definition
  * @{
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t XID :          /*!< USB-IF assigned XID */
    32;
  }
  b;
}USBPD_CertStatVdo_TypeDef;

/**
  * @}
  */

/** @defgroup USBPD_AMAVdo_TypeDef USB PD VDM Alternate Mode Adapter VDO
  * @brief USB PD Alternate Mode Adapter VDO Structure definition
  * @{
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t                AMA_USB_SS_Support  : 3u;  /*!< AMA USB SuperSpeed Signaling Support based on
                                                           @ref USBPD_AmaUsbSsSupport               */
    uint32_t                VBUSRequirement     : 1u;  /*!< VBUS  required  based on
                                                           @ref USBPD_VBusRequirement               */
    uint32_t                VCONNRequirement    : 1u;  /*!< VCONN  required  based on
                                                           @ref USBPD_VConnRequirement              */
    uint32_t                VCONNPower          : 3u;  /*!< VCONN  power  based on
                                                           @ref USBPD_VConnPower                    */
#if defined(USBPD_REV30_SUPPORT)
    uint32_t                Reserved            : 13; /*!< Reserved                                 */
    uint32_t                VDO_Version         : 3u;  /*!< Version Number of the VDO                */
#else
    uint32_t                SSRX2_DirSupport    : 1u;  /*!< SSRX2 Directionality Support (PD2.0) based on
                                                           @ref USBPD_SsDirectionality              */
    uint32_t                SSRX1_DirSupport    : 1u;  /*!< SSRX1 Directionality Support (PD2.0) based on
                                                           @ref USBPD_SsDirectionality              */
    uint32_t                SSTX2_DirSupport    : 1u;  /*!< SSTX2 Directionality Support (PD2.0) based on
                                                           @ref USBPD_SsDirectionality              */
    uint32_t                SSTX1_DirSupport    : 1u;  /*!< SSTX1 Directionality Support (PD2.0) based on
                                                           @ref USBPD_SsDirectionality              */
    uint32_t                Reserved            : 12u; /*!< Reserved                                 */
#endif /* USBPD_REV30_SUPPORT */
    uint32_t                AMAFWVersion        : 4u;  /*!< AMA FW version number (vendor defined)   */
    uint32_t                AMAHWVersion        : 4u;  /*!< AMA HW version number (vendor defined)   */
  }
  b;
}USBPD_AMAVdo_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_DiscoveryIdentity_TypeDef USB PD Discovery identity Structure definition
  * @brief Data received from Discover Identity messages
  * @{
  */
typedef struct
{

  USBPD_IDHeaderVDO_TypeDef IDHeader;             /*!< ID Header VDO                              */
  USBPD_CertStatVdo_TypeDef CertStatVDO;          /*!< Cert Stat VDO                              */
  USBPD_ProductVdo_TypeDef  ProductVDO;           /*!< Product VDO                                */
  USBPD_CableVdo_TypeDef    CableVDO;             /*!< Passive Cable VDO                          */
  USBPD_AMAVdo_TypeDef      AMA_VDO;              /*!< Alternate Mode Adapter VDO                 */
  uint8_t                   CableVDO_Presence:1u; /*!< Indicate Passive Cable VDO presence or not */
  uint8_t                   AMA_VDO_Presence:1u;  /*!< Indicate Alternate Mode Adapter VDO presence or not    */
  uint8_t                   Reserved:6u;          /*!< Reserved bits                              */
} USBPD_DiscoveryIdentity_TypeDef;
/**
  * @}
  */

/** @defgroup USBPD_ModeInfo_TypeDef USB PD Mode Info object Structure definition
  * @brief USB PD Mode Info object Structure definition
  * @{
  */
typedef struct
{
  uint32_t  NumModes;
  uint32_t  Modes[MAX_MODES_PER_SVID];
  uint16_t  SVID;
}USBPD_ModeInfo_TypeDef;

/**
  * @}
  */

/** @defgroup USBPD_SVID_TypeDef USB PD Discovery SVID Structure definition
  * @brief Data received from Discover Identity messages
  * @{
  */
/*
 * Structure to SVID supported by the devices
 */
typedef struct
{
  uint16_t  SVIDs[12u];
  uint8_t  NumSVIDs;
  uint8_t  AllSVID_Received; /*!< Flag to indicate that all the SVIDs have been received.
                                No need to send new SVDM Discovery SVID message */
}USBPD_SVIDInfo_TypeDef;
/**
  * @}
  */

#if defined(USBPD_REV30_SUPPORT)
/**
  * @brief  USBPD Alert Data Object Structure definition
  *
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t Reserved             : 16u; /*!< Reserved */
    uint32_t HotSwappableBatteries: 4u;  /*!< Hot Swappable Batteries is a combination of @ref USBPD_ADO_HOT_SWAP_BATT */
    uint32_t FixedBatteries       : 4u;  /*!< Fixed Batteries is a combination of @ref USBPD_ADO_FIXED_BATT */
    uint32_t TypeAlert            : 8u;  /*!< Type of Alert is a combination of @ref USBPD_ADO_TYPE_ALERT */
  }
  b;
} USBPD_ADO_TypeDef;

/**
  * @brief  USBPD Battery Status Data Object Structure definition
  *
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t Reserved: 8u;
    uint32_t BatteryInfo: 8u; /*!< Based on @ref USBPD_BSDO_BATT_INFO */
    uint32_t BatteryPC: 16u;
  }
  b;
} USBPD_BSDO_TypeDef;

/**
  * @brief  USBPD Source Capabilities Extended Message Structure definition
  *
  */
typedef struct USBPD_SCEDB_TypeDef
{
  uint16_t VID;                 /*!< Vendor ID (assigned by the USB-IF)                   */
  uint16_t PID;                 /*!< Product ID (assigned by the manufacturer)            */
  uint32_t XID;                 /*!< Value provided by the USB-IF assigned to the product */
  uint8_t  FW_revision;         /*!< Firmware version number                              */
  uint8_t  HW_revision;         /*!< Hardware version number                              */
  uint8_t  Voltage_regulation;  /*!< Voltage Regulation                                   */
  uint8_t  Holdup_time;         /*!< Holdup Time                                          */
  uint8_t  Compliance;          /*!< Compliance                                           */
  uint8_t  TouchCurrent;        /*!< Touch Current                                        */
  uint16_t PeakCurrent1;        /*!< Peak Current1                                        */
  uint16_t PeakCurrent2;        /*!< Peak Current2                                        */
  uint16_t PeakCurrent3;        /*!< Peak Current3                                        */
  uint8_t  Touchtemp;           /*!< Touch Temp                                           */
  uint8_t  Source_inputs;       /*!< Source Inputs                                        */
  uint8_t  NbBatteries;         /*!< Number of Batteries/Battery Slots                    */
  uint8_t  SourcePDP;           /*!< Source PDP                                           */
} USBPD_SCEDB_TypeDef;

#if defined(USBPDCORE_SNK_CAPA_EXT)
/**
  * @brief  SKEDB -  Sink Load Characteristics structure definition
  *
  */
typedef union
{
  uint16_t Value;
  struct
  {
    uint16_t PercentOverload  : 5u; /*!< Percent overload in 10% increments Values higher than 25 (11001b) are clipped to 250%.
                                         00000b is the default.                                   */
    uint16_t OverloadPeriod   : 6u; /*!< Overload period in 20ms when bits 0-4 non-zero.          */
    uint16_t DutyCycle        : 4u; /*!< Duty cycle in 5% increments when bits 0-4 are non-zero.  */
    uint16_t VBusVoltageDrop  : 1u; /*!< Can tolerate VBUS Voltage drop.                          */
  } b;
} USBPD_SKEDB_SinkLoadCharac_TypeDef;
/**
  * @brief  USBPD Sink Capabilities Extended Message Structure definition
  *
  */
typedef struct USBPD_SKEDB_TypeDef
{
  uint16_t VID;                 /*!< Vendor ID (assigned by the USB-IF)                             */
  uint16_t PID;                 /*!< Product ID (assigned by the manufacturer)                      */
  uint32_t XID;                 /*!< Value provided by the USB-IF assigned to the product           */
  uint8_t  FW_revision;         /*!< Firmware version number                                        */
  uint8_t  HW_revision;         /*!< Hardware version number                                        */
  uint8_t  SKEDB_Version;       /*!< SKEDB Version (not the specification Version) based on
                                     @ref USBPD_SKEDB_VERSION                                       */
  uint8_t  LoadStep;            /*!< Load Step based on @ref USBPD_SKEDB_LOADSTEP                   */
  USBPD_SKEDB_SinkLoadCharac_TypeDef SinkLoadCharac;  /*!< Sink Load Characteristics                */
  uint8_t  Compliance;          /*!< Compliance based on combination of @ref USBPD_SKEDB_COMPLIANCE */
  uint8_t  Touchtemp;           /*!< Touch Temp based on @ref USBPD_SKEDB_TOUCHTEMP                 */
  uint8_t  BatteryInfo;         /*!< Battery info                                                   */
  uint8_t  SinkModes;           /*!< Sink Modes based on combination of @ref USBPD_SKEDB_SINKMODES  */
  uint8_t  SinkMinimumPDP;      /*!< The Minimum PDP required by the Sink to operate without
                                     consuming any power from its Battery(s) should it have one     */
  uint8_t  SinkOperationalPDP;  /*!< The PDP the Sink requires to operate normally. For Sinks with
                                     a Battery, it is the PDP Rating of the charger supplied with
                                     it or recommended for it.                                      */
  uint8_t  SinkMaximumPDP;      /*!< The Maximum PDP the Sink can consume to operate and
                                     charge its Battery(s) should it have one.                      */
} USBPD_SKEDB_TypeDef;
#endif /* USBPDCORE_SNK_CAPA_EXT */

/**
  * @brief  USBPD Source Status Extended Message Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  uint8_t InternalTemp;          /*!< Source or Sink internal temperature in degrees centigrade         */
  uint8_t PresentInput;          /*!< Present Input                                                     */
  uint8_t PresentBatteryInput;   /*!< Present Battery Input                                             */
  uint8_t EventFlags;            /*!< Event Flags                                                       */
  uint8_t TemperatureStatus;     /*!< Temperature                                                       */
  uint8_t PowerStatus;           /*!< Power Status based on combination of @ref USBPD_SDB_POWER_STATUS  */
} __PACKEDSTRUCTEND USBPD_SDB_TypeDef;

/**
  * @brief  USBPD Get Battery Capabilities Data Block Extended Message Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  uint8_t BatteryCapRef;     /*!< Number of the Battery indexed from zero    */
} __PACKEDSTRUCTEND USBPD_GBCDB_TypeDef;

/**
  * @brief  USBPD Get Battery Status Data Block Extended Message Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  uint8_t BatteryStatusRef;     /*!< Number of the Battery indexed from zero  */
} __PACKEDSTRUCTEND USBPD_GBSDB_TypeDef;

/**
  * @brief  USBPD  Battery Capability Data Block Extended Message Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  uint16_t VID;                       /*!< Vendor ID (assigned by the USB-IF)         */
  uint16_t PID;                       /*!< Product ID (assigned by the manufacturer)  */
  uint16_t BatteryDesignCapa;         /*!< Battery Design Capacity                    */
  uint16_t BatteryLastFullChargeCapa; /*!< Battery last full charge capacity        */
  uint8_t  BatteryType;               /*!< Battery Type                               */
} __PACKEDSTRUCTEND USBPD_BCDB_TypeDef;

/**
  * @brief  USBPD Get Manufacturer Info Info Data Block Extended Message Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  uint8_t ManufacturerInfoTarget;     /*!< Manufacturer Info Target based on @ref USBPD_MANUFINFO_TARGET                      */
  uint8_t ManufacturerInfoRef;        /*!< Manufacturer Info Ref between Min_Data=0 and Max_Data=7 (@ref USBPD_MANUFINFO_REF) */
} __PACKEDSTRUCTEND USBPD_GMIDB_TypeDef;

/**
  * @brief  USBPD Manufacturer Info Data Block Extended Message Structure definition
  *
  */
typedef struct
{
  uint16_t VID;                       /*!< Vendor ID (assigned by the USB-IF)        */
  uint16_t PID;                       /*!< Product ID (assigned by the manufacturer) */
  uint8_t ManuString[22];             /*!< Vendor defined byte array                 */
} USBPD_MIDB_TypeDef;

#if defined(USBPDCORE_FWUPD)
/**
  * @brief  USBPD Firmware Update GET_FW_ID Response Payload Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  USBPD_FWUPD_Status_TypeDef   Status;  /*!< Status Information during Firmware Update      */
  uint16_t  VID;         /*!< USB-IF assigned Vendor ID                                     */
  uint16_t  PID;         /*!< USB-IF assigned Product ID                                    */
  uint8_t   HWVersion;   /*!< Hardware Version                                              */
  uint8_t   SiVersion;   /*!< Silicon Version                                               */
  uint16_t  FWVersion1;  /*!< Most significant component of the firmware version            */
  uint16_t  FWVersion2;  /*!< Second-most significant component of the firmware version     */
  uint16_t  FWVersion3;  /*!< Third-most significant component of the firmware version      */
  uint16_t  FWVersion4;  /*!< Least significant component of the firmware version           */
  uint8_t   ImageBank;   /*!< Image bank for which firmware is requested                    */
  uint8_t   Flags1;      /*!< Flags1                                                        */
  uint8_t   Flags2;      /*!< Flags2                                                        */
  uint8_t   Flags3;      /*!< Flags3                                                        */
  uint8_t   Flags4;      /*!< Flags4                                                        */
} __PACKEDSTRUCTEND USBPD_FWUPD_GetFwIDRspPayload_TypeDef;

/**
  * @brief  USBPD Firmware Update PDFU_INITIATE Request Payload Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  uint16_t  FWVersion1;  /*!< Most significant component of the firmware version            */
  uint16_t  FWVersion2;  /*!< Second-most significant component of the firmware version     */
  uint16_t  FWVersion3;  /*!< Third-most significant component of the firmware version      */
  uint16_t  FWVersion4;  /*!< Least significant component of the firmware version           */
} __PACKEDSTRUCTEND USBPD_FWUPD_PdfuInitReqPayload_TypeDef;

/**
  * @brief  USBPD Firmware Update PDFU_INITIATE Response Payload Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  USBPD_FWUPD_Status_TypeDef   Status;  /*!< Status Information during Firmware Update      */
  uint8_t   WaitTime;         /*!< Wait time                                                */
  uint8_t   MaxImageSize[3u];  /*!< Max image size                                           */
} __PACKEDSTRUCTEND USBPD_FWUPD_PdfuInitRspPayload_TypeDef;

/**
  * @brief  USBPD Firmware Update PDFU_DATA Response Payload Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  USBPD_FWUPD_Status_TypeDef   Status;  /*!< Status Information during Firmware Update      */
  uint8_t   WaitTime;         /*!< Wait time                                                */
  uint8_t   NumDataNR;        /*!< Number of PDFU_DATA_NR Requests                          */
  uint16_t  DataBlockNum;     /*!< Data Block Number of the next PDFU_DATA or PDFU_DATA_NR  */
} __PACKEDSTRUCTEND USBPD_FWUPD_PdfuDataRspPayload_TypeDef;

/**
  * @brief  USBPD Firmware Update PDFU_VALIDATE Response Payload Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  USBPD_FWUPD_Status_TypeDef   Status;  /*!< Status Information during Firmware Update      */
  uint8_t   WaitTime;         /*!< Wait time                                                */
  uint8_t   Flags;            /*!< Flags                                                    */
} __PACKEDSTRUCTEND USBPD_FWUPD_PdfuValidateRspPayload_TypeDef;

/**
  * @brief  USBPD Firmware Update PDFU_DATA_PAUSE Response Payload Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  USBPD_FWUPD_Status_TypeDef   Status;  /*!< Status Information during Firmware Update      */
} __PACKEDSTRUCTEND USBPD_FWUPD_PdfuDataPauseRspPayload_TypeDef;

/**
  * @brief  USBPD Firmware Update VENDOR_SPECIFIC Request Payload Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  uint16_t  VID;                /*!< USB-IF assigned Vendor ID                              */
  uint8_t   VendorDefined[256]; /*!< Vendor defined                                         */
} __PACKEDSTRUCTEND USBPD_FWUPD_VendorSpecificReqPayload_TypeDef;

/**
  * @brief  USBPD Firmware Update VENDOR_SPECIFIC Response Payload Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  USBPD_FWUPD_Status_TypeDef   Status;  /*!< Status Information during Firmware Update      */
  uint16_t  VID;                /*!< USB-IF assigned Vendor ID                              */
  uint8_t   VendorDefined[255]; /*!< Vendor defined                                         */
} __PACKEDSTRUCTEND USBPD_FWUPD_VendorSpecificRspPayload_TypeDef;

/**
  * @brief  USBPD Firmware Update Request Data Block Extended Message Structure definition
  *
  */
typedef __PACKEDSTRUCTBEGIN
{
  uint8_t   ProtocolVersion;  /*!< Protocol Version (@ref USBPD_FWUPD_PROT_VER)             */
  uint8_t   MessageType;      /*!< Firmware Update Message type (@ref USBPD_FWUPD_MSGTYPE)  */
  uint8_t   Payload[258];     /*!< Payload                                         */
} __PACKEDSTRUCTEND USBPD_FRQDB_TypeDef;

#endif /* USBPDCORE_FWUPD */

#ifdef USBPDCORE_PPS
/**
  * @brief  USBPD PPS Status Data Block Extended Message Structure definition
  *
  */
typedef union
{
  uint32_t d32;
  struct
  {
    uint16_t OutputVoltageIn20mVunits;  /*!< Source output voltage in 20mV units.
                                             When set to 0xFFFF, the Source does not support this field.          */
    uint8_t  OutputCurrentIn50mAunits;  /*!< Source output current in 50mA units.
                                             When set to 0xFF, the Source does not support this field.            */
    uint8_t  RealTimeFlags;             /*!< Real Time Flags, combination of @ref USBPD_CORE_DEF_REAL_TIME_FLAGS  */
  } fields;
} USBPD_PPSSDB_TypeDef;
#endif /* USBPDCORE_PPS */

/**
  * @brief  USBPD Country Code Data Block Extended Message Structure definition
  *
  */
typedef struct
{
  uint32_t Length;            /*!< Number of country codes in the message */
  uint16_t *PtrCountryCode;   /*!< Pointer of the country codes (1 to n)  */
} USBPD_CCDB_TypeDef;

/**
  * @brief  USBPD Country Info Data Block Extended Message Structure definition
  *
  */
typedef struct
{
  uint16_t CountryCode;             /*!< 1st and 2nd character of the Alpha-2 Country Code defined by [ISO 3166]  */
  uint32_t Reserved;                /*!< Reserved - Shall be set to 0.   */
  uint8_t  PtrCountrySpecificData;  /*!< Pointer on Content defined by the country authority (0 t 256 bytes). */
} USBPD_CIDB_TypeDef;

#endif /* USBPD_REV30_SUPPORT */

/**
  * @}
  */

/** @defgroup USBPD_CORE_SETTINGS_Exported_Structures USBPD CORE Settings Exported Structures
  * @brief  USBPD Settings Structure definition
  * @{
  */
#if defined(USBPD_REV30_SUPPORT)
typedef union {
  uint16_t PD3_Support;
  struct {
    uint16_t PE_UnchunkSupport                : 1u; /*!< Unchunked support                                                          */
    uint16_t PE_FastRoleSwapSupport           : 1u; /*!< Fast role swap support (not yet implemented)                               */
    uint16_t Is_GetPPSStatus_Supported        : 1u; /*!< Get PPS status message supported by PE                                     */
    uint16_t Is_SrcCapaExt_Supported          : 1u; /*!< Source_Capabilities_Extended message supported by PE                       */
    uint16_t Is_Alert_Supported               : 1u; /*!< Alert message supported by PE                                              */
    uint16_t Is_GetStatus_Supported           : 1u; /*!< Get_Status message supported by PE (Is_Alert_Supported should be enabled)  */
    uint16_t Is_GetManufacturerInfo_Supported : 1u; /*!< Manufacturer_Info message supported by PE                                  */
    uint16_t Is_GetCountryCodes_Supported     : 1u; /*!< Get_Country_Codes message supported by PE                                  */
    uint16_t Is_GetCountryInfo_Supported      : 1u; /*!< Get_Country_Info message supported by PE                                   */
    uint16_t Is_SecurityRequest_Supported     : 1u; /*!< Security_Response message supported by PE                                  */
    uint16_t Is_FirmUpdateRequest_Supported   : 1u; /*!< Firmware update response message supported by PE                           */
    uint16_t Is_SnkCapaExt_Supported          : 1u; /*!< Sink_Capabilities_Extended message supported by PE                         */
    uint16_t reserved                         : 3u; /*!< Reserved bits                                                              */
  } d;
}USBPD_PD3SupportTypeDef;
#endif /* USBPD_REV30_SUPPORT */

typedef struct
{
  USBPD_SupportedSOP_TypeDef PE_SupportedSOP; /*!<  Corresponds to the message managed by the stack and this should be set depending if you want discuss with the cable 
                                                    So if VconnSupport is enabling this field must be set to 
                                                      @ref USBPD_SUPPORTED_SOP_SOP | @ref USBPD_SUPPORTED_SOP_SOP1 | @ref USBPD_SUPPORTED_SOP_SOP2
                                                    else
                                                      @ref USBPD_SUPPORTED_SOP_SOP
                                              */
  USBPD_SpecRev_TypeDef PE_SpecRevision     : 2u; /*!< Spec revision value based on @ref USBPD_SpecRev_TypeDef                         */
  USBPD_PortPowerRole_TypeDef PE_DefaultRole: 1u; /*!< Default port role  based on @ref USBPD_PortPowerRole_TypeDef                    */
  uint32_t PE_RoleSwap                      : 1u; /*!< If enabled, allows the port to have DRP behavior                                */
  uint32_t _empty1                          : 1u; /*!< Reserved bit                                                                    */
  uint32_t PE_VDMSupport                    : 1u; /*!< Support VDM: If not enabled any VDM message received is replied "not supported" */
  uint32_t PE_PingSupport                   : 1u; /*!< support Ping (only for PD3.0): If enabled allows DPM to send ping message       */
  uint32_t PE_CapscounterSupport            : 1u; /*!< If enabled after an amount of message source capabilities not replied, the stack stop the message send.*/
  uint32_t PE_RespondsToDiscovSOP           : 1u; /*!< Can respond successfully to a Discover Identity */
  uint32_t PE_AttemptsDiscovSOP             : 1u; /*!< Can send a Discover Identity */
  uint32_t CAD_TryFeature                   : 2u; /*!< Not yet implemented                                                              */
  uint32_t CAD_AccesorySupport              : 1u; /*!< Not yet implemented                                                              */
  uint32_t CAD_RoleToggle                   : 1u; /*!< If enabled allows the detection state machine switch Rp/Rd means toggle the presented role between source and sink */
  CAD_RP_Source_Current_Adv_Typedef CAD_DefaultResistor  : 2u; /*!< Default RP resistor based on @ref CAD_RP_Source_Current_Adv_Typedef */
  uint32_t CAD_SNKToggleTime                : 8u; /*!< Sink toggle time in ms                                                           */
  uint32_t CAD_SRCToggleTime                : 8u; /*!< Source toggle time in ms                                                         */
#if defined(USBPD_REV30_SUPPORT)
  USBPD_PD3SupportTypeDef PE_PD3_Support;         /*!< PD3 structure support flags based on @ref USBPD_PD3SupportTypeDef                */
#else
  uint16_t reserved                         : 16u; /*!< Reserved bits */
#endif /* USBPD_REV30_SUPPORT */
} USBPD_SettingsTypeDef;

/**
  * @}
  */

/** @defgroup USBPD_CORE_PARAMS_Exported_Structures USBPD CORE Params Exported Structures
  * @brief  USBPD Params Structure definition
  * @{
  */
typedef struct
{

  USBPD_SpecRev_TypeDef               PE_SpecRevision : 2u;  /*!< PE Specification revision                                */
  USBPD_PortPowerRole_TypeDef         PE_PowerRole    : 1u;  /*!< PE Power role                                            */
  USBPD_PortDataRole_TypeDef          PE_DataRole     : 1u;  /*!< PE Data role                                             */
  uint32_t                            PE_SwapOngoing  : 1u;  /*!< Power role swap ongoing flag                             */
  USBPD_VDMVersion_TypeDef            VDM_Version     : 1u;  /*!< VDM version                                              */
  CCxPin_TypeDef                      ActiveCCIs      : 2u;  /*!< Active CC line based on @ref CCxPin_TypeDef              */
  USBPD_POWER_StateTypedef            PE_Power        : 3u;  /*!< Power status based on @ref USBPD_POWER_StateTypedef      */
  uint32_t                            DPM_Initialized : 1u;  /*!< DPM initialized flag                                     */
  uint32_t                            PE_IsConnected  : 1u;  /*!< USB-PD PE stack is connected to CC line                  */
  CCxPin_TypeDef                      VconnCCIs       : 2u;  /*!< VConn  CC line based on @ref CCxPin_TypeDef              */
  uint32_t                            VconnStatus     : 1u;  /*!< VConnStatus USBP_TRUE = vconn on USBPD_FALSE = vconn off */
  CAD_RP_Source_Current_Adv_Typedef   RpResistor      : 2u;  /*!< RpResistor presented                                     */
#if defined(USBPD_REV30_SUPPORT) && defined(USBPDCORE_UNCHUNCKED_MODE)
  uint32_t                            PE_UnchunkSupport: 1u; /*!< Unchunked support                                        */
  uint32_t                            Reserved        : 13u; /*!< Reserved bits                                            */
#else
  uint32_t                            Reserved        : 14u; /*!< Reserved bits                                            */
#endif /* USBPD_REV30_SUPPORT && USBPDCORE_UNCHUNCKED_MODE */
} USBPD_ParamsTypeDef;

/**
  * @}
  */

/**
  * @}
  */

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

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

#endif /* __USBPD_DEF_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
