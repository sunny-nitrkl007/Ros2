/******************************************************************************************************************
 Copyright 2017 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: app_nvm_file_cfg.c

Description:This file contains the nvm_file configuration details
******************************************************************************************************************/

#include "app_nvm_file_cfg.h"
#include "app_nvm_globals.h"
#include "app_nvm_file_access.h"

char NvmFileName[200];
char NvmFilepath[200];
char NvmCalibrationfileName[200];
char NvmLiftCalfileName[200];
char NvmTiltCalfileName[200];

const nvm_file_block_cfg_t app_nvm_file_cfg[]=
{
    {
    	LPS_WEIGH_MACH_SPECIFIC_CFG_NVID,
    	NvmFileName,
        NULL,
        sizeof(LpsSaWeighNvmMachSpecificCfg_t),
        NVM_FILE_CRC_BLK
    },
      {
        LPS_WEIGH_NVM_CAL_NVID,
      	NvmCalibrationfileName,
      	NULL,
      	sizeof(LpsSaCalNvmTbl_t),
      	NVM_FILE_CRC_BLK
      },
      {
        LPS_WEIGH_NVM_LIFT_CAL_NVID,
		NvmLiftCalfileName,
		NULL,
		sizeof(LpsSaLiftCalNvmTbl_t),
		NVM_FILE_CRC_BLK
       }
    ,
	  {
		LPS_WEIGH_NVM_TILT_CAL_NVID,
		NvmTiltCalfileName,
		NULL,
		sizeof(LpsSaTiltCalNvmTbl_t),
		NVM_FILE_CRC_BLK
	   }
};

const unsigned short int BlockSize = (sizeof(app_nvm_file_cfg)/sizeof(app_nvm_file_cfg[0]));
