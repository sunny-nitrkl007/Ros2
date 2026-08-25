/******************************************************************************
**
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%
*** %% COPYRIGHT (C) 2001-2010 CATERPILLAR INC. ALL RIGHTS RESERVED.
*** %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
**
*** FILE:
***     autonomy_diagnostics_config.c
***
*** DESCRIPTION:
***     Autonomy diagnostic configurations.
***
******************************************************************************/

/******************************************************************************
***
***    External function prototypes, data, & symbols
***    -- #Include's --
***
******************************************************************************/

#ifndef   AUTONOMY_DIAGNOSTICS_CONFIG_H_
#include "Loader_autonomy_diagnostics_config.h"
#endif

/******************************************************************************
***
***    Constants defined for this file
***    -- #Define's --
***
******************************************************************************/

/******************************************************************************
***
***    Data types defined for this file
***    -- Struct's, Typedef's, Enum's --
***
******************************************************************************/

/******************************************************************************
***
***    File scope functions defined for this file
***    -- Function Prototypes --
***
******************************************************************************/

/******************************************************************************
***
***    File scope symbols defined for this file
***    -- Symbols --
***
******************************************************************************/

/******************************************************************************
***
***    Global data symbols defined in this file
***    -- Global Symbols --
***
******************************************************************************/
/* Diag Handle and Config for Product ID not Received */
/* maximum allowed description ->             "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char     prod_id_not_recd_diag_text[] = "Product ID not Recd";

/*
** Product ID not Received (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_prod_id_not_recd_lv2_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       PROD_ID_CID,
       FMIIUA,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Product ID not Received  (Level II) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_prod_id_not_recd_lv2_dl =
{
                          /*Flex Text Description                           */
     prod_id_not_recd_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     PROD_ID_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIUA,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Product ID not Received (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_prod_id_not_recd_lv2 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_prod_id_not_recd_lv2_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_prod_id_not_recd_lv2_dl
);


/* Diag Handle and Config for Robot File Missing */
/* maximum allowed description ->             "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char     robot_file_missing_diag_text[] = "Robot File Missing";

/*
** Robot File Missing (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_robot_file_missing_lv2_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       PROD_ID_CID,
       FMIEII,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Robot File Missing  (Level II) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_robot_file_missing_lv2_dl =
{
                          /*Flex Text Description                           */
     robot_file_missing_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     PROD_ID_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIEII,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Robot File Missing (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_robot_file_missing_lv2 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_robot_file_missing_lv2_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_robot_file_missing_lv2_dl
);


/* Diag Handle and Config for Tilt Linkage Position Sensor: Voltage Above Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_link_pos_vlt_abv_norm_diag_text[] = "Tilt Link Volt Above";

/*
** Tilt Linkage Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_link_pos_vlt_abv_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_LNK_POS_CID,
       FMIOB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt Linkage Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_link_pos_vlt_abv_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_link_pos_vlt_abv_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_LNK_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIOB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt Linkage Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_link_pos_vlt_abv_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_link_pos_vlt_abv_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_link_pos_vlt_abv_norm_lv1_dl
);


/* Diag Handle and Config for Tilt Linkage Position Sensor: Voltage Below Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_link_pos_vlt_blw_norm_diag_text[] = "Tilt Link Volt Below";

/*
** Tilt Linkage Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_link_pos_vlt_blw_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_LNK_POS_CID,
       FMIIG,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt Linkage Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_link_pos_vlt_blw_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_link_pos_vlt_blw_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_LNK_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIG,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt Linkage Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_link_pos_vlt_blw_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_link_pos_vlt_blw_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_link_pos_vlt_blw_norm_lv1_dl
);


/* Diag Handle and Config for Tilt Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period */
/* maximum allowed description ->                 "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_link_pos_abnorm_pwm_diag_text[] = "Tilt Link Freq Abnor";

/*
** Tilt Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_link_pos_abnorm_pwm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_LNK_POS_CID,
       FMIIAB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_link_pos_abnorm_pwm_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_link_pos_abnorm_pwm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_LNK_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIAB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_link_pos_abnorm_pwm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_link_pos_abnorm_pwm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_link_pos_abnorm_pwm_lv1_dl
);


/* Diag Handle and Config for Tilt Linkage Position Sensor: Out of Calibration */
/* maximum allowed description ->                "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_link_pos_out_calib_diag_text[] = "Tilt Link Calib Out";

/*
** Tilt Linkage Position Sensor: Out of Calibration (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_link_pos_out_calib_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_LNK_POS_CID,
       FMICAL,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON           /*Continuous Monitor                */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt Linkage Position Sensor: Out of Calibration (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_link_pos_out_calib_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_link_pos_out_calib_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_LNK_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMICAL,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt Linkage Position Sensor: Out of Calibration (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_link_pos_out_calib_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_link_pos_out_calib_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_link_pos_out_calib_lv1_dl
);

/* Lift Sensor */


/* Diag Handle and Config for Lift Linkage Position Sensor: Voltage Above Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_link_pos_vlt_abv_norm_diag_text[] = "Lift Link Volt Above";

/*
** Lift Linkage Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_link_pos_vlt_abv_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_LNK_POS_CID,
       FMIOB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Linkage Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_link_pos_vlt_abv_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_link_pos_vlt_abv_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LIFT_LNK_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIOB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Linkage Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_link_pos_vlt_abv_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_link_pos_vlt_abv_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_link_pos_vlt_abv_norm_lv1_dl
);


/* Diag Handle and Config for Lift Linkage Position Sensor: Voltage Below Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_link_pos_vlt_blw_norm_diag_text[] = "Lift Link Volt Below";

/*
** Lift Linkage Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_link_pos_vlt_blw_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_LNK_POS_CID,
       FMIIG,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Linkage Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_link_pos_vlt_blw_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_link_pos_vlt_blw_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LIFT_LNK_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIG,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Linkage Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_link_pos_vlt_blw_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_link_pos_vlt_blw_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_link_pos_vlt_blw_norm_lv1_dl
);

/* Diag Handle and Config for Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period */
/* maximum allowed description ->                 "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_link_pos_abnorm_pwm_diag_text[] = "Lift Link Freq Abnor";

/*
** Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_link_pos_abnorm_pwm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_LNK_POS_CID,
       FMIIAB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_link_pos_abnorm_pwm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_link_pos_abnorm_pwm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LIFT_LNK_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIAB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_link_pos_abnorm_pwm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_link_pos_abnorm_pwm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_link_pos_abnorm_pwm_lv1_dl
);

/* Diag Handle and Config for lift Linkage Position Sensor: Out of Calibration */
/* maximum allowed description ->                "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_link_pos_out_calib_diag_text[] = "Lift Link Calib Out";

/*
** Lift Linkage Position Sensor: Out of Calibration (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_link_pos_out_calib_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_LNK_POS_CID,
       FMICAL,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON           /*Continuous Monitor                */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Linkage Position Sensor: Out of Calibration (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_link_pos_out_calib_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_link_pos_out_calib_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LIFT_LNK_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMICAL,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Linkage Position Sensor: Out of Calibration (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_link_pos_out_calib_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_link_pos_out_calib_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_link_pos_out_calib_lv1_dl
);


/* Rod End Sensor */

/* Diag Handle and Config for Lift Rod end Position Sensor: Voltage Above Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_re_pos_vlt_abv_norm_diag_text[] = "Lift Rod end Volt Above";

/*
** Lift Rod end Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_re_pos_vlt_abv_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_RE_POS_CID,
       FMIOB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Rod end Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_re_pos_vlt_abv_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_re_pos_vlt_abv_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LIFT_RE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIOB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Rod End Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_re_pos_vlt_abv_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_re_pos_vlt_abv_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_re_pos_vlt_abv_norm_lv1_dl
);


/* Diag Handle and Config for Lift Rod End Position Sensor: Voltage Below Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_re_pos_vlt_blw_norm_diag_text[] = "Lift Rod End Volt Below";

/*
** Lift Rod End Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_re_pos_vlt_blw_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_RE_POS_CID,
       FMIIG,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Rod End Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_re_pos_vlt_blw_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_re_pos_vlt_blw_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LIFT_RE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIG,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Rod End Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_re_pos_vlt_blw_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_re_pos_vlt_blw_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_re_pos_vlt_blw_norm_lv1_dl
);

/* Diag Handle and Config for Rod End Position Sensor: Abnormal Frequency, Pulse Width, or Period */
/* maximum allowed description ->                 "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_re_pos_abnorm_pwm_diag_text[] = "Lift Rod End sensor Freq Abnor";

/*
** Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_re_pos_abnorm_pwm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_RE_POS_CID,
       FMIIAB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_re_pos_abnorm_pwm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_re_pos_abnorm_pwm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
	 LIFT_RE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIAB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_re_pos_abnorm_pwm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_re_pos_abnorm_pwm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_re_pos_abnorm_pwm_lv1_dl
);

/* Head End Position sensor */

/* Diag Handle and Config for Lift Head end Position Sensor: Voltage Above Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_he_pos_vlt_abv_norm_diag_text[] = "Lift Head end Volt Above";

/*
** Lift Head end Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_he_pos_vlt_abv_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_HE_POS_CID,
       FMIOB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Head end Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_he_pos_vlt_abv_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_he_pos_vlt_abv_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LIFT_HE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIOB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Rod End Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_he_pos_vlt_abv_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_he_pos_vlt_abv_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_he_pos_vlt_abv_norm_lv1_dl
);

/* Diag Handle and Config for Lift Head End Position Sensor: Voltage Below Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_he_pos_vlt_blw_norm_diag_text[] = "Lift Head End Volt Below";

/*
** Lift Head End Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_he_pos_vlt_blw_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_HE_POS_CID,
       FMIIG,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Head End Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_he_pos_vlt_blw_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_he_pos_vlt_blw_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LIFT_HE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIG,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Head End Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_he_pos_vlt_blw_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_he_pos_vlt_blw_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_he_pos_vlt_blw_norm_lv1_dl
);
/* Diag Handle and Config for Head End Position Sensor: Abnormal Frequency, Pulse Width, or Period */
/* maximum allowed description ->                 "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char lift_he_pos_abnorm_pwm_diag_text[] = "Lift Head End sensor Freq Abnor";

/*
** Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_lift_he_pos_abnorm_pwm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       LIFT_HE_POS_CID,
       FMIIAB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_lift_he_pos_abnorm_pwm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_he_pos_abnorm_pwm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
	 LIFT_HE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIAB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Linkage Position Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_lift_he_pos_abnorm_pwm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_lift_he_pos_abnorm_pwm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_lift_he_pos_abnorm_pwm_lv1_dl
);


/* Diag Handle and Config for lift Linkage Position Sensor: Out of Calibration */
/* maximum allowed description ->                "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char pay_load_mon_sys_out_calib_diag_text[] = "Payload Mon Calib Out";

/*
** Pay load Monitoring system: Out of Calibration (Level 2) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_pay_load_mon_sys_out_calib_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
    		 PLOAD_MON_SYS_CID,
       FMICAL,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON           /*Continuous Monitor                */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Lift Linkage Position Sensor: Out of Calibration (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_pay_load_mon_sys_out_calib_lv1_dl =
{
                          /*Flex Text Description                           */
		pay_load_mon_sys_out_calib_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
	 PLOAD_MON_SYS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMICAL,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Lift Linkage Position Sensor: Out of Calibration (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_pay_load_mon_sys_out_calib_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_pay_load_mon_sys_out_calib_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_pay_load_mon_sys_out_calib_lv1_dl
);

/*
** Machine Model Not Set (Level 2) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_mach_not_set_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       PROD_ID_CID,
       FMI14,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON           /*Continuous Monitor                */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Machine Model Not Set (Level 2) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_mach_not_set_lv1_dl =
{
                          /*Flex Text Description                           */
		pay_load_mon_sys_out_calib_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
	 PROD_ID_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMI14,               /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Machine Model Not Set (Level 2) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_mach_not_set_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_mach_not_set_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_mach_not_set_lv1_dl
);


/* Rod End Sensor */

/* Diag Handle and Config for Tilt Rod end Pressure Sensor: Voltage Above Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_re_pos_vlt_abv_norm_diag_text[] = "Tilt Rod end Volt Above";

/*
** TILT Rod end Pressure Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_re_pos_vlt_abv_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_RE_POS_CID,
       FMIOB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt Rod end Pressure Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_re_pos_vlt_abv_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_re_pos_vlt_abv_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_RE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIOB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt Rod End Pressure Sensor: Voltage Above Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_re_pos_vlt_abv_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_re_pos_vlt_abv_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_re_pos_vlt_abv_norm_lv1_dl
);


/* Diag Handle and Config for Tilt Rod End Pressure Sensor: Voltage Below Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_re_pos_vlt_blw_norm_diag_text[] = "Tilt Rod End Volt Below";

/*
** Lift Rod End Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_re_pos_vlt_blw_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_RE_POS_CID,
       FMIIG,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt Rod End Pressure Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_re_pos_vlt_blw_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_re_pos_vlt_blw_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_RE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIG,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt Rod End Pressure Sensor: Voltage Below Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_re_pos_vlt_blw_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_re_pos_vlt_blw_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_re_pos_vlt_blw_norm_lv1_dl
);

/* Diag Handle and Config for Rod End Position Sensor: Abnormal Frequency, Pulse Width, or Period */
/* maximum allowed description ->                 "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_re_pos_abnorm_pwm_diag_text[] = "Tilt Rod End sensor Freq Abnor";

/*
** Tilt RE Pressure Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_re_pos_abnorm_pwm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_RE_POS_CID,
       FMIIAB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt RE Pressure Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_re_pos_abnorm_pwm_lv1_dl =
{
                          /*Flex Text Description                           */
     lift_re_pos_abnorm_pwm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_RE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIAB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt RE Pressure Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_re_pos_abnorm_pwm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_re_pos_abnorm_pwm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_re_pos_abnorm_pwm_lv1_dl
);

/* Head End Pressure sensor */

/* Diag Handle and Config for Tilt Head end Pressure Sensor: Voltage Above Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_he_pos_vlt_abv_norm_diag_text[] = "Tilt Head end Volt Above";

/*
** Tilt Head end Pressure Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_he_pos_vlt_abv_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_HE_POS_CID,
       FMIOB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt Head end Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_he_pos_vlt_abv_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_he_pos_vlt_abv_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_HE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIOB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt Rod End Position Sensor: Voltage Above Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_he_pos_vlt_abv_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_he_pos_vlt_abv_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_he_pos_vlt_abv_norm_lv1_dl
);

/* Diag Handle and Config for Lift Head End Position Sensor: Voltage Below Normal */
/* maximum allowed description ->                   "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_he_pos_vlt_blw_norm_diag_text[] = "Tilt Head End Volt Below";

/*
** Tilt Head End Position Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_he_pos_vlt_blw_norm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_HE_POS_CID,
       FMIIG,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt Head End Pressure Sensor: Voltage Below Normal (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_he_pos_vlt_blw_norm_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_he_pos_vlt_blw_norm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_HE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIG,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt Head End Pressure Sensor: Voltage Below Normal (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_he_pos_vlt_blw_norm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_he_pos_vlt_blw_norm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_he_pos_vlt_blw_norm_lv1_dl
);
/* Diag Handle and Config for Head End Position Sensor: Abnormal Frequency, Pulse Width, or Period */
/* maximum allowed description ->                 "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char tilt_he_pos_abnorm_pwm_diag_text[] = "Tilt Head End sensor Freq Abnor";

/*
** Tilt HE Pressure Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_tilt_he_pos_abnorm_pwm_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       TILT_HE_POS_CID,
       FMIIAB,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** Tilt HE Pressure Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_tilt_he_pos_abnorm_pwm_lv1_dl =
{
                          /*Flex Text Description                           */
     tilt_he_pos_abnorm_pwm_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TILT_HE_POS_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIAB,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Tilt HE Pressure Sensor: Abnormal Frequency, Pulse Width, or Period (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_tilt_he_pos_abnorm_pwm_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_tilt_he_pos_abnorm_pwm_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_tilt_he_pos_abnorm_pwm_lv1_dl
);


/*---------------------------------------------------------------------------*/
/* DL_J1939_1_CID_247_FMI_9 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_J1939_1_CID_247_FMI_9_diag_text[] = "DL_J1939_1_CID_247_FMI_9";

/*
** DL_J1939_1_CID_247_FMI_9 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
scl_obd_es_test_config_t const app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_J19391_CID,
       FMIIUA,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_J1939_1_CID_247_FMI_9 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
scl_eddt_fault_config_t  const app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_J1939_1_CID_247_FMI_9_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_J19391_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIUA,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/* DL_J1939_1_CID_247_FMI_9 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1_dl
);


/*---------------------------------------------------------------------------*/
/* DL_J1939_3_CID_5856_FMI_9 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_J1939_3_CID_5856_FMI_9_diag_text[] = "DL_J1939_3_CID_5856_FMI_9";

/*
** DL_J1939_3_CID_5856_FMI_9 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
scl_obd_es_test_config_t const app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_J19393_CID,
       FMIIUA,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_J1939_3_CID_5856_FMI_9 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
scl_eddt_fault_config_t  const app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_J1939_3_CID_5856_FMI_9_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_J19393_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIUA,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/* DL_J1939_3_CID_5856_FMI_9 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1_dl
);

/*---------------------------------------------------------------------------*/
/* DL_J1939_2_CID_2348_FMI_9 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_J1939_2_CID_2348_FMI_9_diag_text[] = "DL_J1939_2_CID_2348_FMI_9";

/*
** DL_J1939_2_CID_2348_FMI_9 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
scl_obd_es_test_config_t const app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_J19392_CID,
       FMIIUA,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_J1939_2_CID_2348_FMI_9 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
scl_eddt_fault_config_t  const app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_J1939_2_CID_2348_FMI_9_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_J19392_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIUA,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/* DL_J1939_2_CID_2348_FMI_9 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1_dl
);

/*---------------------------------------------------------------------------*/
/* DL_CDL_CID_248_FMI_9 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_CDL_CID_248_FMI_9_diag_text[] = "DL_CD_CID_248_FMI_9";

/*
** DL_CDL_CID_248_FMI_9 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
scl_obd_es_test_config_t const app_diagcfg_DL_CDL_CID_248_FMI_9_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_CDL_CID,
       FMIIUA,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_CDL_CID_248_FMI_9 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
scl_eddt_fault_config_t  const app_diagcfg_DL_CDL_CID_248_FMI_9_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_CDL_CID_248_FMI_9_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_CDL_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIUA,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/* DL_CDL_CID_248_FMI_9 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_CDL_CID_248_FMI_9_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_CDL_CID_248_FMI_9_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_CDL_CID_248_FMI_9_lv1_dl
);



              /* FMI 14 stuff */

/*---------------------------------------------------------------------------*/
/* DL_J1939_1_CID_247_FMI_14 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_J1939_1_CID_247_FMI_14_diag_text[] = "DL_J1939_1_CID_247_FMI_14";

/*
** DL_J1939_1_CID_247_FMI_14 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
scl_obd_es_test_config_t const app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_J19391_CID,
       FMI14,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_J1939_1_CID_247_FMI_14 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
scl_eddt_fault_config_t  const app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_J1939_1_CID_247_FMI_14_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_J19391_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMI14,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};




/* DL_J1939_1_CID_247_FMI_14 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1_dl
);




/*---------------------------------------------------------------------------*/
/* DL_J1939_3_CID_5856_FMI_14 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_J1939_3_CID_5856_FMI_14_diag_text[] = "DL_J1939_3_CID_5856_FMI_14";

/*
** DL_J1939_3_CID_5856_FMI_14 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
scl_obd_es_test_config_t const app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_J19393_CID,
       FMI14,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_J1939_3_CID_5856_FMI_14 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
scl_eddt_fault_config_t  const app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_J1939_3_CID_5856_FMI_14_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_J19393_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMI14,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};


/* DL_J1939_3_CID_5856_FMI_14 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1_dl
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* DL_J1939_2_CID_2348_FMI_14 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_J1939_2_CID_2348_FMI_14_diag_text[] = "DL_J1939_2_CID_2348_FMI_14";

/*
** DL_J1939_2_CID_2348_FMI_14 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
scl_obd_es_test_config_t const app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_J19392_CID,
       FMI14,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_J1939_2_CID_2348_FMI_14 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
scl_eddt_fault_config_t  const app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_J1939_2_CID_2348_FMI_14_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_J19392_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMI14,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};


/* DL_J1939_2_CID_2348_FMI_14 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1_dl
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* DL_J1939_1_CID_248_FMI_14 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_CDL_CID_248_FMI_14_diag_text[] = "DL_CDL_CID_248_FMI_14";

/*
** DL_CDL_CID_248_FMI_14 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
scl_obd_es_test_config_t const app_diagcfg_DL_CDL_CID_248_FMI_14_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_CDL_CID,
       FMI14,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_CDL_CID_248_FMI_14 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
scl_eddt_fault_config_t  const app_diagcfg_DL_CDL_CID_248_FMI_14_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_CDL_CID_248_FMI_14_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_CDL_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMI14,              /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};




/* DL_CDL_CID_248_FMI_14 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_CDL_CID_248_FMI_14_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_CDL_CID_248_FMI_14_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_CDL_CID_248_FMI_14_lv1_dl
);


/*---------------------------------------------------------------------------*/
/* DL_ETHERNET_1_CID_3900_FMI_9 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_ETHERNET_1_CID_3900_FMI_9_diag_text[] = "DL_ETH_1_CID_3900_FMI_9";

/*
** DL_ETHERNET_1_CID_3900_FMI_9 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_9_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_ETHERNET1_CID,
       FMIIUA,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_ETHERNET_1_CID_3900_FMI_9 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_9_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_ETHERNET_1_CID_3900_FMI_9_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_ETHERNET1_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMIIUA,                /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/* DL_ETHERNET_1_CID_3900_FMI_9 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_9_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_9_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_9_lv1_dl
);

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* DL_ETHERNET_1_CID_3900_FMI_14 */
/* maximum allowed description ->          "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char DL_ETHERNET_1_CID_3900_FMI_14_diag_text[] = "DL_ETH_1_CID_3900_FMI_14";

/*
** DL_ETHERNET_1_CID_3900_FMI_14 (Level I) Diagnostic Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_14_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_DIAG,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_DIAG_WCI
     (
       DL_ETHERNET1_CID,
       FMI14,
       WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_DIAG),
     1,                   /*Security                                  [none]*/
                          /*Flags                                           */
     (
       SCL_OBD_ES_TST_CFG_CONT_MON          | /*Continuous Monitor                */
       SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /*Use Persistent Data For Initialize*/
       SCL_OBD_ES_TST_CFG_PERSISTENT        | /*Persistently Save Data            */
       SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /*Automatically Clear Old Codes     */
       SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /*Show History Data Externally      */
     ),
                          /*Debounce Times (Should Always Be 0)             */
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Active   Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Confirm  Delay Time [  0.0s]*/
     (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /*Inactive Delay Time [  0.0s]*/
     0x00000000,          /*Snapshot Id                                [N/A]*/
                          /*Freeze Frame Priority                      [N/A]*/
     SCL_OBD_ES_FF_PRIORITY_NORMAL
};

/*
** DL_ETHERNET_1_CID_3900_FMI_14 (Level I) Diagnostic Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_14_lv1_dl =
{
                          /*Flex Text Description                           */
     DL_ETHERNET_1_CID_3900_FMI_14_diag_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     DL_ETHERNET1_CID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     FMI14,                /*Failure Mode Indicator                          */
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/* DL_ETHERNET_1_CID_3900_FMI_14 (Level I) Diagnostic Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_14_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_14_lv1_es,
                          /*Data Link Configuration                         */
     &app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_14_lv1_dl
);

/*---------------------------------------------------------------------------*/

