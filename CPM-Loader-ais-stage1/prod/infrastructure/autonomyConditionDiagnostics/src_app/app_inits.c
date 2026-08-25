/*******************************************************************************
Copyright  2001-2013 Caterpillar Inc. All rights reserved.
--------------------------------------------------------------------------------
File name:  app_inits.c
Description:

   This file provides application initialization functions that are called from
   oel during application startup.
    
*******************************************************************************/

/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include "app_rtos_config.h"

#ifndef APPLIB_PRIVATE_H_
#include <applib_private.h>
#endif

#ifndef  ACDCALMGRINF_H_
#include <AcdCalMgrInf.h>
#endif

#include "../src_nvm/app_nvm_file_init.h"
#include "../src_catdl/app_catdl_init.h"
#include "../src_j1939/app_can_init.h"
#include "../src_hour/app_clock_init.h"
#include "../src_prmsw/app_prmsw_config.h"
#include "../src_j1939/app_scl_flex.h"
#include <taskPlugins/interfaces/PlatformDefinesC.h>
#if CSF_PGT_SERVER_SUPPORTED
#include "../src_pgt_server/app_pgt.h"
#endif

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/

/*******************************************************************************
** -- Function Prototypes --
*******************************************************************************/

static oel_rtos_init_t app_pre_irq_init;
static oel_rtos_init_t app_startup_init;
/*******************************************************************************
** -- Data Definitions
*******************************************************************************/
volatile boolean AppStartupInitFlag = FALSE;

extern char WeighRangeBottomRqstFlg;
extern char WeighRangeSizeRqstFlg;
extern unsigned_8  PidF1AA;
extern unsigned_16 PidD00C9F;
extern unsigned_16 PidD0102D;

      /*
      define init objects

      The objects combine the function and cofig constants.  The
      functions are called through these objects.  The app_pre_irq_init_o
      and app_startup_init_o objects have no config constants.
      */

static const oel_rtos_init_object_t app_pre_irq_init_o =
{
   &app_pre_irq_init /* address of init function */
};

static const oel_rtos_init_object_t app_startup_init_o =
{
   &app_startup_init /* address of init function */
};

      /*
      define pre-irq init list

      This is a list of pointers to init objects.
      */

oel_rtos_init_object_t const* const app_pre_irq_init_list[]=
{
   /* insert pre-irq init objects here */
   &app_pre_irq_init_o,
   NULL                    /* MUST end with NULL */
};


      /*
      define startup init list
      This is a list of pointers to init objects.
      */

oel_rtos_init_object_t const* const app_startup_init_list[]=
{
   &app_startup_init_o,
   NULL                    /* MUST end with NULL */
};

/******************************************************************************
FUNCTION NAME:  app_pre_irq_init
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/

static void app_pre_irq_init(
   oel_rtos_init_object_t const* never_used)
{
   /* touch argument to avoid compiler warning */
   (void)never_used;
      /*
      Insert application and legacy library init code here.
      New components should provide init object that can be directly
      added to the app_pre_irq_init_list[].
      */
}


/******************************************************************************
FUNCTION NAME:  app_startup_init
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/

static void app_startup_init(
   oel_rtos_init_object_t const* never_used)
{
   /* touch argument to avoid compiler warning */
   (void)never_used;

   oel_u_init(NULL);

   /*
      Insert application and legacy library init code here.
      New components should provide init object that can be directly
     added to the app_startup_init_list[].
   */

   /* NVM initialization */
   app_nvm_file_init();

   app_catdl_init();

   /* bdt_init is called in this function */
   app_can_init();

   app_clock_init();

   app_sync_clock_init();

   app_rtc_tz_init();

   app_prmsw_init();

   /* 
   NOTE: BDT must be initialized before calling this function.
         Flexing PIDFC06 security level to 0 so that ET
         does not prompt for factory password for updating its value 
   */  
   app_scl_flex_init();
   
   /*Initialize calibration*/
   init_calibration();

   PidF1AA = 0x00;  /* Store Switch is released  by default */
   WeighRangeBottomRqstFlg = 0;
   WeighRangeSizeRqstFlg = 0;

   /* Enable the calibration via the calibration manager */
   enable_calibration();

   /* Initialize Ethernet PGT */
#if CSF_PGT_SERVER_SUPPORTED
   app_pgt_init();
#endif

   AppStartupInitFlag = TRUE;
} 

