/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaInit.cpp
DESCRIPTION:This file provides the initialization routines for the application software
            for Stadalone LPS system.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <thread>
#include <chrono>

#include <cpm_filter.h>

#include "LpsSaWeighApp.h"

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:LpsSaInit
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/

LpsSaInitErrorType_t LpsSaWeighApp::LpsSaInit()
{
    LpsInitErrorTypes_t lpsWeighRet;

    /* Loading NVM values for Start of weigh and End of Weigh library configuration */
    LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.StartOfWeigh = cnfg_.weighRangeStart;
    LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.EndOfWeigh = (cnfg_.weighRangeStart + cnfg_.weighRangeSize);

    // Load NVM values for configurable reweigh warnings.
    LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.ReweighMaxPitch = cnfg_.reweighMaxPitch;
    LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.ReweighMinPitch = cnfg_.reweighMinPitch;
    LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.ReweighMaxAbsRoll = cnfg_.reweighMaxAbsRoll;
    LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.ReweighMinLiftCylVel = cnfg_.reweighMinLiftCylVel;

    /* Initial value for hydraulic oil temp */
    WeighPidTbl.HydOilTemp = (float)HYDRAULIC_OIL_TEMP_MIN_VALID_DATA;  // which is -32736.0

    /* Initialize tipoff weight and status for weigh lib */
    weighUpdtTbl.TipoffWeight.Val = 0.f;
    weighUpdtTbl.TipoffWeight.Stat = LPS_TIPOFF_WEIGHT_INVALID;

    /* Initialize Lift Lever Info */
    LpsSaWeighInfoTbl.LiftLeverInfo.Faulted = true;  /* initialize to faulted */
    LpsSaWeighInfoTbl.LiftLeverInfo.InCenter = false;
    LpsSaWeighInfoTbl.LiftLeverInfo.InLowerDetent = false;
    LpsSaWeighInfoTbl.LiftLeverInfo.InRaiseDetent = false;
    LpsSaWeighInfoTbl.LiftLeverInfo.LeverInfoAvailable = false;
    LpsSaWeighInfoTbl.LiftLeverInfo.ValveCmdPercent = 0.0f;

    /* Initialize ground speed to 0 (default) */
    weighUpdtTbl.GroundSpeed = 0.f;

    /* Initialize gear status from datalink to BAD */
    weighUpdtTbl.RequestedGear.Stat = LPS_STATUS_BAD;

    /* Initialize battery voltage event to not active */
    weighUpdtTbl.VoltageEventActive = false;

    /* Copy the nvm values into weigh init table */
    copyCalNVMToWeighInitTable(LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.CalibTbl);

    AIS_LOG_INFO("MachSpecificCfg.CalibTbl.Calwt %f", LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.CalibTbl.CalWeight);
    AIS_LOG_INFO("MachSpecificCfg.CalibTbl.ZeroWeight %f", LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.CalibTbl.ZeroWeight);

    /* Initializing the LPS Weighing library */
    lpsWeighRet = LpsInit(&LpsSaInitTbl.WeighInitTbl);

    if (LPS_INIT_SUCCESS != lpsWeighRet) {
        AIS_LOG_WARN("LpsInit return code = %d", lpsWeighRet);
        return LPS_SA_INIT_DEPENDENT_LIBRARY_INIT_FAIL;
    }

    { // Calculate parameters derived from the payload calibration and used for TOA
        float mu, offset;
        if (TRUE == LpsWeighEstimateFrictionCoefficient(&mu, &offset)) {
            LpsSaWeighInfoTbl.TipoffInputs.friction_mu = mu;
            LpsSaWeighInfoTbl.TipoffInputs.friction_offset = offset;
        }
        else {
            LpsSaWeighInfoTbl.TipoffInputs.friction_mu = 0.f;
            LpsSaWeighInfoTbl.TipoffInputs.friction_offset = 0.f;
        }
    }

    // Initialize the filters
    cpm_filter_derivative_filter_init(&LiftCylVelLpFilt, LIFT_CYL_VEL_LP_FLT_KFF, LpsSaInitTbl.WeighInitTbl.ExecRate);

    cpm_filter_derivative_filter_init(&LiftAngVelLpFilt, LIFT_ANG_VEL_LP_FLT_KFF, LpsSaInitTbl.WeighInitTbl.ExecRate);

    cpm_filter_derivative_filter_init(&TiltCylVelLpFilt, TILT_CYL_VEL_LP_FLT_KFF, LpsSaInitTbl.WeighInitTbl.ExecRate);

    cpm_filter_low_pass_filter2_init(&LiftLinkageSensorDcLpFilt,
            LP_FLT_CF_TO_KFF(LIFT_CYL_DC_LP_FLT_CF, LpsSaInitTbl.WeighInitTbl.ExecRate));

    cpm_filter_low_pass_filter2_init(&TiltLinkageSensorDcLpFilt,
            LP_FLT_CF_TO_KFF(TILT_CYL_DC_LP_FLT_CF, LpsSaInitTbl.WeighInitTbl.ExecRate));

    LpsSaWeighInfoTbl.FilterSettleDelay = 0;

    if (GetCalWtRequired()) {
        WeighPidTbl.PloadSysCalWtEntryReqStat = CAL_ENTRY_REQUIRED;
    }
    else {
        WeighPidTbl.PloadSysCalWtEntryReqStat = CAL_ENTRY_NOT_REQUIRED;
    }

    AIS_LOG_INFO("CalStatus %x", payloadCalNvmTbl_.data.CalStatus);

    return LPS_SA_INIT_SUCCESS;
}

boolean LpsSaWeighApp::CalLibInit(void)
{
    LpsCalCfg_t calCfg = {}; /* zero initialize */

    calCfg.CalEnableFlg = nullptr;

    calCfg.CalSlowRaiseCmd = 40;
    calCfg.CalFastRaiseCmd = 100;
    calCfg.CalSlowLowerCmd = -30;
    calCfg.CalFastLowerCmd = -50;

    calCfg.KgPerKpaAtMidExtension = machineProperties.kgPerLiftKpaAtMidExtension; //0.925332325;//0.807422574;
    calCfg.LiftCylHeKpaWithoutBktAtMidExtn = machineProperties.liftKpaNoBucketMidExtension;//1394.712036;//1598.385;
    calCfg.ExecPeriodSec = LpsSaInitTbl.WeighInitTbl.ExecRate;

    calCfg.StageOneCf = LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.LlwTbl.StageOneCf;
    calCfg.StageTwoCf = LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.LlwTbl.StageTwoCf;
    calCfg.StageThreeCf = LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.LlwTbl.StageThreeCf;
    calCfg.DampingRatio = LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.LlwTbl.DampingRate;

    calCfg.LiftCylDiffLimits.RaiseCalPercDiffRangeLimit = 10;
    calCfg.LiftCylDiffLimits.RaiseCalPercDiffAbsLimit = 2.0;
    calCfg.LiftCylDiffLimits.LowerCalPercDiffRangeLimit = 1.0;
    calCfg.LiftCylDiffLimits.LowerCalPercDiffAbsLimit = 2.0;

    calCfg.QualReadEnableOrDisable = AppEnableDisableQualRead;
    calCfg.UpdateNvm = AppUpdtCalNvmTbl;

    if (LpsSaWeighInfoTbl.MachineModelNotSet) {
        calCfg.MachineModelCfg = LPS_STATUS_BAD;
    }

    /* Advanced Platform (AU2020 vs Standalone) */
    if (ADVANCED == getApplicationVariant()) {
        calCfg.EEFIMUCalEnable = true;
        calCfg.AutoCalsEnabled = true;
    }
    else {
        calCfg.EEFIMUCalEnable = false;
        calCfg.AutoCalsEnabled = false;
    }

    /* machine properties */
    calCfg.EEFIMUCalCfgData.internal_msn = machineProperties.internalMsn;
    calCfg.EEFIMUCalCfgData.lift_bore_diameter = machineProperties.liftBoreDiameter;
    calCfg.EEFIMUCalCfgData.lift_rod_diameter = machineProperties.liftRodDiameter;
    calCfg.EEFIMUCalCfgData.num_of_tilt_cylinders = machineProperties.numOfTiltCylinders;
    calCfg.EEFIMUCalCfgData.tilt_bore_diameter = machineProperties.tiltBoreDiameter;
    calCfg.EEFIMUCalCfgData.tilt_rod_diameter = machineProperties.tiltRodDiameter;
    calCfg.EEFIMUCalCfgData.tool_bc_length = machineProperties.toolBcLength;

    // Disable the cal libraries logging.
    LpsCalDisableLogging();

    LpsCalInitErrorTypes_t calRet = LpsCalInit(&calCfg, &payloadCalNvmTbl_.data);

    AIS_LOG_INFO("*************** LpsCalInit %d", calRet);

    if (LPS_CAL_INIT_SUCCESS != calRet) {
        return false;
    }
    else {
        return true;
    }
}

void AppEnableDisableQualRead(unsigned_8 Enable, signed_16* minvalue)
{
    return;
}

void AppUpdtCalNvmTbl(const LpsCalNvmTbl_t* pCalNvmTbl)
{
    temp_thisTask->UpdtCalNvmTbl(pCalNvmTbl);
    //thisTask.UpdtCalNvmTbl(pCalNvmTbl);
}

void LpsSaWeighApp::UpdtCalNvmTbl(const LpsCalNvmTbl_t* pCalNvmTbl)
{
    if (nullptr != pCalNvmTbl) {
        /* Updating NVM with the cal values*/
        payloadCalNvmTbl_.data = *pCalNvmTbl;

        // We only update the calibration counter when the calibration weight is entered and it becomes fully calibrated.
        // Calibration weight entry is required after an empty and full bucket calibration in order to complete the process.
        sealTracker_.reportPayloadCalStatus(GetPayloadCalStatus(), payloadCalNvmTbl_.calCounter);
        sealTracker_.reportCalibrationWeight(payloadCalNvmTbl_.data.CalWeight);
        sealTracker_.reportSpanAdjustFactor(payloadCalNvmTbl_.data.CalAdjust);

        payloadCalNvmTbl_.setSaveNeeded();

        /* Reset the simple cal update variables */
        sumOfAdjustedTruckWts = 0.f;
        sumOfZeroedTruckWts = 0.f;
        updatedSimpleCalFactor = pCalNvmTbl->CalAdjust;

        /*Enable Reinit flag*/
        CalNVMReinitFlag = true;
    }
}

/******************************************************************************
FUNCTION: NvmInitialize
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::NvmInitialize(void)
{
    /* Wait till OEL layer boot up*/
    while (FALSE == OelBootupFlag) {
        static constexpr auto sleepTime = std::chrono::milliseconds(250);
        std::this_thread::sleep_for(sleepTime);
    }

    /* Wait till NVM is read */
    while ((FALSE == LpsSaNvmWeighAppMachSpecificCfgReadFlag) ||
            (FALSE == LpsSaNvmWeighAppCalReadFlag) ||
            (FALSE == LpsSaTiltNvmWeighAppCalReadFlag) ||
            (FALSE == LpsSaLiftNvmWeighAppCalReadFlag)) {
        static constexpr auto sleepTime = std::chrono::milliseconds(250);
        std::this_thread::sleep_for(sleepTime);
        AIS_LOG_ERROR("%d, %d, %d, %d", LpsSaNvmWeighAppMachSpecificCfgReadFlag, LpsSaNvmWeighAppCalReadFlag, LpsSaTiltNvmWeighAppCalReadFlag, LpsSaLiftNvmWeighAppCalReadFlag);
    }
}
