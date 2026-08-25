/*******************************************************************************
 *** Copyright 2014-2018 Caterpillar Inc.  All rights reserved.
 ***
 *** FILE NAME: app_rtc_tz_pid.c
 ***
 ***
 *** DESCRIPTION:
 ***
 *******************************************************************************/

/******************************************************************************
 ***
 ***    External function prototypes, data, & symbols
 ***    -- #Include's --
 ***
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#ifndef __APP_RTC_TZ_PID_H__
#include "app_rtc_tz_pid.h"
#endif

#ifndef __APP_CATDL_INIT_H__
#include "../src_catdl/app_catdl_init.h"
#endif

#ifndef __APP_CAN_INIT_H__
#include "../src_j1939/app_can_init.h"
#endif

#ifndef CLOCK_TM_ZONE_PROTO_H_
#include "clock_tm_zone_proto.h"
#endif

#include "app_clock_init.h"


/******************************************************************************
 ***
 ***    Constants defined for this file
 ***    -- #Define's --
 ***
 ******************************************************************************/
/*
 * This is called in a low priority 100ms update task.
 * It was incorrectly assumed to be 10ms in the past, so things didn't happen
 * as expected, but it was probably fine:
 *      2 / 0.01 = 200 -> 20 seconds
 *     10 / 0.01 = 1000 -> 100 seconds
 * I am correcting the update period, but keeping the same results.
 */
#define UPDATE_PERIOD_SECONDS (0.100)

#define POWER_UP_DELAY (20.0 / UPDATE_PERIOD_SECONDS)
#define RESEND_TIMER  (100.0 / UPDATE_PERIOD_SECONDS)

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
static void app_trans_cb_complete(
        const scl_j1939_link_t Ph_link,
        const int_8 Pi8_init_ecm,
        const unsigned_8 Pu8_session,
        const unsigned_8 Pu8_service,
        const unsigned_8 Pu8_param_type,
        const unsigned_32 Pu32_param,
        const unsigned_16 Pu16_length);

static unsigned_32 app_password_cb_tz(
        const scl_j1939_link_t Ph_link,
        const int_8 Pi8_init_ecm,
        const unsigned_8 Pu8_session,
        const unsigned_8 Pu8_param_type,
        const unsigned_32 Pu32_param,
        const unsigned_8 Pu8_service,
        const unsigned_16 Pu16_length,
        unsigned_16* Ppu16_data_size,
        unsigned_8* Pau8_data);

static unsigned_32 app_password_cb_rtc(
        const scl_j1939_link_t Ph_link,
        const int_8 Pi8_init_ecm,
        const unsigned_8 Pu8_session,
        const unsigned_8 Pu8_param_type,
        const unsigned_32 Pu32_param,
        const unsigned_8 Pu8_service,
        const unsigned_16 Pu16_length,
        unsigned_16* Ppu16_data_size,
        unsigned_8* Pau8_data);


/******************************************************************************
 ***
 ***    File scope symbols defined for this file
 ***    -- Symbols --
 ***
 ******************************************************************************/
/* Client data*/
bool_t tz_received = FALSE;

/* Client data*/
bool_t rtc_received = FALSE;

/* counter to track requested sending message */
static unsigned_16 tz_request_timer;

/* counter to track requested sending message */
static unsigned_16 rtc_request_timer;

/******************************************************************************
 ***
 ***    Global data symbols defined in this file
 ***    -- Global Symbols --
 ***
 ******************************************************************************/
/*------------------------------------------------------------------------------
 *** FUNCTION NAME: appget_tz_request_init_j1939 (PID client code)
------------------------------------------------------------------------------*/
void appget_tz_request_init_j1939(void)
{
    /*
     * We are requesting the timezone information 7 seconds after the
     * clock request.  I don't know where 7 came from.
     */
    tz_request_timer = (unsigned_16)(POWER_UP_DELAY + (7.0 / UPDATE_PERIOD_SECONDS));
}

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: appget_rtc_request_init_j1939 (PID client code)
------------------------------------------------------------------------------*/
void appget_rtc_request_init_j1939(void)
{
    rtc_request_timer = (unsigned_16)POWER_UP_DELAY;
}

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_password_cb_tz
------------------------------------------------------------------------------*/
static unsigned_32 app_password_cb_tz(
        const scl_j1939_link_t Ph_link,
        const int_8 Pi8_init_ecm,
        const unsigned_8 Pu8_session,
        const unsigned_8 Pu8_param_type,
        const unsigned_32 Pu32_param,
        const unsigned_8 Pu8_service,
        const unsigned_16 Pu16_length,
        unsigned_16* Ppu16_data_size,
        unsigned_8* Pau8_data)
{
    (void)Ph_link;
    (void)Pi8_init_ecm;
    (void)Pu8_session;
    (void)Pu16_length;
    (void)Pu8_param_type;
    (void)Pu32_param;

    if ((MA_COMMAND_READ == Pu8_service) &&
            (NULL != Ppu16_data_size) &&
            (20 == *Ppu16_data_size) &&
            (NULL != Pau8_data) &&
            (19 == Pau8_data[0])) {

       tzone_rcv_comm_struct tzone_data;
       clock_tzone_store(&tzone_data, Pau8_data);

       tz_received = TRUE;

       /*
       app_log_alert(__FILE__, __LINE__, "Received time zone.");
       */
       return MA_TRANS_ERROR_NONE;
    }

    app_log_alert(__FILE__, __LINE__, "Failed to receive time zone.");
    return MA_TRANS_ERROR_INTERNAL_FAILURE;
}

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_password_cb_rtc
------------------------------------------------------------------------------*/
static unsigned_32 app_password_cb_rtc(
        const scl_j1939_link_t Ph_link,
        const int_8 Pi8_init_ecm,
        const unsigned_8 Pu8_session,
        const unsigned_8 Pu8_param_type,
        const unsigned_32 Pu32_param,
        const unsigned_8 Pu8_service,
        const unsigned_16 Pu16_length,
        unsigned_16* Ppu16_data_size,
        unsigned_8* Pau8_data)
{
    (void)Ph_link;
    (void)Pi8_init_ecm;
    (void)Pu8_session;
    (void)Pu16_length;
    (void)Pu8_param_type;
    (void)Pu32_param;

    if ((MA_COMMAND_READ == Pu8_service) &&
            (NULL != Ppu16_data_size) &&
            (7 == *Ppu16_data_size) &&
            (NULL != Pau8_data) &&
            (6 == Pau8_data[0])) {

        struct clock_CAN_struct utc;
        struct clock_tm tm;

        /* Get the time from the datalink message */
        utc.clock.quarter_sec = Pau8_data[1]; // a - Seconds
        utc.clock.min         = Pau8_data[2]; // b - Minutes
        utc.clock.hour        = Pau8_data[3]; // c - Hours
        utc.date.month        = Pau8_data[4]; // d - Month
        utc.date.quarter_day  = Pau8_data[5]; // e - Day
        utc.date.year         = Pau8_data[6]; // f - Year

        if (clock_RESULT_SUCCESS == clock_CAN_to_tm (&tm, &utc)) {
            unsigned_32 server_seconds;
            unsigned_32 client_seconds;

            server_seconds = clock_tm_to_seconds(&tm);
            if (clock_RESULT_SUCCESS == clock_getreal_seconds(&client_seconds, NULL)) {
                rtc_received = TRUE;
                if (abs(server_seconds - client_seconds) > 1) {
                    clock_setreal_struct_tm2(&tm, FALSE);

                    { // Clock is different.
                        char log_string[100]; // Long string
                        sprintf(log_string, "Their Seconds: %u, My Seconds: %u", server_seconds, client_seconds);
                        app_log_alert(__FILE__, __LINE__, log_string);
                    }
                }
            }
        }

        /*
        app_log_alert(__FILE__, __LINE__, "Received rtc.");
        */
        return MA_TRANS_ERROR_NONE;
    }

    app_log_alert(__FILE__, __LINE__, "Failed to receive rtc.");
    return MA_TRANS_ERROR_INTERNAL_FAILURE;
}

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_tz_request_update_j1939 (PID client code)
------------------------------------------------------------------------------*/
void appget_tz_request_update_j1939(void)
{
    if (tz_request_timer != 0) {
        tz_request_timer --;
    }

    /* send request message(s) when timer is elapsed*/
    if (tz_request_timer == 0) {
        unsigned_8 session_index = scl_j1939_ma_client_transaction_by_address(
                Ph_Link_app, 0, rtc_master_mid, MA_COMMAND_READ,
                0x80, TZ_PID, 0x01,/* Length/Number requested */
                app_trans_cb_complete, app_password_cb_tz, FALSE);

        if (0xFF == session_index) {
            app_log_alert(__FILE__, __LINE__, "Error requesting time zone.");
        }
        else {
            /*
            char log_string[100]; // Long string
            sprintf(log_string, "Requested time zone, session %i", session_index);
            app_log_alert(__FILE__, __LINE__, log_string);
            */
        }

        /* the request got transmitted- reset request timer */
        tz_request_timer = (unsigned_16)RESEND_TIMER;
        tz_received = FALSE;
    }
}

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_rtc_request_update_j1939 (PID client code)
------------------------------------------------------------------------------*/
void appget_rtc_request_update_j1939(void)
{
    if (rtc_request_timer != 0) {
        rtc_request_timer --;
    }

    /* send request message(s) when timer is elapsed*/
    if (rtc_request_timer == 0) {
        unsigned_8 session_index = scl_j1939_ma_client_transaction_by_address(
                Ph_Link_app, 0, rtc_master_mid, MA_COMMAND_READ,
                0x80, RTC_PID, 0x01, /* Length/Number requested */
                app_trans_cb_complete, app_password_cb_rtc, FALSE);

        if (0xFF == session_index) {
            app_log_alert(__FILE__, __LINE__, "Error requesting rtc.");
        }
        else {
            /*
            char log_string[100]; // Long string
            sprintf(log_string, "Requested rtc, session %i", session_index);
            app_log_alert(__FILE__, __LINE__, log_string);
            */
        }

        /* the request got transmitted- reset request timer */
        rtc_request_timer = (unsigned_16)RESEND_TIMER;
        rtc_received = FALSE;
    }
}

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_trans_cb_complete
------------------------------------------------------------------------------*/
static void app_trans_cb_complete(
        const scl_j1939_link_t Ph_link,
        const int_8 Pi8_init_ecm,
        const unsigned_8 Pu8_session,
        const unsigned_8 Pu8_service,
        const unsigned_8 Pu8_param_type,
        const unsigned_32 Pu32_param,
        const unsigned_16 Pu16_length)
{
    (void)Ph_link;
    (void)Pi8_init_ecm;
    (void)Pu8_session;
    (void)Pu8_service;
    (void)Pu8_param_type;
    (void)Pu32_param;
    (void)Pu16_length;
    /*
    char log_string[100]; // Long string
    sprintf(log_string, "Transaction complete, session %i", Pu8_session);
    app_log_alert(__FILE__, __LINE__, log_string);
    */
}
