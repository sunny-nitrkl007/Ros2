/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaCalibration.cpp
DESCRIPTION:This file provides the update routines for the application software
            for LPS library.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef  _LPS_SA_WEIGHAPP_H_
#include "LpsSaWeighApp.h"
#endif

#include <iostream>
#include <algorithm>

/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/

float_32  LpsWeighPtrDumpWtBuffer[WEIGH_LIB_DUMP_BUFF_SIZE];

unsigned_32 cal_iterm[(CAL_LAST_ITERM_BIT>>5)+1];   /* Default Cal Iterm */

/******************************************************************************
FUNCTION NAME: LpsSaLoadCalibrationTbl
DESCRIPTION: This function reads the calibration table from the ruby file
PARAMETER DESCRIPTION: none
RETURN VALUE: none
*******************************************************************************/
void LpsSaWeighApp::LpsSaLoadDefaultCalibrationTbl(void)
{
    /*
     * Load the Default Calibration Tables
     *  - Before this, payloadCalNvmTbl_ has been default initialized.
     */
    payloadCalNvmTbl_.data.CalStatus = 0;

    {
        ConfigSection cfg;
        if (!getTaskParser().getSection("DefaultCalConfig", cfg)) {
            AIS_LOG_ERROR("DefaultCalConfig section not found");
        }
        else {
            bool calibratedByDefault = false;
            { // Read cal status
                int temp;
                if (cfg.get("CalStat", temp)) {
                    auto calStat = static_cast<LpsWeighCalStatus_t>(temp);
                    if (LPS_WEIGH_SYSTEM_CALIBRATED == calStat) {
                        calibratedByDefault = true;
                    }
                }
            }

            { // Load lift sensor default calibration
                bool allThere = true;
                allThere &= !cfg.get("LiftCylMaxDc", liftCalNvmTbl_.lift_full_raise_dc);
                allThere &= !cfg.get("LiftCylMinDc", liftCalNvmTbl_.lift_full_lower_dc);
                if (allThere && calibratedByDefault) {
                    liftCalNvmTbl_.lift_cal_stat = CAL_LIFT_LINKAGE_MASK;
                }
            }

            if (TILT_SENSOR_TYPE_ROTARY == linkage_table_cnfg.tiltSensorType)
            { // Load tilt sensor default calibration
                bool allThere = true;
                allThere &= !cfg.get("TiltCylMaxDc", tiltCalNvmTbl_.tilt_full_rack_dc);
                allThere &= !cfg.get("TiltCylMinDc", tiltCalNvmTbl_.tilt_full_dump_dc);
                if (allThere && calibratedByDefault) {
                    tiltCalNvmTbl_.tilt_sensor_type = TILT_SENSOR_TYPE_ROTARY;
                    tiltCalNvmTbl_.tilt_cal_stat = CAL_TILT_LINKAGE_MASK;
                }
            }

            { // Load empty bucket payload calibration tables
                bool allThere = true;
                vector<float> tempArr;

                if (cfg.get("SlowRaiseEmptyBktLiftHt", tempArr)) {
                    tempArr.resize(LPS_CAL_CURVE_FIT_NUM_POINTS);
                    std::copy(tempArr.begin(), tempArr.end(), payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt);
                }
                else {
                    allThere = false;
                }

                tempArr.clear();
                if (cfg.get("SlowRaiseEmptyBktLiftPres", tempArr)) {
                    tempArr.resize(LPS_CAL_CURVE_FIT_NUM_POINTS);
                    std::copy(tempArr.begin(), tempArr.end(), payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres);
                }
                else {
                    allThere = false;
                }

                tempArr.clear();
                if (cfg.get("SlowLowerEmptyBktLiftHt", tempArr)) {
                    tempArr.resize(LPS_CAL_CURVE_FIT_NUM_POINTS);
                    std::copy(tempArr.begin(), tempArr.end(), payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt);
                }
                else {
                    allThere = false;
                }

                tempArr.clear();
                if (cfg.get("SlowLowerEmptyBktLiftPres", tempArr)) {
                    tempArr.resize(LPS_CAL_CURVE_FIT_NUM_POINTS);
                    std::copy(tempArr.begin(), tempArr.end(), payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres);
                }
                else {
                    allThere = false;
                }

                allThere &= !cfg.get("FastRaiseEmptyBktDeltaPres", payloadCalNvmTbl_.data.FastRaiseEmptyBktDeltaPres);
                allThere &= !cfg.get("EmptyBktSlowRaiseSpd", payloadCalNvmTbl_.data.EmptyBktSlowRaiseSpd);
                allThere &= !cfg.get("EmptyBktFastRaiseSpd", payloadCalNvmTbl_.data.EmptyBktFastRaiseSpd);

                if (allThere && calibratedByDefault) {
                    payloadCalNvmTbl_.data.CalStatus |= CAL_EMPTY_BKT_CURVE_MASK;
                }
            }

            { // Load full bucket payload calibration tables
                bool allThere = true;
                vector<float> tempArr;

                if (cfg.get("SlowRaiseFullBktLiftHt", tempArr)) {
                    tempArr.resize(LPS_CAL_CURVE_FIT_NUM_POINTS);
                    std::copy(tempArr.begin(), tempArr.end(), payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt);
                }
                else {
                    allThere = false;
                }

                tempArr.clear();
                if (cfg.get("SlowRaiseFullBktLiftPres", tempArr)) {
                    tempArr.resize(LPS_CAL_CURVE_FIT_NUM_POINTS);
                    std::copy(tempArr.begin(), tempArr.end(), payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres);
                }
                else {
                    allThere = false;
                }

                tempArr.clear();
                if (cfg.get("SlowLowerFullBktLiftHt", tempArr)) {
                    tempArr.resize(LPS_CAL_CURVE_FIT_NUM_POINTS);
                    std::copy(tempArr.begin(), tempArr.end(), payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt);
                }
                else {
                    allThere = false;
                }

                tempArr.clear();
                if (cfg.get("SlowLowerFullBktLiftPres", tempArr)) {
                    tempArr.resize(LPS_CAL_CURVE_FIT_NUM_POINTS);
                    std::copy(tempArr.begin(), tempArr.end(), payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres);
                }
                else {
                    allThere = false;
                }

                allThere &= !cfg.get("FastRaiseFullBktDeltaPres", payloadCalNvmTbl_.data.FastRaiseFullBktDeltaPres);
                allThere &= !cfg.get("FullBktSlowRaiseSpd", payloadCalNvmTbl_.data.FullBktSlowRaiseSpd);
                allThere &= !cfg.get("FullBktFastRaiseSpd", payloadCalNvmTbl_.data.FullBktFastRaiseSpd);

                if (allThere && calibratedByDefault) {
                    payloadCalNvmTbl_.data.CalStatus |= CAL_FULL_BKT_CURVE_MASK;
                }
            }

            // Load default calibration weight.
            if (cfg.get("Calwt", payloadCalNvmTbl_.data.CalWeight)) {
                if (calibratedByDefault) {
                    payloadCalNvmTbl_.data.CalStatus |= CAL_BKT_WT_MASK;
                }
            }
        }

        // If empty and full curves are done, then velocity compensation is done.
        if ((0 != (payloadCalNvmTbl_.data.CalStatus & CAL_EMPTY_BKT_CURVE_MASK)) &&
                (0 != (payloadCalNvmTbl_.data.CalStatus & CAL_FULL_BKT_CURVE_MASK))) {
            payloadCalNvmTbl_.data.CalStatus |= CAL_VELCAL;
        }
    }

    AIS_LOG_INFO("Default calibration status %X", payloadCalNvmTbl_.data.CalStatus);

    // Simple Cal Factor is always defaulted to 0
    payloadCalNvmTbl_.data.CalAdjust = 0.f;

    // Zero is always defaulted to 0
    payloadCalNvmTbl_.data.ZeroWeight = 0.f;
}
/******************************************************************************
FUNCTION NAME:LpsSaInitWeighTbl
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
bool LpsSaWeighApp::LpsSaInitWeighTbl()
{
    bool everythingOk = true;
    LpsInitTbl_t& weighInitTbl = LpsSaInitTbl.WeighInitTbl;
    LpsMachSpecificCfg_t& machSpecificCfg = weighInitTbl.MachSpecificCfg;

    // Weighing library set to 20 msec execution rate
    getTaskConfig().get("CPMExecRate", weighInitTbl.ExecRate);
    AIS_LOG_INFO("CPMExecRate %f", weighInitTbl.ExecRate);

    // Load the default calibration table from the ruby file
    LpsSaLoadDefaultCalibrationTbl();

    // Copying default calibration table into the weigh init table.
    copyCalNVMToWeighInitTable(machSpecificCfg.CalibTbl);

    { // Read machine spec config
        ConfigSection rubyCfg;
        if (!getTaskParser().getSection("MachineSpecificConfig",rubyCfg)) {
            AIS_LOG_ERROR("MachineSpecificConfig section not found");
            everythingOk = false;
        }
        else {
            { // Get hydraulic oil type
                int temp;
                if (rubyCfg.get("HydOilType", temp)) {
                    machSpecificCfg.HydOilType = static_cast<LpsHydOilType_t>(temp);
                }
                else {
                    machSpecificCfg.HydOilType = LPS_OIL_TYPE_SAE_10W;
                }
            }

            /*Note :-
               Temperature compensation coefficients are defaulted to zeros to provide no
               hydraulic oil temperature compensation
            */
            rubyCfg.get("LiftCylHeLineLoss2ndOrdrCoeff", machSpecificCfg.HydPressLossCoeff.LiftCylHeLineLoss2ndOrdrCoeff);
            rubyCfg.get("LiftCylHeLineLoss1stOrdrCoeff", machSpecificCfg.HydPressLossCoeff.LiftCylHeLineLoss1stOrdrCoeff);
            rubyCfg.get("LiftCylReLineLoss2ndOrdrCoeff", machSpecificCfg.HydPressLossCoeff.LiftCylReLineLoss2ndOrdrCoeff);
            rubyCfg.get("LiftCylReLineLoss1stOrdrCoeff", machSpecificCfg.HydPressLossCoeff.LiftCylReLineLoss1stOrdrCoeff);

            { // Read start of weigh configuration from ruby file.
                float startOfWeigh = DEFAULT_WEIGH_RANGE_START;
                if (rubyCfg.get("StartOfWeigh", startOfWeigh)) {
                    WeighRangeConfig.defaultStartOfWeighRange = startOfWeigh;
                }
                else {
                    WeighRangeConfig.defaultStartOfWeighRange = DEFAULT_WEIGH_RANGE_START;
                }
            }

            { // Read end of weigh configuration from ruby file
                float endOfWeigh;
                if (rubyCfg.get("EndOfWeigh", endOfWeigh)) {
                    WeighRangeConfig.defaultEndOfWeighRange = endOfWeigh;
                }
                else {
                    WeighRangeConfig.defaultEndOfWeighRange = DEFAULT_WEIGH_RANGE_END;
                }
            }

            { //Read minimum weigh range size from ruby file
                float minimumWeighRangeSize;
                if (rubyCfg.get("WeighRangeMin", minimumWeighRangeSize)) {
                    WeighRangeConfig.minimumWeighRangeSize = minimumWeighRangeSize;
                }
                else {
                    WeighRangeConfig.minimumWeighRangeSize = DEFAULT_MIN_WEIGH_RANGE_SIZE;
                }
            }

            { //Read minimum weigh range start configuration from ruby file
                float minimumWeighRangeStart;
                if (rubyCfg.get("MinWeighRangeStart", minimumWeighRangeStart)) {
                    WeighRangeConfig.minimumWeighRangeStart = minimumWeighRangeStart;
                }
                else {
                    WeighRangeConfig.minimumWeighRangeStart = DEFAULT_MIN_WEIGH_RANGE_START;
                }
            }

            { //Read Maximum weigh range end configuration from ruby file
                float maximumWeighRangeEnd;
                if (rubyCfg.get("MaxWeighRangeEnd", maximumWeighRangeEnd)) {
                    WeighRangeConfig.maximumWeighRangeEnd = maximumWeighRangeEnd;
                }
                else {
                    WeighRangeConfig.maximumWeighRangeEnd = DEFAULT_MAX_WEIGH_RANGE_END;
                }
            }

            /* Low Lift Weigh Configuration */
            rubyCfg.get("FilterFactorMean", machSpecificCfg.LlwTbl.FilterFactorMean);
            rubyCfg.get("FilterFactorVariance", machSpecificCfg.LlwTbl.FilterFactorVariance);
            rubyCfg.get("ErrorBand", machSpecificCfg.LlwTbl.ErrorBand);
            rubyCfg.get("MinimumConfidenceTime", machSpecificCfg.LlwTbl.MinimumConfidenceTime);
            rubyCfg.get("AutoWeighRangeConfThr", machSpecificCfg.LlwTbl.AutoWeighRangeConfThr);
            rubyCfg.get("AutoWeighMinLiftHt", machSpecificCfg.LlwTbl.AutoWeighMinLiftHt);
            rubyCfg.get("WtCf", machSpecificCfg.LlwTbl.WtCf);
            rubyCfg.get("StageOneCf", machSpecificCfg.LlwTbl.StageOneCf);
            rubyCfg.get("StageTwoCf", machSpecificCfg.LlwTbl.StageTwoCf);
            rubyCfg.get("StageThreeCf", machSpecificCfg.LlwTbl.StageThreeCf);
            rubyCfg.get("DampingRate", machSpecificCfg.LlwTbl.DampingRate);

            { // Read Optional Low Pass Weight Filter Configuration
                bool WtLpsOptional;
                float WtLpsOptionalCf;
                if (rubyCfg.get("WtLpsOptional", WtLpsOptional) &&
                        rubyCfg.get("WtLpsOptionalCf", WtLpsOptionalCf)) {
                    machSpecificCfg.LlwTbl.WtLpsOptional = WtLpsOptional;
                    machSpecificCfg.LlwTbl.WtLpsOptionalCf = WtLpsOptionalCf;
                }
                else {
                    machSpecificCfg.LlwTbl.WtLpsOptional = false;
                }
            }

            /* Angular velocity Low Pass Filter Corner Frequency (rad/s) */
            if (!rubyCfg.get("LiftAngVelFilterCf", machSpecificCfg.LiftVelFilterCf)) {
                machSpecificCfg.LiftVelFilterCf = std::max(machSpecificCfg.LlwTbl.StageOneCf,
                        std::max(machSpecificCfg.LlwTbl.StageTwoCf, machSpecificCfg.LlwTbl.StageThreeCf));
            }

            /* Zero weight configuration */
            {
                int temp = LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_HIGH;
                rubyCfg.get("ZeroBktWtAccuracyLimit", temp);
                machSpecificCfg.ZeroBktWtAccuracyLimit = static_cast<LpsWeighBktWtAccuracy_t>(temp);
            }

            rubyCfg.get("ZeroRangeLimit", machSpecificCfg.ZeroRangeLimit);
            rubyCfg.get("ZeroAdjInitTimeInterval", machSpecificCfg.ZeroAdjInitTimeInterval);
            rubyCfg.get("ZeroAdjAutoTimeInterval", machSpecificCfg.ZeroAdjAutoTimeInterval);
            if (!rubyCfg.get("ZeroAdjAutoTimeIntervalFast", machSpecificCfg.ZeroAdjAutoTimeIntervalFast)) {
                machSpecificCfg.ZeroAdjAutoTimeIntervalFast = machSpecificCfg.ZeroAdjAutoTimeInterval;
            }
            rubyCfg.get("ZeroAdjOilTempWarnThreshold", machSpecificCfg.ZeroAdjOilTempWarnThreshold);
            if (!rubyCfg.get("ZeroAdjOilTempDelta", machSpecificCfg.ZeroAdjOilTempDelta)) {
                machSpecificCfg.ZeroAdjOilTempDelta = LPS_OIL_TEMP_DELTA;
            }

            /* Tilt Compensation Config */
            rubyCfg.get("TiltCompGainScalar", machSpecificCfg.TiltComp.TiltCompGainScalar);
            rubyCfg.get("TiltCompEmptyBktWtGain", machSpecificCfg.TiltComp.TiltCompEmptyBktWtGain);
            rubyCfg.get("TiltCompMaxGain", machSpecificCfg.TiltComp.TiltCompMaxGain);

            if (!rubyCfg.get("LiftCylExtPctAxis", CalibTblRuby.lift_cyl_ext_pct_axis)) {
                AIS_LOG_ERROR("LiftCylExtPctAxis read fail.");
            }

            if (!rubyCfg.get("TiltCompGainData", CalibTblRuby.tilt_comp_gain_data)) {
                AIS_LOG_ERROR("TiltCompGainData read fail.");
            }

            if (!rubyCfg.get("TiltCylExtPctAxis", CalibTblRuby.tilt_cyl_ext_pct_axis)) {
                AIS_LOG_ERROR("TiltCylExtPctAxis read fail.");
            }

            machSpecificCfg.TiltMap.NumColumns = CalibTblRuby.lift_cyl_ext_pct_axis.size();
            machSpecificCfg.TiltMap.ColumnAxis = CalibTblRuby.lift_cyl_ext_pct_axis.data();
            machSpecificCfg.TiltMap.NumRows = CalibTblRuby.tilt_cyl_ext_pct_axis.size();
            machSpecificCfg.TiltMap.RowAxis = CalibTblRuby.tilt_cyl_ext_pct_axis.data();
            machSpecificCfg.TiltMap.Data = CalibTblRuby.tilt_comp_gain_data.data();

            rubyCfg.get("DigTargetWt", machSpecificCfg.DigConfig.DigTargetWt);
            rubyCfg.get("DigDurationMaxLimit", machSpecificCfg.DigConfig.DigDurationMaxLimit);
            rubyCfg.get("DigDurationMinLimit", machSpecificCfg.DigConfig.DigDurationMinLimit);
            rubyCfg.get("DigStartLimit", machSpecificCfg.DigConfig.DigStartLimit);
            rubyCfg.get("DigEndLimit", machSpecificCfg.DigConfig.DigEndLimit);
            rubyCfg.get("LiftLowerVelocityLimit", machSpecificCfg.DigConfig.LiftLowerVelocityLimit);

            rubyCfg.get("TiltCylExtThreshold", machSpecificCfg.DumpConfig.TiltCylExtThreshold);
            rubyCfg.get("TiltCylExtThresholdStrict", machSpecificCfg.DumpConfig.TiltCylExtThresholdStrict);
            rubyCfg.get("FullDumpBktAngleThreshold", machSpecificCfg.DumpConfig.FullDumpBktAngleThreshold);
            rubyCfg.get("PartDumpBktAngleThreshold", machSpecificCfg.DumpConfig.PartDumpBktAngleThreshold);
            rubyCfg.get("FullRackBktAngleThreshold", machSpecificCfg.DumpConfig.FullRackBktAngleThreshold);
            rubyCfg.get("TiltAngleABCThreshold", machSpecificCfg.DumpConfig.TiltAngleABCThreshold);

            rubyCfg.get("FastFiltWtCf", machSpecificCfg.LiveWeighConfig.FastFiltWtCf);
            rubyCfg.get("SlowFiltWtCf", machSpecificCfg.LiveWeighConfig.SlowFiltWtCf);

            { /* Read Default Machine Pitch Cal Offset */
                float tipoffPitchCalOffset = 0.f;
                rubyCfg.get("TipoffPitchCalOffset", tipoffPitchCalOffset);
                cnfg_.tipoffPitchCalOffset = tipoffPitchCalOffset;
            }

            /* Calibration Information used to calculate empty bucket weight and full calibration weight */
            if (rubyCfg.get("QR_Min_HydOilTemp_celsius", WeighPidTbl.QR_HydOilTempMin_C)) {
                AIS_LOG_INFO("HydOilTempMin: %d", WeighPidTbl.QR_HydOilTempMin_C);
            }
            else {
                WeighPidTbl.QR_HydOilTempMin_C = 40;    // default if not defined in .rb
                AIS_LOG_INFO("HydOilTempMin(default): %d", WeighPidTbl.QR_HydOilTempMin_C);
            }

            if (rubyCfg.get("QR_Min_LiftCylVelocity_mm_sec", WeighPidTbl.QR_LiftCylVelMin_mm_sec)) {
                AIS_LOG_INFO("LiftCylVelMin: %d", WeighPidTbl.QR_LiftCylVelMin_mm_sec);
            }
            else {
                WeighPidTbl.QR_LiftCylVelMin_mm_sec = -25;    // default if not defined in .rb
                AIS_LOG_INFO("LiftCylVelMin(default): %d", WeighPidTbl.QR_LiftCylVelMin_mm_sec);
            }

            if (rubyCfg.get("QR_Max_LiftCylVelocity_mm_sec", WeighPidTbl.QR_LiftCylVelMax_mm_sec)) {
                AIS_LOG_INFO("LiftCylVelCalValue: %d", WeighPidTbl.QR_LiftCylVelMax_mm_sec);
            }
            else {
                WeighPidTbl.QR_LiftCylVelMax_mm_sec = 25;    // default if not defined in .rb
                AIS_LOG_INFO("LiftCylVelCalValue(default): %d", WeighPidTbl.QR_LiftCylVelMax_mm_sec);
            }

            rubyCfg.get("FilterTauDelay", LpsSaWeighInfoTbl.FilterTauDelay);
            rubyCfg.get("PwmcycleRate_hz", LpsSaWeighInfoTbl.PwmcycleRate);
        }
    }

    weighInitTbl.DumpWtBuffer.PtrDumpWtBuffer = LpsWeighPtrDumpWtBuffer;
    weighInitTbl.DumpWtBuffer.bufferSize = WEIGH_LIB_DUMP_BUFF_SIZE;

    if (!LpsSaLoadKinematicsTbl()) {
        AIS_LOG_ERROR("Failed to load Kinematics.");
        everythingOk = false;
    }

    { // Read the Machine Type
        ConfigSection machineType;
        if (!getTaskParser().getSection("MachineType", machineType)) {
            AIS_LOG_ERROR("MachineType section not found");
            everythingOk = false;
        }
        else {
            machineType.get("InternalMsn", machineMSN);

            // raise diagnostic if machine model is NOT_SET (default)
            if ("NOT00000" == machineMSN) {
                MachineModelNotSetOut setDiag;
                setAutonomyCondition(setDiag);
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::MACHINE_MODEL_NOT_SET] = true;
                LpsSaWeighInfoTbl.MachineModelNotSet = TRUE;
            }
            else {
                // Machine model has been selected
                clearAutonomyCondition<MachineModelNotSetOut>();
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::MACHINE_MODEL_NOT_SET] = false;
                LpsSaWeighInfoTbl.MachineModelNotSet = FALSE;
            }
        }
    }

    if (!LpsSaLoadMachineProperties()) {
        AIS_LOG_ERROR("Failed to load Machine Properties.");
        everythingOk = false;
    }

    machSpecificCfg.NoOfLiftCyls = machineProperties.numOfLiftCylinders;
    machSpecificCfg.LiftCylBoreDia = machineProperties.liftBoreDiameter;
    machSpecificCfg.LiftCylRodDia = machineProperties.liftRodDiameter;
    machSpecificCfg.LiftArmAbLength = machineProperties.liftArmAbLength;
    machSpecificCfg.KgPerLiftKpaAtMidExtension = machineProperties.kgPerLiftKpaAtMidExtension;
    machSpecificCfg.LiftKpaNoBucketMidExtension = machineProperties.liftKpaNoBucketMidExtension;
    machSpecificCfg.AccelCompSupported = machineProperties.accelCompSupported;
    machSpecificCfg.IMUCompSupported = machineProperties.imuCompSupported;
    machSpecificCfg.IMUCompLoadCGAngleOffset = machineProperties.imuCompLoadCGAngleOffset;

    if (linkage_table_cnfg.outLiftCylLen.size() == linkage_table_cnfg.inLiftAngle.size()) {
        // Table sizes match, good
        machSpecificCfg.LiftLinkageMap.angleDeg = linkage_table_cnfg.inLiftAngle.data();
        machSpecificCfg.LiftLinkageMap.cylLenMm = linkage_table_cnfg.outLiftCylLen.data();
        machSpecificCfg.LiftLinkageMap.numPoints = linkage_table_cnfg.inLiftAngle.size();
    }
    else {
        // Table sizes don't match.
        machSpecificCfg.LiftLinkageMap.angleDeg = nullptr;
        machSpecificCfg.LiftLinkageMap.cylLenMm = nullptr;
        machSpecificCfg.LiftLinkageMap.numPoints = 0;
    }

    if (linkage_table_cnfg.outLiftAngleGain.size() == linkage_table_cnfg.inLiftAngle.size()) {
        // Table sizes match, good
        machSpecificCfg.LiftLinkageMap.gainDegPerMm = linkage_table_cnfg.outLiftAngleGain.data();
    }
    else {
        // Table sizes don't match.
        machSpecificCfg.LiftLinkageMap.gainDegPerMm = nullptr;
    }

    if (linkage_table_cnfg.outLiftPresNoBucket.size() == linkage_table_cnfg.inLiftAngle.size()) {
        // Table sizes match, good
        machSpecificCfg.LiftLinkageMap.presNoBucketkPa = linkage_table_cnfg.outLiftPresNoBucket.data();
    }
    else {
        // Table sizes don't match.
        machSpecificCfg.LiftLinkageMap.presNoBucketkPa = nullptr;
    }

    if ((linkage_table_cnfg.outLiftAngleAccel.size() == linkage_table_cnfg.inLiftAngle.size()) &&
            (linkage_table_cnfg.liftRefVel > 0.f)) {
        // Table sizes match, good
        machSpecificCfg.LiftLinkageMap.accelDegPerSec2 = linkage_table_cnfg.outLiftAngleAccel.data();
        machSpecificCfg.LiftLinkageMap.refVelMmPerSec = linkage_table_cnfg.liftRefVel;
    }
    else {
        // Table sizes don't match.
        machSpecificCfg.LiftLinkageMap.accelDegPerSec2 = nullptr;
        machSpecificCfg.LiftLinkageMap.refVelMmPerSec = 0.f;
    }

    return everythingOk;
}
