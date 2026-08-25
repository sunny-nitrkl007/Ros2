/*******************************************************************************
 ***
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *** %%                                                                        %%
 *** %%  COPYRIGHT (C) 2015 CATERPILLAR INC.   ALL RIGHTS RESERVED.            %%
 *** %%      This work contains proprietary information which may              %%
 *** %%      constitute a trade secret and/or be confidential.                 %%
 *** %%                                                                        %%
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 **
 *
 * FILE NAME: app_catdl_init.c
 *
 * DESCRIPTION: Product ID support for CDL over J1939
 *
 * LANGUAGE: ANSI C
 *
 * FUNCTION LIST: app_catdl_init()
 *
 * HISTORY :
 *
 *******************************************************************************/
#ifndef   STD_TYPES_H_
#include <std_types.h>
#endif

#ifndef __APP_CATDL_INIT_H__
#include "app_catdl_init.h"
#endif

#include <cdl2_proto.h>
#include <oel_rtos_posix.h>

#include "app_disp_brightness.h"
#include "../src_j1939/app_j1939_map.h"

/*******************************************************************************
 ***
 ***    Referenced Functions, Data, & Symbols
 ***    -- #Include's --
 ***
 *******************************************************************************/

oel_rtos_resource_t *cdl_resource;


void app_catdl_init(void)
{
    /*update the pie for the display brightness PID*/
    appget_disp_brightness_init_j1939();
}
