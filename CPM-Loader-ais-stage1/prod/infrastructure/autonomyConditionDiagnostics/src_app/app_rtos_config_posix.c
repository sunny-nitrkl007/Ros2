/*******************************************************************************
** COPYRIGHT (C) 2011-2012 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME:  app_rtos_config_posix.c
DESCRIPTION:
*******************************************************************************/

/*******************************************************************************
** -- #Include's --
*******************************************************************************/
//#include <stdlib.h> // abort()
#include <oel_rtos.h>
#include <oel_rtos_posix.h>

#include "app_rtos_config.h"


static void app_wdog_strobe(void);
static oel_rtos_action_t app_wdog_handler_noop;
//static oel_rtos_action_t app_wdog_handler_abort;

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
         /*
         Task Priorities
         */

/* Priority of -99 is Highest Priority
 * Larger numbers are lower priority
 */
#define TASK_SCL_J1939_CONTROL_PRIORITY (-21)
#define TASK_CAL_MGR_PRIORITY (-10)
#define TASK_PGT_PRIORITY (-8)
#define TASK_BDT_PRIORITY (-5)
#define TASK_ETH_PORT_STATS_PRIORITY (-5)
#define TASK_LOW_PRIORITY_PRIORITY (-1)


/*******************************************************************************
** -- Data Definitions
*******************************************************************************/
static oel_rtos_timer_alarm_handler_t app_wdog_alarm_handler_noop = {
    &app_wdog_handler_noop
};

//static oel_rtos_timer_alarm_handler_t app_wdog_alarm_handler_abort = {
//    &app_wdog_handler_abort
//};

      /*
      task ids
      */
   oel_rtos_task_id_t task_scl_j1939_control_id;
   oel_rtos_task_id_t task_cal_mgr_id;
   oel_rtos_task_id_t task_pgt_id;
   oel_rtos_task_id_t task_bdt_id;
   oel_rtos_task_id_t task_eth_port_stats_id;
   oel_rtos_task_id_t task_100ms_low_priority_id;

      /*
      task config
      */

static const oel_rtos_posix_task_config_t task_config[] =
{
    /*
     * J1939 Control Task
     *  This task should be one of the highest priority tasks so that it can perform
     *  J1939 communications with minimum latency.
     */
    OEL_RTOS_POSIX_TASK_CONFIG_I1(
            &task_scl_j1939_control_id, /* RTOS_TASK_ID_PTR */
            OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.000), /* DELAY */
            TASK_SCL_J1939_CONTROL_PERIOD, /* PERIOD */
            TASK_SCL_J1939_CONTROL_PERIOD + OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.100), /* WDOG_DELAY */
            &app_wdog_alarm_handler_noop, /* WDOG_HANDLER */
            80, /* TASK_STACK_THRESHOLD */
            "task_scl_j1939_control", /* NAME */
            task_scl_j1939_control, /* ENTRY */
            2000, /* STACK_SIZE - Will be set to MAX(STACK_SIZE, PTHREAD_STACK_MIN) */
            TASK_SCL_J1939_CONTROL_PRIORITY, /* PRIORITY */
            SCHED_FIFO), /* SCHEDULING POLICY */

    /*
     * Calibration Manager Task
     *  The calibration manager protocol needs to keep a regular heartbeat and cannot be delayed
     *  by other lesser priority tasks.
     */
    OEL_RTOS_POSIX_TASK_CONFIG_I1(
            &task_cal_mgr_id, /* RTOS_TASK_ID_PTR */
            OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.000), /* DELAY */
            TASK_CAL_MGR_PERIOD, /* PERIOD */
            TASK_CAL_MGR_PERIOD + OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.100), /* WDOG_DELAY */
            &app_wdog_alarm_handler_noop, /* WDOG_HANDLER */
            80, /* TASK_STACK_THRESHOLD */
            "task_cal_mgr", /* NAME */
            task_cal_mgr, /* ENTRY */
            2000, /* STACK_SIZE - Will be set to MAX(STACK_SIZE, PTHREAD_STACK_MIN) */
            TASK_CAL_MGR_PRIORITY, /* PRIORITY */
            SCHED_FIFO), /* SCHEDULING POLICY */

    /*
     * PGT Task
     *  Runs csf_httpd and Ethernet PGT.
     */
    OEL_RTOS_POSIX_TASK_CONFIG_I1(
            &task_pgt_id, /* RTOS_TASK_ID_PTR */
            OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.000), /* DELAY */
            TASK_PGT_PERIOD, /* PERIOD */
            TASK_PGT_PERIOD + OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.100), /* WDOG_DELAY */
            &app_wdog_alarm_handler_noop, /* WDOG_HANDLER */
            80, /* TASK_STACK_THRESHOLD */
            "task_httpd", /* NAME */
            task_pgt, /* ENTRY */
            2000, /* STACK_SIZE - Will be set to MAX(STACK_SIZE, PTHREAD_STACK_MIN) */
            TASK_PGT_PRIORITY, /* PRIORITY */
            SCHED_FIFO), /* SCHEDULING POLICY */

    /*
     * BDT Task
     *  Separated from NVM/background task so that it can run without waiting for NVM
     *  Long WDOG delay because some BDT callbacks are unfortunately blocking and take a while.
     */
    OEL_RTOS_POSIX_TASK_CONFIG_I1(
            &task_bdt_id, /* RTOS_TASK_ID_PTR */
            OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.000), /* DELAY */
            TASK_BDT_PERIOD, /* PERIOD */
            TASK_BDT_PERIOD + OEL_RTOS_SECONDS_DBL_TO_U32_L20(2.000), /* WDOG_DELAY */
            &app_wdog_alarm_handler_noop, /* WDOG_HANDLER */
            80, /* TASK_STACK_THRESHOLD */
            "task_bdt", /* NAME */
            task_bdt, /* ENTRY */
            2000, /* STACK_SIZE - Will be set to MAX(STACK_SIZE, PTHREAD_STACK_MIN) */
            TASK_BDT_PRIORITY, /* PRIORITY */
            SCHED_FIFO), /* SCHEDULING POLICY */

    /*
     * Ethernet Port Statistics Task
     *  The ethernet port statistics should not be updated faster than every 500ms
     *  and should not be delayed by NVM operations.
     */
    OEL_RTOS_POSIX_TASK_CONFIG_I1(
            &task_bdt_id, /* RTOS_TASK_ID_PTR */
            OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.000), /* DELAY */
            TASK_ETH_PORT_STATS_PERIOD, /* PERIOD */
            TASK_ETH_PORT_STATS_PERIOD + OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.500), /* WDOG_DELAY */
            &app_wdog_alarm_handler_noop, /* WDOG_HANDLER */
            80, /* TASK_STACK_THRESHOLD */
            "task_eth_port_stats", /* NAME */
            task_eth_port_stats, /* ENTRY */
            2000, /* STACK_SIZE - Will be set to MAX(STACK_SIZE, PTHREAD_STACK_MIN) */
            TASK_ETH_PORT_STATS_PRIORITY, /* PRIORITY */
            SCHED_FIFO), /* SCHEDULING POLICY */

    /*
     * Other/Background Task
     *  This task is used a the lowest priority background task, however, it cannot
     *  be implemented as a true background/idle task without eating up 100% CPU.
     *  Therefore, we run it periodically and allow a large WDOG delay.
     */
    OEL_RTOS_POSIX_TASK_CONFIG_I1(
            &task_100ms_low_priority_id, /* RTOS_TASK_ID_PTR */
            TASK_LOW_PRIORITY_PERIOD, /* DELAY */
            TASK_LOW_PRIORITY_PERIOD, /* PERIOD */
            TASK_LOW_PRIORITY_PERIOD + OEL_RTOS_SECONDS_DBL_TO_U32_L20(10.00), /* WDOG_DELAY */
            &app_wdog_alarm_handler_noop, /* WDOG_HANDLER */
            80, /* TASK_STACK_THRESHOLD */
            "task_low_priority", /* NAME */
            task_low_priority, /* ENTRY */
            1024, /* STACK_SIZE - Will be set to MAX(STACK_SIZE, PTHREAD_STACK_MIN) */
            TASK_LOW_PRIORITY_PRIORITY, /* PRIORITY */
            SCHED_FIFO), /* SCHEDULING POLICY */

};


#define NUMBER_OF_TASKS (sizeof(task_config) / sizeof(oel_rtos_posix_task_config_t))


      /*
      rtos config
      */
oel_rtos_posix_config_t app_oel_rtos_posix_config =
   OEL_RTOS_POSIX_CONFIG(
      NUMBER_OF_TASKS,                          /* NUMBER_OF_TASKS */
      task_config,                              /* TASK_CONFIG */
      app_pre_irq_init_list,                    /* PRE_IRQ_INIT */
      app_startup_init_list,                    /* STARTUP_INIT */
      OEL_RTOS_SECONDS_DBL_TO_U32_L20(0.100),   /* WDOG_STARTUP_DELAY */
      NULL,                                     /* WDOG_HANDLER */
      app_wdog_strobe,                          /* WDOG_STROBE */
      TRUE,                                     /* STACK_CHECK_ENABLE */
      80,                                       /* SYS_STACK_THRESHOLD */
      NULL,                                     /* STACK_HANDLER */
      TRUE,                                     /* DEBUG */
      OEL_RTOS_SECONDS_DBL_TO_U64_L32(0.01));   /* TICK_PERIOD */


static void app_wdog_strobe(void) {
}

static void app_wdog_handler_noop(oel_rtos_action_object_t* oel_rtos_action_object) {
    (void)oel_rtos_action_object;
}

//static void app_wdog_handler_abort(oel_rtos_action_object_t* oel_rtos_action_object) {
//    (void)oel_rtos_action_object;
//    abort();
//}

