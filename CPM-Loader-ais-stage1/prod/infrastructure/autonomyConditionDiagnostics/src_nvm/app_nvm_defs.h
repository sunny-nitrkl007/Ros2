/******************************************************************************************************************
 Copyright 2005-2013 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: app_nvm_defs.h

Description:  Header file for the application NVM definitions.
******************************************************************************************************************/

#ifndef __APP_NVM_DEFS_H__
#define __APP_NVM_DEFS_H__
/***********************************************************************
***
***    Referenced Functions, Data, & Symbols
***    -- #Include's --
***
***********************************************************************/
#include <bsp_nvm_serial_defs.h>    /* for device constants */
#include <nvm.h>
#include "app_flags.h"


/***********************************************************************
***
***    Constants defined for this file
***    -- #Define's --
***
***********************************************************************/

/********************************************************************
* #define list for the entire NVM device:
*********************************************************************/
/* Use a void pointer as type of QSMCM starting address. The type will
   be recasted in bsp function bsp_nvm_device_cfg(). */
#define QSMCM (* (void*)(0x00305000))

/* All the platform specific defines for initialising nvm are moved
   to app_flags.h
   Platform names are renamed with appropriate memory device names
   NVM1 is removed
   MMRAM and BBRAM defines are added
*/


/**************************************************
* #define list for device NVM0:
***************************************************/
#if (defined AT25640_fl)
   #define NVM0_ADDR            ((unsigned_32) &QSMCM)
   #define NVM0_SIZE            BSP_NVM_AT25640_SIZE
   #define NVM0_PAGE_SIZE       BSP_NVM_AT25640_PAGE_SIZE
#elif (defined AT25256_fl)
   #define NVM0_ADDR             ((unsigned_32) &QSMCM)
      /*
      Updated to match change in bsp_nvm_serial_defs.h in which
      BSP_NVM_AT25256_SIZE changed from 32768 back to 32678 which was an error
      but must be retained for m1. A new macro name
      BSP_NVM_AT25256_SIZE_CORRECTED was added for use by other platforms.
      The interface should be improved.  Applications should not
      even know the device type!!!!!!!!!!!!!
      */
   #if (defined NVM_SIZE_CORRECTED_F1)
      #define NVM0_SIZE             BSP_NVM_AT25256_SIZE
   #else
      #define NVM0_SIZE             BSP_NVM_AT25256_SIZE_CORRECTED
   #endif
   #define NVM0_PAGE_SIZE       BSP_NVM_AT25256_PAGE_SIZE
#elif (defined M95640_fl)
   #define NVM0_ADDR             ((unsigned_32) &QSMCM)
   #define NVM0_SIZE             BSP_NVM_M95640_SIZE
   #define NVM0_PAGE_SIZE       BSP_NVM_M95640_PAGE_SIZE
#elif (defined X25160_fl)           
   #define NVM0_ADDR                ((unsigned_32) &QSMCM)
   #define NVM0_SIZE                BSP_NVM_X25160_SIZE
   #define NVM0_PAGE_SIZE       BSP_NVM_X25160_PAGE_SIZE
#elif (defined X25LC256_f1)
   #define NVM0_ADDR          (unsigned_32)(0xFE000000)
   #ifdef SMALL_MEM
      /* the NVM0_SIZE is reduced due to the memory constraint in 5GC board*/
      #ifdef OBD_CRIT_CONFIG_fl
         #define NVM0_SIZE          3072
      #else
         #define NVM0_SIZE          1460
      #endif
   #elif ULTRA_SMALL_MEM_f1
   #define NVM0_SIZE          1024
   #else
   #define NVM0_SIZE          8192
   #endif
   #define NVM0_PAGE_SIZE     32
#elif (defined NVSRAM_DEVICE_fl) 
/* A5E6I1 doesnot support EEPROM. 
   NVM0 device configured to support NVSRAM */ 
   #define NVM0_ADDR           NVSRAM_ADDRESS
   #define NVM0_SIZE           0x80000
   #define NVM0_PAGE_SIZE      0x80000
#elif (defined MRAM_DEVICE_fl)
/* A5L3I1 doesnot support EEPROM. 
   NVM0 device configured to support MRAM */
   #define NVM0
   #define NVM0_ADDR           MRAM_ADDRESS
   #define NVM0_SIZE           0x80000
   #define NVM0_PAGE_SIZE      0x80000
#elif (defined X24LC512_f1)
   #define NVM0_ADDR          (unsigned_32)(0x00000000)
   #define NVM0_SIZE          BSP_NVM_24LC512_SIZE
   #define NVM0_PAGE_SIZE     BSP_NVM_24LC512_PAGE_SIZE
#elif (defined M95512_fl)
   #define NVM0_ADDR             ((unsigned_32) &QSMCM)
   #define NVM0_SIZE             BSP_NVM_M95512_SIZE
   #define NVM0_PAGE_SIZE        BSP_NVM_M95512_PAGE_SIZE
#elif (defined M95256_fl)
   #define NVM0_ADDR             ((unsigned_32) &QSMCM)
   #define NVM0_SIZE             BSP_NVM_M95256_SIZE
   #define NVM0_PAGE_SIZE        BSP_NVM_M95256_PAGE_SIZE
#elif (defined M95512_V2_fl)
   #define NVM0_ADDR             ((unsigned_32) &QSMCM)
   #define NVM0_SIZE             BSP_NVM_M95512_SIZE
   #define NVM0_PAGE_SIZE        BSP_NVM_M95512_PAGE_SIZE
#elif (defined AT25080B_f1)
   #define NVM0
   #define NVM0_ADDR          ((unsigned_32)0x00000000)
   #define NVM0_SIZE          BSP_NVM_AT25080B_SIZE       
   #define NVM0_PAGE_SIZE     BSP_NVM_AT25080B_PAGE_SIZE   
#endif


#define APP_NVM0_SIZE       NVM0_SIZE
#define APP_NVM0_ADDR       NVM0_ADDR
#define APP_NVM0_NUMBER     0


/**************************************************
* #define list for BBRAM:
***************************************************/

#if (defined N2_PI1)
#define NVM1_ADDR           0x80000000
#define NVM1_SIZE           2097152 /* 2M */
#define NVM1_PAGE_SIZE      2097152
#elif (defined A4N1I1)
#define NVM1_ADDR           0xC00000
#define NVM1_SIZE           2097152 /* 2M */
#define NVM1_PAGE_SIZE      2097152
#elif (defined E4_PI2)
#define NVM1_ADDR           0xC00000
#define NVM1_SIZE           APP_NVM_OBD_ES_SIZ /* 128 K */
#define NVM1_PAGE_SIZE      131072
#elif (defined A4E2I2)
#define NVM1_ADDR           0xC00000
#define NVM1_SIZE           131072 /* 128 K */
#define NVM1_PAGE_SIZE      131072
#elif (defined A5E4)
#define NVM1_ADDR           0x38000000
#define NVM1_SIZE           131072 /* 128 K */
#define NVM1_PAGE_SIZE      131072
#elif (defined A5L3I1)
#define NVM1_ADDR           NVM0_ADDR
#define NVM1_SIZE           NVM0_SIZE /* MRAM size */
#define NVM1_PAGE_SIZE      NVM0_PAGE_SIZE
#elif (defined A4GXI1 || defined A4G4I1)
#define NVM1_SIZE           131072
#define NVM1_ADDR           0x01000000
#define NVM1_PAGE_SIZE      131072
#elif (defined A5M4I1)
#define NVM1_ADDR           0x30000000
#define NVM1_SIZE           131072 /* NVSRAM size */
#define NVM1_PAGE_SIZE      131072
#elif (defined INT_MRAM_DEVICE_fl)
/* A5M81 doesnot have NVM support . 
   Part of Internal RAM configured as NVM1. */
#define NVM1_ADDR           INT_MRAM_ADDRESS
#define NVM1_SIZE           INT_MRAM_SIZE
#else
#define NVM1_ADDR           0
#define NVM1_SIZE           0
#define NVM1_PAGE_SIZE      0
#endif

#define APP_NVM1_SIZE       NVM1_SIZE
#define APP_NVM1_ADDR       NVM1_ADDR
#define APP_NVM1_NUMBER     0



/**********************************************
* Define the return values for app_nvm_cfg(). 
***********************************************/
#define APP_NVM_SUCCESS           0
#define APP_NVM_BLK_INIT_ERR      1
#define APP_NVM_BLK_WR_ERR        100
#define APP_NVM_DEV_INIT_ERR      253
#define APP_NVM_DEV_CFG_ERR       254
#define APP_NVM_DEVICE_FLAG_ERR   255


#endif /* __APP_NVM_DEFS_H__ */
