/*******************************************************************************
***
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%                                                                       %%
*** %%  COPYRIGHT (C) 2017 CATERPILLAR INC.   ALL RIGHTS RESERVED.           %%
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
*******************************************************************************/

/*******************************************************************************
***
***    Referenced Functions, Data, & Symbols
***    -- #Include's --
***
*******************************************************************************/

#include "app_rtos_config.h"
#include "../src_nvm/app_nvm_file_init.h"
#include "../src_nvm/app_nvm_file_access.h"

volatile boolean OelBootupFlag = FALSE;
volatile boolean LpsSaNvmWeighAppMachSpecificCfgReadFlag;
volatile boolean LpsSaNvmWeighAppCalReadFlag;
volatile boolean LpsSaTiltNvmWeighAppCalReadFlag;
volatile boolean LpsSaLiftNvmWeighAppCalReadFlag;

/*******************************************************************************
***
*** FUNCTION NAME: task_1x()
***
*** DESCRIPTION:
***
***
*******************************************************************************/
void task_1x(void)
{
  oel_rtos_task_context_t my_task_context;

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

      /* wait for synchronizing event */
      oel_rtos_event_pend(my_task_context, OEL_RTOS_TIME_EVENT, NULL);
      
      nvm_file_update();

      OelBootupFlag = TRUE;

   } /* end while() */

} /* end eol_main_task() */
