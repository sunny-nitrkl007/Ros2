/******************************************************************************************************************
 Copyright 2005-2013 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
FILE NAME :app_nvm_globals.h

DESCRIPTION :Global data for nvm.
******************************************************************************************************************/
#ifndef __APP_NVM_GLOBALS_H__
#define __APP_NVM_GLOBALS_H__

#include <nvm.h>
#include <std_types.h>

/*
#include <clock_nvm_blk_mod.h>
*/
#ifndef CLOCK_NVM_OFST_MOD_H_
#include <clock_nvm_ofst_mod.h>
#endif

/*  Define APP_NVM blocks specific to this application.  Define the 
 *  integrity, block size in bytes, actual block bytes used, block 
 *  identification (BID), type of BID (cached or not), offset in bytes 
 *  from the start of the base block, next block pointer, and previous 
 *  block pointer.
 *
 *  Extensibility is supported for RAW and CHK_SUM blocks only.
 *  Extension blocks must have the same BID as the base block.
 *  Mixing RAW and CHK_SUM blocks with the same BID is not supported.
 *  Mixing cached and uncached blocks with the same BID is not 
 *  supported.
 */

#define APP_NVM0_INT          NVM_RAW_BLK
#define APP_NVM0_TYP          NVM_BLOCK_CACHED
#define APP_NVM0_NEX          {0xFF,0xFFFF}
#define APP_NVM0_PRE          {0xFF,0xFFFF}     /* base block */

#define APP_NVM_CLOCK_IDX     0
#define APP_NVM_CLOCK_SIZ     128

#define APP_NVM1_INT          NVM_RAW_BLK
#define APP_NVM2_INT          NVM_RAW_BLK

#define clock_nvm_MAX_NVM_BLOCK_LEN \
      ((clock_nvm_ofst_t)( 2*255+ 22 )) /* for format 1 */


#define APP_NVM_CLOCK_BID     NVM_CORE_CLOCK_1_ID 

//#define APP_NVM_OBD_ES_IDX    1
//#define APP_NVM_OBD_ES_SIZ    1560

//#define APP_NVM_OBD_ES_BID    (NVM_MIN_APP_DATA_ID + 0x00)
//#define APP_RTC_DIST_TIMEZONE (NVM_MIN_APP_DATA_ID + 0x01)
//#define APP_PROD_ID_NVM       (NVM_MIN_APP_DATA_ID + 0x02)

//#define NVMDID                (NVM_MIN_APP_DATA_ID + 0x20)
//#define APP_RTC_DIST_TIMEZONE_SZ 25

/* sizes of the caches have been reduced since D5G1I1 has limited memory*/ 
#define APP_NVM_CLOCK_SIZ_SMALL_MEM     128
/*For EEPROMs with less than 1k size, like with A5M15I1 
  the size of the cache is reduced to support 11 faults
  */
#ifdef ULTRA_SMALL_MEM_f1               
#define APP_NVM_OBD_ES_SIZ_SMALL_MEM    564
#else
#define APP_NVM_OBD_ES_SIZ_SMALL_MEM    1300
#endif

/* Define the total count of the application APP_NVM blocks defined above
   including the signature block and the filler block. */
#define APP_NVM0_TOTAL_BLK_CNT      1

extern NVM_block_init_table_entry_t app_nvm_BIT[];
 
//extern NVM_block_handle_t     app_nvm_file_block_table[];
#if (defined NVSRAM_DEVICE_fl) /* A5E6I1 does not support EEPROM */
/* Declare the NVM drivers in nvsram_ddt_ptr. */
extern NVM_driver_table_t nvsram_ddt_ptr;
/*For memory device MRAM and BBRAM*/
#elif (defined MRAM_DEVICE_fl) 
/* Declare the NVM drivers in mram_ddt_ptr. */
extern NVM_driver_table_t mram_ddt_ptr;
#else
/* Declare the NVM drivers in eeprom_ddt_ptr. */
extern NVM_driver_table_t     eeprom_ddt_ptr;

/* buffer for eeprom cache */
extern unsigned_8 app_eeprom_cache1[];
extern unsigned_8 app_eeprom_cache2[];
#endif

#endif
