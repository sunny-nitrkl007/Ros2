/******************************************************************************
**
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%
*** %% COPYRIGHT (C) 2010-2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
*** %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
**
*** FILE:
***     autonomy_event_config.h
***
*** DESCRIPTION:
***     This is the definition include file for the module.  All of the 
***     contents in this file are intended for other tasks/modules to use.
***
***     IMPORTANT NOTE:  All global variables should be considered read-only
***                      by any external module that accesses them.
***                      It is forbidden for any routines besides those
***                      local to this module to alter these variables.
***
******************************************************************************/

#ifndef AUTONOMY_EVENT_CONFIG_H_
#define AUTONOMY_EVENT_CONFIG_H_

/*****************************************************************************
***
***    External function prototypes, data, & symbols
***    -- nested #Include's --
***
*****************************************************************************/

#ifndef   STD_TYPES_H_
#include <std_types.h>
#endif

#ifndef   SCL_CI_ES_H
#include <scl_ci_es.h>
#endif

#ifndef   CATDLLIB_FID_DEF_H_
#include <catdllib_fid_def.h>
#endif

#ifndef   AUTONOMY_DEV_IDS_H_
#include "diagnosticEventConfig/autonomy_dev_ids.h"  /* for development ids */
#endif

/*****************************************************************************
***
***    Constants defined in this file
***    -- #Define's --
***
*****************************************************************************/
#define APP_AUTONOMY_EVENT_SCL_OBD_MAX_FAULTS (10)

#define TASKS_EID (10013)
#define TASKC_EID (10012)
#define LOW_ECU_VOLTAGE_EID (1379)
#define HIGH_ECU_VOLTAGE_EID (1378)
#define PAYLOAD_OVERLOAD_LIMIT_EXCEEDED_EID (2126)
#define BEMSIM_EID (63028)
#define PAYLOAD_MEMORY_LOW_EID (2083)
#define PAYLOAD_MEMORY_FULL_EID (2138)
#define PAYLOAD_LFT_NOT_SEALED_EID (63091)


/*****************************************************************************
***
***    Data types defined in this file
***    -- Struct's, Typedef's, Enum's --
***
*****************************************************************************/

/*****************************************************************************
***
***    Functions prototyped in this file
***    -- Function Prototypes --
***
*****************************************************************************/

/*****************************************************************************
***
***    Data declared in this file
***    -- Global Symbols --
***
*****************************************************************************/
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_taskS_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_taskC_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_low_ecu_power_input_voltage_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_high_ecu_power_input_voltage_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_PayLd_OverLd_LimitExceeded_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_BEMSim_derate_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_BEMSim_chargeLevel_Warning_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_payload_memory_low_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_payload_memory_full_evnt_lv1;
extern scl_ci_es_fault_cfg_eddt_t app_evntcfg_payload_lft_not_sealed_evnt_lv1;

#endif /* AUTONOMY_EVENT_CONFIG_H_ */
