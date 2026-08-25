/******************************************************************************
**
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%
*** %% COPYRIGHT (C) 2001-2012, 2016 CATERPILLAR INC. ALL RIGHTS RESERVED.
*** %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
**
*** FILE:
***     app_clock_init.h
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
#ifndef __APP_CLOCK_INIT_H__
#define __APP_CLOCK_INIT_H__

#include <oel_rtos_posix.h>
#ifndef   CLOCK_DEFS_H_
#include <clock_defs.h>
#endif
#include <oel_rtos.h>
#include <clock_proto.h>
#include <clock_struct.h>


#include <synclk.h>
#include <synclk_proto.h>
#include <synclk_defs.h>
#include <scl_synclk_dl_independent.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
***
***    Constants defined for this file
***    -- #Define's --
***
******************************************************************************/

#define PID_DIAGNOSTIC_CLOCK_ID                       0x5E
/*
$5E Diagnostic Clock

    Used to read and write the diagnostic clock. The write is used to 
    program new ECMs placed on an existing piece of equipment. Write 
    security for this parameter must be maintained at factory level.

    Parameter Data Length:          2 Bytes
    Data Type:                      Unsigned Word
    Resolution:                     1 hours/bit
    Valid Data Range:               0 to 65535 hours
    Transmission Update Period:     As Needed
    Format:
        PID    Data
        $5E    a a
            a a - Diagnostic clock
*/

/******************************************************************************
***
***    Data types defined for this file
***    -- Struct's, Typedef's, Enum's --
***
******************************************************************************/
#define SAVEPERIOD 1000//(360000)
#define NUM_WALKING_BYTES (28)

 typedef enum
 {
    RTC_TYPE_STANDALONE = 0,
    RTC_TYPE_DISTRIBUTED_CLIENT,
    RTC_TYPE_DISTRIBUTED_SERVER,
    RTC_TYPE_PID_CLIENT,
    RTC_TYPE_PID_SERVER
 } rtc_type_e;

 typedef enum
 {
    RTC_SECURITY_NOT_SECURE = 0,
    RTC_SECURITY_SECURE
 } rtc_scurity_e;

/******************************************************************************
***
***    File scope functions defined for this file
***    -- Function Prototypes --
***
******************************************************************************/
extern void app_clock_init( void );
extern void app_sync_clock_init( void );
extern void app_rtc_tz_init( void );
extern void app_clock_update( void );
extern void app_sync_clock_update( void );
extern void app_rtc_tz_update(void);

extern void app_log_alert(const char* file, int lineNum, const char* message);

/******************************************************************************
***
***    File scope symbols defined for this file
***    -- Symbols --
***
******************************************************************************/


/******************************************************************************
***
***    Global data symbols defined in this file
***    -- Global Symbols --
***
******************************************************************************/
extern unsigned_8 EngineRunning;
extern const clock_config_t app_clock_config;
extern clock_result_t clockReturnVal;
extern clock_time_t currentHour_Sec;
extern unsigned_8 app_scl_synclk_master_mid;

#ifdef __cplusplus
}
#endif

#endif /* __APP_CLOCK_INIT_H__ */

