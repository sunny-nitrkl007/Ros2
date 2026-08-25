/******************************************************************************
**
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%
*** %% COPYRIGHT (C) 2001-2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
*** %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
**
*** FILE:
***     autonomy_event_config.c
***
*** DESCRIPTION:
***     Autonomy event configurations.
***
******************************************************************************/

/******************************************************************************
***
***    External function prototypes, data, & symbols
***    -- #Include's --
***
******************************************************************************/

#ifndef   AUTONOMY_EVENT_CONFIG_H_
#include "Loader_autonomy_event_config.h"
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

/* Task Stalled event */
/* maximum allowed description ->           "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char       taskS_evnt_text[] =        "EngDataGather Cond13";

/*
** Task Stalled (Level I) Event Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_evntcfg_taskS_evnt_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_EVENT,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_EVENT
     (
         TASKS_EID,
         WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT),
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
** Task Stalled (Level I) Event Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_evntcfg_taskS_evnt_lv1_dl =
{
                          /*Flex Text Description                           */
     taskS_evnt_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TASKS_EID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     0,                   /*Failure Mode Indicator                     [N/A]*/
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Task Stalled (Level I) Event Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_evntcfg_taskS_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_evntcfg_taskS_evnt_lv1_es,
                          /*Data Link Configuration                         */
     &app_evntcfg_taskS_evnt_lv1_dl
);



/* Task Crashed event */
/* maximum allowed description ->           "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char       taskC_evnt_text[] =        "EngDataGather Cond12";

/*
** Task Crashed (Level I) Event Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_evntcfg_taskC_evnt_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_EVENT,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_EVENT
     (
         TASKC_EID,
         WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT),
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
** Task Crashed (Level I) Event Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_evntcfg_taskC_evnt_lv1_dl =
{
                          /*Flex Text Description                           */
     taskC_evnt_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     TASKC_EID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     0,                   /*Failure Mode Indicator                     [N/A]*/
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Task Crashed (Level I) Event Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_evntcfg_taskC_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_evntcfg_taskC_evnt_lv1_es,
                          /*Data Link Configuration                         */
     &app_evntcfg_taskC_evnt_lv1_dl
);

/* Low ECU Power Input Voltage event */
/* maximum allowed description ->                           "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char       low_ecu_power_input_voltage_evnt_text[] = "Low ECU InputVoltage";

/*
** Low ECU Power Input Voltage (Level 1) Event Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_evntcfg_low_ecu_power_input_voltage_evnt_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_EVENT,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_EVENT
     (
         LOW_ECU_VOLTAGE_EID,
         WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT),
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
** Low ECU Power Input Voltage(Level 1) Event Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_evntcfg_low_ecu_power_input_voltage_evnt_lv1_dl =
{
                          /*Flex Text Description                           */
     low_ecu_power_input_voltage_evnt_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     LOW_ECU_VOLTAGE_EID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     0,                   /*Failure Mode Indicator                     [N/A]*/
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*Low ECU Power Input Voltage (Level 3) Event Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_evntcfg_low_ecu_power_input_voltage_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_evntcfg_low_ecu_power_input_voltage_evnt_lv1_es,
                          /*Data Link Configuration                         */
     &app_evntcfg_low_ecu_power_input_voltage_evnt_lv1_dl
);

/* High ECU Power Input Voltage event */
/* maximum allowed description ->                           "xxxxxxxxxxxxxxxxxxxx" (20 chars) */
const char       high_ecu_power_input_voltage_evnt_text[] = "High ECUInputVoltage";

/*
** High ECU Power Input Voltage (Level 1) Event Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_evntcfg_high_ecu_power_input_voltage_evnt_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_EVENT,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_EVENT
     (
         HIGH_ECU_VOLTAGE_EID,
         WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT),
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
** High ECU Power Input Voltage(Level 1) Event Configuration (Data Link)
**     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_evntcfg_high_ecu_power_input_voltage_evnt_lv1_dl =
{
                          /*Flex Text Description                           */
     high_ecu_power_input_voltage_evnt_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     HIGH_ECU_VOLTAGE_EID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     0,                   /*Failure Mode Indicator                     [N/A]*/
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*High ECU Power Input Voltage (Level 2) Event Configuration
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_evntcfg_high_ecu_power_input_voltage_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_evntcfg_high_ecu_power_input_voltage_evnt_lv1_es,
                          /*Data Link Configuration                         */
     &app_evntcfg_high_ecu_power_input_voltage_evnt_lv1_dl
);

/*  PayLoad OverLoad Limit Exceed */


const char       Payload_Overload_Limit_Exceeded_evnt_text[] = "Payload Overload Limit Exceeded";

/*
** PayLoad OverLoad Limit Exceed (Level 1) Event Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_evntcfg_PayLd_OverLd_LimitExceeded_evnt_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_EVENT,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_EVENT
     (
         PAYLOAD_OVERLOAD_LIMIT_EXCEEDED_EID,
         WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT),
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
** PayLoad OverLoad Limit Exceed (Level 1) Event Configuration (Event System)
** **     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_evntcfg_PayLd_OverLd_LimitExceeded_evnt_lv1_dl =
{
                          /*Flex Text Description                           */
		Payload_Overload_Limit_Exceeded_evnt_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     PAYLOAD_OVERLOAD_LIMIT_EXCEEDED_EID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     0,                   /*Failure Mode Indicator                     [N/A]*/
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*
** PayLoad OverLoad Limit Exceed (Level 1) Event Configuration (Event System)
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_evntcfg_PayLd_OverLd_LimitExceeded_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_evntcfg_PayLd_OverLd_LimitExceeded_evnt_lv1_es,
                          /*Data Link Configuration                         */
     &app_evntcfg_PayLd_OverLd_LimitExceeded_evnt_lv1_dl
);

/*  BEM Simulator Events */

const char       BEMSim_derate_evnt_text[] = "Battery Electric Simulator:Machine Derate:Recharge Now";

/*
** BEM Simulator De-rate Charge Level Below Threshold (Level 1) Event Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_evntcfg_BEMSim_derate_evnt_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_EVENT,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_EVENT
     (
         BEMSIM_EID,
         WARN_LEVEL2
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT),
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
** BEM Simulator De-rate Charge Level Below Threshold (Level 1) Event Configuration (Event System)
** **     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_evntcfg_BEMSim_derate_evnt_lv1_dl =
{
                          /*Flex Text Description                           */
        BEMSim_derate_evnt_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     BEMSIM_EID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL2,
     0,                   /*Failure Mode Indicator                     [N/A]*/
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};


/*
** BEM Simulator De-rate Charge Level Below Threshold (Level 1) Event Configuration (Event System)
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_evntcfg_BEMSim_derate_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_evntcfg_BEMSim_derate_evnt_lv1_es,
                          /*Data Link Configuration                         */
     &app_evntcfg_BEMSim_derate_evnt_lv1_dl
);

const char       BEMSim_chargeLevel_Warning_evnt_text[] = "Battery Electric Simulator:Machine Derate:Recharge Soon";

/*
** BEM Simulator De-rate Charge Level Below Threshold (Level 1) Event Configuration (Event System)
**     Event System configuration for one specific diagnostic.
*/
static scl_obd_es_test_config_t const app_evntcfg_BEMSim_chargeLevel_Warning_evnt_lv1_es =
{
                          /*Fault Class                         [Diagnostic]*/
     SCL_OBD_ES_CLASS_EVENT,
     SCL_OBD_ES_GRP_NONE, /*Readiness Group                            [N/A]*/
                          /*Persistent Id                                   */
     SCL_OBD_PERSISTENT_ID_CDL_EVENT
     (
         BEMSIM_EID,
         WARN_LEVEL1
     ),
                          /*Persistent Priority                             */
     SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT),
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
** BEM Simulator De-rate Charge Level Below Threshold (Level 1) Event Configuration (Event System)
** **     Data Link configuration for one specific diagnostic.
*/
static scl_eddt_fault_config_t  const app_evntcfg_BEMSim_chargeLevel_Warning_evnt_lv1_dl =
{
                          /*Flex Text Description                           */
        BEMSim_chargeLevel_Warning_evnt_text,
                          /*Event Type                  [Maintenance & Data]*/
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP,
     FALSE,               /*Histogram Available                         [no]*/
                          /*CID                                             */
     BEMSIM_EID,
                          /*Warning Category Indicator                      */
     WARN_LEVEL1,
     0,                   /*Failure Mode Indicator                     [N/A]*/
     0,                   /*PID Associated With Event                  [N/A]*/
     FALSE,               /*Enable DSI On PID Associated With Event?   [N/A]*/
};

/*
** BEM Simulator De-rate Charge Level Below Threshold (Level 1) Event Configuration (Event System)
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t      app_evntcfg_BEMSim_chargeLevel_Warning_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT
(
                          /*Event System Configuration                      */
     &app_evntcfg_BEMSim_chargeLevel_Warning_evnt_lv1_es,
                          /*Data Link Configuration                         */
     &app_evntcfg_BEMSim_chargeLevel_Warning_evnt_lv1_dl
);

/*
 * Payload Memory Low (Level 1) Event Configuration (Event System)
 *     Event System configuration for one specific event.
 */
static scl_obd_es_test_config_t const app_evntcfg_payload_memory_low_evnt_lv1_es = {
    SCL_OBD_ES_CLASS_EVENT, /* Fault Class [Event] */
    SCL_OBD_ES_GRP_NONE, /* Readiness Group [N/A] */
    SCL_OBD_PERSISTENT_ID_CDL_EVENT(PAYLOAD_MEMORY_LOW_EID, WARN_LEVEL1), /* Persistent Id */
    SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT), /* Persistent Priority */
    1, /* Security [none] */
    ( /* Flags */
        SCL_OBD_ES_TST_CFG_CONT_MON          | /* Continuous Monitor */
        SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /* Use Persistent Data For Initialize */
        SCL_OBD_ES_TST_CFG_PERSISTENT        | /* Persistently Save Data */
        SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /* Automatically Clear Old Codes */
        SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /* Show History Data Externally */
    ),
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Active Delay Time [0.0s] */
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Confirm Delay Time [0.0s] */
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Inactive Delay Time [0.0s] */
    0x00000000, /* Snapshot Id [N/A] */
    SCL_OBD_ES_FF_PRIORITY_NORMAL /* Freeze Frame Priority [N/A] */
};

/*
 * Payload Memory Low (Level 1) Event Configuration (Event System)
 *     Data Link configuration for one specific event.
 */
static scl_eddt_fault_config_t const app_evntcfg_payload_memory_low_evnt_lv1_dl = {
    "Payload Memory Low", /* Flex Text Description */
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP, /* Event Type [Maintenance & Data] */
     FALSE, /* Histogram Available [no] */
     PAYLOAD_MEMORY_LOW_EID, /* EID */
     WARN_LEVEL1, /* Warning Category Indicator */
     0, /* Failure Mode Indicator [N/A] */
     0, /* PID Associated With Event [N/A] */
     FALSE, /* Enable DSI On PID Associated With Event? [N/A] */
};

/*
** Payload Memory Low (Level 1) Event Configuration (Event System)
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t app_evntcfg_payload_memory_low_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT(
        &app_evntcfg_payload_memory_low_evnt_lv1_es, /* Event System Configuration */
        &app_evntcfg_payload_memory_low_evnt_lv1_dl /* Data Link Configuration */
        );

/*
 * Payload Memory Full (Level 1) Event Configuration (Event System)
 *     Event System configuration for one specific event.
 */
static scl_obd_es_test_config_t const app_evntcfg_payload_memory_full_evnt_lv1_es = {
    SCL_OBD_ES_CLASS_EVENT, /* Fault Class [Event] */
    SCL_OBD_ES_GRP_NONE, /* Readiness Group [N/A] */
    SCL_OBD_PERSISTENT_ID_CDL_EVENT(PAYLOAD_MEMORY_FULL_EID, WARN_LEVEL1), /* Persistent Id */
    SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT), /* Persistent Priority */
    1, /* Security [none] */
    ( /* Flags */
        SCL_OBD_ES_TST_CFG_CONT_MON          | /* Continuous Monitor */
        SCL_OBD_ES_TST_CFG_PERSISTENT_ACTIVE | /* Use Persistent Data For Initialize */
        SCL_OBD_ES_TST_CFG_PERSISTENT        | /* Persistently Save Data */
        SCL_OBD_ES_TST_CFG_AUTO_CLEAR_SHM    | /* Automatically Clear Old Codes */
        SCL_OBD_ES_TST_CFG_STORE_CONFIRM       /* Show History Data Externally */
    ),
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Active Delay Time [0.0s] */
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Confirm Delay Time [0.0s] */
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Inactive Delay Time [0.0s] */
    0x00000000, /* Snapshot Id [N/A] */
    SCL_OBD_ES_FF_PRIORITY_NORMAL /* Freeze Frame Priority [N/A] */
};

/*
 * Payload Memory Full (Level 1) Event Configuration (Event System)
 *     Data Link configuration for one specific event.
 */
static scl_eddt_fault_config_t const app_evntcfg_payload_memory_full_evnt_lv1_dl = {
    "Payload Memory Low", /* Flex Text Description */
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP, /* Event Type [Maintenance & Data] */
     FALSE, /* Histogram Available [no] */
     PAYLOAD_MEMORY_FULL_EID, /* EID */
     WARN_LEVEL1, /* Warning Category Indicator */
     0, /* Failure Mode Indicator [N/A] */
     0, /* PID Associated With Event [N/A] */
     FALSE, /* Enable DSI On PID Associated With Event? [N/A] */
};

/*
** Payload Memory Full (Level 1) Event Configuration (Event System)
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t app_evntcfg_payload_memory_full_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT(
        &app_evntcfg_payload_memory_full_evnt_lv1_es, /* Event System Configuration */
        &app_evntcfg_payload_memory_full_evnt_lv1_dl /* Data Link Configuration */
        );

/*
 * Payload Legal For Trade Not Sealed (Level 1) Event Configuration (Event System)
 *     Event System configuration for one specific event.
 */
static scl_obd_es_test_config_t const app_evntcfg_payload_lft_not_sealed_evnt_lv1_es = {
    SCL_OBD_ES_CLASS_EVENT, /* Fault Class [Event] */
    SCL_OBD_ES_GRP_NONE, /* Readiness Group [N/A] */
    SCL_OBD_PERSISTENT_ID_CDL_EVENT(PAYLOAD_LFT_NOT_SEALED_EID, WARN_LEVEL1), /* Persistent Id */
    SCL_CI_ES_WCI_TO_PP(WARN_LEVEL1, SCL_OBD_ES_CLASS_EVENT), /* Persistent Priority */
    1, /* Security [none] */
    ( /* Flags */
        SCL_OBD_ES_TST_CFG_CONT_MON /* Continuous Monitor */
    ),
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Active Delay Time [0.0s] */
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Confirm Delay Time [0.0s] */
    (oel_second_u32_l20_t)( 0.0*(1UL<<20) ), /* Inactive Delay Time [0.0s] */
    0x00000000, /* Snapshot Id [N/A] */
    SCL_OBD_ES_FF_PRIORITY_NORMAL /* Freeze Frame Priority [N/A] */
};

/*
 * Payload Legal For Trade Not Sealed (Level 1) Event Configuration (Event System)
 *     Data Link configuration for one specific event.
 */
static scl_eddt_fault_config_t const app_evntcfg_payload_lft_not_sealed_evnt_lv1_dl = {
    "Payload Legal For Trade Measurement Not Sealed", /* Flex Text Description */
     SCL_EDDT_EVENT_TYPE_MAINT_AND_OP, /* Event Type [Maintenance & Data] */
     FALSE, /* Histogram Available [no] */
     PAYLOAD_LFT_NOT_SEALED_EID, /* EID */
     WARN_LEVEL1, /* Warning Category Indicator */
     0, /* Failure Mode Indicator [N/A] */
     0, /* PID Associated With Event [N/A] */
     FALSE, /* Enable DSI On PID Associated With Event? [N/A] */
};

/*
** Payload Legal For Trade Not Sealed (Level 1) Event Configuration (Event System)
**     This structure is used to provide the diagnostic configuration to
**     initialize one specific diagnostic.
*/
scl_ci_es_fault_cfg_eddt_t app_evntcfg_payload_lft_not_sealed_evnt_lv1 =
SCL_CI_ES_FAULT_CFG_EDDT(
        &app_evntcfg_payload_lft_not_sealed_evnt_lv1_es, /* Event System Configuration */
        &app_evntcfg_payload_lft_not_sealed_evnt_lv1_dl /* Data Link Configuration */
        );

