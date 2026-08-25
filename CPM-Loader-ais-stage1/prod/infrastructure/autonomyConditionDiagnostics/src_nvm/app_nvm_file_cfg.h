/*******************************************************************************
 Copyright 2011-2016 Caterpillar Inc. All rights reserved.
--------------------------------------------------------------------------------
File name: app_nvm_file_cfg.h

Description:This file contains the nvm_file configuration details
*******************************************************************************/

#ifndef __APP_NVM_FILE_CFG_H__
#define __APP_NVM_FILE_CFG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <nvm.h>
#include <nvm_data_id_defs.h>
#include "app_flags.h"

#define APP_NVM_MAX_NUM_BLOCKS (13)

/* Maps to "NVM" key in AutonomyConditionDiagnostics.rb */
typedef enum
{
    NVM_KEY_DIAGNOSTICS_EVENTS_LOG = 0,
    NVM_KEY_SERVICE_CLOCK_PRIMARY,
    NVM_KEY_SERVICE_CLOCK_SECONDARY,
    NVM_KEY_RTC_TIME_ZONE_OLD,
    NVM_KEY_RTC_TIME_ZONE_NEW,
    NVM_KEY_RTC_SECURE,
    NVM_KEY_PRODUCTID,
    NVM_KEY_SEA1,
    NVM_KEY_SEA2,
    NVM_KEY_SEA3,
    NVM_KEY_J39_HEALTH_BDT_CAN_A,
    NVM_KEY_J39_HEALTH_BDT_CAN_B,
    NVM_KEY_EQUIPMENT_ID,
} nvm_file_key_e;

/* Number of nvm file blocks */
extern int nvm_file_block_size;
extern nvm_file_block_cfg_t* app_nvm_file_cfg;

/* NVM file block handles */
extern NVM_block_handle_t app_nvm_file_block_table[];
#ifdef __cplusplus
}
#endif

#endif /* __APP_NVM_FILE_CFG_H__ */
