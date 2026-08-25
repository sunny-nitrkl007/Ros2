/*******************************************************************************
 *** Copyright 2017 Caterpillar Inc.  All rights reserved.
 ***
 *** FILE NAME: app_disp_brightness.h
 ***
 ***
 *** DESCRIPTION:
 ***
 *******************************************************************************/
#ifndef __APP_DISP_BRIGHTNESS_H__
#define __APP_DISP_BRIGHTNESS_H__

#include <cdl2_proto.h>

/******************************************************************************
 ***
 ***    External function prototypes, data, & symbols
 ***    -- #Include's --
 ***
 ******************************************************************************/
#define TASK_1X_LPTIM (0.100)

/* 2 seconds */
#define POWER_UP_DELAY (2.0/TASK_1X_LPTIM)

/* 1 second */
#define RESEND_TIMER  (1.0/TASK_1X_LPTIM)

extern void appget_disp_brightness_init_j1939(void);
extern void appget_disp_brightness_update_j1939(void);
extern Cdl_gen_ovr_parm_06_t  appget_dispbrightness_request_parm;
extern unsigned_8 pidD0022B_pdata[];
extern bool_t disp_brightness_received;
extern unsigned_16 dispbrightness_request_timer;

#endif /* __APP_DISP_BRIGHTNESS_H__ */
