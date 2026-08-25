/*******************************************************************************
***
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%                                                                       %%
*** %%  COPYRIGHT (C) 2003-2009 CATERPILLAR INC.   ALL RIGHTS RESERVED.      %%
*** %%                                                                       %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
***
***
*** DESCRIPTION : This is the task1 for app template 2.
***
*** REQUIRED HEADER :
***
*** LANGUAGE :                  
***    ANSI C
***
*** COMPILE FLAGS :
***
*** METHOD:
***
*** NOTES:
***
*** GLOBALS USED:
***
*** History:  Initial version, March 3rd, 2003     Leyan Xu
*******************************************************************************/

/*******************************************************************************
***
***    Referenced Functions, Data, & Symbols
***    -- #Include's --
***
*******************************************************************************/
#include <sys/prctl.h>

#include <scl_prmsw.h>
#include "app_rtos_config.h"
#ifndef  ACDCALMGRINF_H_
#include <AcdCalMgrInf.h>
#endif
#include "../src_hour/app_clock_init.h"
#include "../src_prmsw/passwd.h"
#include "../src_nvm/app_nvm_file_init.h"
#include "../src_prmsw/app_prmsw_config.h"
#include <bdt_proto.h>
#include <bdt_data_access.h>
#include <bdt_action.h>
#include <taskPlugins/interfaces/PlatformDefinesC.h>
#if SCL_STAT_DB_SUPPORTED
#include <scl_stat_db.h>
#endif
#include "../src_j1939/app_health_j39.h" 
#include "../src_j1939/app_can_init.h"
#if CSF_PGT_SERVER_SUPPORTED
#include "../src_pgt_server/app_pgt.h"
#endif

static unsigned_32 task_scl_j1939_control_counter;
static unsigned_32 task_cal_mgr_counter;
static unsigned_32 task_pgt_counter;
static unsigned_32 task_bdt_counter;
static unsigned_32 task_eth_port_stats_counter;
static unsigned_32 task_low_priority_counter;

bool_t isDatalinkDiagInhibited = TRUE;
bool_t isDatalinkDiagDisabled = TRUE;

/*******************************************************************************
***
*** FUNCTION NAME: task_scl_j1939_control()
***
*** DESCRIPTION:
***   The main periodic task for the app template 2 application.
***
*******************************************************************************/
void task_scl_j1939_control(void)
{
   oel_rtos_task_context_t my_task_context;

   // Set my name
   prctl(PR_SET_NAME, __func__, 0, 0, 0);

   /*
   Perform rtos startup initialization.  Must be the first code in the first
   running task.  Subsequent calls are ignored so it can be included in all
   tasks.  This is not required for rtos implementations with a startup task
   (such as OSE Delta).
   */
   oel_rtos_startup_init();

   /*
   Get my task context once and remember it to avoid the overhead of doing
   it again.
   */
   oel_rtos_task_get_context(&my_task_context);

   /*
   Periodic Main Loop
   */
   while (TRUE)   /* until loss of power or reset */
   {
      /*
      wait for synchronizing event
      */
      oel_rtos_event_pend(my_task_context, OEL_RTOS_TIME_EVENT, NULL);
      
      scl_j1939_periodic_ctrl_task();

      app_health_j39_update(isDatalinkDiagInhibited, isDatalinkDiagDisabled);

      app_j1939_build_pgn_0000_update();

      ++task_scl_j1939_control_counter;
   } /* end while() */

} /* end eol_main_task() */

void task_cal_mgr(void)
{
   oel_rtos_task_context_t my_task_context;

   // Set my name
   prctl(PR_SET_NAME, __func__, 0, 0, 0);

   /*
   Perform rtos startup initialization.  Must be the first code in the first
   running task.  Subsequent calls are ignored so it can be included in all
   tasks.  This is not required for rtos implementations with a startup task
   (such as OSE Delta).
   */
   oel_rtos_startup_init();

   /*
   Get my task context once and remember it to avoid the overhead of doing
   it again.
   */
   oel_rtos_task_get_context(&my_task_context);

   /*
   Periodic Main Loop
   */
   while (TRUE)   /* until loss of power or reset */
   {
      /*
      wait for synchronizing event
      */
      oel_rtos_event_pend(my_task_context, OEL_RTOS_TIME_EVENT, NULL);

      /* update call manager */
      acd_cal_mgr_update();

      ++task_cal_mgr_counter;
   } /* end while() */
} /* cal_mgr_task() */

void task_pgt(void) {
    oel_rtos_task_context_t my_task_context;

    /* Set my name */
    prctl(PR_SET_NAME, __func__, 0, 0, 0);

    /* Get my task context once and remember it to avoid the overhead of doing it again. */
    oel_rtos_task_get_context(&my_task_context);

    /* Periodic Main Loop */
    while (TRUE) {
       /* wait for synchronizing event */
       oel_rtos_event_pend(my_task_context, OEL_RTOS_TIME_EVENT, NULL);

#if CSF_PGT_SERVER_SUPPORTED
       app_pgt_update();
#endif

       ++task_pgt_counter;
    }
}

void task_bdt(void) {
    oel_rtos_task_context_t my_task_context;

    // Set my name
    prctl(PR_SET_NAME, __func__, 0, 0, 0);

    /*
    Get my task context once and remember it to avoid the overhead of doing
    it again.
    */
    oel_rtos_task_get_context(&my_task_context);

    /*
    Periodic Main Loop
    */
    while (TRUE) {
       /*
       wait for synchronizing event
       */
       oel_rtos_event_pend(my_task_context, OEL_RTOS_TIME_EVENT, NULL);

       bdt_main();

       ++task_bdt_counter;
    }
}

void task_eth_port_stats(void) {
    oel_rtos_task_context_t my_task_context;

    // Set my name
    prctl(PR_SET_NAME, __func__, 0, 0, 0);

    /*
    Get my task context once and remember it to avoid the overhead of doing
    it again.
    */
    oel_rtos_task_get_context(&my_task_context);

    /*
    Periodic Main Loop
    */
    while (TRUE) {
       /*
       wait for synchronizing event
       */
       oel_rtos_event_pend(my_task_context, OEL_RTOS_TIME_EVENT, NULL);

#if SCL_STAT_DB_SUPPORTED
       scl_stat_db_update();
#endif

       ++task_eth_port_stats_counter;
    }
}

void task_low_priority(
   void)
{
   oel_rtos_task_context_t my_task_context;

   // Set my name
   prctl(PR_SET_NAME, __func__, 0, 0, 0);

   /*
   Perform rtos startup initialization.  Must be the first code in the first
   running task.  Subsequent calls are ignored so it can be included in all
   tasks.  This is not required for rtos implementations with a startup task
   (such as OSE Delta).
   */
   oel_rtos_startup_init();

   /*
   Get my task context once and remember it to avoid the overhead of doing
   it again.
   */
   oel_rtos_task_get_context(&my_task_context);

   /*
   Process periodic or event driven actions
   */
   while (TRUE)   /* until loss of power or reset */
   {
       /* wait for synchronizing event */
      oel_rtos_event_pend(my_task_context, OEL_RTOS_TIME_EVENT, NULL);

      app_clock_update();

      app_sync_clock_update();

      app_rtc_tz_update();

      app_prmsw_update();

      nvm_file_update();

      ++task_low_priority_counter;
   }
}
