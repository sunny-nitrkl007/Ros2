/******************************************************************************
 **
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *** %%
 *** %% COPYRIGHT (C) 2001-2012, 2016 CATERPILLAR INC. ALL RIGHTS RESERVED.
 *** %%
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 **
 *** FILE:
 ***     app_clock_init.c
 ***
 *** DESCRIPTION:
 ***     This file contains a function that initializes the clock ( service
 ***     hourmeter ) module.
 ***
 ******************************************************************************/

/******************************************************************************
 ***
 ***    External function prototypes, data, & symbols
 ***    -- #Include's --
 ***
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#ifndef __APP_CLOCK_INIT_H__
#include "app_clock_init.h"
#endif
#ifndef __APP_CAN_INIT_H__
#include "../src_j1939/app_can_init.h"
#endif
#ifndef __APP_RTC_TZ_PID_H__
#include "app_rtc_tz_pid.h"
#endif
#include "../src_nvm/app_nvm_globals.h"
#include "../src_nvm/app_nvm_file_cfg.h"
#include "../src_j1939/app_j1939_map.h"
#include <oel_pack.h>
#include <clock_rtc_sec.h>
#include <clock_proto.h>
#include <clock_tm_zone_proto.h>
#include <scl_rtc_j1939_client.h>
#include <scl_rtc_j1939_client_config.h>
#include <scl_rtc_j1939_server.h>
#include <scl_rtc_j1939_server_config.h>
#include <scl_rtc_j1939_auth_key_externs.h>
#include <scl_tz_j1939_client.h>
#include <scl_tz_j1939_client_config.h>
#include <scl_tz_j1939_server.h>
#include <scl_tz_j1939_server_config.h>
#include <scl_tz_j1939_auth_key_externs.h>

/******************************************************************************
 ***
 ***    Constants defined for this file
 ***    -- #Define's --
 ***
 ******************************************************************************/
/* counter to track requested sending message */

#define TASK_1X_LPTIM (0.010)

#define SYNCLK_RESEND_TIMER  (5.0/TASK_1X_LPTIM)
#define GET_NAME_FIELDS2( name ) \
{\
    name.Mu8_industry_group, \
    name.Mu8_vehicle_system, \
    name.Mu8_vehicle_system_instance, \
    name.Mu8_function, \
    name.Mu8_function_instance, \
    name.Mu8_ecu_instance, \
    name.Mu16_manufacturer_code, \
    name.Mu32_identity_number\
}

static unsigned_8 PrevEngineRunning = -1;
clock_time_t currentHour_Sec= -1;
uint8_t app_keyswitch_status = TRUE;
clock_result_t clockReturnVal;
synclk_arb_t app_scl_synclk_hierarchy_index = SYNCLK_NOT_MASTER_CAP;

/******************************************************************************
 ***
 ***    Data types defined for this file
 ***    -- Struct's, Typedef's, Enum's --
 ***
 ******************************************************************************/

/******************************************************************************
 ***
 ***    File scope functions defined for this file
 ***    -- Function Prototypes --
 ***
 ******************************************************************************/

/******************************************************************************
 ***
 ***    File scope symbols defined for this file
 ***    -- Symbols --
 ***
 ******************************************************************************/
scl_rtc_j1939_client_config_t client_cfg;
scl_tz_j1939_client_config_t tz_client_cfg;
scl_rtc_j1939_server_config_t server_cfg;
scl_tz_j1939_server_config_t tz_server_cfg;
clock_rtc_sec_config_t rtc_sec_cfg;
/******************************************************************************
 ***
 ***    Global data symbols defined in this file
 ***    -- Global Symbols --
 ***
 ******************************************************************************/

/** clock semaphore config
 */
static const oel_rtos_semaphore_pi_vconfig_t clock_semaphore_vconfig =
        OEL_RTOS_SEMAPHORE_PI_VCONFIG(1);

const clock_config_t app_clock_config =
{
        OEL_BASE(&clock_semaphore_vconfig)
};

/******************************************************************************
FUNCTION NAME:  app_notify_mar

DESCRIPTION: Function that the synclk library will call if this ECU is in the
   role of synclk system slave and transitions into or out of the auto-sync
   window (+/- 50 hours different than master node).

PARAMETER DESCRIPTION:
man_adj_req - TRUE means a Service Technician needs to manually synchronize
   this node with the master.  FALSE means a Serivce Tecnicion is no longer
   needed.

RETURN VALUE:   none
 ****************************************************************************/

void app_notify_mar(boolean man_adj_req)
{
    unsigned_8 tx_data[6]={0x06,0xF9,0xAA,0xFF,0xFF,0xFF};

    tx_data[3] = (unsigned_8)man_adj_req;
    scl_j1939_que_tx_msg_by_address(
            Ph_Link_app,                     /* Handle identifying J1939 connection */
            0x00AA00,                   /* Debug_PGN */
            6,                          /* priority */
            SCL_J1939_GLOBAL_ADDRESS,   /* destination address */
            0,                          /* virtual ECM index */
            tx_data,                    /* pointer to message */
            6);                         /* number of bytes to tx */
    return;
}

/******************************************************************************
FUNCTION NAME:  app_notify_man_adj

DESCRIPTION: Function that the synclk library will call if this ECU is in the
role of synclk system master,  and a service tool performs a manual adjustment.

PARAMETER DESCRIPTION:
      shm_before - value of the SHM before the manual adjustment
      shm_after - value of the SHM after the manual adjustment

RETURN VALUE: none
 *******************************************************************************/
void app_notify_man_adj(unsigned_32 shm_before, unsigned_32 shm_after)
{
    unsigned_8 tx_data[5];
    unsigned_8 *buff;


    buff=tx_data;
    (void)shm_before;
    (void)shm_after;
    OEL_PACK_LE_8(buff,0x04);
    OEL_PACK_LE_32_NO_INCR(buff,shm_after);

    scl_j1939_que_tx_msg_by_address(
            Ph_Link_app,                     /* Handle identifying J1939 connection */
            0x00BB00,                   /* PGN */
            6,                          /* priority */
            SCL_J1939_GLOBAL_ADDRESS,   /* destination address */
            0,                          /* virtual ECM index */
            tx_data,                    /* pointer to message */
            5);                         /* number of bytes to tx */
    return;
}

/******************************************************************************
FUNCTION NAME:  app_scl_keyswitch_status

DESCRIPTION: Function that calls to send KeySw Status

PARAMETER DESCRIPTION: None

RETURN VALUE: none
 *******************************************************************************/

void app_scl_keyswitch_status(boolean *keysw_status){
    *keysw_status = app_keyswitch_status;
}

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<<<<<<<<<<<<                                                        >>>>>>>>>>>
<<<<<<<<<<<<              Start of code for this file               >>>>>>>>>>>
<<<<<<<<<<<<                                                        >>>>>>>>>>>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

void app_clock_init( void )
/*BEFORE: This function should be called from the startup task.
//
//AFTER:  The clock module has been intialized.
//        The keyon clock has started counting.
//        Clock_runservice() can be called to start the service hourmeter.
 */
{
    bool_least_t keepLooping = TRUE;

    /* General clock initialization */
    clockReturnVal = clock_ginit();

    clock_cfg_num_nvm_blocks(2);

    /* set non-volatile save periods */
    clockReturnVal |= clock_cfg_set_save_period ( clock_ID_CLOCK_SERVICE,
            clock_ID_NVM_BLOCK_PRIMARY, SAVEPERIOD );
    clockReturnVal |= clock_cfg_set_save_period ( clock_ID_CLOCK_SERVICE,
            clock_ID_NVM_BLOCK_SECONDARY, SAVEPERIOD );
    clockReturnVal |= clock_cfg_set_save_period ( clock_ID_CLOCK_KEYON,
            clock_ID_NVM_BLOCK_PRIMARY, SAVEPERIOD );
    clockReturnVal |= clock_cfg_set_save_period ( clock_ID_CLOCK_KEYON,
            clock_ID_NVM_BLOCK_SECONDARY, SAVEPERIOD ); //SAVEPERIOD

    /*Set size of walking pattern of clock module*/
    clock_cfg_set_wb_size( clock_ID_CLOCK_KEYON,
            clock_ID_NVM_BLOCK_PRIMARY, NUM_WALKING_BYTES);

    clock_cfg_set_wb_size( clock_ID_CLOCK_SERVICE,
            clock_ID_NVM_BLOCK_PRIMARY, NUM_WALKING_BYTES);

    clock_cfg_set_wb_size( clock_ID_CLOCK_KEYON,
            clock_ID_NVM_BLOCK_SECONDARY, NUM_WALKING_BYTES);

    clock_cfg_set_wb_size( clock_ID_CLOCK_SERVICE,
            clock_ID_NVM_BLOCK_SECONDARY, NUM_WALKING_BYTES);

    while ( keepLooping )
    {
        clockReturnVal= clock_go();

        switch ( clockReturnVal )
        {
        case clock_RESULT_SUCCESS:
            keepLooping = FALSE;
            break;

        case clock_RESULT_ERR_UNFMT_1:
        case clock_RESULT_ERR_UNFMT_2:
        case clock_RESULT_ERR_WRONG_FMT_1:
        case clock_RESULT_ERR_WRONG_FMT_2:
            clockReturnVal = clock_erase_and_reformat__result ( clockReturnVal );
            (void)clockReturnVal;
            break;

        default:
            break;
        }
    }

    return;
}

void app_scl_j1939_ma_client_status_cb(
        const scl_j1939_link_t Ph_link,
        const int_8 Pi8_init_ecm,
        const unsigned_8 Pu8_session,
        const unsigned_8 Pu8_status,
        const unsigned_8 Pu8_service,
        const unsigned_8 Pu8_param_type,
        const unsigned_32 Pu32_param,
        const unsigned_16 Pu16_length,
        const unsigned_32 Pu32_error_code)
{
    (void)Ph_link;
    (void)Pi8_init_ecm;
    (void)Pu8_session;
    (void)Pu8_status;
    (void)Pu8_service;
    (void)Pu8_param_type;
    (void)Pu32_param;
    (void)Pu16_length;
    (void)Pu32_error_code;
    if (MA_TRANS_ERROR_NONE != Pu32_error_code) {
        // problem
        char log_string[100]; // Long string
        sprintf(log_string, "MAP Client Error: session %i, status %i, error_code %i", Pu8_session, Pu8_status, Pu32_error_code);
        app_log_alert(__FILE__, __LINE__, log_string);
    }
}

void app_rtc_tz_init( void )
{
    clock_rtc_sec_config_t temp_rtc_sec_cfg = CLOCK_RTC_SEC_CONFIG_V0(1092,1200,app_nvm_file_cfg[NVM_KEY_RTC_TIME_ZONE_NEW].data_id );
    rtc_sec_cfg = temp_rtc_sec_cfg;

    if(RTC_TYPE_STANDALONE == rtc_type || RTC_TYPE_PID_SERVER == rtc_type) //stand alone (rtc set, tz set from ET) or pid server (same as telematics)
    {
        clock_rtc_init();
        clock_tzone_init(app_nvm_file_block_table[NVM_KEY_RTC_TIME_ZONE_OLD]);
    }
    else if(RTC_TYPE_DISTRIBUTED_CLIENT == rtc_type) //distributed client (rtc sync, tz sync from distributed server)
    {
        clock_rtc_init_v2();
        clock_tzone_init_v2(app_nvm_file_cfg[NVM_KEY_RTC_TIME_ZONE_NEW].data_id);
    }
    else if(RTC_TYPE_DISTRIBUTED_SERVER == rtc_type) //distributed server (*mixed strategy)
    {
        // *mixed strategy means, old way of initialization for ET support
        // and new way of initialization for client-server communication
        // Note: There are two NVM blocks allocated for new and old TZ
        clock_rtc_init();
        clock_tzone_init(app_nvm_file_block_table[NVM_KEY_RTC_TIME_ZONE_OLD]);

        clock_rtc_init_v2();
        clock_tzone_init_v2(app_nvm_file_cfg[NVM_KEY_RTC_TIME_ZONE_NEW].data_id);
    }
    else if(RTC_TYPE_PID_CLIENT == rtc_type) // pid client (rtc set, tz set from telematics)
    {
        clock_rtc_init();
        clock_tzone_init(app_nvm_file_block_table[NVM_KEY_RTC_TIME_ZONE_OLD]);

        scl_j1939_set_ma_client_configuration(
            Ph_Link_app, /* scl_j1939_link_t Ph_link */
            app_scl_j1939_ma_client_status_cb,     /* Callback */
            app_scl_j1939_max_ma_sessions);

        appget_rtc_request_init_j1939();
        appget_tz_request_init_j1939();
    }

    if(RTC_TYPE_DISTRIBUTED_CLIENT == rtc_type)
    {
        scl_rtc_j1939_client_config_t temp_client_cfg = SCL_RTC_J1939_CLIENT_CFG_I1(
                2000,
                SCL_RTC_J1939_CLIENT_DISC_SERV,
                rtc_master_mid,
                GET_NAME_FIELDS2(j1939_name_address),
                SCL_J1939_NAME_FULL(
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF ));

        scl_tz_j1939_client_config_t temp_tz_client_cfg = SCL_TZ_J1939_CLIENT_CFG_I0(
                SCL_TZ_J1939_CLIENT_DISC_SERV,
                rtc_master_mid,
                GET_NAME_FIELDS2(j1939_name_address),
                SCL_J1939_NAME_FULL(
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF,
                        0xFF ));
        client_cfg = temp_client_cfg;
        tz_client_cfg = temp_tz_client_cfg;

        scl_rtc_j1939_client_init(&client_cfg, Ph_Link_app, 0 );
        scl_tz_j1939_client_init(&tz_client_cfg, Ph_Link_app, 0);

        if(RTC_SECURITY_SECURE == rtc_security)
        {
            clock_rtc_sec_init(&rtc_sec_cfg);
            scl_rtc_j1939_client_enable_secure((void*)scl_rtc_j1939_auth_key_am1_key1);
            scl_tz_j1939_client_enable_secure((void*)scl_tz_j1939_auth_key_am1_key1);
        }
    }
    else if(RTC_TYPE_DISTRIBUTED_SERVER == rtc_type) //server
    {
        scl_rtc_j1939_server_config_t temp_server_cfg =  SCL_RTC_J1939_SERVER_CFG_I1(SCL_RTC_J1939_SERVER_TYPE_MASTER,0x3E8);
        scl_tz_j1939_server_config_t temp_tz_server_cfg = SCL_TZ_J1939_SERVER_CFG_I0(SCL_TZ_J1939_SERVER_TYPE_MASTER);

        server_cfg = temp_server_cfg;
        tz_server_cfg = temp_tz_server_cfg;

        scl_rtc_j1939_server_init(&server_cfg,Ph_Link_app, 0 );
        scl_tz_j1939_server_init(&tz_server_cfg,Ph_Link_app, 0);

        if(RTC_SECURITY_SECURE == rtc_security)
        {
            clock_rtc_sec_init(&rtc_sec_cfg);
            clock_rtc_set_sec_enabled(TRUE);
            scl_rtc_j1939_server_sec_init((void*)scl_rtc_j1939_auth_key_am1_key1,Ph_Link_app);
            scl_tz_j1939_server_sec_init((void*)scl_tz_j1939_auth_key_am1_key1,Ph_Link_app);
        }
    }
}

void app_sync_clock_init( void )
{
    if ( TRUE == can_init_completed && scl_synclk_j1939_ginit(app_scl_synclk_hierarchy_index, Ph_Link_app, 0) == clockReturnVal )
    {
        if ( synclk_register_mar(app_notify_mar) != clockReturnVal )
        {
            //printf("synclk_register_mar FAILED\n");
        }

        if ( synclk_register_man_adj(app_notify_man_adj) != clockReturnVal )
        {
            //printf("synclk_register_man_adj FAILED\n");
        }

        if ( synclk_register_ksw_status(app_scl_keyswitch_status) != clockReturnVal )
        {
            //printf("synclk_register_ksw_status FAILED\n");
        }

    }
    else
    {
        //printf("scl_synclk_cdl_ginit FAILED\n");
    }
}

void app_clock_update(void)
{
    clock_update();

    if(EngineRunning != PrevEngineRunning)
    {
        PrevEngineRunning = EngineRunning;
        (EngineRunning == TRUE) ? clock_runservice(TRUE): clock_runservice(FALSE);
    }

    currentHour_Sec  = clock_getservice_sec();
}

void app_sync_clock_update(void)
{
    synclk_update();
}

void app_rtc_tz_update(void)
{
    if(RTC_TYPE_STANDALONE == rtc_type)
    {
        struct clock_tm clock_rtc_time;
        bool_least_t sec = FALSE;
        tzone_tx_comm_struct app_tm_zone_tx_tbl;
        if(clock_getreal_struct_tm(&clock_rtc_time) == clock_RESULT_SUCCESS)
        {
            //printf("SA RTC:%d/%d/%d %d:%d:%d\n", clock_rtc_time.tm_mday, clock_rtc_time.tm_mon + 1, clock_rtc_time.tm_year + clock_TM_0_YEAR, clock_rtc_time.tm_hour,clock_rtc_time.tm_min,clock_rtc_time.tm_sec);
            if(clock_tzone_get(&app_tm_zone_tx_tbl, &sec) == clock_RESULT_SUCCESS)
            {
                //printf("SA TZ:%d\n", app_tm_zone_tx_tbl.tzone_id);
            }
        }
    }
    else if(RTC_TYPE_DISTRIBUTED_CLIENT == rtc_type)
    {
        //unsigned_8 tz_status, rtc_missing_status;  // Set but not used
        struct clock_tm clock_rtc_time;
        unsigned_8 app_tm_zone_buff1[20];
        tzone_tx_comm_struct app_tm_zone_tx_tbl;

        if(clock_getreal_struct_tm(&clock_rtc_time) == clock_RESULT_SUCCESS)
        {
            //printf("Client RTC:%d/%d/%d %d:%d:%d\n", clock_rtc_time.tm_mday, clock_rtc_time.tm_mon + 1, clock_rtc_time.tm_year + clock_TM_0_YEAR, clock_rtc_time.tm_hour,clock_rtc_time.tm_min,clock_rtc_time.tm_sec);
            clock_tzone_retrieve(&app_tm_zone_tx_tbl, &app_tm_zone_buff1[0]);//output, local buffer
            //printf("Synced Timezone ID from server=%d\n",app_tm_zone_tx_tbl.tzone_id);
            //tz_comp_status = scl_tz_j1939_client_compat_status();
            //tz_status = scl_tz_j1939_client_std_tz_missing();
            //rtc_missing_status =  scl_rtc_j1939_client_pub_time_missing();    // Set but not used
            //rtc_error_status = scl_rtc_j1939_client_pub_time_error();         // Set but not used
            //printf("tz_comp_status %d, tz_status %d, rtc_missing_status %d, rtc_error_status %d \n",tz_comp_status, tz_status, rtc_missing_status, rtc_error_status);
        }
    }
    else if(RTC_TYPE_DISTRIBUTED_SERVER == rtc_type)
    {
        struct clock_tm clock_rtc_time;
        bool_least_t sec = FALSE;
        tzone_tx_comm_struct app_tm_zone_tx_tbl;
        if(clock_getreal_struct_tm(&clock_rtc_time) == clock_RESULT_SUCCESS)
        {
            //printf("Server RTC:%d/%d/%d %d:%d:%d\n", clock_rtc_time.tm_mday, clock_rtc_time.tm_mon + 1, clock_rtc_time.tm_year + clock_TM_0_YEAR, clock_rtc_time.tm_hour,clock_rtc_time.tm_min,clock_rtc_time.tm_sec);
            if(clock_tzone_get(&app_tm_zone_tx_tbl, &sec) == clock_RESULT_SUCCESS)
            {
                //printf("Server TZ:%d\n", app_tm_zone_tx_tbl.tzone_id);
            }
        }
    }
    else if(RTC_TYPE_PID_CLIENT == rtc_type) // ET/Telematics
    {
        struct clock_tm clock_rtc_time;
        bool_least_t sec = FALSE;
        tzone_tx_comm_struct app_tm_zone_tx_tbl;
        if(clock_getreal_struct_tm(&clock_rtc_time) == clock_RESULT_SUCCESS)
        {
            //printf("ET/Telematics RTC:%d/%d/%d %d:%d:%d\n", clock_rtc_time.tm_mday, clock_rtc_time.tm_mon + 1, clock_rtc_time.tm_year + clock_TM_0_YEAR, clock_rtc_time.tm_hour,clock_rtc_time.tm_min,clock_rtc_time.tm_sec);
            if(clock_tzone_get(&app_tm_zone_tx_tbl, &sec) == clock_RESULT_SUCCESS)
            {
                //printf("ET/Telematics TZ:%d\n", app_tm_zone_tx_tbl.tzone_id);
            }
        }

        appget_rtc_request_update_j1939();
        appget_tz_request_update_j1939();
    }
    else if(RTC_TYPE_PID_SERVER == rtc_type)
    {
        struct clock_tm clock_rtc_time;
        bool_least_t sec = FALSE;
        tzone_tx_comm_struct app_tm_zone_tx_tbl;
        if(clock_getreal_struct_tm(&clock_rtc_time) == clock_RESULT_SUCCESS)
        {
            //printf("ET/Telematics RTC:%d/%d/%d %d:%d:%d\n", clock_rtc_time.tm_mday, clock_rtc_time.tm_mon + 1, clock_rtc_time.tm_year + clock_TM_0_YEAR, clock_rtc_time.tm_hour,clock_rtc_time.tm_min,clock_rtc_time.tm_sec);
            if(clock_tzone_get(&app_tm_zone_tx_tbl, &sec) == clock_RESULT_SUCCESS)
            {
                //printf("ET/Telematics TZ:%d\n", app_tm_zone_tx_tbl.tzone_id);
            }
        }
    }
}
