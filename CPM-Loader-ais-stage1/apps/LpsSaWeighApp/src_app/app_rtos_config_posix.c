/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME:  app_rtos_config_posix.c
DESCRIPTION:
*******************************************************************************/

/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include "app_rtos_config.h"
#include <oel_oop.h>
#include <oel_rtos_posix.h>
#include <hal_watchdog_proto.h>
#include <hal_timer_proto.h>
#include <hal_spi.h>
#include <oel_rtos.h>


static void app_wdog_strobe(void);
static oel_rtos_action_t app_wdog_handler_noop;

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
         /*
         Task Priorities
         */
#define TASK_1X_PRIORITY         -1



/*******************************************************************************
** -- Data Definitions
*******************************************************************************/

static oel_rtos_timer_alarm_handler_t app_wdog_alarm_handler_noop = {
    &app_wdog_handler_noop
};

      /*
      task ids
      */
   oel_rtos_task_id_t scl_task_1x_id;
      /*
      task config
      */

static const oel_rtos_posix_task_config_t task_config[] =
{
    OEL_RTOS_POSIX_TASK_CONFIG_I1(
      &scl_task_1x_id, /* RTOS_TASK_ID_PTR */
      SCL_TASK_1X_PERIOD, /* DELAY */
      SCL_TASK_1X_PERIOD, /* PERIOD */
      SCL_TASK_1X_PERIOD +
      OEL_RTOS_SECONDS_DBL_TO_U32_L20(10.00),/* WDOG_DELAY */
      &app_wdog_alarm_handler_noop, /* WDOG_HANDLER */
      80, /* TASK_STACK_THRESHOLD */
      "task_1x", /* NAME */
      task_1x, /* ENTRY */
      2000, /* STACK_SIZE */
      TASK_1X_PRIORITY, /* PRIORITY */
      SCHED_FIFO), /* POLICY */
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


static void app_wdog_strobe(void)
{
}

static void app_wdog_handler_noop(oel_rtos_action_object_t* oel_rtos_action_object) {
    (void)oel_rtos_action_object;
}
