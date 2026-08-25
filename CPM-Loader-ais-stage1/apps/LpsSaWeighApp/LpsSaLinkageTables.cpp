/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaLinkageTables.cpp
DESCRIPTION: Functions required for select the appropriate map for selected application
             to calculate lift and tilt parameters
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef  _LPS_SA_WEIGHAPP_H_
#include "LpsSaWeighApp.h"
#endif

#include <math.h>
#include <iostream>

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
#define NORMALIZE(VAL, MIN, MAX) ((((float)(VAL) - (float)(MIN)) / ((float)(MAX) - (float)(MIN))) * 100.f)


/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/
static float normalize(float val, float min, float max);

/******************************************************************************
FUNCTION NAME:normalize
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
static float normalize(float val, float min, float max) {
    float norm;

    if (val > max) {
        /* If exceeds max, set to 100% */
        norm = 100.0;
    }
    else if (val < min) {
        /* If less than min, set to 0% */
        norm = 0.0;
    }
    else {
        norm = ((val - min)/(max - min)) * 100.f;
    }

    return norm;
}

/******************************************************************************
FUNCTION NAME: LpsSaLoadKinematicsTbl
DESCRIPTION: This function reads the kinematics table from the ruby file
PARAMETER DESCRIPTION: none
RETURN VALUE: none
*******************************************************************************/
bool LpsSaWeighApp::LpsSaLoadKinematicsTbl(void)
{
    ConfigSection rubyCfg;

    if (!getTaskParser().getSection("MachineSpecificConfig", rubyCfg)) {
        AIS_LOG_ERROR("MachineSpecificConfig section not found");
        return false;
    }

    bool everythingOk = true;

    uint8_t tiltSensorType;
    if (rubyCfg.get("tiltSensorType", tiltSensorType)) {
        linkage_table_cnfg.tiltSensorType = static_cast<LpsSaTiltSensorType_t>(tiltSensorType);
    }
    else {
        linkage_table_cnfg.tiltSensorType = TILT_SENSOR_TYPE_ROTARY; // default to Rotary tilt sensor
    }
    AIS_LOG_INFO("tiltSensorType %u",linkage_table_cnfg.tiltSensorType);

    linkage_table_cnfg.hydPresSensorSlope = HYD_PRES_SENSOR_SLOPE;    // default to HYD_PRES_SENSOR_SLOPE 0.0018
    rubyCfg.get("hydPresSensorSlope", linkage_table_cnfg.hydPresSensorSlope);
    AIS_LOG_INFO("hydPresSensorSlope %f",linkage_table_cnfg.hydPresSensorSlope);

    linkage_table_cnfg.hydPresSensorIntercept = HYD_PRES_SENSOR_INTERCEPT;    // default to HYD_PRES_SENSOR_INTERCEPT  5.0
    rubyCfg.get("hydPresSensorIntercept", linkage_table_cnfg.hydPresSensorIntercept);
    AIS_LOG_INFO("hydPresSensorIntercept %f",linkage_table_cnfg.hydPresSensorIntercept);

    linkage_table_cnfg.invertLiftDc = 0;    // default to NO INVERT
    rubyCfg.get("invertLiftDc", linkage_table_cnfg.invertLiftDc);
    AIS_LOG_INFO("invertLiftDc %d",linkage_table_cnfg.invertLiftDc);

    linkage_table_cnfg.invertTiltDc = 0;    // default to NO INVERT
    rubyCfg.get("invertTiltDc", linkage_table_cnfg.invertTiltDc);
    AIS_LOG_INFO("invertTiltDc %d",linkage_table_cnfg.invertTiltDc);

    if (rubyCfg.get("inLiftDc", linkage_table_cnfg.inLiftDc)) {
        AIS_LOG_INFO("\n inLiftDc \n");
        for (const auto& value : linkage_table_cnfg.inLiftDc) {
            AIS_LOG_INFO("%f", value);
        }
    }
    else {
        AIS_LOG_ERROR("inLiftDc read fail.");
        everythingOk = false;
    }

    if (rubyCfg.get("outLiftAngle", linkage_table_cnfg.outLiftAngle)) {
        if (linkage_table_cnfg.outLiftAngle.size() == linkage_table_cnfg.inLiftDc.size()) {
            AIS_LOG_INFO("\n outLiftAngle \n");
            for (const auto& value : linkage_table_cnfg.outLiftAngle) {
                AIS_LOG_INFO("%f", value);
            }
        }
        else {
            AIS_LOG_ERROR("inLiftDc and outLiftAngle size mismatch.");
            everythingOk = false;
        }
    }
    else {
        AIS_LOG_ERROR("outLiftAngle read fail.");
        everythingOk = false;
    }

    if (rubyCfg.get("inTiltDc", linkage_table_cnfg.inTiltDc)) {
        AIS_LOG_INFO("\n inTiltDc \n");
        for (const auto& value : linkage_table_cnfg.inTiltDc) {
            AIS_LOG_INFO("%f", value);
        }
    }
    else {
        AIS_LOG_ERROR("inTiltDc read fail.");
        everythingOk = false;
    }

    if (rubyCfg.get("inLiftAngle", linkage_table_cnfg.inLiftAngle)) {
        AIS_LOG_INFO("\n inLiftAngle \n");
        for (const auto& value : linkage_table_cnfg.inLiftAngle) {
            AIS_LOG_INFO("%f", value);
        }
    }
    else {
        AIS_LOG_ERROR("inLiftAngle read fail.");
        everythingOk = false;
    }

    if (rubyCfg.get("outLiftCylLen", linkage_table_cnfg.outLiftCylLen)) {
        if (linkage_table_cnfg.outLiftCylLen.size() == linkage_table_cnfg.inLiftAngle.size()) {
            AIS_LOG_INFO("\n outLiftCylLen \n");
            for (const auto& value : linkage_table_cnfg.outLiftCylLen) {
                AIS_LOG_INFO("%f", value);
            }
        }
        else {
            AIS_LOG_ERROR("inLiftAngle and outLiftCylLen size mismatch.");
            everythingOk = false;
        }
    }
    else {
        AIS_LOG_ERROR("outLiftCylLen read fail.");
        everythingOk = false;
    }

    if (rubyCfg.get("outLiftAngleGain", linkage_table_cnfg.outLiftAngleGain)) {
        if (linkage_table_cnfg.outLiftAngleGain.size() != linkage_table_cnfg.inLiftAngle.size()) {
            AIS_LOG_ERROR("inLiftAngle and outLiftAngleGain size mismatch.");
            everythingOk = false;
        }
    }
    else {
        // This is not an error, it just might not be provided.
        AIS_LOG_INFO("outLiftAngleGain read error.");
        linkage_table_cnfg.outLiftAngleGain.clear();
    }

    if (rubyCfg.get("outLiftPresNoBucket", linkage_table_cnfg.outLiftPresNoBucket)) {
        if (linkage_table_cnfg.outLiftPresNoBucket.size() != linkage_table_cnfg.inLiftAngle.size()) {
            AIS_LOG_ERROR("inLiftAngle and outLiftPresNoBucket size mismatch.");
            everythingOk = false;
        }
    }
    else {
        // This is not an error, it just might not be provided.
        AIS_LOG_INFO("outLiftPresNoBucket read error.");
        linkage_table_cnfg.outLiftPresNoBucket.clear();
    }

    if (rubyCfg.get("outLiftAngleAccel", linkage_table_cnfg.outLiftAngleAccel)) {
        if (linkage_table_cnfg.outLiftAngleAccel.size() != linkage_table_cnfg.inLiftAngle.size()) {
            AIS_LOG_ERROR("inLiftAngle and outLiftAngleAccel size mismatch.");
            everythingOk = false;
        }
    }
    else {
        // This is not an error, it just might not be provided.
        AIS_LOG_INFO("outLiftAngleAccel read error.");
        linkage_table_cnfg.outLiftAngleAccel.clear();
    }

    // Read the lift reference cylinder velocity.
    if (!rubyCfg.get("liftRefVel", linkage_table_cnfg.liftRefVel)) {
        linkage_table_cnfg.liftRefVel = 0.f;
    }
    AIS_LOG_INFO("liftRefVel %f", linkage_table_cnfg.liftRefVel);

    // Read outTiltAngle, outTiltCylLen and sensor dependent parameters
    if (!rubyCfg.get("outTiltAngle", linkage_table_cnfg.outTiltAngle) ||
        !rubyCfg.get("outTiltCylLen", linkage_table_cnfg.outTiltCylLen)) {
        AIS_LOG_ERROR("outTiltAngle and/or outTiltCylLen read failed");
        return false;
    }
    else {
        std::size_t outTiltAngleSizeExpected;
        std::size_t outTiltCylLenSizeExpected;

        if (linkage_table_cnfg.tiltSensorType == TILT_SENSOR_TYPE_ROTARY) {
            // Read in xTileAngle, yLiftAngle and calculate outTiltAngle, outTiltCylLen expected sizes
            if (!rubyCfg.get("xTiltAngle", linkage_table_cnfg.xTiltAngle) ||
                !rubyCfg.get("yLiftAngle", linkage_table_cnfg.yLiftAngle)) {
                AIS_LOG_ERROR("xTiltAngle and/or yLiftAngle read failed");
                return false;
            }

            AIS_LOG_INFO("\n xTiltAngle \n");
            for (const auto& value : linkage_table_cnfg.xTiltAngle) {
                AIS_LOG_INFO("%f", value);
            }

            AIS_LOG_INFO("\n yLiftAngle \n");
            for (const auto& value : linkage_table_cnfg.yLiftAngle) {
                AIS_LOG_INFO("%f", value);
            }

            outTiltAngleSizeExpected = linkage_table_cnfg.inTiltDc.size();
            outTiltCylLenSizeExpected = linkage_table_cnfg.xTiltAngle.size() * linkage_table_cnfg.yLiftAngle.size();
        }
        else { // TILT_SENSOR_TYPE_INLINE
            // Read in xTiltCylLen, yLiftCylLen and calculate outTiltAngle, outTiltCylLen expected sizes
            if (!rubyCfg.get("xTiltCylLen", linkage_table_cnfg.xTiltCylLen) ||
                !rubyCfg.get("yLiftCylLen", linkage_table_cnfg.yLiftCylLen)) {
                AIS_LOG_ERROR("xTiltCylLen and/or yLiftCylLen read failed");
                return false;
            }

            AIS_LOG_INFO("\n xTiltCylLen \n");
            for (const auto& value : linkage_table_cnfg.xTiltCylLen) {
                AIS_LOG_INFO("%f", value);
            }

            AIS_LOG_INFO("\n yLiftCylLen \n");
            for (const auto& value : linkage_table_cnfg.yLiftCylLen) {
                AIS_LOG_INFO("%f", value);
            }

            outTiltAngleSizeExpected = linkage_table_cnfg.xTiltCylLen.size() * linkage_table_cnfg.yLiftCylLen.size();
            outTiltCylLenSizeExpected = linkage_table_cnfg.inTiltDc.size();
        }

        // Confirm outTileAngle size matches expected
        if (linkage_table_cnfg.outTiltAngle.size() != outTiltAngleSizeExpected) {
            AIS_LOG_ERROR("outTiltAngle size mismatch.");
            return false;
        }

        AIS_LOG_INFO("\n outTiltAngle \n");
        for (const auto& value : linkage_table_cnfg.outTiltAngle) {
            AIS_LOG_INFO("%f", value);
        }

        // Confirm outTiltCylLen size matches expected
        if (linkage_table_cnfg.outTiltCylLen.size() != outTiltCylLenSizeExpected) {
            AIS_LOG_ERROR("outTiltCylLen size mismatch.");
            return false;
        }

        AIS_LOG_INFO("\n outTiltCylLen \n");
        for (const auto& value : linkage_table_cnfg.outTiltCylLen) {
            AIS_LOG_INFO("%f", value);
        }
    }

    if (rubyCfg.get("inTiltAngle", linkage_table_cnfg.inTiltAngle)) {
        AIS_LOG_INFO("\n inTiltAngle \n");
        for (const auto& value : linkage_table_cnfg.inTiltAngle) {
            AIS_LOG_INFO("%f", value);
        }
    }
    else {
        AIS_LOG_ERROR("inTiltAngle read fail.");
        everythingOk = false;
    }

    if (rubyCfg.get("outBktAngle", linkage_table_cnfg.outBktAngle)) {
        if (linkage_table_cnfg.outBktAngle.size() == linkage_table_cnfg.inTiltAngle.size()) {
            AIS_LOG_INFO("\n outBktAngle \n");
            for (const auto& value : linkage_table_cnfg.outBktAngle) {
                AIS_LOG_INFO("%f", value);
            }
        }
        else {
            AIS_LOG_ERROR("inTiltAngle and outBktAngle size mismatch.");
            everythingOk = false;
        }
    }
    else {
        AIS_LOG_ERROR("outBktAngle read fail.");
        everythingOk = false;
    }

    // kinematic min and max tilt cyl length
    if (rubyCfg.get("KnmaticsTiltCylLenMinMax", linkage_table_cnfg.KnmaticsTiltCylExtMinMax)) {
        if (2 == linkage_table_cnfg.KnmaticsTiltCylExtMinMax.size()) {
            AIS_LOG_INFO("\n KnmaticsTiltCylLenMinMax \n");
            for (const auto& value : linkage_table_cnfg.KnmaticsTiltCylExtMinMax) {
                AIS_LOG_INFO("%f", value);
            }
        }
        else {
            AIS_LOG_ERROR("KnmaticsTiltCylLenMinMax size != 2.");
            everythingOk = false;
        }
    }
    else {
        AIS_LOG_ERROR("KnmaticsTiltCylLenMinMax read fail.");
        everythingOk = false;
    }

    // kinematic min and max tilt angle ABC limits in degrees
    if (rubyCfg.get("KnmaticsTiltAngleMinMax", linkage_table_cnfg.KnmaticsTiltAngleMinMax)) {
        if (2 == linkage_table_cnfg.KnmaticsTiltAngleMinMax.size()) {
            AIS_LOG_INFO("\n KnmaticsTiltAngleMinMax \n");
            for (const auto& value : linkage_table_cnfg.KnmaticsTiltAngleMinMax) {
                AIS_LOG_INFO("%f", value);
            }
        }
        else {
            AIS_LOG_ERROR("KnmaticsTiltAngleMinMax size != 2.");
            everythingOk = false;
        }
    }
    else {
        AIS_LOG_ERROR("KnmaticsTiltAngleMinMax read fail.");
        everythingOk = false;
    }

    // kinematic min and max lift cyl length
    if (rubyCfg.get("KnmaticsLiftCylLenMinMax", linkage_table_cnfg.KnmaticsLiftCylExtMinMax)) {
        if (2 == linkage_table_cnfg.KnmaticsLiftCylExtMinMax.size()) {
            AIS_LOG_INFO("\n KnmaticsLiftCylLenMinMax \n");
            for (const auto& value : linkage_table_cnfg.KnmaticsLiftCylExtMinMax) {
                AIS_LOG_INFO("%f", value);
            }
        }
        else {
            AIS_LOG_ERROR("KnmaticsLiftCylLenMinMax size != 2.");
            everythingOk = false;
        }
    }
    else {
        AIS_LOG_ERROR("KnmaticsLiftCylLenMinMax read fail.");
        everythingOk = false;
    }

    return everythingOk;
}


bool LpsSaWeighApp::LpsSaLoadMachineProperties(void)
{
    ConfigSection rubyCfg;

    if (!getTaskParser().getSection("MachineSpecificConfig", rubyCfg)) {
        AIS_LOG_ERROR("MachineSpecificConfig section not found");
        return false;
    }

    bool everythingOk = true;
    bool isAdvVariant = (ADVANCED == getApplicationVariant());

    AIS_LOG_INFO("MachineMSN: %s", machineMSN.c_str());

    // Internal Msn
    try {
        std::string internal_msn_str = machineMSN.substr(3,5);
        machineProperties.internalMsn = std::stoi(internal_msn_str, 0, 10);
        AIS_LOG_INFO("InternalMsn: %d", machineProperties.internalMsn);
    }
    catch (...) {
        if (isAdvVariant) {
            AIS_LOG_ERROR("Error while parsing for internal msn");
            everythingOk = false;
        }
    }

    // ToolBcLen
    if (rubyCfg.get("ToolBcLen", machineProperties.toolBcLength)) {
        AIS_LOG_INFO("ToolBcLen: %f", machineProperties.toolBcLength);
    }
    else if (isAdvVariant) {
        AIS_LOG_ERROR("ToolBcLen not available in Cfg file");
        everythingOk = false;
    }

    // ToolBcAngle
    if (rubyCfg.get("ToolBcAngle", machineProperties.toolBcAngle)) {
        AIS_LOG_INFO("ToolBcAngle: %f", machineProperties.toolBcAngle);
    }
    else if (isAdvVariant) {
        AIS_LOG_ERROR("ToolBcAngle not available in Cfg file");
        everythingOk = false;
    }

    // NoOfTiltCyls
    if (rubyCfg.get("NoOfTiltCyls", machineProperties.numOfTiltCylinders)) {
        AIS_LOG_INFO("NoOfTiltCyls: %d", machineProperties.numOfTiltCylinders);
    }
    else if (isAdvVariant) {
        AIS_LOG_ERROR("NoOfTiltCyls not available in Cfg file");
        everythingOk = false;
    }

    // TiltCylBoreDia
    if (rubyCfg.get("TiltCylBoreDia", machineProperties.tiltBoreDiameter)) {
        AIS_LOG_INFO("TiltCylBoreDia: %f", machineProperties.tiltBoreDiameter);
    }
    else if (isAdvVariant) {
        AIS_LOG_ERROR("TiltCylBoreDia not available in Cfg file");
        everythingOk = false;
    }

    // TiltCylRodDia
    if (rubyCfg.get("TiltCylRodDia", machineProperties.tiltRodDiameter)) {
        AIS_LOG_INFO("TiltCylRodDia: %f", machineProperties.tiltRodDiameter);
    }
    else if (isAdvVariant) {
        AIS_LOG_ERROR("TiltCylRodDia not available in Cfg file");
        everythingOk = false;
    }

    // NoOfLiftCyls
    if (rubyCfg.get("NoOfLiftCyls", machineProperties.numOfLiftCylinders)) {
        AIS_LOG_INFO("NoOfLiftCyls: %d", machineProperties.numOfLiftCylinders);
    }
    else {
        AIS_LOG_ERROR("NoOfLiftCyls not available in Cfg file");
        everythingOk = false;
    }

    // LiftCylBoreDia
    if (rubyCfg.get("LiftCylBoreDia", machineProperties.liftBoreDiameter)) {
        AIS_LOG_INFO("LiftCylBoreDia: %f", machineProperties.liftBoreDiameter);
    }
    else {
        AIS_LOG_ERROR("LiftCylBoreDia not available in Cfg file");
        everythingOk = false;
    }

    // LiftCylRodDia
    if (rubyCfg.get("LiftCylRodDia", machineProperties.liftRodDiameter)) {
        AIS_LOG_INFO("LiftCylRodDia: %f", machineProperties.liftRodDiameter);
    }
    else {
        AIS_LOG_ERROR("LiftCylRodDia not available in Cfg file");
        everythingOk = false;
    }

    // DigTargetWt
    if (rubyCfg.get("DigTargetWt", machineProperties.ratedPayload)) {
        AIS_LOG_INFO("DigTargetWt: %f", machineProperties.ratedPayload);
    }
    else if (isAdvVariant) {
        AIS_LOG_ERROR("DigTargetWt not available in Cfg file");
        everythingOk = false;
    }

    // LoaderBktPayldTrgtWt
    if (rubyCfg.get("LoaderBktPayldTrgtWt", machineProperties.bucketPayloadTargetWeightDefault)) {
        AIS_LOG_INFO("LoaderBktPayldTrgtWt: %f", machineProperties.bucketPayloadTargetWeightDefault);
    }
    else {
        machineProperties.bucketPayloadTargetWeightDefault = 0.f;
        AIS_LOG_INFO("LoaderBktPayldTrgtWt missing, default to: %f", machineProperties.bucketPayloadTargetWeightDefault);
    }

    // LiftArmAbLength
    if (rubyCfg.get("LiftArmAbLength", machineProperties.liftArmAbLength)) {
        AIS_LOG_INFO("LiftArmAbLength: %f", machineProperties.liftArmAbLength);
    }
    else {
        machineProperties.liftArmAbLength = 0.f;
        AIS_LOG_INFO("LiftArmAbLength missing, default to: %f", machineProperties.liftArmAbLength);
    }

    // KgPerLiftKpaAtMidExtension
    if (rubyCfg.get("KgPerLiftKpaAtMidExtension", machineProperties.kgPerLiftKpaAtMidExtension)) {
        AIS_LOG_INFO("KgPerLiftKpaAtMidExtension: %f", machineProperties.kgPerLiftKpaAtMidExtension);
    }
    else {
        machineProperties.kgPerLiftKpaAtMidExtension = 0.f;
        AIS_LOG_INFO("KgPerLiftKpaAtMidExtension missing, default to: %f", machineProperties.kgPerLiftKpaAtMidExtension);
    }

    // LiftKpaNoBucketMidExtension
    if (rubyCfg.get("LiftKpaNoBucketMidExtension", machineProperties.liftKpaNoBucketMidExtension)) {
        AIS_LOG_INFO("LiftKpaNoBucketMidExtension: %f", machineProperties.liftKpaNoBucketMidExtension);
    }
    else {
        machineProperties.liftKpaNoBucketMidExtension = 0.f;
        AIS_LOG_INFO("LiftKpaNoBucketMidExtension missing, default to: %f", machineProperties.liftKpaNoBucketMidExtension);
    }

    // HydOilTempOptional
    if (rubyCfg.get("HydOilTempOptional", machineProperties.hydOilTempOptional)) {
        AIS_LOG_INFO("HydOilTempOptional: %d", machineProperties.hydOilTempOptional);
    }
    else  {
        // Default to not optional.
        machineProperties.hydOilTempOptional = false;
        AIS_LOG_INFO("HydOilTempOptional missing, default to: %d", machineProperties.hydOilTempOptional);
    }

    // HydOilTempEnabledDefault
    if (rubyCfg.get("HydOilTempEnabledDefault", machineProperties.hydOilTempEnabledDefault)) {
        if (false == machineProperties.hydOilTempOptional) {
            // Force default to Enabled since it is not Optional
            machineProperties.hydOilTempEnabledDefault = true;
        }
    }
    else  {
        // Default to Enabled.
        machineProperties.hydOilTempEnabledDefault = true;
    }
    AIS_LOG_INFO("HydOilTempEnabledDefault is set to: %d", machineProperties.hydOilTempEnabledDefault);

    // AccelCompSupported
    if (rubyCfg.get("AccelCompSupported", machineProperties.accelCompSupported)) {
        AIS_LOG_INFO("AccelCompSupported: %d", machineProperties.accelCompSupported);
    }
    else {
        // Default to not supported.
        machineProperties.accelCompSupported = false;
        AIS_LOG_INFO("AccelCompSupported missing, default to: %d", machineProperties.accelCompSupported);
    }

    // IMU based compensation supported
    if (rubyCfg.get("LegalForTradeSupported", machineProperties.legalForTradeSupported)) {
        AIS_LOG_INFO("LegalForTradeSupported: %d", machineProperties.legalForTradeSupported);
    }
    else {
        // Default to not supported.
        machineProperties.legalForTradeSupported = false;
        AIS_LOG_INFO("LegalForTradeSupported missing, default to: %d", machineProperties.legalForTradeSupported);
    }

    // If LFT is supported, then IMU-based compensation is supported
    // In the future, this could be different where IMU-based compensation is supported
    // even without support for LFT, but not for now.
    machineProperties.imuCompSupported = machineProperties.legalForTradeSupported;

    // IMUCompLoadCGAngleOffset
    if (rubyCfg.get("IMUCompLoadCGAngleOffset", machineProperties.imuCompLoadCGAngleOffset)) {
        AIS_LOG_INFO("IMUCompLoadCGAngleOffset: %f", machineProperties.imuCompLoadCGAngleOffset);
    }
    else {
        machineProperties.imuCompLoadCGAngleOffset = 2.f;
        AIS_LOG_INFO("IMUCompLoadCGAngleOffset missing, default to: %f", machineProperties.imuCompLoadCGAngleOffset);
    }

    return everythingOk;
}

/******************************************************************************
FUNCTION NAME: lpsSaGetLiftPosition
DESCRIPTION: Calculates the lift cylinder extension from the lift angle
PARAMETER DESCRIPTION: float liftAngle
RETURN VALUE: LpsSaLiftPosition_t
*******************************************************************************/
LpsSaLiftPosition_t LpsSaWeighApp::lpsSaGetLiftPosition(float liftAngle, bool liftOk)
{
    LpsSaLiftPosition_t liftPosition;
    liftPosition.angle = liftAngle;
    
    if ((liftOk) && (NULL != linkage_table_cnfg.outLiftCylLen.data())) {

        liftPosition.percentAngle = normalize(
                liftAngle,
                linkage_table_cnfg.inLiftAngle[0],
                linkage_table_cnfg.inLiftAngle[linkage_table_cnfg.inLiftAngle.size()-1]);

        /* Convert lift angle to cylinder length */
        float cylinderLength = LpsLookup(
                liftAngle,
                linkage_table_cnfg.inLiftAngle.data(),
                linkage_table_cnfg.outLiftCylLen.data(),
                linkage_table_cnfg.inLiftAngle.size());

        liftPosition.cylinderLength = cylinderLength;

        liftPosition.cylinderExtension = cylinderLength - linkage_table_cnfg.KnmaticsLiftCylExtMinMax[0];

        liftPosition.percentCylinderLength = normalize(
                cylinderLength,
                linkage_table_cnfg.outLiftCylLen[0],
                linkage_table_cnfg.outLiftCylLen[linkage_table_cnfg.outLiftCylLen.size()-1]);

        liftPosition.status = LPS_STATUS_OK;
    }
    else {
        liftPosition.percentAngle = 0;
        liftPosition.cylinderLength = 0;
        liftPosition.percentCylinderLength = 0;
        liftPosition.cylinderExtension = 0;
        liftPosition.status = LPS_STATUS_BAD;
    }

    return liftPosition;
}

/******************************************************************************
FUNCTION NAME: lpsSaGetLiftAngle
DESCRIPTION: Calculates the lift angle from lift dc
PARAMETER DESCRIPTION:
RETURN VALUE: float linkage_angle
*******************************************************************************/
float LpsSaWeighApp::lpsSaGetLiftAngle(float liftLinkageDc, bool liftOk)
{
    float liftAngle;

    if (liftOk) {
        /* Apply LPF on Dutycycle */
        cpm_filter_low_pass_filter2(&LiftLinkageSensorDcLpFilt, liftLinkageDc);

        float sensorDutyCycleFiltered = LiftLinkageSensorDcLpFilt.y;

        DebugLpsSaXCPChannels.DebugLiftCylDc = sensorDutyCycleFiltered;
        WeighPidTbl.LiftLinkageSensorDc = sensorDutyCycleFiltered;

        /* Normalize sensor duty cycle */
        float normalizedPosition = normalize(sensorDutyCycleFiltered,
                liftCalNvmTbl_.lift_full_lower_dc, liftCalNvmTbl_.lift_full_raise_dc);

        /* Convert duty cycle to lift angle */
        liftAngle = LpsLookup(normalizedPosition,
                linkage_table_cnfg.inLiftDc.data(),
                linkage_table_cnfg.outLiftAngle.data(),
                linkage_table_cnfg.inLiftDc.size());
    }
    else {
        // Reset the filter
        cpm_filter_low_pass_filter2_reset(&LiftLinkageSensorDcLpFilt);

        // Not good
        liftAngle = 0;
    }

    return liftAngle;
}

/******************************************************************************
FUNCTION NAME: lpsSaGetNormalizedTiltPosition
DESCRIPTION: Calculates normalized tilt position from dc
PARAMETER DESCRIPTION:
RETURN VALUE: float linkage_angle
*******************************************************************************/
float LpsSaWeighApp::lpsSaGetNormalizedTiltPosition(float tiltLinkageDc, bool tiltOk)
{
    float normalizedPosition;

    if (tiltOk) {
        /* Apply LPF to duty cycle */
        cpm_filter_low_pass_filter2(&TiltLinkageSensorDcLpFilt, tiltLinkageDc);

        float tiltSensorDutyCycleFiltered = TiltLinkageSensorDcLpFilt.y;

        DebugLpsSaXCPChannels.DebugTiltCylDc = tiltSensorDutyCycleFiltered;
        WeighPidTbl.TiltLinkageSensorDc = tiltSensorDutyCycleFiltered;

        /* Normalize sensor duty cycle for tilt */
        normalizedPosition = normalize(tiltSensorDutyCycleFiltered,
                tiltCalNvmTbl_.tilt_full_dump_dc, tiltCalNvmTbl_.tilt_full_rack_dc);
    }
    else {
        /* Reset the filter */
        cpm_filter_low_pass_filter2_reset(&TiltLinkageSensorDcLpFilt);

        normalizedPosition = 0;
    }

    return normalizedPosition;
}

/******************************************************************************
FUNCTION NAME: lpsSaGetTiltAngle
DESCRIPTION: Calculates the tilt angle from normalized tilt position
PARAMETER DESCRIPTION:
RETURN VALUE: float tilt linkage_angle
*******************************************************************************/
float LpsSaWeighApp::lpsSaGetTiltAngle(float tiltNormalizedPosition, bool tiltOk)
{
    (void)tiltOk;
    float tiltAngle = LpsLookup(tiltNormalizedPosition,
                        linkage_table_cnfg.inTiltDc.data(),
                        linkage_table_cnfg.outTiltAngle.data(),
                        linkage_table_cnfg.inTiltDc.size());

    return tiltAngle;
}

/******************************************************************************
FUNCTION NAME: lpsSaGetTiltRotaryPosition
DESCRIPTION: Calculates the tilt cylinder length from the given lift and
             tilt angles for rotary sensors
PARAMETER DESCRIPTION: float liftAngle, float tiltAngle
RETURN VALUE: LpsSaTiltPosition_t
*******************************************************************************/
LpsSaTiltPosition_t LpsSaWeighApp::lpsSaGetTiltRotaryPosition(float liftAngle, bool liftOk, float tiltAngle, bool tiltOk)
{
    LpsSaTiltPosition_t tiltPosition;
    tiltPosition.angle = tiltAngle;

    if ((liftOk) && (tiltOk) && (!linkage_table_cnfg.outTiltCylLen.empty())) {

        tiltPosition.percentAngle = normalize(tiltAngle,
                linkage_table_cnfg.KnmaticsTiltAngleMinMax[0],
                linkage_table_cnfg.KnmaticsTiltAngleMinMax[1]);

        /* Convert  lift angle to cylinder length */
        tiltPosition.cylinderLength = LpsLookup2d(
                liftAngle,
                tiltAngle,
                linkage_table_cnfg.yLiftAngle.data(),
                linkage_table_cnfg.xTiltAngle.data(),
                linkage_table_cnfg.outTiltCylLen.data(),
                linkage_table_cnfg.yLiftAngle.size(),
                linkage_table_cnfg.xTiltAngle.size());

        tiltPosition.cylinderExtension = tiltPosition.cylinderLength - linkage_table_cnfg.KnmaticsTiltCylExtMinMax[0];

        tiltPosition.percentCylinderLength = normalize(
                tiltPosition.cylinderLength,
                linkage_table_cnfg.KnmaticsTiltCylExtMinMax[0],
                linkage_table_cnfg.KnmaticsTiltCylExtMinMax[1]);

        tiltPosition.bucketAngle = liftAngle + LpsLookup(tiltAngle,
                linkage_table_cnfg.inTiltAngle.data(),
                linkage_table_cnfg.outBktAngle.data(),
                linkage_table_cnfg.inTiltAngle.size());

        tiltPosition.status = LPS_STATUS_OK;
    }
    else {
        tiltPosition.percentAngle = 0;
        tiltPosition.cylinderLength = 0;
        tiltPosition.percentCylinderLength = 0;
        tiltPosition.bucketAngle = 0;
        tiltPosition.cylinderExtension = 0;
        tiltPosition.status = LPS_STATUS_BAD;
    }

    return tiltPosition;
}   

/******************************************************************************
FUNCTION NAME: lpsSaGetTiltInlinePosition
DESCRIPTION: Calculates the tilt angle from the given lift and
             tilt cylinder length for inline sensors
PARAMETER DESCRIPTION: float liftAngle, float liftCylinderLength, float tiltCylinderLength
RETURN VALUE: LpsSaTiltPosition_t
*******************************************************************************/
LpsSaTiltPosition_t LpsSaWeighApp::lpsSaGetTiltInlinePosition(float liftAngle, float liftCylinderLength, bool liftOk, float tiltCylinderLength, bool tiltOk)
{
    LpsSaTiltPosition_t tiltPosition;
    tiltPosition.cylinderLength = tiltCylinderLength;

    if ((liftOk) && (tiltOk) && (!linkage_table_cnfg.outTiltAngle.empty())) {

        tiltPosition.cylinderExtension = tiltCylinderLength - linkage_table_cnfg.KnmaticsTiltCylExtMinMax[0];

        tiltPosition.percentCylinderLength = normalize(
            tiltCylinderLength,
            linkage_table_cnfg.KnmaticsTiltCylExtMinMax[0],
            linkage_table_cnfg.KnmaticsTiltCylExtMinMax[1]);

        /* Convert cylinder length lift to angle */
        float tiltAngle = LpsLookup2d(
                liftCylinderLength,
                tiltCylinderLength,
                linkage_table_cnfg.yLiftCylLen.data(),
                linkage_table_cnfg.xTiltCylLen.data(),
                linkage_table_cnfg.outTiltAngle.data(),
                linkage_table_cnfg.yLiftCylLen.size(),
                linkage_table_cnfg.xTiltCylLen.size());

        // Map the calibrated angle limits to nominal limits
        if (GetTiltCylCalStatus()) {
            float inMin = tiltCalNvmTbl_.tilt_full_dump_stop_angle;
            float inMax = tiltCalNvmTbl_.tilt_full_rack_stop_angle;
            float outMin = linkage_table_cnfg.KnmaticsTiltAngleMinMax[0];
            float outMax = linkage_table_cnfg.KnmaticsTiltAngleMinMax[1];
            float temp = (tiltAngle - inMin) / (inMax - inMin);
            tiltAngle = (temp * (outMax - outMin)) + outMin;
        }

        tiltPosition.angle = tiltAngle;

        tiltPosition.percentAngle = normalize(tiltAngle,
                linkage_table_cnfg.KnmaticsTiltAngleMinMax[0],
                linkage_table_cnfg.KnmaticsTiltAngleMinMax[1]);

        tiltPosition.bucketAngle = liftAngle + LpsLookup(tiltAngle,
                linkage_table_cnfg.inTiltAngle.data(),
                linkage_table_cnfg.outBktAngle.data(),
                linkage_table_cnfg.inTiltAngle.size());

        tiltPosition.status = LPS_STATUS_OK;
    }
    else {
        tiltPosition.percentAngle = 0;
        tiltPosition.cylinderLength = 0;
        tiltPosition.percentCylinderLength = 0;
        tiltPosition.bucketAngle = 0;
        tiltPosition.cylinderExtension = 0;
        tiltPosition.status = LPS_STATUS_BAD;
    }

    return tiltPosition;
}
