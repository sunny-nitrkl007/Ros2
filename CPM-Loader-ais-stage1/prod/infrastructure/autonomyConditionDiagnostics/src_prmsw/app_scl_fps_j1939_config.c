/*************************************************************************
 Copyright 2009 Caterpillar Inc. All rights reserved.
--------------------------------------------------------------------------
File name: app_scl_fps_j1939_config.c

Description:This file contains the ecm and link configuration details.
 *************************************************************************/

#include "app_scl_fps_j1939_config.h"

scl_fps_j1939_link_t *app_scl_fps_j1939_link;

/* ECM Configuration   */
const scl_fps_j1939_ecm_config_t app_scl_fps_j1939_ecm_config =  
    SCL_FPS_J1939_ECM_CFG_I1(1);
/* Link Configuration */
scl_fps_j1939_link_config_t app_scl_fps_j1939_link_config = 
    SCL_FPS_J1939_LINK_CFG_I1();

