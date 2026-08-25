/*******************************************************************************
* Copyright 2001-2008 Caterpillar Inc.  All rights reserved.
--------------------------------------------------------------------------------
FILE NAME:  app_rtos_config.h
DESCRIPTION:
*******************************************************************************/
#ifndef __APP_RTOS_CONFIG_H__
#define __APP_RTOS_CONFIG_H__

/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <oel_rtos.h>
#include <scl_j1939.h>
#include "../src_nvm/app_nvm_file_cfg.h"

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/

         /*
         Task Periods

         Task periods are defined as macros to support their use in
         ROM configuration data.  They should be adjusted to match the run
         time values that have been adjusted to supported values.
         */

oel_rtos_function_t task_scl_j1939_control;
oel_rtos_function_t task_cal_mgr;
oel_rtos_function_t task_pgt;
oel_rtos_function_t task_bdt;
oel_rtos_function_t task_eth_port_stats;
oel_rtos_function_t task_low_priority;

#define TASK_10MS_PERIOD        OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.01)
#define TASK_100MS_PERIOD       OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.10)
#define TASK_500MS_PERIOD       OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.50)

#define TASK_SCL_J1939_CONTROL_PERIOD   TASK_10MS_PERIOD
#define TASK_CAL_MGR_PERIOD             TASK_100MS_PERIOD
#define TASK_BDT_PERIOD                 TASK_100MS_PERIOD
#define TASK_PGT_PERIOD                 TASK_100MS_PERIOD
#define TASK_ETH_PORT_STATS_PERIOD      TASK_500MS_PERIOD
#define TASK_LOW_PRIORITY_PERIOD        TASK_100MS_PERIOD


      /*
      events for task task_multi_event
      */

/*******************************************************************************
** -- Function Prototypes --
*******************************************************************************/

/*******************************************************************************
** -- Data Declarations
*******************************************************************************/

      /*
      arrays of pointers to initialization functions
      */
extern oel_rtos_init_object_t const* const app_pre_irq_init_list[];
extern oel_rtos_init_object_t const* const app_startup_init_list[];

      /*
      task ids
      */

      /*
      resources
      */
extern const scl_j1939_config_t app_scl_j1939_config;

#endif /* #ifndef __APP_RTOS_CONFIG_H__ */
