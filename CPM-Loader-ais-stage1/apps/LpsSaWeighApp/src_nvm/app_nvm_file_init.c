/******************************************************************************************************************
 Copyright 2017 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: app_nvm_file_init.c

Description:This file contains startup and initialization code for nvm_file
******************************************************************************************************************/

#include "app_nvm_file_init.h"
#include "app_nvm_file_cfg.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include "app_nvm_file_access.h"


NVM_block_handle_t  LpsSaWeighMachSpecificCfgNvmBlkHndl;
NVM_block_handle_t  LpsSaWeighCalNvmBlkHndl;

NVM_block_handle_t  LpsSaWeighTiltCalNvmBlkHndl;
NVM_block_handle_t  LpsSaWeighLiftCalNvmBlkHndl;

/* DEPRECATED: Holds actual values stored in NVM for machine specific configuration */
volatile LpsSaWeighNvmMachSpecificCfg_t LpsSaWeighNvmMachSpecificCfg;

volatile LpsSaCalNvmTbl_t LpsSaWeighDefCal = {0};

/* Holds actual values stored in NVM for cal values */
volatile LpsSaCalNvmTbl_t LpsSaWeighNvmCal;

/* DEPRECATED: Holds actual values stored in NVM for LIFT cal values */
volatile LpsSaLiftCalNvmTbl_t LpsSaWeighLiftNvmCal;

/* DEPRECATED: Holds actual values stored in NVM for Tilt cal values */
volatile LpsSaTiltCalNvmTbl_t LpsSaWeighTiltNvmCal;


/*Flag will be set when NVM file is not exist and the flag will be reset
*when the file was created or if the file already exist */

boolean NvmWeighingAppMachSpecificCfgFileExistsFlag=FALSE;
boolean NvmWeighingAppCalFileExistsFlag=FALSE;
boolean NvmWeighingAppLiftCalFileExistsFlag=FALSE;
boolean NvmWeighingAppTiltCalFileExistsFlag=FALSE;


static void app_nvm_weighapp_machine_spec_config_file_init();
static void app_nvm_calibration_values_init();
static void app_nvm_tilt_cal_values_init();
static void app_nvm_lift_cal_values_init();
struct stat nvm ={0};

/******************************************************************************************************************
Function name: app_nvm_file_init

Description:  
Intialzie the nvm_file. 

Parameter Description: 
    
Return Description:     
    None
*****************************************************************************************************************/
void app_nvm_file_init(void)
{
    char command[200];
    char id_char[100];
    struct stat nvmfile ={0};

    LpsSaNvmWeighAppMachSpecificCfgReadFlag  = FALSE;
    LpsSaNvmWeighAppCalReadFlag  = FALSE;
    LpsSaLiftNvmWeighAppCalReadFlag  = FALSE;
    LpsSaTiltNvmWeighAppCalReadFlag  = FALSE;

    strcpy(NvmFilepath, getenv("HOME"));
    strcat(NvmFilepath, "/appdata/CPM/LpsSaWeighApp/nvm");
    strcpy(id_char,"/NvmWeighAppMachSpecificCfg.txt");
    strcpy(NvmFileName, NvmFilepath);
    strcat(NvmFileName, id_char);
    strcpy(command, "mkdir --mode=755 -p ");
    strcat(command, NvmFilepath);

    /* Create location for NVM log, if it doesn't already exist.*/
    if(stat(NvmFilepath, &nvm) == -1)
    {
        int sys_ret = system(command);
        (void)sys_ret;
    }

    strcpy(id_char,"/NvmWeighAppCalibrationVal.txt");
    strcpy(NvmCalibrationfileName, NvmFilepath);
    strcat(NvmCalibrationfileName, id_char);

    strcpy(id_char,"/NvmWeighAppLiftCalVal.txt");
    strcpy(NvmLiftCalfileName, NvmFilepath);
    strcat(NvmLiftCalfileName, id_char);

    strcpy(id_char,"/NvmWeighAppTiltCalVal.txt");
    strcpy(NvmTiltCalfileName, NvmFilepath);
    strcat(NvmTiltCalfileName, id_char);

    if (stat(NvmFileName, &nvmfile) == 0) {
        NvmWeighingAppMachSpecificCfgFileExistsFlag = TRUE;
    }

    if (stat(NvmCalibrationfileName, &nvmfile) == 0) {
        NvmWeighingAppCalFileExistsFlag = TRUE;
    }

    if (stat(NvmLiftCalfileName, &nvmfile) == 0) {
        NvmWeighingAppLiftCalFileExistsFlag = TRUE;
    }

    if (stat(NvmTiltCalfileName, &nvmfile) == 0) {
        NvmWeighingAppTiltCalFileExistsFlag = TRUE;
    }

    if (NVM_SUCCESS == nvm_file_init(BlockSize))
    {
        int indx;
        for (indx = 0; indx < BlockSize; indx++)
        {
            unsigned int ret;

            ret = nvm_file_block_cfg(&app_nvm_file_cfg[indx]);

            if (NVM_SUCCESS == ret)
            {
                printf("nvm_file_block_cfg SUCCESS \n");
                switch (indx)
                {
                    case 0:
                        app_nvm_weighapp_machine_spec_config_file_init();
                        break;
                    case 1:
                        app_nvm_calibration_values_init();
                        break;
                    case 2:
                        app_nvm_lift_cal_values_init();
                        break;
                    case 3:
                        app_nvm_tilt_cal_values_init();
                        break;
                    default:/*Reset the after the folder was created*/
                        break;
                }
            }
            else
            {
                printf("nvm_file_block_cfg  %d failed\n", indx);
            }
        }
    }
    else
    {
        printf("nvm_file_init failed\n");
    }
}


static void app_nvm_weighapp_machine_spec_config_file_init()
{
    /* Only init if file already exists */
    if (NvmWeighingAppMachSpecificCfgFileExistsFlag == FALSE) {
        LpsSaNvmWeighAppMachSpecificCfgReadFlag = TRUE;
        return;
    }

    unsigned int ret;

    ret = nvm_block_init(
            NVM_BLOCK_CACHED,
            (unsigned_8 *)&LpsSaWeighNvmMachSpecificCfg,
            LPS_WEIGH_MACH_SPECIFIC_CFG_NVID,
            NVM_CURRENT_TASK_PRIORITY,
            &LpsSaWeighMachSpecificCfgNvmBlkHndl);


    if (NVM_SUCCESS != ret) {
        app_nvm_file_remove(LPS_WEIGH_MACH_SPECIFIC_CFG_NVID);
        NvmWeighingAppMachSpecificCfgFileExistsFlag = FALSE;
    }

    LpsSaNvmWeighAppMachSpecificCfgReadFlag = TRUE;
}

static void app_nvm_calibration_values_init()
{
    /* Only init if file already exists */
    if (NvmWeighingAppCalFileExistsFlag == FALSE) {
        LpsSaNvmWeighAppCalReadFlag = TRUE;
        return;
    }

    unsigned int ret;

    ret = nvm_block_init(
            NVM_BLOCK_CACHED,
            (unsigned_8 *)&LpsSaWeighNvmCal,
            LPS_WEIGH_NVM_CAL_NVID,
            NVM_CURRENT_TASK_PRIORITY,
            &LpsSaWeighCalNvmBlkHndl);

    if (NVM_SUCCESS != ret) {
        app_nvm_file_remove(LPS_WEIGH_NVM_CAL_NVID);
        NvmWeighingAppCalFileExistsFlag = FALSE;
    }

    LpsSaNvmWeighAppCalReadFlag = TRUE;
}



static void app_nvm_lift_cal_values_init()
{
    /* Only init if file already exists */
    if (NvmWeighingAppLiftCalFileExistsFlag == FALSE) {
        LpsSaLiftNvmWeighAppCalReadFlag = TRUE;
        return;
    }

    unsigned int ret;

    ret = nvm_block_init(
            NVM_BLOCK_CACHED,
            (unsigned_8 *)&LpsSaWeighLiftNvmCal,
            LPS_WEIGH_NVM_LIFT_CAL_NVID,
            NVM_CURRENT_TASK_PRIORITY,
            &LpsSaWeighLiftCalNvmBlkHndl);

    if (NVM_SUCCESS != ret) {
        app_nvm_file_remove(LPS_WEIGH_NVM_LIFT_CAL_NVID);
        NvmWeighingAppLiftCalFileExistsFlag = FALSE;
    }

    LpsSaLiftNvmWeighAppCalReadFlag = TRUE;
}

static void app_nvm_tilt_cal_values_init()
{
    /* Only init if file already exists */
    if (NvmWeighingAppTiltCalFileExistsFlag == FALSE) {
        LpsSaTiltNvmWeighAppCalReadFlag = TRUE;
        return;
    }

    unsigned int ret;

    ret = nvm_block_init(
            NVM_BLOCK_CACHED,
            (unsigned_8 *)&LpsSaWeighTiltNvmCal,
            LPS_WEIGH_NVM_TILT_CAL_NVID,
            NVM_CURRENT_TASK_PRIORITY,
            &LpsSaWeighTiltCalNvmBlkHndl);

    if (NVM_SUCCESS != ret) {
        app_nvm_file_remove(LPS_WEIGH_NVM_TILT_CAL_NVID);
        NvmWeighingAppTiltCalFileExistsFlag = FALSE;
    }

    LpsSaTiltNvmWeighAppCalReadFlag = TRUE;
}
