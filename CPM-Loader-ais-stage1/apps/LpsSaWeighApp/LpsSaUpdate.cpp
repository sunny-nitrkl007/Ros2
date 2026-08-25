/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaUpdate.cpp
DESCRIPTION:This file provides the update routines for the application software
            for LPS library.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <mutex>

#ifndef  _LPS_SA_WEIGHAPP_H_
#include "LpsSaWeighApp.h"
#endif

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:LpsSaUpdt
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
LpsSaUpdtErrorType_t  LpsSaWeighApp::LpsSaUpdt(void)
{
    LpsUpdtErrorTypes_t   lpsWeighRet;

    if (CalNVMReinitFlag) {
        LpsCalReinitNvmData(&payloadCalNvmTbl_.data);
        ReInitWeighLib();
        CalNVMReinitFlag = false;
    }

    lpsWeighRet = LpsSaUpdateWeighingLib();

    // Get payload ratio
    LpsSaWeighInfoTbl.Payload.payloadRatioStatus = LpsGetPayloadRatio(
            &LpsSaWeighInfoTbl.Payload.payloadRatioRaw,
            &LpsSaWeighInfoTbl.Payload.payloadRatio);

    // Collecting Best available bucket weight and payload calculation method output from weighing library
    LpsWeighBestBktWt_t lpsWeighBestBktWt = LpsWeighGetBestAvailableBktWt();

    // Populating SCS interfaces for Best available bucket weight and payload calculation method
    LpsSaWeighInfoTbl.BestBktWtInTonnes = lpsWeighBestBktWt.Wt;
    LpsSaWeighInfoTbl.PayloadCalcMeth = lpsWeighBestBktWt.PayloadCalcMeth;
    LpsSaWeighInfoTbl.BestBktWtLatched = lpsWeighBestBktWt.Latched;
    LpsSaWeighInfoTbl.LatchConditionsOK = lpsWeighBestBktWt.LatchConditionsOK;

    // Get Dig State from Weigh Library
    LpsSaWeighInfoTbl.DigStat  = LpsWeighGetDigState();

    // Get Dump State from Weigh Library
    LpsSaWeighInfoTbl.DumpStat = LpsWeighGetDumpState();

    // Collecting Weigh Range Indicator state from Weigh Library
    LpsSaWeighInfoTbl.Indicator = LpsWeighGetWeighRangeIndicator();

    UpdateEvents();

    if (LPS_UPDT_SUCCESS == lpsWeighRet) {
        return LPS_SA_UPDT_SUCCESS;
    }
    else {
        return LPS_SA_UPDT_DEPENDENT_LIBRARY_UPDT_FAIL;
    }
}

void LpsSaWeighApp::setPayloadNotCalibrated()
{
    auto oldStat = payloadCalNvmTbl_.data.CalStatus;

    payloadCalNvmTbl_.data.CalStatus &= ~(CAL_EMPTY_BKT_CURVE_MASK | CAL_FULL_BKT_CURVE_MASK | CAL_BKT_WT_MASK | CAL_VELCAL);
    sealTracker_.reportPayloadCalStatus(false, payloadCalNvmTbl_.calCounter);

    if (oldStat != payloadCalNvmTbl_.data.CalStatus) {
        payloadCalNvmTbl_.setSaveNeeded();
        CalNVMReinitFlag = true;
    }

}

void LpsSaWeighApp::setLiftLinkageSensorCalibrated(float fullLowerDutyCycle, float fullRaiseDutyCycle)
{
    // Write the new values to storage
    liftCalNvmTbl_.lift_full_lower_dc = fullLowerDutyCycle;
    liftCalNvmTbl_.lift_full_raise_dc = fullRaiseDutyCycle;

    liftCalNvmTbl_.lift_cal_stat = CAL_LIFT_LINKAGE_MASK;
    ++liftCalNvmTbl_.lift_cal_counter;
    sealTracker_.reportLiftPositionSensorCalStatus(true, liftCalNvmTbl_.lift_cal_counter);

    liftCalNvmTbl_.setSaveNeeded();

    // reset the datalink flags
    liftCalNvmTbl_.lift_full_lower_dc_recd = false;
    liftCalNvmTbl_.lift_full_raise_dc_recd = false;

    // This triggers everything to use the new values
    CalNVMReinitFlag = true;
}

bool LpsSaWeighApp::setLiftLinkageSensorFullLowerCalValue(float fullLowerDutyCycle)
{
    bool newCalibration = false;

    if (liftCalNvmTbl_.lift_full_raise_dc_recd) {
        // If the other one already changed, then just save the calibration.
        liftCalNvmTbl_.lift_full_lower_dc_recd = true;
        liftCalNvmTbl_.lift_full_lower_dc = fullLowerDutyCycle;

        // If the calibration counter is 0, this is just the initial value, not a new calibration
        if (0 != liftCalNvmTbl_.lift_cal_counter) {
            newCalibration = true;
            AIS_LOG_ALERT("New cal values received. Lift Cal Counter: %d", liftCalNvmTbl_.lift_cal_counter);
        }

        setLiftLinkageSensorCalibrated(liftCalNvmTbl_.lift_full_lower_dc, liftCalNvmTbl_.lift_full_raise_dc);
    }
    else if ((liftCalNvmTbl_.lift_full_lower_dc != fullLowerDutyCycle) ||
            (0 == (liftCalNvmTbl_.lift_cal_stat & CAL_LIFT_LINKAGE_MASK))) {
        // If this one changed, then mark it.
        AIS_LOG_ALERT("New lower calibration values detected old = %f, new = %f", liftCalNvmTbl_.lift_full_lower_dc, fullLowerDutyCycle);
        liftCalNvmTbl_.lift_full_lower_dc_recd = true;
        liftCalNvmTbl_.lift_full_lower_dc = fullLowerDutyCycle;
    }

    return newCalibration;
}

bool LpsSaWeighApp::setLiftLinkageSensorFullRaiseCalValue(float fullRaiseDutyCycle)
{
    bool newCalibration = false;

    if (liftCalNvmTbl_.lift_full_lower_dc_recd) {
        // If the other one already changed, then just save the calibration.
        liftCalNvmTbl_.lift_full_raise_dc_recd = true;
        liftCalNvmTbl_.lift_full_raise_dc = fullRaiseDutyCycle;

        // If the calibration counter is 0, this is just the initial value, not a new calibration
        if (0 != liftCalNvmTbl_.lift_cal_counter) {
            newCalibration = true;
            AIS_LOG_ALERT("New cal values received. Lift Cal Counter: %d", liftCalNvmTbl_.lift_cal_counter);
        }

        setLiftLinkageSensorCalibrated(liftCalNvmTbl_.lift_full_lower_dc, liftCalNvmTbl_.lift_full_raise_dc);
    }
    else if ((liftCalNvmTbl_.lift_full_raise_dc != fullRaiseDutyCycle) ||
            (0 == (liftCalNvmTbl_.lift_cal_stat & CAL_LIFT_LINKAGE_MASK))) {
        // If this one changed, then mark it.
        AIS_LOG_ALERT("New raise calibration values detected old = %f, new = %f", liftCalNvmTbl_.lift_full_raise_dc, fullRaiseDutyCycle);
        liftCalNvmTbl_.lift_full_raise_dc_recd = true;
        liftCalNvmTbl_.lift_full_raise_dc = fullRaiseDutyCycle;
    }

    return newCalibration;
}

bool LpsSaWeighApp::setTiltLinkageSensorFullDumpCalValue(float fullDumpDutyCycle)
{
    bool newCalibration = false;

    if (tiltCalNvmTbl_.tilt_full_rack_dc_recd) {
        // If the other one already changed, then just save the calibration.
        tiltCalNvmTbl_.tilt_full_dump_dc_recd = true;
        tiltCalNvmTbl_.tilt_full_dump_dc = fullDumpDutyCycle;

        // If the calibration counter is 0, this is just the initial value, not a new calibration
        if (0 != tiltCalNvmTbl_.tilt_cal_counter) {
            newCalibration = true;
            AIS_LOG_ALERT("New cal values received. Tilt Cal Counter: %d", tiltCalNvmTbl_.tilt_cal_counter);
        }

        setTiltLinkageSensorCalibrated(tiltCalNvmTbl_.tilt_full_dump_dc, tiltCalNvmTbl_.tilt_full_rack_dc);
    }
    else if ((tiltCalNvmTbl_.tilt_full_dump_dc != fullDumpDutyCycle) ||
            (0 == (tiltCalNvmTbl_.tilt_cal_stat & CAL_TILT_LINKAGE_MASK))) {
        // If this one changed, then mark it.
        AIS_LOG_ALERT("New dump calibration values detected old = %f, new = %f", tiltCalNvmTbl_.tilt_full_dump_dc, fullDumpDutyCycle);
        tiltCalNvmTbl_.tilt_full_dump_dc_recd = true;
        tiltCalNvmTbl_.tilt_full_dump_dc = fullDumpDutyCycle;
    }

    return newCalibration;
}

bool LpsSaWeighApp::setTiltLinkageSensorFullRackCalValue(float fullRackDutyCycle)
{
    bool newCalibration = false;

    if (tiltCalNvmTbl_.tilt_full_dump_dc_recd) {
        // If the other one already changed, then just save the calibration.
        tiltCalNvmTbl_.tilt_full_rack_dc_recd = true;
        tiltCalNvmTbl_.tilt_full_rack_dc = fullRackDutyCycle;

        // If the calibration counter is 0, this is just the initial value, not a new calibration
        if (0 != tiltCalNvmTbl_.tilt_cal_counter) {
            newCalibration = true;
            AIS_LOG_ALERT("New cal values received. Tilt Cal Counter: %d", tiltCalNvmTbl_.tilt_cal_counter);
        }

        setTiltLinkageSensorCalibrated(tiltCalNvmTbl_.tilt_full_dump_dc, tiltCalNvmTbl_.tilt_full_rack_dc);
    }
    else if ((tiltCalNvmTbl_.tilt_full_rack_dc != fullRackDutyCycle) ||
            (0 == (tiltCalNvmTbl_.tilt_cal_stat & CAL_TILT_LINKAGE_MASK))) {
        // If this one changed, then mark it.
        AIS_LOG_ALERT("New rack calibration values detected old = %f, new = %f", tiltCalNvmTbl_.tilt_full_rack_dc, fullRackDutyCycle);
        tiltCalNvmTbl_.tilt_full_rack_dc_recd = true;
        tiltCalNvmTbl_.tilt_full_rack_dc = fullRackDutyCycle;
    }

    return newCalibration;
}

void LpsSaWeighApp::setLiftLinkageSensorNotCalibrated()
{
    // Write the new status to storage
    liftCalNvmTbl_.lift_cal_stat = 0;
    sealTracker_.reportLiftPositionSensorCalStatus(false, liftCalNvmTbl_.lift_cal_counter);

    liftCalNvmTbl_.setSaveNeeded();

    // This triggeres everything to use the new status
    CalNVMReinitFlag = true;
}

void LpsSaWeighApp::setTiltLinkageSensorCalibrated(float fullDumpDutyCycle, float fullRackDutyCycle)
{
    // Write the new values to storage
    tiltCalNvmTbl_.tilt_sensor_type = linkage_table_cnfg.tiltSensorType;
    tiltCalNvmTbl_.tilt_full_dump_dc = fullDumpDutyCycle;
    tiltCalNvmTbl_.tilt_full_rack_dc = fullRackDutyCycle;
    tiltCalNvmTbl_.tilt_full_dump_stop_angle = linkage_table_cnfg.KnmaticsTiltAngleMinMax[0];
    tiltCalNvmTbl_.tilt_full_rack_stop_angle = linkage_table_cnfg.KnmaticsTiltAngleMinMax[1];

    tiltCalNvmTbl_.tilt_cal_stat = CAL_TILT_LINKAGE_MASK;
    ++tiltCalNvmTbl_.tilt_cal_counter;
    sealTracker_.reportTiltPositionSensorCalStatus(true, tiltCalNvmTbl_.tilt_cal_counter);

    tiltCalNvmTbl_.setSaveNeeded();

    /* Reset the dl received flags */
    tiltCalNvmTbl_.tilt_full_rack_dc_recd = false;
    tiltCalNvmTbl_.tilt_full_dump_dc_recd = false;

    // This triggers everything to use the new values
    CalNVMReinitFlag = true;
}

void LpsSaWeighApp::setTiltLinkageSensorCalAngleLimits(float fullDumpAngle, float fullRackAngle)
{
    // Write the new values to storage
    tiltCalNvmTbl_.tilt_full_dump_stop_angle = fullDumpAngle;
    tiltCalNvmTbl_.tilt_full_rack_stop_angle = fullRackAngle;
    tiltCalNvmTbl_.setSaveNeeded();

    // This triggers everything to use the new values
    CalNVMReinitFlag = true;
}

void LpsSaWeighApp::setTiltLinkageSensorNotCalibrated()
{
    tiltCalNvmTbl_.tilt_cal_stat = 0;
    sealTracker_.reportTiltPositionSensorCalStatus(false, tiltCalNvmTbl_.tilt_cal_counter);

    tiltCalNvmTbl_.setSaveNeeded();

    // This triggeres everything to use the new status
    CalNVMReinitFlag = true;
}

void LpsSaWeighApp::ReInitWeighLib(void)
{
    LpsSaInitErrorType_t initRet = LpsSaInit();
    if (LPS_SA_INIT_SUCCESS != initRet) {
        AIS_LOG_ERROR("Weigh Library Reinit Failed = %d", initRet);
    }
    else {
        AIS_LOG_INFO("Weigh Library Reinit Success.");
    }
}

void LpsSaWeighApp::CalLibUpdt()
{
    LpsCalUpdate_t  calUpdtTbl = {};
    LpsCalUpdtErrorTypes_t calRet;

    /*
     * Here we need to get the pressure that is already
     * compensated for temp and natural linkage accel
     */
    if ((LPS_STATUS_OK == weighUpdtTbl.LiftCylHePres.Stat) &&
            (LPS_STATUS_OK == weighUpdtTbl.LiftCylRePres.Stat)) {
        calUpdtTbl.LiftDiffPress.Stat = LPS_STATUS_OK;
    }
    else {
        calUpdtTbl.LiftDiffPress.Stat = LPS_STATUS_BAD;
    }
    calUpdtTbl.LiftDiffPress.Val = LpsGetLiftCylPressureV0();

    calUpdtTbl.LiftCylLengthNorm = weighUpdtTbl.LiftCylLengthNorm;

    calUpdtTbl.LiftCylVel = weighUpdtTbl.LiftCylVel;

    calUpdtTbl.HydOilTemp = weighUpdtTbl.HydOilTemp; //test data 0x45

    calUpdtTbl.TiltCylLengthNorm = weighUpdtTbl.TiltCylLengthNorm;

    calUpdtTbl.TiltAngleABCPercent = weighUpdtTbl.TiltAngleABCPercent;

    calUpdtTbl.BktAngle = weighUpdtTbl.BktAngle;

    // What is this!!!!!
    // TODO: Does last calibration status even work?
    calUpdtTbl.ServiceHourMeter = 0;

    calUpdtTbl.DumpState = (LpsWeighBktDumpStat_t)LpsSaWeighInfoTbl.DumpStat;

    /* engine speed */
    if (IS_KNOWN2S(LpsSaWeighInfoTbl.EngineSpeedRPM)) {
        calUpdtTbl.EngineSpeed.Stat = LPS_STATUS_OK;
        calUpdtTbl.EngineSpeed.Val = LpsSaWeighInfoTbl.EngineSpeedRPM;
    }
    else {
        /* bad */
        calUpdtTbl.EngineSpeed.Stat = LPS_STATUS_BAD;
        calUpdtTbl.EngineSpeed.Val = 0;
    }

    if ((ADVANCED == getApplicationVariant()) || (payloadCalNvmTbl_.legalForTradeInstalled)) {
        /*
         * Reference IMU is required for Legal For Trade
         * It is also available and diagnosed for the ADVANCED product, so we calibrate with it
         * in case they calibrate prior to installing LFT.
         */
        { // Linear Acceleration Vector
            auto v = chassisImu_.imu.calibratedLinAccelVector();
            calUpdtTbl.ReferenceIMUData.LinAcc[0] = v.x();
            calUpdtTbl.ReferenceIMUData.LinAcc[1] = v.y();
            calUpdtTbl.ReferenceIMUData.LinAcc[2] = v.z();
        }

        { // Angular Velocity Vector
            auto v = chassisImu_.imu.calibratedAngVelVector();
            calUpdtTbl.ReferenceIMUData.AngVel[0] = v.x();
            calUpdtTbl.ReferenceIMUData.AngVel[1] = v.y();
            calUpdtTbl.ReferenceIMUData.AngVel[2] = v.z();
        }

        calUpdtTbl.ReferenceIMUData.Pitch = chassisImu_.imu.pitchDegrees();
        calUpdtTbl.ReferenceIMUData.Roll = chassisImu_.imu.rollDegrees();
        calUpdtTbl.ReferenceIMUData.Stat = chassisImu_.imuOk ? LPS_STATUS_OK : LPS_STATUS_BAD;
    }
    else {
        /*
         * Reference IMU is not used unless Legal For Trade
         * We cannot provide the IMU for STANDALONE calibration when LFT is not installed
         * because we do not require or diagnose the IMU in that configuration.
         * Since we don't diagnose it, we cannot use it.
         */
        calUpdtTbl.ReferenceIMUData.LinAcc[0] = 0.f;
        calUpdtTbl.ReferenceIMUData.LinAcc[1] = 0.f;
        calUpdtTbl.ReferenceIMUData.LinAcc[2] = 0.f;
        calUpdtTbl.ReferenceIMUData.AngVel[0] = 0.f;
        calUpdtTbl.ReferenceIMUData.AngVel[1] = 0.f;
        calUpdtTbl.ReferenceIMUData.AngVel[2] = 0.f;
        calUpdtTbl.ReferenceIMUData.Pitch = 0.f;
        calUpdtTbl.ReferenceIMUData.Roll = 0.f;
        calUpdtTbl.ReferenceIMUData.Stat = LPS_STATUS_OK;
    }

    /* set rest of IMU Cal Data */
    calUpdtTbl.EEFIMUData.LinAcc[0] = LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelX;
    calUpdtTbl.EEFIMUData.LinAcc[1] = LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelY;
    calUpdtTbl.EEFIMUData.LinAcc[2] = LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelZ;
    calUpdtTbl.EEFIMUData.Stat = LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.imu_accel_bad ? LPS_STATUS_BAD : LPS_STATUS_OK;

    calUpdtTbl.LiftAngle = LpsSaWeighInfoTbl.TipoffInputs.lift_angle;
    calUpdtTbl.TiltExtension = LpsSaWeighInfoTbl.TipoffInputs.tilt_extension;

    calUpdtTbl.TiltHePressure.Val = LpsSaWeighInfoTbl.TipoffInputs.tilt_he_pressure;
    calUpdtTbl.TiltHePressure.Stat = static_cast<LpsStat_t>(
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_he_press_bad);

    calUpdtTbl.TiltRePressure.Val = LpsSaWeighInfoTbl.TipoffInputs.tilt_re_pressure;
    calUpdtTbl.TiltRePressure.Stat = static_cast<LpsStat_t>(
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_re_press_bad);

    calUpdtTbl.SteeringAngle = LpsSaWeighInfoTbl.TipoffInputs.steering_angle;

    /* Update lift lever info */
    calUpdtTbl.LiftLeverInfo = LpsSaWeighInfoTbl.LiftLeverInfo;

    calUpdtTbl.TiltValveCmdPercent = LpsSaWeighInfoTbl.TiltValveCommand.Val;

    /* Update cal override acknowledge */
    calUpdtTbl.CalOvrAcknowledge = LpsSaWeighInfoTbl.CalOvrAcknowledge;

    /* Battery voltage diagnostic */
    calUpdtTbl.VoltageEventActive = weighUpdtTbl.VoltageEventActive;

    /* Legal for Trade Seal Status */
    calUpdtTbl.LegalForTradeSealed = weighUpdtTbl.LegalForTradeSealed;

    { // Update cal library
        std::lock_guard<std::mutex> lck(calLibMtx_);
        calRet = LpsCalUpdate(&calUpdtTbl);
    }

    if (LPS_CAL_UPDT_SUCCESS != calRet) {
        AIS_LOG_ERROR("LpsCalUpdate returned %d.", calRet);
    }
}

void LpsSaWeighApp::copyCalNVMToWeighInitTable(LpsCalNvmTbl_t& to) {

    // Copy calibration data
    to = payloadCalNvmTbl_.data;

    // The weigh app uses a different definition of cal status.
    // THIS IS MESSED UP!
    if (GetPayloadMonSysCalStatus()) {
        to.CalStatus = LPS_WEIGH_SYSTEM_CALIBRATED;
    }
    else {
        to.CalStatus = LPS_WEIGH_SYSTEM_UNCALIBRATED;
    }
}
