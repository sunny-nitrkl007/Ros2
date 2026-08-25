/******************************************************************************
**
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%
*** %% COPYRIGHT (C) 2010-2010 CATERPILLAR INC. ALL RIGHTS RESERVED.
*** %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
**
*** FILE:
***     autonomy_diagnostics_config.h
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

#ifndef AUTONOMY_DIAGNOSTICS_CONFIG_H_
#define AUTONOMY_DIAGNOSTICS_CONFIG_H_

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

#define APP_AUTONOMY_DIAG_SCL_OBD_MAX_FAULTS  50

#define PROD_ID_CID                    967
#define TILT_LNK_POS_CID               351
#define LIFT_LNK_POS_CID               350
#define LIFT_RE_POS_CID                769
#define LIFT_HE_POS_CID                364
#define TILT_RE_POS_CID                458
#define TILT_HE_POS_CID                765
#define HYD_OIL_TEMP_CID               600
#define PLOAD_MON_SYS_CID              2183
#define ENG_CID                        590
#define TRANS_CID                      296
#define ANALYSIS_CID                   1089
#define DISPLAY_CID                    2448
#define IMPL_CID                       596
#define DL_J19391_CID                  247
#define DL_J19392_CID                  2348
#define DL_J19393_CID                  5856
#define DL_ETHERNET1_CID               3900
#define DL_CDL_CID                     248

/*****************************************************************************
***
***    Data types defined in this file
***    -- Struct's, Typedef's, Enum's --
***
*****************************************************************************/
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_prod_id_not_recd_lv2;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_robot_file_missing_lv2;

extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_link_pos_vlt_abv_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_link_pos_vlt_blw_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_link_pos_abnorm_pwm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_link_pos_out_calib_lv1;

extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_link_pos_vlt_abv_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_link_pos_vlt_blw_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_link_pos_abnorm_pwm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_link_pos_out_calib_lv1;

extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_re_pos_vlt_abv_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_re_pos_vlt_blw_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_re_pos_abnorm_pwm_lv1;

extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_he_pos_vlt_abv_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_he_pos_vlt_blw_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_lift_he_pos_abnorm_pwm_lv1;

extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_re_pos_vlt_abv_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_re_pos_vlt_blw_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_re_pos_abnorm_pwm_lv1;

extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_he_pos_vlt_abv_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_he_pos_vlt_blw_norm_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_tilt_he_pos_abnorm_pwm_lv1;

extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_pay_load_mon_sys_out_calib_lv1;

extern scl_ci_es_fault_cfg_eddt_t 	 app_diagcfg_mach_not_set_lv1;

extern scl_obd_es_test_config_t      const app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1_es;
extern scl_eddt_fault_config_t       const app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1_dl;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1;
extern scl_obd_es_test_config_t      const app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1_es;
extern scl_eddt_fault_config_t       const app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1_dl;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1;

extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_9_lv1;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_14_lv1;

extern scl_obd_es_test_config_t      const app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1_es;
extern scl_eddt_fault_config_t       const app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1_dl;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1;
extern scl_obd_es_test_config_t      const app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1_es;
extern scl_eddt_fault_config_t       const app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1_dl;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1;

extern scl_obd_es_test_config_t      const app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1_es;
extern scl_eddt_fault_config_t       const app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1_dl;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1;
extern scl_obd_es_test_config_t      const app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1_es;
extern scl_eddt_fault_config_t       const app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1_dl;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1;

extern scl_obd_es_test_config_t      const app_diagcfg_DL_CDL_CID_248_FMI_9_lv1_es;
extern scl_eddt_fault_config_t       const app_diagcfg_DL_CDL_CID_248_FMI_9_lv1_dl;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_CDL_CID_248_FMI_9_lv1;
extern scl_obd_es_test_config_t      const app_diagcfg_DL_CDL_CID_248_FMI_14_lv1_es;
extern scl_eddt_fault_config_t       const app_diagcfg_DL_CDL_CID_248_FMI_14_lv1_dl;
extern scl_ci_es_fault_cfg_eddt_t    app_diagcfg_DL_CDL_CID_248_FMI_14_lv1;


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

#endif /* AUTONOMY_DIAGNOSTICS_CONFIG_H_ */
