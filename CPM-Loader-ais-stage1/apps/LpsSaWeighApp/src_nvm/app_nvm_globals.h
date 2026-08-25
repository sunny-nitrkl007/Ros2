/******************************************************************************************************************
 Copyright 2017 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
FILE NAME :app_nvm_globals.h

DESCRIPTION :Global data for nvm.
******************************************************************************************************************/
#ifndef __APP_NVM_GLOBALS_H__
#define __APP_NVM_GLOBALS_H__

#include <nvm.h>
#include <std_types.h>

typedef enum
{
	LPS_WEIGH_MACH_SPECIFIC_CFG_NVID = NVM_MIN_APP_DATA_ID,
	LPS_WEIGH_NVM_ZERO_WEIGHT_NVID,
	LPS_WEIGH_NVM_CAL_NVID,
	LPS_WEIGH_NVM_LIFT_CAL_NVID,
	LPS_WEIGH_NVM_TILT_CAL_NVID
} LpsWeighNvmId_t;

/*For EEPROMs with less than 1k size, like with A5M15I1
  the size of the cache is reduced to support 11 faults
  */
#ifdef ULTRA_SMALL_MEM_f1
#define APP_NVM_OBD_ES_SIZ_SMALL_MEM    564
#else
#define APP_NVM_OBD_ES_SIZ_SMALL_MEM    1300
#endif

extern struct stat nvm;
extern NVM_block_handle_t  LpsSaWeighMachSpecificCfgNvmBlkHndl ;
extern NVM_block_handle_t  LpsSaWeighCalNvmBlkHndl;
extern NVM_block_handle_t  LpsSaWeighTiltCalNvmBlkHndl;
extern NVM_block_handle_t  LpsSaWeighLiftCalNvmBlkHndl;

extern nvm_file_block_cfg_t const app_nvm_file_cfg[];
extern char NvmFileName[200];
extern char NvmFilepath[200];
extern const unsigned short int BlockSize;
extern char NvmCalibrationfileName[200];
extern char NvmLiftCalfileName[200];
extern char NvmTiltCalfileName[200];

#if (defined NVSRAM_DEVICE_fl) /* A5E6I1 does not support EEPROM */
/* Declare the NVM drivers in nvsram_ddt_ptr. */
extern NVM_driver_table_t nvsram_ddt_ptr;
/*For memory device MRAM and BBRAM*/
#elif (defined MRAM_DEVICE_fl)
/* Declare the NVM drivers in mram_ddt_ptr. */
extern NVM_driver_table_t mram_ddt_ptr;
#else
/* Declare the NVM drivers in eeprom_ddt_ptr. */
#endif

#endif
