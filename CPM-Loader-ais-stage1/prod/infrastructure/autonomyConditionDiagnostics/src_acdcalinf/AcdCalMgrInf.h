/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: AcdCalMgrInf.h
DESCRIPTION:
*******************************************************************************/
#ifndef ACDCALMGRINF_H_
#define ACDCALMGRINF_H_
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <CalInfConnect.h>
#include <AcdCalParamHndlr.h>		// for AcdCalParamHndlr.c

#include <cat_std_types.h>
#include <scl_j1939_struct.h>
#include <scl_qr_j1939.h>

#include <oel_pack.h>
#include <oel_assert.h>
#include <std_t.h>
#include <scl_j1939.h>
#include <scl_pgb_j1939.h>
#include <oel_rtos.h>
#include<cdl2_proto.h>
#include <scl_util.h>
#include <sys/time.h>

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

#if defined (__cplusplus)
}
#endif

extern CAL_MGR_Work_t cal_mgr_workspace;
extern const CAL_MGR_Cnfg_t cal_mgr_cnfg;

/*******************************************************************************
***
*** File scope symbols defined for this file
*** -- Symbols --
***
*******************************************************************************/
extern CAL_MGR_MR_E cal_mgr_callback(CAL_MGR_MC_E cmd,
    	                                   CAL_MGR_Work_t   *cmw,
    	                                   const CAL_MGR_Cnfg_t   *cmc,
    	                                   CAL_MGR_Cal_t    *cal);


extern void init_calibration();
extern void enable_calibration();
extern void acd_cal_mgr_update();
extern void init_cal_can();

/*******************************************************************************
***
*** Constants defined for this file
*** -- #Define's --
***
*******************************************************************************/
/* Calibration ID's for SA */
#define SA_EMPTY_BUCKET_CAL_ID   (0x0256)
#define SA_FULL_BUCKET_CAL_ID  (0x0257)
#define SA_LIFT_LINKAGE_CAL_ID (0x00D0)
#define SA_TILT_LINKAGE_CAL_ID    (0x00D1)      /* ROTARY SENSORS */
#define SA_TILT_INLINE_LINKAGE_CAL_ID  (0x015E) /* INLINE SENSORS */

/* Calibration ID's for ADV */
#define ADV_EMPTY_BUCKET_CAL_ID   (0x03A9)
#define ADV_FULL_BUCKET_CAL_ID  (0x03AA)


#define SA_EMPTY_BUCKET_ENTRY           \
{                                       \
    SA_EMPTY_BUCKET_CAL_ID,             \
    cal_mgr_callback,                   \
    &cal_sa_work,                       \
    &cal_sa_cnfg                        \
}

#define SA_FULL_BUCKET_ENTRY            \
{                                       \
    SA_FULL_BUCKET_CAL_ID,              \
    cal_mgr_callback,                   \
    &cal_sa_work,                       \
    &cal_sa_cnfg                        \
}

#define SA_LIFT_LINKAGE_ENTRY           \
{                                       \
    SA_LIFT_LINKAGE_CAL_ID,             \
    cal_mgr_callback,                   \
    &cal_sa_work,                       \
    &cal_sa_cnfg                        \
}

#define SA_TILT_LINKAGE_ENTRY           \
{                                       \
    SA_TILT_LINKAGE_CAL_ID,             \
    cal_mgr_callback,                   \
    &cal_sa_work,                       \
    &cal_sa_cnfg                        \
}

#define SA_TILT_INLINE_LINKAGE_ENTRY    \
{                                       \
    SA_TILT_INLINE_LINKAGE_CAL_ID,      \
    cal_mgr_callback,                   \
    &cal_sa_work,                       \
    &cal_sa_cnfg                        \
}

#define ADV_EMPTY_BUCKET_ENTRY          \
{                                       \
    ADV_EMPTY_BUCKET_CAL_ID,             \
    cal_mgr_callback,                   \
    &cal_sa_work,                       \
    &cal_sa_cnfg                        \
}

#define ADV_FULL_BUCKET_ENTRY           \
{                                       \
    ADV_FULL_BUCKET_CAL_ID,              \
    cal_mgr_callback,                   \
    &cal_sa_work,                       \
    &cal_sa_cnfg                        \
}

#define EH_ARRAY_ENTRIES(ARRAY_VARIABLE_NAME)                                \
                                         (  sizeof (ARRAY_VARIABLE_NAME) /   \
                                            sizeof (ARRAY_VARIABLE_NAME[0])  \
                                         )

#define CAL_MGR_APP_CAL_LIST SA_EMPTY_BUCKET_ENTRY,        \
                             SA_FULL_BUCKET_ENTRY,         \
                             SA_LIFT_LINKAGE_ENTRY,        \
                             SA_TILT_INLINE_LINKAGE_ENTRY, \
                             SA_TILT_LINKAGE_ENTRY,        \
                             ADV_EMPTY_BUCKET_ENTRY,       \
                             ADV_FULL_BUCKET_ENTRY

#define CAL_MGR_CALS    ( EH_ARRAY_ENTRIES( cal_mgr_cals ) )

#define CAL_MGR_SRVS   ( EH_ARRAY_ENTRIES( cal_mgr_srvs ) )

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/

#define CCM_CALS        4

/*Calibration Module Loop Time In Nano-Seconds*/
#define    CAL_NS_LPTIM        10000000

/*Calibration Module Loop Time In Seconds*/
#define    CAL_LPTIM          (CAL_NS_LPTIM/1000000000.0)

/* SA calibration configuration dummy structure */
typedef struct {
    char a;
} CAL_SACnfg_t;

/* SA calibration workspace structure */
typedef struct {
    void *ptr;
} CAL_SAWork_t;

#endif /* ACDCALMGRINF_H_ */

