/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaCalMgr.h
DESCRIPTION:
*******************************************************************************/
#ifndef LPSSACALMGR_H_
#define LPSSACALMGR_H_
/*******************************************************************************
** -- #Include's --
*******************************************************************************/


#if defined (__cplusplus)
extern "C"
{
#endif
#ifndef CAL_MGR_H_
#include  <cal_mgr.h>
#endif
#ifndef CDL2_MOD_PROTO_H_
#include  <cdl2_mod_proto.h>
#endif

#include <oel_rtos_posix.h>
#include <oel_oop.h>
#include <clock_struct.h>
#ifndef boolean
#include <cat_std_types.h>
#endif
#include <cdl2_struct.h>
#include <cdl_mod_struct.h>
#include <scl_j1939_struct.h>
#include <cdl_critics_defs.h>
#include <cdl2_defs.h>
#include <oel_rtos.h>
#include <cdl2_mod_defs.h>
#include <catdl_public.h>


#ifndef   SCL_CI_ES_H
#include <scl_ci_es.h>
#endif

#if defined (__cplusplus)
}
#endif

///* Calibration ID's for SA */

#define SA_EMPTY_BUCKET_CAL_ID      (0x0256)
#define SA_FULL_BUCKET_CAL_ID       (0x0257)
#define ADV_EMPTY_BUCKET_CAL_ID     (0x03A9)
#define ADV_FULL_BUCKET_CAL_ID      (0x03AA)


#define SA_LIFT_LINKAGE_CAL_ID      (0x00D0)
#define SA_TILT_LINKAGE_CAL_ID      (0x00D1)    // ROTARY SENSORS
#define SA_TILT_INLINE_LINKAGE_CAL_ID  (0x015E) // INLINE SENSORS

/*
** Percent - Common Scaling of 0.00152587890625 %/bit
**           where 100 Percent is equal to (1*(1<<16))
*/
#define PERCENT_COMMON_RADIX        (16)

#define ROUND_VAL(NUM)                  ( (NUM < 0) ? (NUM - 0.5) : (NUM + 0.5) )

#define CONVERT_PERCENT( PERCENT )      (int_least32_t)( ROUND_VAL( ((double)(PERCENT)  * (1UL<<PERCENT_COMMON_RADIX)) / 100 ) )    /* -1 to 1 X (1 << 16) */


/****************************
** TILT   Communicated Step Numbers
****************************/
typedef enum {

    /* Communicated Setup Steps */
    CAL_TILT_LINKAGE_SETTING_UP  = 1,
    CAL_TILT_LINKAGE_SETUP_WAITING_FOR_PARKBRAKE,
    CAL_TILT_LINKAGE_SETUP_COMPLETE,
    /* Communicated Flow Steps */
    CAL_TILT_LINKAGE_FULL_RACK  =  CAL_TILT_LINKAGE_SETUP_COMPLETE,
    CAL_TILT_LINKAGE_FULL_LOWER,
    CAL_TILT_LINKAGE_FULL_RAISE,
    CAL_TILT_LINKAGE_FULL_DUMP
} CAL_TILT_LINKAGE_STEPS_E;


/****************************
** TILT INLINE SENSOR  Communicated Step Numbers
****************************/
typedef enum {
    /* Communicated Flow Steps */
    CAL_TILT_INLINE_LINKAGE_LIFT_INIT  =  CAL_TILT_LINKAGE_SETUP_COMPLETE,
    CAL_TILT_INLINE_LINKAGE_FULL_DUMP,
    CAL_TILT_INLINE_LINKAGE_FULL_RACK,
    CAL_TILT_INLINE_LINKAGE_FULL_RAISE,
    CAL_TILT_INLINE_LINKAGE_DUMP_STOP,
    CAL_TILT_INLINE_LINKAGE_RACK_STOP,
    CAL_TILT_INLINE_LINKAGE_FULL_LOWER
} CAL_TILT_INLINE_LINKAGE_STEPS_E;

/****************************
** LIFT   Communicated Step Numbers
****************************/
typedef enum {

    /* Communicated Setup Steps */
    CAL_LIFT_LINKAGE_SETTING_UP  = 1,
    CAL_LIFT_LINKAGE_SETUP_WAITING_FOR_PARKBRAKE,
    CAL_LIFT_LINKAGE_SETUP_COMPLETE,
    /* Communicated Flow Steps */
    CAL_LIFT_LINKAGE_FULL_RAISE  =  CAL_LIFT_LINKAGE_SETUP_COMPLETE,
    CAL_LIFT_LINKAGE_FULL_RACK,
    CAL_LIFT_LINKAGE_FULL_LOWER
} CAL_LIFT_LINKAGE_STEPS_E;


/**************************
**  Cal NEUTSETPT Workspace
**************************/
typedef struct
{
    float       lift_full_raise_dc;
    float       lift_full_lower_dc;
    float       tilt_full_rack_dc;
    float       tilt_full_dump_dc;
    float       tilt_dump_stop_angle;
    float       tilt_rack_stop_angle;
} CAL_Linkage_Work_t;

extern CAL_Linkage_Work_t            cal_linkage_workspace;

#define LINKAGE_NUM_CAL_POINTS (2)
#define CAL_LINKAGE_ABORTED_BY_ECM       (0x0009)
#define CAL_MACHINE_MODEL_NOT_SET		 (0x1041)
#define CAL_PAYLOAD_LEGAL_FOR_TRADE_SEALED (0x136C)

#endif /* LPSSACALMGR_H_ */

