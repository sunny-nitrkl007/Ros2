/******************************************************************************
 **           COPYRIGHT (C) 1999-2005 CATERPILLAR INC. ALL RIGHTS RESERVED.
 **            This work contains Caterpillar's proprietary information, which
 **            may constitute a trade secret and/or be confidential.  Copyright
 **            notice is precautionary only and does not imply publication.
-------------------------------------------------------------------------------
FILE NAME:      app_token_block0.c
DESCRIPTION:    This file defines the application's code token block and
                corresponding file description block.

Vamsi Putumbaka modified this file on 02/01/05 to enable unexpected reset 
checking.
 ******************************************************************************/

/******************************************************************************
 ** -- #Include's --
 ******************************************************************************/

#include <hal_boot.h>
#include <oel_rtos.h>

/******************************************************************************
 ** -- #Define, Struct's, Typedef's, Enum's --
 ******************************************************************************/

/******************************************************************************
 ** -- Function Prototypes --
 ******************************************************************************/

/******************************************************************************
 ** -- Data Declarations --
 ******************************************************************************/

/*
 * In order to get some hal_boot functions to work, like 'hal_boot_get_sw_fileid',
 * the app token block must be defined.
 *
 * Normally, the app token block would be placed at a specific address and boot
 * would look at that address to find the token block which contains some useful
 * info for launching the app.  In linux, this is totally useless, but we still
 * need to define the token block so that hal_boot can pretend to pull out information
 * about the software.
 *
 * In our application, oel_sys_init is not called by boot, but by the AIS Task
 * 'commInitialize' method.  This means that we don't need to place the token block
 * at a specific address for boot to find it.
 *
 * Also, I'm not sure who would be setting the SW part number and SW file id...
 * It looks like that is just going to be ffffffffff... does this matter?
 *
 * Basically, we just need a token block called 'app_token' so that stuff will link.
 */
HAL_BOOT_DEFINE_FDB_0(
    app_fdb,        /* name of file description block           */
    oel_sys_init,   /* application starting address             */
    1,              /* Enable unexpected reset checking        */
    10,             /* max # unexpected resets before lockout   */
    0x40000000      /* 2^-30 S/bit for watchdog period          */
);

/*
 * HAL_BOOT_DEFINE_TOKEN_BLOCK doesn't initilize the union with the correct number
 * of braces, so we have to disable the warning.
 */
#pragma GCC diagnostic ignored "-Wmissing-braces"
HAL_BOOT_DEFINE_TOKEN_BLOCK(app_token, app_fdb);
