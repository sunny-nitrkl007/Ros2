/*******************************************************************************
 Copyright 2017 Caterpillar Inc. All rights reserved.
--------------------------------------------------------------------------------
File name: app_nvm_file_access.h

Description:This file contains test code to test the nvm_file feature 
*******************************************************************************/
#ifndef __APP_NVM_FILE_ACCESS_H__
#define __APP_NVM_FILE_ACCESS_H__

#ifndef STD_TYPES_H_
#include <std_types.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif
#include <cal_mgr.h>

#include <LpsCommonTypeDef.h>
#include <LpsCalAppPublic.h>

#include "app_nvm_globals.h"

// DEPRACATED - DO NOT EVER CHANGE
typedef struct {
    float  _WeighRangeBottom; /* weigh range Bottom in % */
    float  _WeighRangeSize;   /* weigh range Size in %   */
    unsigned short int _PloadOvrloadWarnEn;
    float  _CalWt;
    float  _activeSimpleCalFactor;
    float  _updatedSimpleCalFactor;
    float _LastPloadWt;
    float _LoaderBktPloadTrgtWt;
} LpsSaWeighNvmMachSpecificCfg_t;

// DEPRACATED - DO NOT EVER CHANGE
typedef struct {
    unsigned_16 _CalNvmBlkVerNo;
    unsigned_16 _CalStat; //steps_complete renamed to CalStat.
    float_32 _SlowRaiseEmptyBktLiftHt[11];
    float_32 _SlowRaiseEmptyBktLiftPres[11];
    float_32 _SlowRaiseFullBktLiftHt[11];
    float_32 _SlowRaiseFullBktLiftPres[11];
    float_32 _SlowLowerEmptyBktLiftHt[11];
    float_32 _SlowLowerEmptyBktLiftPres[11];
    float_32 _SlowLowerFullBktLiftHt[11];
    float_32 _SlowLowerFullBktLiftPres[11];
    float_32 _FastRaiseEmptyBktDeltaPres;
    float_32 _FastRaiseFullBktDeltaPres;
    float_32 _EmptyBktSlowRaiseSpd;
    float_32 _EmptyBktFastRaiseSpd;
    float_32 _FullBktSlowRaiseSpd;
    float_32 _FullBktFastRaiseSpd;
    float_32 _Calwt;
    unsigned_32 _ShmEmptyMain; //use these vars.
    unsigned_32 _ShmFullMain;
    float_32 _EmptyTemp;
    float_32 _FullTemp;
    float_32 _EmptyBucketWeightEst;
    unsigned_32 unused_LifeTimeTotalPassCount;
    float_32 unused_LifeTimeTotalPayloadWeight;
    float_32 _CalAdjust;
    float_32 _ZeroWeight;
    float_32 unused_StartOfWeigh;
    float_32 unused_EndOfWeigh;
    float_32 unused_LoaderBktPayldTrgtWt;
    unsigned_32 _Reserved[42];
} LpsSaCalNvmTbl_t;

// DEPRACATED - DO NOT EVER CHANGE
typedef struct {
    float _tilt_full_rack_dc;
    float _tilt_full_dump_dc;
    unsigned short _tilt_cal_stat; //steps_complete renamed to CalStat.
} LpsSaTiltCalNvmTbl_t;

// DEPRACATED - DO NOT EVER CHANGE
typedef struct {
    float _lift_full_raise_dc;
    float _lift_full_lower_dc;
    unsigned short _lift_cal_stat; //steps_complete renamed to CalStat.
} LpsSaLiftCalNvmTbl_t;

extern volatile boolean OelBootupFlag;

extern volatile boolean LpsSaNvmWeighAppMachSpecificCfgReadFlag;
extern boolean NvmWeighingAppMachSpecificCfgFileExistsFlag;

extern volatile boolean LpsSaNvmWeighAppCalReadFlag;
extern boolean NvmWeighingAppCalFileExistsFlag;

extern volatile boolean LpsSaTiltNvmWeighAppCalReadFlag;
extern boolean NvmWeighingAppTiltCalFileExistsFlag;

extern volatile boolean LpsSaLiftNvmWeighAppCalReadFlag;
extern boolean NvmWeighingAppLiftCalFileExistsFlag;

/* DEPRACATED: Holds actual values stored in NVM for machine specific configuration */
extern volatile LpsSaWeighNvmMachSpecificCfg_t LpsSaWeighNvmMachSpecificCfg;

/* DEPRACATED: Holds actual values stored in NVM for cal values */
extern volatile LpsSaCalNvmTbl_t LpsSaWeighNvmCal;

/* DEPRECATED: Holds actual values stored in NVM for Lift cal values */
extern volatile LpsSaLiftCalNvmTbl_t LpsSaWeighLiftNvmCal;

/* DEPRECATED: Holds actual values stored in NVM for Tilt cal values */
extern volatile LpsSaTiltCalNvmTbl_t LpsSaWeighTiltNvmCal;

boolean app_nvm_file_remove(LpsWeighNvmId_t nvm_id);

#ifdef __cplusplus
}
#endif

#endif
