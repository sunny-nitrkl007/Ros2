/******************************************************************************************************************
 Copyright 2009 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: app_scl_fps_config.c

Description:
This file contains the call back function which stores the Component id and Product id.
 ************************************************************************************************************/


#include <std_t.h>
#include <string.h>
#include "app_scl_fps_config.h"
#include <oel_assert.h>
#include "../src_j1939/app_j1939_map.h"

uint_least8_t app_comp_id_length = 8;
uint_least8_t app_product_id_length = PIDF82D_LENGTH;
scl_fps_data_quality_t app_comp_id_data_quality = SCL_FPS_DATA_QUALITY_OK;
scl_fps_data_quality_t app_product_id_data_quality = SCL_FPS_DATA_QUALITY_OK;


/* SCL FPS Configuration */
static const oel_rtos_resource_pi_vconfig_t resource_scl_fps_vconfig =
    OEL_RTOS_RESOURCE_PI_VCONFIG( );


scl_fps_cfg_t app_scl_fps_config = SCL_FPS_CFG_I1(
    OEL_BASE(&resource_scl_fps_vconfig),
    OPEN_RC_MAX,
    (scl_fps_auth_data_ifc_t*)&fps_auth_data,
    (void *)OPEN_RC_MAX );

void fps_auth_data(void *context,scl_fps_auth_data_t* auth_data, scl_fps_rc_cfg_t* rc_cfg)
{

    if( rc_cfg == NULL )
    {
        return;
    }
    (void)context;

	memcpy(auth_data->comp_id, &pidF810_pdata, app_comp_id_length);
	memcpy(auth_data->product_id, &pidF82D_pdata, app_product_id_length);
	auth_data->comp_id_length = app_comp_id_length;
	auth_data->product_id_length = app_product_id_length;
	auth_data->comp_id_data_quality = app_comp_id_data_quality;
	auth_data->product_id_data_quality = app_product_id_data_quality;

    return;
}



/* Reason Codes supported by CPM MWL application 

$FC06 Total Load (RC4)
   - Used to read and program the total number of loads carried over the lifetime of the machine

*/
scl_fps_rc_cfg_t app_scl_fps_rc_4_config =  SCL_FPS_RC_CFG_I1(
                                              RC_4_TOTALS, 
                                              NULL, 
                                              NULL,
                                              NULL,
                                              NULL,
                                              (scl_fps_rc_token_ifc_t*)&app_fps_rc_4_token_callbk,
                                              NULL
                                              );

/* Callback function for PIDFC06 since it is requested to be reason code 4*/
uint_least16_t app_fps_rc_4_token_callbk( void* context, uint_least16_t reason_code)
{
   (void) reason_code;
   (void) context; 
   return 0;
}
