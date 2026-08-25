/******************************************************************************************************************
 Copyright 2008 Caterpillar Inc. All rights reserved.
 -------------------------------------------------------------------------------------------------------------------
 File name: app_scl_fps_j1939_config.h

Description:    This file contains the public interface to scl_fps.
 ******************************************************************************************************************/
#ifndef APP_SCL_FPS_J1939_CONFIG_H_
#define APP_SCL_FPS_J1939_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <std_t.h>
#include <scl_fps_j1939.h>
#include <scl_fps.h>

    extern scl_fps_j1939_link_t *app_scl_fps_j1939_link;

    extern const scl_j1939_config_t app_scl_j1939_config;
    extern const scl_fps_j1939_ecm_config_t app_scl_fps_j1939_ecm_config;
    extern scl_fps_j1939_link_config_t app_scl_fps_j1939_link_config;

#ifdef __cplusplus
}
#endif
#endif /* #ifndef APP_SCL_FPS_J1939_CONFIG_H_ */
