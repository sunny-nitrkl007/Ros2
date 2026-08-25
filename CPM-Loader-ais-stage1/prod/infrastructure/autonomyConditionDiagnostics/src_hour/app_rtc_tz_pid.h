/*******************************************************************************
 *** Copyright 2014-2018 Caterpillar Inc.  All rights reserved.
 ***
 *** FILE NAME: app_rtc_tz_pid.h
 ***
 ***
 *** DESCRIPTION:
 ***
 *******************************************************************************/
#ifndef __APP_RTC_TZ_PID_H__
#define __APP_RTC_TZ_PID_H__
#include <oel_pack.h>
#include <clock_proto.h>
#include <cdl2_proto.h>
#include <scl_j1939_struct.h>
#include <scl_j1939_proto.h>
#include <stdlib.h>
#include <cat_std_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTC_PID    0xF835
#define TZ_PID     0xFA27

/******************************************************************************
 ***
 ***    External function prototypes, data, & symbols
 ***    -- #Include's --
 ***
 ******************************************************************************/
extern bool_t tz_received;
extern bool_t rtc_received;

extern void appget_tz_request_init_j1939(void);
extern void appget_rtc_request_init_j1939(void);
extern void appget_tz_request_update_j1939(void);
extern void appget_rtc_request_update_j1939(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_RTC_TZ_PID_H__ */
