/*******************************************************************************
Copyright  2017 Caterpillar Inc. All rights reserved.
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

#include <oel_u.h>

#include "../src_nvm/app_nvm_file_init.h"
#include "../src_nvm/app_nvm_file_access.h"

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/

/*******************************************************************************
** -- Function Prototypes --
*******************************************************************************/
extern const ApplicationGet_t application_get_table;
      /*
      declare init functions
      */

static oel_rtos_init_t app_pre_irq_init;
static oel_rtos_init_t app_startup_init;

/*******************************************************************************
** -- Data Definitions
*******************************************************************************/

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

   OelBootupFlag = FALSE;

   /* NVM initialization */
   app_nvm_file_init();

} 

