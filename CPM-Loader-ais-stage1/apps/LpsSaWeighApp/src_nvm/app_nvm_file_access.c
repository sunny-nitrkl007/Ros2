/*******************************************************************************
 Copyright 2017 Caterpillar Inc. All rights reserved.
--------------------------------------------------------------------------------
File name: app_nvm_file_access.c

Description:This file contains test code to test the nvm_file feature 
 *******************************************************************************/
#include "app_nvm_globals.h"
#include "app_nvm_file_access.h"
#include <stdio.h>

/*******************************************************************************
Function name: app_nvm_file_remove

Description:
remove the nvm_file.

Parameter Description:
    LPS_WEIGH_MACH_SPECIFIC_CFG_NVID
    LPS_WEIGH_NVM_ZERO_WEIGHT_NVID
    LPS_WEIGH_NVM_CAL_NVID
    LPS_WEIGH_NVM_LIFT_CAL_NVID
    LPS_WEIGH_NVM_TILT_CAL_NVID

Return Description:
    nvm_id NVM ID of the file
 *******************************************************************************/
boolean app_nvm_file_remove(LpsWeighNvmId_t nvm_id)
{
    switch(nvm_id)
    {
    case LPS_WEIGH_MACH_SPECIFIC_CFG_NVID:
        if (remove(NvmFileName) == 0) {
            return TRUE;
        }
        break;

    case LPS_WEIGH_NVM_CAL_NVID:
        if (remove(NvmCalibrationfileName) == 0) {
            return TRUE;
        }
        break;

    case LPS_WEIGH_NVM_LIFT_CAL_NVID:
        if (remove(NvmLiftCalfileName) == 0) {
            return TRUE;
        }
        break;

    case LPS_WEIGH_NVM_TILT_CAL_NVID:
        if (remove(NvmTiltCalfileName) == 0) {
            return TRUE;
        }
        break;

    default:
        printf("app_nvm_file_remove ERROR: unsupported file ID %u", (int)nvm_id);
        break;
    }
    return FALSE;
}
