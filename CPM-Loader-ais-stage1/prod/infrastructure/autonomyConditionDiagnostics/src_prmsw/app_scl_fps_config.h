/******************************************************************************************************************
 Copyright 2008 Caterpillar Inc. All rights reserved.
 -------------------------------------------------------------------------------------------------------------------
 File name: app_scl_fps_j1939_config.h

Description:    This file contains the public interface to scl_fps.
 ******************************************************************************************************************/
#ifndef APP_SCL_FPS_CONFIG_H_
#define APP_SCL_FPS_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <oel_rtos_posix.h>
#include <std_t.h>
#include <scl_fps.h>
#include <oel_rtos.h>
#include <scl_j1939.h>


/* Reason Codes supported by CPM MWL application 

$FC06 Total Load (RC4)
   - Used to read and program the total number of loads carried over the lifetime of the machine

*/
#define RC_4_TOTALS         4
#define OPEN_RC_MAX        16

    extern scl_fps_rc_cfg_t app_scl_fps_rc_4_config;
    extern uint_least16_t app_fps_rc_4_token_callbk( void* context, uint_least16_t reason_code);
    extern scl_fps_cfg_t app_scl_fps_config;
    extern void fps_auth_data(void* context,scl_fps_auth_data_t* auth_data, scl_fps_rc_cfg_t* rc_cfg);

    extern uint_least8_t app_comp_id_length;
    extern uint_least8_t app_product_id_length;
    extern scl_fps_data_quality_t app_comp_id_data_quality;
    extern scl_fps_data_quality_t app_product_id_data_quality;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef APP_SCL_FPS_CONFIG_H_ */
