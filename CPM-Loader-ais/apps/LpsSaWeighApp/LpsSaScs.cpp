/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaCda.cpp
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <cmath>

#include <chrono/tz.hpp>

#include <boost/filesystem.hpp>

#ifndef  _LPS_SA_WEIGHAPP_H_
#include "LpsSaWeighApp.h"
#endif

#ifndef __LPS_PRIVATE_H__
#include <LpsPrivate.h>
#endif

#include <lps_sea_defs.h>
#include <LpsCalPublic.h>

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
#define TO_INT16_PID(value) (std::max(-32736.f, std::min(32767.f, std::roundf(value))))

/*******************************************************************************
** -- Function Prototypes --
*******************************************************************************/
/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/
/******************************************************************************
FUNCTION NAME:LpsSaScsChkForReqst
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::LpsSaScsChkForReqst()
{
    cpm_common_interfaces::msg::LpsSaWeighReqstChannel request;

    /* Retrieve SCS channel data */
    while (LpsSaWeighScsReqstIn->get(request)) {
        switch (request.command.value) {
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::ZERO):
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::RESET_BEST_BUCKET_WEIGHT):
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::CAPTURE_CYLINDER_EXTENSION_REFERENCE):
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::CLEAR_REWEIGH_WARNING): {
            // These commands are handled later. request_ stays on its real
            // old type (needs .reInit()), so convert field-by-field.
            request_.command = static_cast<LpsSaWeighReqstChannel::Command>(request.command.value);
            request_.appName = request.app_name;
            request_.appRequestId = request.app_request_id;
            request_.arg.b = request.arg_b;
            request_.arg.f1 = request.arg_f1;
            request_.arg.f2 = request.arg_f2;
            request_.arg.s = request.arg_s;
            request_.arg.u = request.arg_u;
            request_.arg.map.clear();
            for (const auto& fp : request.arg_map) {
                request_.arg.map.emplace_back(fp.first, fp.second);
            }
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_WEIGH_RANGE): {
            setWeighRange(request.arg_f1, request.arg_f2);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_BUCKET_PAYLOAD_TARGET_WEIGHT): {
            cnfg_.bucketPayloadTargetWeight = request.arg_f1;
            cnfg_.setSaveNeeded();
            AIS_LOG_NOTICE("BucketPayloadTargetWeight = %f", cnfg_.bucketPayloadTargetWeight);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_OVERLOAD_WARNING_ENABLE): {
            cnfg_.overloadWarningEnabled = request.arg_b;
            cnfg_.setSaveNeeded();
            AIS_LOG_NOTICE("OverloadWarningEnabled = %d", cnfg_.overloadWarningEnabled);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_HYD_OIL_TEMP_ENABLE): {
            LpsSaScsSendReqstResponse(request, setHydOilTempEnableStatus(request.arg_b));
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_IMU_COMP_ENABLE): {
            LpsSaScsSendReqstResponse(request, setIMUCompEnableStatus(request.arg_b));
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_AUDIBLE_WEIGHT_ENABLE): {
            LpsSaScsSendReqstResponse(request, setAudibleWeightEnableStatus(request.arg_b));
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_LFT_SEALED_FLASH_ENABLE): {
            if (payloadCalNvmTbl_.legalForTradeInstalled) {
                LpsSaScsSendReqstResponse(request, setFlashEnableStatus(request.arg_b));
            }
            else
            {
                LpsSaScsSendReqstResponse(request, false);
                AIS_LOG_ERROR("Failed to write flash enable status");
            }
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_CALIBRATION_WEIGHT): {
            float calibrationWeight = request.arg_f1;
            AIS_LOG_NOTICE("Write calibration weight request received: %f", calibrationWeight);

            if (((payloadCalNvmTbl_.data.CalWeight != calibrationWeight) ||
                    ((payloadCalNvmTbl_.data.CalStatus & CAL_BKT_WT_MASK) != CAL_BKT_WT_MASK)) && (calibrationWeight > 0.0)) {
                AIS_LOG_NOTICE("Calibration weight set: %f", calibrationWeight);

                // Update calibration weight.
                payloadCalNvmTbl_.data.CalWeight = calibrationWeight;
                sealTracker_.reportCalibrationWeight(payloadCalNvmTbl_.data.CalWeight);

                payloadCalNvmTbl_.data.CalStatus |= CAL_BKT_WT_MASK;

                // The advanced cal adjust table is tracked by the calCounter.
                payloadCalNvmTbl_.setAdvCalAdjust({{0.f, 0.f}, {calibrationWeight, calibrationWeight}});

                if (GetPayloadCalStatus()) {
                    ++payloadCalNvmTbl_.calCounter; // This is a new completed calibration.
                    sealTracker_.reportPayloadCalStatus(true, payloadCalNvmTbl_.calCounter);
                }

                // Zero Call Data without resetting the simple calibration truck history list.
                // TODO: us15073 - Reset SimpleCal truck history list on Cal Weight changes. (Jeff Budill)

                payloadCalNvmTbl_.data.ZeroWeight = 0.f;

                payloadCalNvmTbl_.data.CalAdjust = 0.f;
                sealTracker_.reportSpanAdjustFactor(payloadCalNvmTbl_.data.CalAdjust);

                payloadCalNvmTbl_.setSaveNeeded();

                updatedSimpleCalFactor = 0.f;
                sumOfAdjustedTruckWts = 0.f;
                sumOfZeroedTruckWts = 0.f;

                CalNVMReinitFlag = true;

                // Log the calibration results on successful calibration wt write
                logWeighCalResults();
            }

            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_MACHINE_PITCH_CAL_OFFSET): {
            setIMUPitchCalOffsetNVM(request.arg_f1);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_LFT_SEALED): {
            sealTracker_.seal(request.arg_b);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_LIFT_POSITION_SENSOR_ID): {
            sealTracker_.reportLiftPositionSensorId(request.arg_s);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_TILT_POSITION_SENSOR_ID): {
            sealTracker_.reportTiltPositionSensorId(request.arg_s);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_LIFT_HE_PRESSURE_SENSOR_ID): {
            sealTracker_.reportLiftHeadEndPressureSensorId(request.arg_s);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_LIFT_RE_PRESSURE_SENSOR_ID): {
            sealTracker_.reportLiftRodEndPressureSensorId(request.arg_s);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_HYDRAULIC_OIL_TEMP_SENSOR_ID): {
            sealTracker_.reportHydraulicOilTemperatureSensorId(request.arg_s);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_WORK_TOOL_ID): {
            sealTracker_.reportWorkToolId(request.arg_s);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_PAYLOAD_OUT_OF_CAL): {
            // make payload out of cal when AU2020 and NOT Legal for Trade, since we use calibration from impl
            setPayloadNotCalibrated();
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::NOTIFY_TICKET_NUMBER_WRITE): {
            sealTracker_.reportTicketNumberWrite(request.arg_u);
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::PUBLISH_SERVICE_HISTORY): {
            const std::string filePath(makeTempPath("ServiceHistory.json"));
            if (sealTracker_.publish(filePath)) {
                LpsSaScsSendReqstResponse(request, true, filePath);
                AIS_LOG_INFO("Service history published");
            }
            else {
                LpsSaScsSendReqstResponse(request, false);
                AIS_LOG_ERROR("Failed to publish service history");
            }
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::PUBLISH_LIFT_SENSOR_CALIBRATION): {
            const std::string filePath(makeTempPath("LiftSensorCalibration.json"));
            if (liftCalNvmTbl_.publish(filePath)) {
                LpsSaScsSendReqstResponse(request, true, filePath);
                AIS_LOG_INFO("Lift sensor calibration published");
            }
            else {
                LpsSaScsSendReqstResponse(request, false);
                AIS_LOG_ERROR("Failed to publish lift sensor calibration");
            }
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::PUBLISH_TILT_SENSOR_CALIBRATION): {
            const std::string filePath(makeTempPath("TiltSensorCalibration.json"));
            if (tiltCalNvmTbl_.publish(filePath)) {
                LpsSaScsSendReqstResponse(request, true, filePath);
                AIS_LOG_INFO("Tilt sensor calibration published");
            }
            else {
                LpsSaScsSendReqstResponse(request, false);
                AIS_LOG_ERROR("Failed to publish tilt sensor calibration");
            }
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::PUBLISH_WEIGH_CALIBRATION): {
            const std::string filePath(makeTempPath("WeighCalibration.json"));
            if (payloadCalNvmTbl_.publish(filePath)) {
                LpsSaScsSendReqstResponse(request, true, filePath);
                AIS_LOG_INFO("Weigh calibration published");
            }
            else {
                LpsSaScsSendReqstResponse(request, false);
                AIS_LOG_ERROR("Failed to publish weigh calibration");
            }
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::PUBLISH_WEIGH_CONFIGURATION): {
            const std::string filePath(makeTempPath("WeighConfiguration.json"));
            if (cnfg_.publish(filePath)) {
                LpsSaScsSendReqstResponse(request, true, filePath);
                AIS_LOG_INFO("Weigh configuration published");
            }
            else {
                LpsSaScsSendReqstResponse(request, false);
                AIS_LOG_ERROR("Failed to publish weigh configuration");
            }
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::PUBLISH_RECENT_WEIGH_RESULTS): {
            const std::string filePath(makeTempPath("RecentWeighResults.json"));
            if (publishRecentWeighResults(filePath)) {
                LpsSaScsSendReqstResponse(request, true, filePath);
                AIS_LOG_INFO("Weigh results published");
            }
            else {
                LpsSaScsSendReqstResponse(request, false);
                AIS_LOG_ERROR("Failed to publish weigh results");
            }
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_REWEIGH_MAX_PITCH): {
            float reweighMaxPitch = request.arg_f1;
            AIS_LOG_NOTICE("reweighMaxPitch = %f", reweighMaxPitch);
            cnfg_.reweighMaxPitch = reweighMaxPitch;
            sealTracker_.reportGenericConfigurationChange("MaxPitch:" + std::to_string(reweighMaxPitch));
            cnfg_.setSaveNeeded();
            CalNVMReinitFlag = TRUE;
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_REWEIGH_MIN_PITCH): {
            float reweighMinPitch = request.arg_f1;
            AIS_LOG_NOTICE("reweighMinPitch = %f", reweighMinPitch);
            cnfg_.reweighMinPitch = reweighMinPitch;
            sealTracker_.reportGenericConfigurationChange("MinPitch:" + std::to_string(reweighMinPitch));
            cnfg_.setSaveNeeded();
            CalNVMReinitFlag = TRUE;
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_REWEIGH_MAX_ABS_ROLL): {
            float reweighMaxAbsRoll = request.arg_f1;
            AIS_LOG_NOTICE("reweighMaxAbsRoll = %f", reweighMaxAbsRoll);
            cnfg_.reweighMaxAbsRoll = reweighMaxAbsRoll;
            sealTracker_.reportGenericConfigurationChange("MaxRoll:" + std::to_string(reweighMaxAbsRoll));
            cnfg_.setSaveNeeded();
            CalNVMReinitFlag = TRUE;
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_REWEIGH_MIN_LIFT_CYL_VEL): {
            float reweighMinLiftCylVel = request.arg_f1;
            AIS_LOG_NOTICE("reweighMinLiftCylVel = %f", reweighMinLiftCylVel);
            cnfg_.reweighMinLiftCylVel = reweighMinLiftCylVel;
            sealTracker_.reportGenericConfigurationChange("MinLiftVel:" + std::to_string(reweighMinLiftCylVel));
            cnfg_.setSaveNeeded();
            CalNVMReinitFlag = TRUE;
            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::WRITE_ADVANCED_CALIBRATION_ADJUSTMENT): {
            AIS_LOG_NOTICE("Write advanced calibration adjustment received.");

            // The advanced cal adjust table is tracked by the calCounter.
            {
                std::vector<std::pair<float, float>> argMapOld;
                for (const auto& fp : request.arg_map) {
                    argMapOld.emplace_back(fp.first, fp.second);
                }
                payloadCalNvmTbl_.setAdvCalAdjust(argMapOld);
            }

            if (GetPayloadCalStatus()) {
                ++payloadCalNvmTbl_.calCounter; // This is a new completed calibration.
                sealTracker_.reportPayloadCalStatus(true, payloadCalNvmTbl_.calCounter);
            }

            // Zero Call Data without resetting the simple calibration truck history list.
            // TODO: us15073 - Reset SimpleCal truck history list on Cal Weight changes. (Jeff Budill)

            payloadCalNvmTbl_.data.ZeroWeight = 0.f;

            payloadCalNvmTbl_.data.CalAdjust = 0.f;
            sealTracker_.reportSpanAdjustFactor(payloadCalNvmTbl_.data.CalAdjust);

            payloadCalNvmTbl_.setSaveNeeded();

            updatedSimpleCalFactor = 0.f;
            sumOfAdjustedTruckWts = 0.f;
            sumOfZeroedTruckWts = 0.f;

            CalNVMReinitFlag = true;

            // Log the calibration results on successful calibration wt write
            logWeighCalResults();

            LpsSaScsSendReqstResponse(request, true);
            return;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::RUN_TEST): {
            std::string fileName = boost::filesystem::path(request.arg_s).filename().string();
            if (!fileName.empty()) {
                auto pos = fileName.find_first_of('.');
                if (0 != pos) { // Can't start with '.'
                    if (std::string::npos == pos) {
                        // No dot
                        fileName += ".csv";
                    }
                    testFixture_.runTestPlan(tempRoot_ / "test" / fileName);
                }
            }
            break;
        }
        case (cpm_common_interfaces::msg::WeighReqstChannelCommand::RECORD_TEST): {
            if (request.arg_u > 0) {
                testFixture_.recordTestPlan(tempRoot_ / "test" / "record.csv", std::chrono::seconds(request.arg_u));
            }
            break;
        }
        default: {
            AIS_LOG_ERROR("Unsupported request command.");
            break;
        }
        }
    }

    return;
}

/******************************************************************************
FUNCTION NAME:LpsSaScsSendReqstResponse
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
bool LpsSaWeighApp::LpsSaScsSendReqstResponse(LpsSaWeighReqstChannel::Command command, bool success)
{
    if (command != request_.command) {
        AIS_LOG_ERROR("Request and response command mismatch.");
        return false;
    }

    bool rVal = LpsSaScsSendReqstResponse(request_, success);

    // Clear out the request since it has been handled.
    request_.reInit();

    return rVal;
}

bool LpsSaWeighApp::LpsSaScsSendReqstResponse(const LpsSaWeighReqstChannelStorage& request, bool success, const std::string& arg1) {
    // Build the response
    cpm_common_interfaces::msg::LpsSaWeighRespChannel response; // Default timepoint is now
    response.app_name = request.appName;
    response.app_request_id = request.appRequestId;
    response.command.value = static_cast<uint8_t>(request.command);
    response.success = success;
    response.arg1 = arg1;

    /* send response SCS channel */
    if (LpsSaWeighScsRespOut) {
        if (LpsSaWeighScsRespOut->publish(response)) {
            AIS_LOG_INFO("Published response, command=%d, success=%d", response.command.value , success);
            return true;
        }
    }

    AIS_LOG_ERROR("Failed to publish response, command=%d, success=%d", response.command.value , success);
    return false;
}

bool LpsSaWeighApp::LpsSaScsSendReqstResponse(const cpm_common_interfaces::msg::LpsSaWeighReqstChannel& request, bool success, const std::string& arg1) {
    // Build the response
    cpm_common_interfaces::msg::LpsSaWeighRespChannel response; // Default timepoint is now
    response.app_name = request.app_name;
    response.app_request_id = request.app_request_id;
    response.command = request.command;
    response.success = success;
    response.arg1 = arg1;

    /* send response SCS channel */
    if (LpsSaWeighScsRespOut) {
        if (LpsSaWeighScsRespOut->publish(response)) {
            AIS_LOG_INFO("Published response, command=%d, success=%d", response.command.value , success);
            return true;
        }
    }

    AIS_LOG_ERROR("Failed to publish response, command=%d, success=%d", response.command.value , success);
    return false;
}

/******************************************************************************
FUNCTION NAME: AisJhmDataServerTxRead
DESCRIPTION:            
PARAMETER DESCRIPTION:                        
RETURN VALUE:             
*******************************************************************************/
void LpsSaWeighApp::AisJhmDataServerTxRead()
{
    AisJhm2TxChannel AisJhm2TxIn;
    while (AisJhm2TxInputScs->get(AisJhm2TxIn)) {
        if (AisJhm2TxIn.simplecal_data.newDataFlag) {
            AIS_LOG_DEBUG("### AIS Adj wt = %f", AisJhm2TxIn.simplecal_data.adjtruckweight);
            AIS_LOG_DEBUG("### AIS Zeroed wt = %f", AisJhm2TxIn.simplecal_data.zeroedTruckWt);

            sumOfAdjustedTruckWts += AisJhm2TxIn.simplecal_data.adjtruckweight;
            sumOfZeroedTruckWts += AisJhm2TxIn.simplecal_data.zeroedTruckWt;

            if (sumOfZeroedTruckWts > 0.0f) {
                updatedSimpleCalFactor = sumOfAdjustedTruckWts/sumOfZeroedTruckWts;
                AIS_LOG_NOTICE("Updated simple cal factor = %f", updatedSimpleCalFactor);
            }
        }

        if (AisJhm2TxIn.tipoff_weight_adjust_data.newDataFlag) {
            auto tipOffWeight1 = AisJhm2TxIn.tipoff_weight_adjust_data.tipOffWeight1;
            auto weighRangeWeight1 = AisJhm2TxIn.tipoff_weight_adjust_data.weighRangeWeight1;
            auto tipOffWeight2 = AisJhm2TxIn.tipoff_weight_adjust_data.tipOffWeight2;
            auto weighRangeWeight2 = AisJhm2TxIn.tipoff_weight_adjust_data.weighRangeWeight2;

            // If weight 1 is not given, use weight 2 twice
            if (!std::isfinite(tipOffWeight1) ||
                    !std::isfinite(weighRangeWeight1)) {
                tipOffWeight1 = tipOffWeight2;
                weighRangeWeight1 = weighRangeWeight2;
            }

            // If weight 2 is not given, use weight 1 twice
            if (!std::isfinite(tipOffWeight2) ||
                    !std::isfinite(weighRangeWeight2)) {
                tipOffWeight2 = tipOffWeight1;
                weighRangeWeight2 = weighRangeWeight1;
            }

            AIS_LOG_DEBUG("tipOffWeight1: %f", tipOffWeight1);
            AIS_LOG_DEBUG("weighRangeWeight1: %f", weighRangeWeight1);
            AIS_LOG_DEBUG("tipOffWeight2: %f", tipOffWeight2);
            AIS_LOG_DEBUG("weighRangeWeight2: %f", weighRangeWeight2);

            auto calAdjust = LpsSaWeighInfoTbl.TipoffInputs.simple_cal_factor;
            auto zeroWeight = LpsSaWeighInfoTbl.TipoffInputs.zero_offset;
            auto anchorFactor = cnfg_.toaAnchoredFactor;
            auto anchorZero = cnfg_.toaAnchoredZeroOffset;

            AIS_LOG_DEBUG("calAdjust: %f", calAdjust);
            AIS_LOG_DEBUG("zeroWeight: %f", zeroWeight);

            AIS_LOG_DEBUG("anchorFactor: %f", anchorFactor);
            AIS_LOG_DEBUG("anchorZero: %f", anchorZero);

            // Undo existing simple cal
            float tipOffWeight1NoSc = tipOffWeight1 / calAdjust + zeroWeight;
            float weighRangeWeight1NoSc = weighRangeWeight1 / calAdjust + zeroWeight;
            float tipOffWeight2NoSc = tipOffWeight2 / calAdjust + zeroWeight;
            float weighRangeWeight2NoSc = weighRangeWeight2 / calAdjust + zeroWeight;

            // Undo existing anchoring
            float tipOffWeight1NoAnchor = tipOffWeight1NoSc / anchorFactor + anchorZero;
            float tipOffWeight2NoAnchor = tipOffWeight2NoSc / anchorFactor + anchorZero;

            float newAnchorFactor, newAnchorZero;

            // Calculate new anchoring values
            if (std::abs(tipOffWeight1NoAnchor - tipOffWeight2NoAnchor) >= (machineProperties.ratedPayload / 3.f)) {
                // The two different weights given are far enough apart to use a line.
                newAnchorFactor = (weighRangeWeight1NoSc - weighRangeWeight2NoSc) / (tipOffWeight1NoAnchor - tipOffWeight2NoAnchor);
                newAnchorZero = tipOffWeight2NoAnchor - (weighRangeWeight2NoSc / newAnchorFactor);
            }
            else {
                // The two different weights are too close together, just use an offset.
                newAnchorFactor = 1.f;
                newAnchorZero = ((tipOffWeight2NoAnchor + tipOffWeight1NoAnchor) - (weighRangeWeight2NoSc + weighRangeWeight1NoSc)) / 2.f;
            }

            AIS_LOG_DEBUG("newAnchorFactor: %f", newAnchorFactor);
            AIS_LOG_DEBUG("newAnchorZero:%f", newAnchorZero);

            // check validity of new anchor values: new anchor values are not finite values or payload is not calibrated
            if (!std::isfinite(newAnchorFactor) ||
                    !std::isfinite(newAnchorZero) ||
                    !GetPayloadCalStatus()) {
                // set anchor status to rejected and we will keep old anchor values
                LpsSaWeighInfoTbl.ToaAnchorStatus =
                        LpsSaWeighTxChannelStorage::ToaAnchorStatus_t::REJECTED;
            }
            else {
                // publish accepted and update anchor values
                cnfg_.toaAnchoredFactor = newAnchorFactor;
                cnfg_.toaAnchoredZeroOffset = newAnchorZero;
                cnfg_.setSaveNeeded();

                // unlatch the tipoff weight which it might be holding with old anchor values
                LpsSaWeighInfoTbl.TipoffInputs.unlatch_trigger = true;

                // set anchor status to accepted
                LpsSaWeighInfoTbl.ToaAnchorStatus =
                        LpsSaWeighTxChannelStorage::ToaAnchorStatus_t::ACCEPTED;
            }
        }
        else if (AisJhm2TxIn.tipoff_weight_adjust_data.reset) {
            resetToaAnchorValues();
        }
    }

    return;
}

/******************************************************************************
FUNCTION NAME: AisJhmDataServerTxRead
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::LpsSaSEAStatus( )
{
    /*TODO: We are initializing the status of the SEA to installed/enabled,
     * we should probably disable the SEA if we do not receive a SEA object
     * in the first x minutes */
    AutonomyConditionDiagnosticsTxInterface txData;
    while (AutonomyConditionDiagnosticsTxInputChannel->get(txData)) {
        for (const auto & element : txData.seaList) {
            if (element.reason_code == LPS_SEA_REASON_CODE_149) {
                LpsSaWeighInfoTbl.SEALevel1EssentialsInstalled = txData.checkSEAEnableStatus(element.status);
            }
            else if (element.reason_code == LPS_SEA_LFT_REASON_CODE_312) {
                bool installed = txData.checkSEAEnableStatus(element.status);

                // Update if a change is detected
                if (installed != payloadCalNvmTbl_.legalForTradeInstalled) {
                    payloadCalNvmTbl_.legalForTradeInstalled = installed;

                    payloadCalNvmTbl_.setSaveNeeded();

                    CalNVMReinitFlag = true;

                    if (installed) {
                        AIS_LOG_INFO("Legal For Trade SEA is Installed & Enabled");
                    }
                }

                // Update the seal tracker
                sealTracker_.reportLegalForTradeEnableStatus(installed);
            }
        }
    }

    if (LpsSaWeighInfoTbl.SEALevel1EssentialsInstalled) {
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_NOT_INSTALLED] = false;
    }
    else {
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_NOT_INSTALLED] = true;
    }
}

void LpsSaWeighApp::LpsSaBattVoltageRead( )
{
    /* send a request for SystemHardwareHealth at configured period */
    SystemHardwareHealthRequestOutput_.setNewDataFlag();
    SystemHardwareHealthRequestOutput_.send();

    SystemHardwareHealth rxData;    
    while (SystemHardwareHealthInput_->get(rxData)) {
        auto voltage = rxData.battery_voltage;

        // Battery Low condition
        if (voltage > 32) {
            if (!LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_HIGH]) {
                setAutonomyCondition(BatteryHigh("Battery High"));
                LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_HIGH] = true;
            }            
        }
        else if (LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_HIGH]) {
            clearAutonomyCondition<BatteryHigh>();
            LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_HIGH] = false;
        }  

        // Battery Low condition
        if (voltage < 9) {
            if (!LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_LOW]) {
                setAutonomyCondition(BatteryLow("Battery Low"));
                LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_LOW] = true;
            }            
        }
        else if (LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_LOW]) {
            clearAutonomyCondition<BatteryLow>();
            LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_LOW] = false;
        } 
    }
}


/******************************************************************************
FUNCTION NAME:LpsSaJobMgrScsRx
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::LpsSaWeighingScsRx( )
{
    /*Read the requests from JM App*/
    LpsSaScsChkForReqst();

    /*Read the param from JM*/
    LpsJobMgrTxRead();

    /* Read simple cal params from AisJhmDataServer. */
    AisJhmDataServerTxRead();

    /* Read Payload SEA status */
    LpsSaSEAStatus();

    /* Read params from Machine Interface */
    LpsSaBmiRead();

    /* Read and process battery voltage */
    LpsSaBattVoltageRead();

    // Make sure we are printing in the units that are defined by the display settings
    if (nullptr != displayStateInput_) {
        LpsSaUIDisplayStateInterface displayState;
        while (displayStateInput_->get(displayState)) {
            const LpsSaUIDisplayState& state = displayState.state;
            LpsSaWeighInfoTbl.inVerificationMode = state.isInVerificationMode();
            LpsSaWeighInfoTbl.weightUnits = state.getSettings().weightUnits;
            LpsSaWeighInfoTbl.weightInterval = state.getWeightInterval();
            LpsSaWeighInfoTbl.weightCapacity = state.getWeightCapacity();
            if (!LpsSaWeighInfoTbl.inVerificationMode) {
                // Only report changes in weight interval if not in verification node.
                sealTracker_.reportWeightInterval(LpsSaWeighInfoTbl.weightInterval, state.getWeightDecimalPrecision(), LpsSaWeighInfoTbl.weightUnits);
            }
        }
    }

    // Report ticket retention period to seal tracker.
    if (nullptr != printerCnfgInput_) {
        LpsSaTotalsPrinterCnfgInterface printerCnfg;
        while (printerCnfgInput_->get(printerCnfg)) {
            sealTracker_.reportTicketRetentionPeriod(printerCnfg.config.truckTicket.retentionPeriod);
        }
    }

    /* Receive local time offset and override the local time offset in chrono/print.hpp */
    if (nullptr != shmClockInput_) {
        ShmClock shmClock;
        while (shmClockInput_->get(shmClock)) {
            int32_t offset = shmClock.get_UTC_offset();
            tzone_tx_comm_struct tzone;
            if (tes_common_ais::get_tz_struct(tzone, shmClock)) {
                if ((tzInfo_.offset != offset) || (tzInfo_.index != tzone.tzone_id)) {
                    std::string tzStr = tes_common_ais::makeTZString(tzone);
                    if (tes_common_ais::setTZString(tzStr)) {
                        AIS_LOG_INFO("Set TZ environment variable to '%s'.", tzStr.c_str());
                        tes_common_ais::clearLocalTimeOffsetOverride();
                    }
                    else {
                        AIS_LOG_ERROR("Could not set TZ environment variable to '%s'.", tzStr.c_str());
                        tes_common_ais::setLocalTimeOffsetOverride(std::chrono::minutes(offset));
                    }
                    tzInfo_.offset = offset;
                    tzInfo_.index = tzone.tzone_id;
                }
            }
            else {
                AIS_LOG_ERROR("Could not get tzone_tx_comm_struct");
            }

            serviceHourMeter_ = shmClock.get_SHM();
        }
    }
}

/******************************************************************************
FUNCTION NAME: LpsSaScsSendZeroRqst
DESCRIPTION:
PARAMETER DESCRIPTION: Publishes a zero request on the JobMgr Reqst channel
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::LpsSaScsSendZeroRqst()
{
    bool scsReqstRet=false;

    if (LpsSaJobMgrScsReqstOut) {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::ZERO;
        scsReqstRet = LpsSaJobMgrScsReqstOut->publish(req);
    }

    if (!scsReqstRet) {
        AIS_LOG_ERROR("LpsSaScsSendZeroRqst Fail to Send Command::ZERO");
    }
}
/******************************************************************************
FUNCTION NAME: LpsSaWeighingScsTx
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE: true if successful, else false
*******************************************************************************/
bool LpsSaWeighApp::LpsSaWeighingScsTx()
{
    bool scsCmdRet = false;

    if (nullptr != LpsSaWeighScsTxOut) {
        LpsSaWeighTxChannel txOut;

        txOut.DigStat = LpsSaWeighInfoTbl.DigStat;

        if (GetPayloadMonSysCalStatus()) {
            txOut.CalStat = LPS_WEIGH_SYSTEM_CALIBRATED;
        }
        else {
            txOut.CalStat = LPS_WEIGH_SYSTEM_UNCALIBRATED;
        }

        txOut.DumpStat = LpsSaWeighInfoTbl.DumpStat;
        txOut.BestBktWtInTonnes = LpsSaWeighInfoTbl.BestBktWtInTonnes;
        txOut.WarmupLiftsRequired = LpsRemainingWarmupLiftsRequired();
        txOut.PayloadCalcMeth = LpsSaWeighInfoTbl.PayloadCalcMeth;
        txOut.bktWtLatchedFlag = LpsSaWeighInfoTbl.BestBktWtLatched;
        txOut.latchConditionsMet = LpsSaWeighInfoTbl.LatchConditionsOK;
        txOut.zeroAvailable = LpsWeighIsZeroWeightAvailable();
        txOut.Payload = LpsSaWeighInfoTbl.Payload;
        txOut.LftSealStatus = sealTracker_.getSealStatus();
        txOut.flashEnabled = cnfg_.flashEnabled;

        { // Sensor & Work Tool Identifiers
            const auto& seal = sealTracker_.getSeal();
            txOut.LiftPositionSensorId = seal.liftPositionSensorId;
            txOut.TiltPositionSensorId = seal.tiltPositionSensorId;
            txOut.LiftHeadEndPressureSensorId = seal.liftHeadEndPressureSensorId;
            txOut.LiftRodEndPressureSensorId = seal.liftRodEndPressureSensorId;
            txOut.HydraulicOilTemperatureSensorId = seal.hydraulicOilTemperatureSensorId;
            txOut.ImuSensorId = seal.imuSerialNumber;
            txOut.WorkToolId = seal.workToolId;
            txOut.ImplementSerialNum = seal.inputModuleEcmSerialNumber;
        }

        txOut.LiftPosition = LpsSaWeighInfoTbl.LiftPosition;
        txOut.LiftCylVel = LpsSaWeighInfoTbl.LiftCylVel;
        txOut.TiltPosition = LpsSaWeighInfoTbl.TiltPosition;
        txOut.WeighRange.WeighRangeBottom = cnfg_.weighRangeStart;
        txOut.WeighRange.WeighRangeSize = cnfg_.weighRangeSize;
        txOut.PidData.OverloadWarningEnabled = cnfg_.overloadWarningEnabled;
        txOut.CAN11MessageTimeoutFlag = LpsSaWeighInfoTbl.CAN11MessageTimeoutFlag;
        txOut.ToaAnchoredZeroOffset = cnfg_.toaAnchoredZeroOffset;
        txOut.ToaAnchoredFactor = cnfg_.toaAnchoredFactor;
        txOut.ToaAnchorStatus = LpsSaWeighInfoTbl.ToaAnchorStatus;

        if (chassisImu_.imuOk) {
            auto calibratedLinAccelVector = chassisImu_.imu.calibratedLinAccelVector();
            txOut.PidData.MachineRearLateralAcceleration = TO_INT16_PID(calibratedLinAccelVector.y() * 100.f);
            txOut.PidData.MachineRearLongitudinalAcceleration = TO_INT16_PID(calibratedLinAccelVector.x() * 100.f);
            txOut.PidData.MachineRearVerticalAcceleration = TO_INT16_PID(calibratedLinAccelVector.z() * 100.f);

            txOut.PidData.MachinePitch = TO_INT16_PID(chassisImu_.imu.pitchDegrees() * 10.f);
            txOut.PidData.MachineSlope = TO_INT16_PID(chassisImu_.imu.pitchGrade() * 10.f);

            /*
             * The PID definitions for roll are really bad.
             * What they call side slope is not really side slope of the ground, but roll
             * of the machine about the machine x-axis, converted to grade units.
             * This is different than the slope of the ground from the "world" reference frame where the x-axis
             * is perpendicular to the gravity vector instead of in the direction of machine motion.
             */
            txOut.PidData.MachineRearRoll = TO_INT16_PID(chassisImu_.imu.rollDegrees() * 100.f);
            txOut.PidData.MachineRearSideSlope = TO_INT16_PID(chassisImu_.imu.rollGrade() * 10.f);
            txOut.PidData.MachineRoll = txOut.PidData.MachineRearRoll;
            txOut.PidData.MachineSideSlope = TO_INT16_PID(-txOut.PidData.MachineRearSideSlope);
        }
        else {
            txOut.PidData.MachineRearLateralAcceleration = UNKNOWN2S + FMICNM; /* dsi */
            txOut.PidData.MachineRearLongitudinalAcceleration = UNKNOWN2S + FMICNM; /* dsi */
            txOut.PidData.MachineRearVerticalAcceleration = UNKNOWN2S + FMICNM; /* dsi */

            txOut.PidData.MachinePitch = UNKNOWN2S + FMICNM;
            txOut.PidData.MachineSlope = UNKNOWN2S + FMICNM;
            txOut.PidData.MachineRearRoll = UNKNOWN2S + FMICNM;
            txOut.PidData.MachineRearSideSlope = UNKNOWN2S + FMICNM;
            txOut.PidData.MachineRoll = UNKNOWN2S + FMICNM;
            txOut.PidData.MachineSideSlope = UNKNOWN2S + FMICNM;
        }

        txOut.PidData.TipoffPitchCalOffset = cnfg_.tipoffPitchCalOffset;
        txOut.PidData.HydOilTempEnabled = cnfg_.hydOilTempEnabled;
        txOut.PidData.AudibleWeightEnabled = cnfg_.audibleWeightEnabled;

        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::LOWER_STALL] = LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_LOWER_STALL];
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::RAISE_STALL] = LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_RAISE_STALL];
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::INSUFFICIENT_DATA] = false;
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::REWEIGH_PRESSURE_CHANGING] = LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_PRESSURE_CHANGING];
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::REWEIGH_INCONSISTENT] = LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_INCONSISTENT];
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::REWEIGH_SPEED_CHANGING] = LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_SPEED_CHANGING];
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::REWEIGH_NOT_RACKED_EXCESSIVE_PITCH] =
                LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_NOT_RACKED] ||
                LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_EXCESSIVE_PITCH];
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::REWEIGH_STOPPED_IN_RANGE] = LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_STOPPED_IN_RANGE];
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::REWEIGH_LIFT_TOO_SLOW] = LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_LIFT_TOO_SLOW];
        txOut.PidData.ProdMeasureWeighStatus[ACDWeighStatus::INSUFFICIENT_DATA] = LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_WARMUP_LIFT];
        /* send audible tone command */
        txOut.PidData.AudibleWeightCommand = getAudibleCommand();

        txOut.PidData.PloadSysCalWtEntryReqStat = WeighPidTbl.PloadSysCalWtEntryReqStat;

        if (CAL_ENTRY_REQUIRED == WeighPidTbl.PloadSysCalWtEntryReqStat) {
            txOut.PidData.LastPloadWt = cnfg_.lastSuggestedCalWeight;
        }
        else {
            txOut.PidData.LastPloadWt = -1.f;
        }

        txOut.CalWt = payloadCalNvmTbl_.data.CalWeight;
        txOut.Indicator =  LpsSaWeighInfoTbl.Indicator;
        txOut.LiftStalled = LpsWrk.LpsStallDetect.liftStalled;
        txOut.LiftValveCommand = LpsSaWeighInfoTbl.LiftValveCommand;
        txOut.TiltValveCommand = LpsSaWeighInfoTbl.TiltValveCommand;
        txOut.ZeroWeight = payloadCalNvmTbl_.data.ZeroWeight;
        txOut.SimpleCalAdjust = payloadCalNvmTbl_.data.CalAdjust;
        txOut.EventState = LpsSaWeighInfoTbl.EventState;
        txOut.DiagState = LpsSaWeighInfoTbl.DiagState;
        txOut.InfoState = LpsSaWeighInfoTbl.InfoState;

        if (txOut.DiagState.none()) {
            // No diagnostics, check battery voltage events and imu w/lft
            if (STATUS_GOOD != LpsChkInputStat()) {
                txOut.PidData.BktPayloadData = BUCKET_PAYLOAD_NOT_AVAILABLE;                    
                txOut.ShowExclamationPoint = true;
                txOut.BucketFullyRacked = true;
                txOut.ExcessivePitch = false;
            }
            else {            
                txOut.ShowExclamationPoint = false;
                txOut.BucketFullyRacked = LpsWeighFullRackDetectStrict();
                txOut.ExcessivePitch = LpsGetExcessivePitchStatus();
                txOut.PidData.BktPayloadData = BUCKET_PAYLOAD_AVAILABLE;
            }
        }
        else {
            // We have at least one diagnostic.
            txOut.ShowExclamationPoint = true;
            txOut.BucketFullyRacked = true;
            txOut.ExcessivePitch = false;

            // Don't show warm-up required if there is any diagnostic
            txOut.WarmupLiftsRequired = 0;

            if (txOut.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_NOT_INSTALLED]) {
                // Suppress all other info/event/diag if SEA is not installed/enabled
                txOut.DiagState.reset();
                txOut.InfoState.reset();
                txOut.EventState.reset();
                txOut.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_NOT_INSTALLED] = true;

                txOut.PidData.BktPayloadData = BUCKET_PAYLOAD_NOT_INSTALLED;

                // Don't say calibration weight entry is required if not installed.
                txOut.PidData.PloadSysCalWtEntryReqStat = CAL_ENTRY_NOT_REQUIRED;
            }
            else {
                // We are installed with at least one diagnostic.
                txOut.PidData.BktPayloadData = BUCKET_PAYLOAD_NOT_AVAILABLE;
            }
        }

        /* engine speed to auto-cals */
        txOut.EngineSpeedRPM = LpsSaWeighInfoTbl.EngineSpeedRPM;

        /*Production Measurement Sensor Status*/
        txOut.PidData.ProdMeasureSensorStatus.LiftLinkDC = WeighPidTbl.LiftLinkageSensorDc; // Lift Linkage Position Sensor Duty Cycle

        if (LpsSaWeighInfoTbl.LiftPosition.status == LPS_STATUS_BAD) {
            txOut.PidData.ProdMeasureSensorStatus.LiftCylPos = UNKNOWN2U + FMICNM;
        }
        else {
            txOut.PidData.ProdMeasureSensorStatus.LiftCylPos = weighUpdtTbl.LiftCylLengthNorm.Val;/*Lift Cylinder Position*/
        }

        if (weighUpdtTbl.LiftCylHePres.Stat == LPS_STATUS_BAD) {
            txOut.PidData.ProdMeasureSensorStatus.LiftCylHEPres = UNKNOWN2U + FMICNM;
        }
        else {
            txOut.PidData.ProdMeasureSensorStatus.LiftCylHEPres = weighUpdtTbl.LiftCylHePres.Val;/*Lift Cylinder Head End Pressure*/
        }

        if (weighUpdtTbl.LiftCylRePres.Stat == LPS_STATUS_BAD) {
            txOut.PidData.ProdMeasureSensorStatus.LiftCylREPres = UNKNOWN2U + FMICNM;
        }
        else {
            txOut.PidData.ProdMeasureSensorStatus.LiftCylREPres = weighUpdtTbl.LiftCylRePres.Val;/*Lift Cylinder Rod End Pressure*/
        }

        txOut.PidData.ProdMeasureSensorStatus.TiltSensorConfig = (linkage_table_cnfg.tiltSensorType == TILT_SENSOR_TYPE_ROTARY) ?
            LpsSaWeighTxChannel::ROTARY_POSITION_SENSOR : LpsSaWeighTxChannel::IN_CYLINDER_POSITION_SENSOR;

        if (LpsSaWeighInfoTbl.TiltCylHePres.Stat == LPS_STATUS_BAD) {
            txOut.PidData.ProdMeasureSensorStatus.TiltCylHEPres = UNKNOWN2U + FMICNM;
        }
        else {
            txOut.PidData.ProdMeasureSensorStatus.TiltCylHEPres = LpsSaWeighInfoTbl.TiltCylHePres.Val;/*Tilt Cylinder Head End Pressure*/
        }

        if (LpsSaWeighInfoTbl.TiltCylRePres.Stat == LPS_STATUS_BAD) {
            txOut.PidData.ProdMeasureSensorStatus.TiltCylREPres = UNKNOWN2U + FMICNM;
        }
        else {
            txOut.PidData.ProdMeasureSensorStatus.TiltCylREPres = LpsSaWeighInfoTbl.TiltCylRePres.Val;/*Tilt Cylinder Rob End Pressure*/
        }

        txOut.PidData.ProdMeasureSensorStatus.TiltLinkDC = WeighPidTbl.TiltLinkageSensorDc; // Tilt Linkage Position Sensor Duty Cycle
        txOut.PidData.ProdMeasureSensorStatus.HydOilTemp = WeighPidTbl.HydOilTemp;
        /* Linkage Sensor Calibrated Limits [-75722]*/
        txOut.PidData.LinkSensorCalLim.LiftPosSensorFullRaiseDC = liftCalNvmTbl_.lift_full_raise_dc; // Lift Linkage Position Sensor Full Raise Duty Cycle
        txOut.PidData.LinkSensorCalLim.LiftPosSensorFullLowerDC = liftCalNvmTbl_.lift_full_lower_dc; // Lift Linkage Position Sensor Full Lower Duty Cycle
        txOut.PidData.LinkSensorCalLim.TiltPosSensorFullRackDC = tiltCalNvmTbl_.tilt_full_rack_dc; // Tilt Linkage Position Sensor Full Rackback Duty Cycle
        txOut.PidData.LinkSensorCalLim.TiltPosSensorFullDumpDC = tiltCalNvmTbl_.tilt_full_dump_dc; // Tilt Linkage Position Sensor Full Dump Duty Cycle
        txOut.PidData.PloadSysZeroStat = GetZeroStat() ;/*Payload Remove Last Pass Button Display Status*/

        txOut.PidData.LoaderBktPloadTgtWt = cnfg_.bucketPayloadTargetWeight;
        txOut.PidData.LoaderBktPloadTgtWtPer = LpsWeighGetBucketLoadFactor();

        WeighPidTbl.PloadSysZeroReqStat = LpsWeighGetAutoZeroUpdateStatus();
        txOut.PidData.PloadSysZeroReqStat = static_cast<uint16_t>(WeighPidTbl.PloadSysZeroReqStat);/* Payload System Zero Requirement Status */
        txOut.PidData.QR_HydOilTempMin_C = WeighPidTbl.QR_HydOilTempMin_C;;
        txOut.PidData.QR_LiftCylVelMin_mm_sec = WeighPidTbl.QR_LiftCylVelMin_mm_sec;
        txOut.PidData.QR_LiftCylVelMax_mm_sec = WeighPidTbl.QR_LiftCylVelMax_mm_sec;

        txOut.PayloadCalInProgress = LpsCalCalInProgress();

        txOut.TestStatus = testFixture_.getTestStatus();

        /* Broadcasting Weighing App elements */
        scsCmdRet = LpsSaWeighScsTxOut->publish(txOut);
    }

    if (!scsCmdRet) {
        AIS_LOG_WARN("'LpsSaWeighingScsTx' function return code = %d", scsCmdRet);
    }

    return scsCmdRet;
}

/******************************************************************************
FUNCTION NAME: LpsSaWeighScsInitDebugTx
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::LpsSaWeighScsInitDebugTx()
{
    LpsSaWeighInitDebugChannel txOut;
    if (LpsSaWeighScsInitDebugOut )
    {
        /* Fill in the data to publish */
        txOut.m_WeighInitTbl = LpsWrk.InitTbl;

        /* Broadcasting Weighing App elements */
        LpsSaWeighScsInitDebugOut->publish( txOut );
    }
    else
    {
        /* do nothing */
    }
}

/******************************************************************************
FUNCTION NAME: LpsSaWeighScsDebugTx
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
 *******************************************************************************/
void LpsSaWeighApp::LpsSaWeighScsDebugTx()
{
    if (LpsSaWeighScsDebugOut) {
        // Check for a new weigh range weight and make a log entry.
        if (LPS_WEIGH_WRW_STATUS_VALID == LpsWrk.WrwTbl.OpTbl.Status) {
            bool newWeighRangeWeighResults = false;

            if (LPS_WEIGH_WRW_STATUS_VALID != DebugLpsSaXCPChannels.m_LpsWrk.WrwTbl.OpTbl.Status) {
                // It previously was not valid, must be new
                newWeighRangeWeighResults = true;
            }
            else if ((LPS_IN_WEIGH_RANGE_WEIGHING == DebugLpsSaXCPChannels.m_LpsWrk.WrwTbl.OpTbl.Indicator) &&
                    (LPS_ABOVE_WEIGH_RANGE == LpsWrk.WrwTbl.OpTbl.Indicator)) {
                // We were in the weigh range weighing, now we are above the weigh range.
                newWeighRangeWeighResults = true;
            }

            if (newWeighRangeWeighResults) {
                // We have a new weigh range weigh available.
                logWeighRangeWeighUpdate();
            }
        }

        // Fill in the data to publish
        DebugLpsSaXCPChannels.m_LpsWrk = LpsWrk;

        DebugLpsSaXCPChannels.DebugLiftCylHePress = weighUpdtTbl.LiftCylHePres.Val;
        DebugLpsSaXCPChannels.DebugLiftCylHePressStatus = weighUpdtTbl.LiftCylHePres.Stat;
        DebugLpsSaXCPChannels.DebugLiftCylRePress = weighUpdtTbl.LiftCylRePres.Val;
        DebugLpsSaXCPChannels.DebugLiftCylRePressStatus = weighUpdtTbl.LiftCylRePres.Stat;

        DebugLpsSaXCPChannels.DebugTiltCylHePress = LpsSaWeighInfoTbl.TiltCylHePres.Val;
        DebugLpsSaXCPChannels.DebugTiltCylHePressStatus = LpsSaWeighInfoTbl.TiltCylHePres.Stat;
        DebugLpsSaXCPChannels.DebugTiltCylRePress = LpsSaWeighInfoTbl.TiltCylRePres.Val;
        DebugLpsSaXCPChannels.DebugTiltCylRePressStatus = LpsSaWeighInfoTbl.TiltCylRePres.Stat;

        DebugLpsSaXCPChannels.DebugLiftCylRawExt  = LpsSaWeighInfoTbl.LiftPosition.cylinderLength;
        DebugLpsSaXCPChannels.DebugLiftCylFiltExt = LpsSaWeighInfoTbl.LiftPosition.cylinderLength;
        DebugLpsSaXCPChannels.DebugLiftCylNormLen = LpsSaWeighInfoTbl.LiftPosition.percentCylinderLength;
        DebugLpsSaXCPChannels.DebugLiftCylVel = LpsSaWeighInfoTbl.LiftCylVel.Val;
        DebugLpsSaXCPChannels.DebugLiftAngle = LpsSaWeighInfoTbl.LiftPosition.angle;
        DebugLpsSaXCPChannels.DebugLiftAngVel = LpsSaWeighInfoTbl.LiftAngVel.Val;
        DebugLpsSaXCPChannels.DebugLiftPositionStatus = LpsSaWeighInfoTbl.LiftPosition.status;

        DebugLpsSaXCPChannels.DebugTiltCylRawLen = LpsSaWeighInfoTbl.TiltPosition.cylinderLength;
        DebugLpsSaXCPChannels.DebugTiltCylNormLen = LpsSaWeighInfoTbl.TiltPosition.percentCylinderLength;
        DebugLpsSaXCPChannels.DebugTiltCylVel = LpsSaWeighInfoTbl.TiltCylVel.Val;
        DebugLpsSaXCPChannels.DebugTiltAngle = LpsSaWeighInfoTbl.TiltPosition.angle;
        DebugLpsSaXCPChannels.DebugTiltAngleABC = LpsSaWeighInfoTbl.TiltPosition.percentAngle;
        DebugLpsSaXCPChannels.DebugTiltPositionStatus = LpsSaWeighInfoTbl.TiltPosition.status;

        DebugLpsSaXCPChannels.DebugBucketAngle = weighUpdtTbl.BktAngle.Val;
        DebugLpsSaXCPChannels.DebugBucketAngleStatus = weighUpdtTbl.BktAngle.Stat;
        DebugLpsSaXCPChannels.DebugHydOilTemp = weighUpdtTbl.HydOilTemp.Val;
        DebugLpsSaXCPChannels.DebugHydOilTempStatus = weighUpdtTbl.HydOilTemp.Stat;
        DebugLpsSaXCPChannels.DebugRequestedGear = weighUpdtTbl.RequestedGear.Val;
        DebugLpsSaXCPChannels.DebugRequestedGearStatus = weighUpdtTbl.RequestedGear.Stat;
        DebugLpsSaXCPChannels.DebugWeighUpdtLoaderBktPayldTrgtWt = weighUpdtTbl.LoaderBktPayldTrgtWt;
        DebugLpsSaXCPChannels.DebugWeighUpdtClockKeyonSec = weighUpdtTbl.ClockKeyonSec;
        DebugLpsSaXCPChannels.DebugWeighUpdtPassCount = weighUpdtTbl.PassCount;

        { // Get gravity vector
            auto gravityVector = chassisImu_.imu.gravityVector();
            DebugLpsSaXCPChannels.chassisImuGravityX = gravityVector.x();
            DebugLpsSaXCPChannels.chassisImuGravityY = gravityVector.y();
            DebugLpsSaXCPChannels.chassisImuGravityZ = gravityVector.z();
        }

        { // Get estimated velocity
            auto estimatedLinVelVector = chassisImu_.imu.estimatedLinVelVector();
            DebugLpsSaXCPChannels.chassisImuVelocityX = estimatedLinVelVector.x();
            DebugLpsSaXCPChannels.chassisImuVelocityY = estimatedLinVelVector.y();
            DebugLpsSaXCPChannels.chassisImuVelocityZ = estimatedLinVelVector.z();
        }

        { // Get IMU testpoints - Bias Correction
            DebugLpsSaXCPChannels.chassisImuBiasMagJerk = chassisImu_.imu.magJerk_;
            DebugLpsSaXCPChannels.chassisImuBiasMagAngAccel = chassisImu_.imu.magAngAccel_;
            DebugLpsSaXCPChannels.chassisImuBiasMaxAngVel = chassisImu_.imu.maxAngVel_;

            auto biasAngVel = chassisImu_.imu.biasAngVelVector();
            DebugLpsSaXCPChannels.chassisImuBiasAngVelX = biasAngVel.x();
            DebugLpsSaXCPChannels.chassisImuBiasAngVelY = biasAngVel.y();
            DebugLpsSaXCPChannels.chassisImuBiasAngVelZ = biasAngVel.z();
        }

        { // Get IMU testpoints - Sensor Linear Acceleration
            auto sensorLinAcc = chassisImu_.sensorLinAcc;
            DebugLpsSaXCPChannels.chassisImuLinAccX = sensorLinAcc.x();
            DebugLpsSaXCPChannels.chassisImuLinAccY = sensorLinAcc.y();
            DebugLpsSaXCPChannels.chassisImuLinAccZ = sensorLinAcc.z();
        }

        { // Get IMU testpoints - Sensor Angular Velocity
            auto sensorAngVel = chassisImu_.sensorAngVel;
            DebugLpsSaXCPChannels.chassisImuAngVelX = sensorAngVel.x();
            DebugLpsSaXCPChannels.chassisImuAngVelY = sensorAngVel.y();
            DebugLpsSaXCPChannels.chassisImuAngVelZ = sensorAngVel.z();
        }

        // Get IMU pitch and roll
        DebugLpsSaXCPChannels.chassisImuPitch = chassisImu_.imu.pitchDegrees();
        DebugLpsSaXCPChannels.chassisImuRoll = chassisImu_.imu.rollDegrees();

        // Get raw IMU sensor status
        DebugLpsSaXCPChannels.chassisImuSensorLinAccStatus = chassisImu_.sensorLinAccStatus;
        DebugLpsSaXCPChannels.chassisImuSensorAngVelStatus = chassisImu_.sensorAngVelStatus;

        // Broadcasting Weighing App elements
        LpsSaWeighScsDebugOut->publish(DebugLpsSaXCPChannels);
    }
}

/******************************************************************************
FUNCTION: PublishCalFromNvmPayload
DESCRIPTION: Prepares an SCS object with the key-on available calibration data and send the data out
PARAMETER DESCRIPTION: None
RETURN VALUE: None
*******************************************************************************/
void LpsSaWeighApp::PublishCalFromNvmPayload( void )
{
   LpsSaNvmCalDataChannel scsPayload; // Default constructor clears the initial container
   AIS_LOG_INFO("Populating SCS object with key-on NVM calibration data");

   memcpy(scsPayload.lps_sa_nvm_calibration_data_main.raise_slow_empty_lift_heights,
           payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt,
           sizeof(scsPayload.lps_sa_nvm_calibration_data_main.raise_slow_empty_lift_heights));
   memcpy(scsPayload.lps_sa_nvm_calibration_data_main.raise_slow_empty_pressures,
           payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres,
           sizeof(scsPayload.lps_sa_nvm_calibration_data_main.raise_slow_empty_pressures));
   memcpy(scsPayload.lps_sa_nvm_calibration_data_main.lower_slow_empty_lift_heights,
           payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt,
           sizeof(scsPayload.lps_sa_nvm_calibration_data_main.lower_slow_empty_lift_heights));
   memcpy(scsPayload.lps_sa_nvm_calibration_data_main.lower_slow_empty_pressures,
           payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres,
           sizeof(scsPayload.lps_sa_nvm_calibration_data_main.lower_slow_empty_pressures));
   memcpy(scsPayload.lps_sa_nvm_calibration_data_main.raise_slow_full_lift_heights,
           payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt,
           sizeof(scsPayload.lps_sa_nvm_calibration_data_main.raise_slow_full_lift_heights));
   memcpy(scsPayload.lps_sa_nvm_calibration_data_main.raise_slow_full_pressures,
           payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres,
           sizeof(scsPayload.lps_sa_nvm_calibration_data_main.raise_slow_full_pressures));
   memcpy(scsPayload.lps_sa_nvm_calibration_data_main.lower_slow_full_lift_heights,
           payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt,
           sizeof(scsPayload.lps_sa_nvm_calibration_data_main.lower_slow_full_lift_heights));
   memcpy(scsPayload.lps_sa_nvm_calibration_data_main.lower_slow_full_pressures,
           payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres,
           sizeof(scsPayload.lps_sa_nvm_calibration_data_main.lower_slow_full_pressures));

   scsPayload.lps_sa_nvm_calibration_data_main.raise_speed_empty_slow = payloadCalNvmTbl_.data.EmptyBktSlowRaiseSpd;
   scsPayload.lps_sa_nvm_calibration_data_main.raise_speed_empty_fast = payloadCalNvmTbl_.data.EmptyBktFastRaiseSpd;
   scsPayload.lps_sa_nvm_calibration_data_main.raise_fast_delta_p_empty = payloadCalNvmTbl_.data.FastRaiseEmptyBktDeltaPres;
   scsPayload.lps_sa_nvm_calibration_data_main.raise_speed_full_slow = payloadCalNvmTbl_.data.FullBktSlowRaiseSpd;
   scsPayload.lps_sa_nvm_calibration_data_main.raise_speed_full_fast = payloadCalNvmTbl_.data.FullBktFastRaiseSpd;
   scsPayload.lps_sa_nvm_calibration_data_main.raise_fast_delta_p_full = payloadCalNvmTbl_.data.FastRaiseFullBktDeltaPres;

   scsPayload.lps_sa_nvm_calibration_data_debug.lift_full_lower_DC_inf_value = static_cast<int_32>(liftCalNvmTbl_.lift_full_lower_dc);
   scsPayload.lps_sa_nvm_calibration_data_debug.lift_full_raise_DC_inf_value = static_cast<int_32>(liftCalNvmTbl_.lift_full_raise_dc);
   scsPayload.lps_sa_nvm_calibration_data_debug.tilt_full_dump_DC_inf_value = static_cast<int_32>(tiltCalNvmTbl_.tilt_full_dump_dc);
   scsPayload.lps_sa_nvm_calibration_data_debug.tilt_full_rack_DC_inf_value = static_cast<int_32>(tiltCalNvmTbl_.tilt_full_rack_dc);
   scsPayload.lps_sa_nvm_calibration_data_debug.tilt_dump_stop_angle_inf_value = static_cast<int_32>(tiltCalNvmTbl_.tilt_full_dump_stop_angle);
   scsPayload.lps_sa_nvm_calibration_data_debug.tilt_rack_stop_angle_inf_value = static_cast<int_32>(tiltCalNvmTbl_.tilt_full_rack_stop_angle);

   scsPayload.lps_sa_nvm_calibration_data_debug.pcs_cal_weight = payloadCalNvmTbl_.data.CalWeight;
   scsPayload.lps_sa_nvm_calibration_data_debug.zero_weight = payloadCalNvmTbl_.data.ZeroWeight;
   scsPayload.lps_sa_nvm_calibration_data_debug.cal_adjust = payloadCalNvmTbl_.data.CalAdjust;
   scsPayload.lps_sa_nvm_calibration_data_debug.empty_temp = payloadCalNvmTbl_.data.EmptyTemp;
   scsPayload.lps_sa_nvm_calibration_data_debug.full_temp = payloadCalNvmTbl_.data.FullTemp;
   scsPayload.lps_sa_nvm_calibration_data_debug.empty_bucket_weight_est = payloadCalNvmTbl_.data.EmptyBucketWeightEst;

   scsPayload.lps_sa_nvm_calibration_data_debug.hyd_oil_type_index = static_cast<int_32>(LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.HydOilType);

   // Read IMU Cal Results from file
   LpsCalIMUResults_t imu_cal_results = {};
   readIMUCalResultsFromFile(imu_cal_results);
   scsPayload.imu_cal_results = imu_cal_results;

   /* The following fields are not yet populated:
    * lps_sa_nvm_calibration_data_debug.pcs_vel_slope
    * lps_sa_nvm_calibration_data_debug.temp_slope
    * lps_sa_nvm_calibration_data_debug.cal_adjust_temp
    */

   LpsNvmDumpChanOut->publish(scsPayload);
}

