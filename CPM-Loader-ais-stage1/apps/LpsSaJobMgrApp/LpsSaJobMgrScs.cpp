/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaJobMgrScs.cc
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include "LpsSaJobMgrApp.h"

#include <chrono>
#include <unordered_set>

#include <scl_dcli_svc_types.h>

#include <chrono/print.hpp>
#include <chrono/tz.hpp>
#include <interfaces/DataLinkData/SecuritySystemCurrentKeyParam.h>
#include <interfaces/DataLinkData/InterfaceTypes.h>
#include <interfaces/EventDiagnosticData/EventDiagnosticData.h>
#include <LoaderdiagnosticEventConfig/Loader_autonomy_event_config.h>

#include <fileio/oflocker.hpp>

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
#define STG4 3                        //STG4 is index 3 in the array of STG_values.
#define HORN_PORT_NUMBER   OutputChannel::Port::OUTPUT_SINK_3    //OUTPUT_APP_PORT_SINK3_PIN29_GPIO88

#define PAYLOAD_DETAIL_JSON_FILENAME (R"(/tmp/appdata/CPM/LpsSaJobMgrApp/PayloadDetails.json)")

/*******************************************************************************
** -- Function Prototypes --
*******************************************************************************/
/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/
/******************************************************************************
FUNCTION LpsSaJobMgrApp::LpsSaJobMgrScsChkForReqst
DESCRIPTION: It will check the request from UI through SCS channel by polling method
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrScsChkForReqst()
{
    // Check for requests that have already been made and let those get handled.
    if (TRUE == LpsSaJobMgrWmInput.clear_request_status) {
        // Can only handle one request at a time.
        return;
    }

    { // Check for store request
        SwitchInputScs STG_Input;
        bool storeRequest = false;

        /* The store button has higher priority compared to LPS Job Mgr request. */
        while (LpsSaSwitchInput->get(STG_Input)) {
            if (STG_Input.get_STG_value(STG4) == STG::CLOSED) {
                storeRequest = true;
            }
        }

        if (LpsJobMgrJobTrackerInfoTbl.StorePIDActive) {
            storeRequest = true;
            LpsJobMgrJobTrackerInfoTbl.StorePIDActive = false;
        }

        if (storeRequest) {
            if (LpsJobMgrJobTrackerInfoTbl.OperationMode == LPS_SA_JOB_MGR_STANDBY_MODE) {
                // We are in standby, ignore store
                storeRequest = false;
            }
            else {
                // We are not in standby, check LFT store allow status
                auto status = getLftStoreAllowedStatus();
                if (LftStoreAllowedStatus_t::ALLOW_STORE != status) {
                    if (LftStoreAllowedStatus_t::REJECT_STORE_LOW_ACCURACY == status) {
                        storeRejectedExpireTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
                    }
                    storeRequest = false;
                }
            }
        }

        /*
         * If store request has happened (either through STG or datalink), then do store
         * before proceeding with any other requests
         */
        if (storeRequest) {
            LpsSaJobMgrWmInput.store_request_status = true;
            return;
        }
    }

    {
        /* check for tipoff assist state and process command to enter/exit tipoff
         * immediately
         */
        if (LpsJobMgrJobTrackerInfoTbl.TipoffAssistPIDActive &&
                !LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive) {
            /*
             * Enter Manual Tip-Off
             * If we are not in tip-off, then enter tip-off
             * If we are already in tip-off, then enter it "again"
             *   - unlatch bucket weight
             *   - disable auto tip-off exit conditions
             */
            LpsSaJobMgrWmInput.change_mode_excess = true;
            return;
        }
        else if (!LpsJobMgrJobTrackerInfoTbl.TipoffAssistPIDActive &&
                LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive) {
            /*
             * Exit Manual Tip-Off
             */
            if (LpsSaJobMgrWmOutput.tip_off_active) {
                LpsSaJobMgrWmInput.change_mode_weigh = true;
                return;
            }
        }
        else {
            /* no change in Tipoff Assist state */
        }
    }

    // Check for manual add PID
    if (LpsJobMgrJobTrackerInfoTbl.ManualAddPIDActive) {
    	LpsSaJobMgrWmInput.manual_add_request = true;
    	LpsJobMgrJobTrackerInfoTbl.ManualAddPIDActive = false;
    	return;
    }

    // Read display state and go into standby if we are in verification mode.
    if (nullptr != displayStateInput_) {
        LpsSaUIDisplayStateInterface displayState;
        while (displayStateInput_->get(displayState)) {
            const LpsSaUIDisplayState& state = displayState.state;
            LpsJobMgrJobTrackerInfoTbl.inVerificationMode = state.isInVerificationMode();
            if (LpsJobMgrJobTrackerInfoTbl.inVerificationMode &&
                    (LpsJobMgrJobTrackerInfoTbl.OperationMode != LPS_SA_JOB_MGR_STANDBY_MODE)) {
                LpsSaJobMgrWmInput.standby_request_status = TRUE;
                AIS_LOG_NOTICE("Enter standby when in verification mode.");
            }
        }
    }

    LpsSaJobMgrReqstChannel reqIn;
    bool loadRecordChanged = false;
    bool configChanged = false;

    /* Retrieve SCS channel data */
    while (LpsSaJobMgrScsReqstIn->get(reqIn)) {
        bool success = true;

        // Select target type first because this sets the context for the sub-requests.
        if (LpsSaJobMgrReqstChannel::Command::WRITE_TARGET_TYPE == reqIn.command) {
            tasks_.setTargetType(reqIn.data.targetType);
            LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck
            AIS_LOG_NOTICE("Command::WRITE_TARGET_TYPE success: %d", reqIn.data.targetType);
            loadRecordChanged = true;
        }

        /*
         * Check the *new* style requests and handle them
         */
        for (const auto& r : reqIn.requests) {
            switch (r.command) {
            case (LpsSaJobMgrReqstCommand::WRITE_MATERIAL_ID): {
                tasks_.getCurrentTaskLoad().setMaterialId(r.materialId());
                AIS_LOG_NOTICE("Load Change - materialId");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_MATERIAL_NAME): {
                tasks_.getCurrentTaskLoad().setMaterialName(r.materialName());
                AIS_LOG_NOTICE("Load Change - materialName");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_MATERIAL_DENSITY): {
                tasks_.getCurrentTaskLoad().setMaterialDensity(r.materialDensity());
                AIS_LOG_NOTICE("Load Change - materialDensity");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_TRUCK_ID): {
                tasks_.getCurrentTaskLoad().setTruckId(r.truckId());
                AIS_LOG_NOTICE("Load Change - truckId");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_TRUCK_NAME): {
                tasks_.getCurrentTaskLoad().setTruckName(r.truckName());
                AIS_LOG_NOTICE("Load Change - truckName");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_TRUCK_TARGET_WEIGHT): {
                tasks_.getCurrentTaskLoad().setTotalTargetWeight(r.truckTargetWeight());
                AIS_LOG_NOTICE("Load Change - truckTargetWeight");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_TAG1): {
                tasks_.getCurrentTaskLoad().setTag1(r.tagName(), r.tagValue());
                AIS_LOG_NOTICE("WRITE_TAG1 = %s (%s)", r.tagValue().c_str(), r.tagName().c_str());
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_TAG2): {
                tasks_.getCurrentTaskLoad().setTag2(r.tagName(), r.tagValue());
                AIS_LOG_NOTICE("WRITE_TAG2 = %s (%s)", r.tagValue().c_str(), r.tagName().c_str());
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_TAG3): {
                tasks_.getCurrentTaskLoad().setTag3(r.tagName(), r.tagValue());
                AIS_LOG_NOTICE("WRITE_TAG3 = %s (%s)", r.tagValue().c_str(), r.tagName().c_str());
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (LpsSaJobMgrReqstCommand::WRITE_TAG4): {
                tasks_.getCurrentTaskLoad().setTag4(r.tagName(), r.tagValue());
                AIS_LOG_NOTICE("WRITE_TAG4 = %s (%s)", r.tagValue().c_str(), r.tagName().c_str());
                loadRecordChanged = true;
                break; // out of switch-case
            }
            default: {
                break; // out of switch-case
            }
            }
        }

        bool breakOut = false;

        switch (reqIn.command) {
        case (LpsSaJobMgrReqstChannel::Command::ZERO): {
            /* Request received for Weighing App,set the request flag */
            LpsSaJobMgrWmInput.zero_request_status = TRUE;
            AIS_LOG_NOTICE("Command::ZERO");
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::MINUS_ONE): {
            /* Request received for Weighing App,set the request flag */
            LpsSaJobMgrWmInput.minus_one_request_status = TRUE;
            AIS_LOG_NOTICE("Command::MINUS_ONE");
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::CLEAR): {
            /* Request received for Weighing App,set the request flag */
            LpsSaJobMgrWmInput.clear_request_status = TRUE;
            AIS_LOG_NOTICE("Command::CLEAR");
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::STORE): {
            bool storeRequest = true;

            if (LpsJobMgrJobTrackerInfoTbl.OperationMode == LPS_SA_JOB_MGR_STANDBY_MODE) {
                // We are in standby, ignore store
                storeRequest = false;
            }
            else {
                // We are not in standby, check LFT store allow status
                auto status = getLftStoreAllowedStatus();
                if (LftStoreAllowedStatus_t::ALLOW_STORE != status) {
                    if (LftStoreAllowedStatus_t::REJECT_STORE_LOW_ACCURACY == status) {
                        storeRejectedExpireTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
                    }
                    storeRequest = false;
                }
            }

            if (storeRequest) {
                LpsSaJobMgrWmInput.store_request_status = true;
            }

            AIS_LOG_NOTICE("Command::STORE");
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_PAYLOAD_NEXT_SUBTOTAL): {
            /* set the request flag */
            LpsJobMgrJobTrackerInfoTbl.splitModeNextPayloadCmd = true;

            //for lps_tracker, make it look like a store so the subtotal is closed
            LpsSaJobMgrWmInput.store_request_status = TRUE;

            AIS_LOG_NOTICE("Command::WRITE_PAYLOAD_NEXT_SUBTOTAL");
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TARGET_TYPE): {
            // This case is handled outside of the switch
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TOTAL_TARGET_WEIGHT): {

            tasks_.getCurrentTaskLoad().setTotalTargetWeight(reqIn.data.totalTargetWeight);
            AIS_LOG_NOTICE("Command::WRITE_TOTAL_TARGET_WEIGHT success: %f", reqIn.data.totalTargetWeight);
            loadRecordChanged = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::STANDBY_ACTIVATE): {
            /* set the request flag */
            // ONLY DO THIS IF...
            if (LpsJobMgrJobTrackerInfoTbl.OperationMode != LPS_SA_JOB_MGR_STANDBY_MODE) {
                LpsSaJobMgrWmInput.standby_request_status = TRUE;
                AIS_LOG_NOTICE("Command::STANDBY_ACTIVATE");
                breakOut = true;
            }
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::STANDBY_DEACTIVATE): {
            /* set the request flag */
            // ONLY DO THIS IF...
            if ((LpsJobMgrJobTrackerInfoTbl.OperationMode == LPS_SA_JOB_MGR_STANDBY_MODE) &&
                    (!LpsJobMgrJobTrackerInfoTbl.inVerificationMode)) {
                LpsSaJobMgrWmInput.change_mode_weigh = TRUE;
                AIS_LOG_NOTICE("Command::STANDBY_DEACTIVATE");
                breakOut = true;
            }
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::TIPOFF_MODE_TOGGLE): {
            /* only allow toggle if Tip Assist is not Active */
            if (!LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive) {
                /* set the request flag */
                LpsSaJobMgrWmInput.tipoff_toggle_request_status = TRUE;
                breakOut = true;
            }
            AIS_LOG_NOTICE("Command::TIPOFF_MODE_TOGGLE");
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::MANUAL_TIPOFF_ACTIVATE): {
            /* set the request flag */
            LpsJobMgrJobTrackerInfoTbl.ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_ACTIVE;
            LpsSaJobMgrWmInput.change_mode_excess = TRUE;
            AIS_LOG_NOTICE("Command::MANUAL_TIPOFF_ACTIVATE");
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::MANUAL_TIPOFF_DEACTIVATE): {
            /* only exit manual tipoff if Tip Assist is not Active */
            if (!LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive) {
                /* set the request flag */
                LpsJobMgrJobTrackerInfoTbl.ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_AVAILABLE;
                LpsSaJobMgrWmInput.change_mode_weigh = TRUE;
                breakOut = true;
            }
            AIS_LOG_NOTICE("Command::MANUAL_TIPOFF_DEACTIVATE");
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::MANUAL_ADD): {
            /* Set the request flag */
            LpsSaJobMgrWmInput.manual_add_request = TRUE;
            AIS_LOG_NOTICE("Command::MANUAL_ADD");
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TIPOFF_TRIGGER_TYPE): {
            /* Writing  Tip-off Trigger Data to NVM */
            config_.tipOffTriggerType = (uint8_t)reqIn.data.tipoffTriggerType;
            configChanged = true;

            AIS_LOG_NOTICE("Command::WRITE_TIPOFF_TRIGGER_TYPE = %d", reqIn.data.tipoffTriggerType);
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TIPOFF_MODE): {
            LpsSaJobMgrWmInput.tip_off_mode = reqIn.data.tipoffMode;
            LpsSaJobMgrWmInput.tip_off_mode_request_status = TRUE;

            /* Writing Tipoff State to NVM */
            config_.tipOffMode = (uint8_t)reqIn.data.tipoffMode;
            configChanged = true;

            AIS_LOG_NOTICE("Command::WRITE_TIPOFF_MODE = %d", reqIn.data.tipoffMode);
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::REWEIGH): {
            /* set the request flag */
            LpsSaJobMgrWmInput.reweigh_request_status = TRUE;
            AIS_LOG_NOTICE("Command::REWEIGH");
            breakOut = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_HORN_ON_STORE_ENABLED): {
            config_.hornStoreEnable = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_HORN_ON_STORE_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_STORE_PASS_COUNT): {
            // range check 1..999
            if (reqIn.data.autoStorePassCount < LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MIN) {
                reqIn.data.autoStorePassCount = LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MIN;
            }
            else if (reqIn.data.autoStorePassCount > LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MAX) {
                reqIn.data.autoStorePassCount = LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MAX;
            }
            config_.autoStorePassCount = reqIn.data.autoStorePassCount;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_AUTO_STORE_PASS_COUNT = %d", reqIn.data.autoStorePassCount);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_MATERIAL_ID_ENABLED): {
            config_.autoMaterialIdEnabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_AUTO_MATERIAL_ID_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_TRUCK_ID_ENABLED): {
            config_.autoTruckIdEnabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_AUTO_TRUCK_ID_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_MANUAL_ADD_ENABLED): {
            config_.manualAddEnabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_MANUAL_ADD_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_MULTI_TASK_ENABLED): {
            config_.multiTaskEnabled = reqIn.data.enabled;
            configChanged = true;

            // since multitask has changed, we will reset the LFT disable state
            tasks_.allTasksResetLFTDisableState();
            loadRecordChanged = true;

            AIS_LOG_NOTICE("Command::WRITE_MULTI_TASK_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_MULTI_TASK_COUNT): {
            tasks_.setNumberOfTasks(reqIn.data.taskNumber);
            loadRecordChanged = true;

            AIS_LOG_NOTICE("Command::WRITE_MULTI_TASK_COUNT = %d", reqIn.data.taskNumber);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_SPLIT_MODE_ENABLED): {
            config_.splitModeEnabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_SPLIT_MODE_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_LFT_DISABLED): {
            tasks_.currentTaskSetLFTDisable(reqIn.data.enabled);
            AIS_LOG_NOTICE("Command::WRITE_LFT_DISABLED = %d", reqIn.data.enabled);
            loadRecordChanged = true;
            break; // out of switch-case
        }
        case (LpsSaJobMgrReqstChannel::Command::SELECT_TASK): {
            if (config_.multiTaskEnabled) {
                tasks_.setTaskNumber(reqIn.data.taskNumber);
                LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck
                AIS_LOG_NOTICE("Command::SELECT_TASK = %d", reqIn.data.taskNumber);
                loadRecordChanged = true;
            }
            else {
                AIS_LOG_WARN("Command::SELECT_TASK rejected, multi-task disabled.");
            }
            break; // out of switch-case
        }
        case (LpsSaJobMgrReqstChannel::Command::SELECT_SUBTOTAL): {
            /* set the request flag, if selected is different than current */
            if (reqIn.data.subtotalIndex != tasks_.getCurrentTaskLoad().getCurrentSubtotalIndex()) {
                LpsJobMgrJobTrackerInfoTbl.selectSubtotalCmd = true;

                LpsJobMgrJobTrackerInfoTbl.subtotalIndex = reqIn.data.subtotalIndex;

                //for lps_tracker, make it look like a store so the subtotal is closed
                LpsSaJobMgrWmInput.store_request_status = TRUE;

                AIS_LOG_NOTICE("Command::SELECT_SUBTOTAL = %d", reqIn.data.subtotalIndex);
                loadRecordChanged = true;
            }
            break; // out of switch-case
        }
        case (LpsSaJobMgrReqstChannel::Command::PAYLOAD_DETAILS_FILE_REQUEST): {
            tes_common_ais::OFlocker ofl;
            ofl.open(PAYLOAD_DETAIL_JSON_FILENAME);
            tasks_.getCurrentTaskLoad().payloadDetailsToJson(ofl.ofstream());
            ofl.close();
            AIS_LOG_NOTICE("Command::PAYLOAD_DETAILS_FILE_REQUEST");
            break; // out of switch-case
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_SUBTOTAL_INFO): {
            auto command = reqIn.data.subtotalInfo.command;
            auto newStepNumber = reqIn.data.subtotalInfo.newStepNumber;
            auto currentStepNumber = reqIn.data.subtotalInfo.currentStepNumber;
            auto targetWeight = reqIn.data.subtotalInfo.targetWeight;
            auto targetPasses = reqIn.data.subtotalInfo.targetPasses;
            auto targetProportion = reqIn.data.subtotalInfo.targetProportion;
            auto materialName = reqIn.data.subtotalInfo.materialName;
            auto materialId = reqIn.data.subtotalInfo.materialId;
            auto materialDensity = reqIn.data.subtotalInfo.materialDensity;
            auto iconType = reqIn.data.subtotalInfo.iconType;

            if (command == "INSERT") {
                tasks_.getCurrentTaskLoad().insertSubtotal(newStepNumber, targetWeight, targetPasses, targetProportion, materialName, materialId, materialDensity, iconType);
            }
            else if (command == "EDIT") {
                tasks_.getCurrentTaskLoad().editSubtotal(currentStepNumber, newStepNumber, targetWeight, targetPasses, targetProportion, materialName, materialId, materialDensity, iconType);
            }
            else if (command == "DELETE") {
                tasks_.getCurrentTaskLoad().removeSubtotal(currentStepNumber);
                LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck
            }
            else {
                AIS_LOG_ERROR("Invalid command: %s", command.c_str());
            }

            AIS_LOG_NOTICE("Command::WRITE_SUBTOTAL_INFO");
            loadRecordChanged = true;
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::SELECT_NEXT_TASK): {
            if (config_.multiTaskEnabled) {
                tasks_.setTaskNumberNext();
                LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck
                AIS_LOG_NOTICE("Command::SELECT_NEXT_TASK = %d", tasks_.getCurrentTaskNumber());
                loadRecordChanged = true;
            }
            else {
                AIS_LOG_WARN("Command::SELECT_NEXT_TASK rejected, multi-task disabled.");
            }
            break; // out of switch-case
        }
        case (LpsSaJobMgrReqstChannel::Command::SELECT_PREVIOUS_TASK): {
            if (config_.multiTaskEnabled) {
                tasks_.setTaskNumberPrevious();
                LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck
                AIS_LOG_NOTICE("Command::SELECT_PREVIOUS_TASK = %d", tasks_.getCurrentTaskNumber());
                loadRecordChanged = true;
            }
            else {
                AIS_LOG_WARN("Command::SELECT_PREVIOUS_TASK rejected, multi-task disabled.");
            }
            break; // out of switch-case
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TRUCK_LIST_ENABLED): {
            config_.truckListEnabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TRUCK_LIST_ENABLED = %d", reqIn.data.enabled);
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_MATERIAL_LIST_ENABLED): {
            config_.materialListEnabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_MATERIAL_LIST_ENABLED = %d", reqIn.data.enabled);
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TAG1_ENABLED): {
            config_.tag1Enabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TAG1_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TAG2_ENABLED): {
            config_.tag2Enabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TAG2_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TAG3_ENABLED): {
            config_.tag3Enabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TAG3_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::WRITE_TAG4_ENABLED): {
            config_.tag4Enabled = reqIn.data.enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TAG4_ENABLED = %d", reqIn.data.enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (LpsSaJobMgrReqstChannel::Command::NONE):
        default: {
            break;
        }
        }

        // Send response to the request.
        sendReqstResponse(reqIn, success);

        if (breakOut) {
            break;
        }
    }

    if (loadRecordChanged) {
        tasks_.save();
    }

    if (configChanged) {
        saveConfig();
    }
}

/* Send the response to JobMgr Helper */
bool LpsSaJobMgrApp::sendReqstResponse(const LpsSaJobMgrReqstChannel& request, bool success) {
    // Build the response
    LpsSaJobMgrRespChannel response; // Default timepoint is now
    response.appName = request.appName;
    response.appRequestId = request.appRequestId;
    response.command = request.command;
    response.success = success;

    if (LpsSaJobMgrRespChannelOutput_) {   /* Publish Response */
        if (LpsSaJobMgrRespChannelOutput_->publish(response)) {
            AIS_LOG_INFO("Published response, command=%d, success=%d", request.command, success);
            return true;
        }
    }

    AIS_LOG_ERROR("Failed to publish response, command=%d, success=%d", request.command, success);
    return false;
}


/******************************************************************************
FUNCTION NAME: AisJhmDataServerTxRead
DESCRIPTION:            
PARAMETER DESCRIPTION:                        
RETURN VALUE:             
*******************************************************************************/
void LpsSaJobMgrApp::AisJhmDataServerTxRead()
{
    AisJhm2TxChannel AisJhm2TxIn;
    while (AisJhm2TxInputScs->get(AisJhm2TxIn)) {
        if (AisJhm2TxIn.simplecal_data.newDataFlag) {
            simpleCal_.eraseEntry(AisJhm2TxIn.simplecal_data.timeStamp);
        }
    }
}

/******************************************************************************
FUNCTION NAME: LpsSaJobMgrScsEddtRead
DESCRIPTION: Look for any diagnostics or events from other ECMs.
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrScsEddtRead()
{
    /*
     * Tip-Off Assist CDL Event Ids
     *  63038   Tip-Off Assist Disarmed Due to Time Out
     *  63039   Tip-Off Assist Disarmed Due to Payload Not Achievable
     *  63040   Tip-Off Assist Disarmed Due to Tilt Lever Racked
     *  63041   Tip-Off Assist Disarmed Due to Not in Pile Tip-Off Mode
     *  63042   Tip-Off Assist Disarmed Due to Truck Target Not Set
     *  63043   Tip-Off Assist Disarmed Due to Tilt Lever Released
     *  63044   Tip-Off Assist Disarmed Due to End of Travel
     *  63045   Tip-Off Assist Armed
     *  63046   Tip-Off Assist Complete
     *  63078   Tip-Off Assist Disarmed Due to Excessive Machine Speed
     */
    static const std::unordered_set<uint_fast16_t> toaEids{
        63038, 63039, 63040, 63041, 63042, 63043, 63044, 63045, 63046, 63078
    };

    if (nullptr != eddtInputChannel_) {
        EventDiagnosticData data;

        // Clear TipoffAssistActiveEid
        LpsJobMgrJobTrackerInfoTbl.TipoffAssistActiveEid = 0;       

        while (eddtInputChannel_->get(data)) {
            const auto* pDiags = data.getListOfEventDiagnostics();

            if (nullptr == pDiags) {           
                continue;
            }

            bool memoryFull = false;

            for (const auto& diag : *pDiags) {
                if (2 != diag.category) {
                    // only looking for events (not diagnostics)
                    continue;
                }

                if ((SCL_DCLI_FAULT_ACTIVE == diag.status) ||
                        (SCL_DCLI_FAULT_ACTIVE_AND_LOGGED == diag.status)) {
                    uint_fast16_t eid = diag.getCID();
                    if (toaEids.count(eid) > 0) {
                        LpsJobMgrJobTrackerInfoTbl.TipoffAssistActiveEid = eid;
                    }
                    else if (PAYLOAD_MEMORY_FULL_EID == eid) {
                        memoryFull = true;
                    }
                }
            }

            LpsJobMgrJobTrackerInfoTbl.memoryFull = memoryFull;
        }
    }
}

/******************************************************************************
FUNCTION NAME: AisJhmDataServerTxRead
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrScsSHMRead( )
{
    if (nullptr != ShmClockInputScs) {
        ShmClock shmClock;
        while (ShmClockInputScs->get(shmClock)) {
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
FUNCTION NAME: LpsSaJobMgrScsDataLinkDataRead( )
DESCRIPTION: Polls DataLinkData input channel
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrScsDataLinkDataRead( )
{
#define STORE_BUTTON_PID    0xF1AA
#define TIPOFF_ASSSIST_PID  0xD118CE
#define MANUAL_ADD_PID     0xD11890

    DataLinkData dlData;

    while (dataLinkDataInput_->get(dlData)) {
        for (auto& dlParam : dlData.GetParams()) {
            // Skip this if no new data is received.
            if (!dlParam.IsPIDDataReceived()) {
                continue;
            }

            if (dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PID) {
                if (dlParam.GetParamId() == STORE_BUTTON_PID) {
                    if (0 == dlParam.GetLastValueDsi()) {
                        uint8_t dlValue = dlParam.GetLastGoodValue<uint8_t>();
                        bool currentlyDepressed = (dlValue == 0x01) ? true : false;

                        if ((!LpsJobMgrJobTrackerInfoTbl.StorePIDPreviouslyDepressed) && currentlyDepressed) {
                            LpsJobMgrJobTrackerInfoTbl.StorePIDActive = TRUE;
                            AIS_LOG_INFO("Store PID Depressed");
                        }

                        LpsJobMgrJobTrackerInfoTbl.StorePIDPreviouslyDepressed = currentlyDepressed;
                    }
                    else {
                    	//DSI
                    	LpsJobMgrJobTrackerInfoTbl.StorePIDPreviouslyDepressed = false;
                    }
                }
                else if (dlParam.GetParamId() == TIPOFF_ASSSIST_PID) {
                    if( 0 == dlParam.GetLastValueDsi()) {
                        if (dlParam.GetLastValueEng() == 0x000E) {
                            /* Tipoff Assist is Active */
                            if (!LpsJobMgrJobTrackerInfoTbl.TipoffAssistPIDActive) {
                                // Transitioning to TOA Active
                                stats_.notifyTipoffAssistActivation();
                            }
                            LpsJobMgrJobTrackerInfoTbl.TipoffAssistPIDActive = true;
                        }
                        else {
                            /* Tipoff Assist is not Active */
                            LpsJobMgrJobTrackerInfoTbl.TipoffAssistPIDActive = false;
                        }
                        AIS_LOG_INFO("Rxed Data for PID 0x%X with Val =  %f   stat = %d ", dlParam.GetParamId(), dlParam.GetLastValueEng(), dlParam.GetLastValueDsi());
                    }
                    else {
                        /* Tipoff Assist not Active, if DSI */
                        LpsJobMgrJobTrackerInfoTbl.TipoffAssistPIDActive = false;
                    }
                }
                else if (dlParam.GetParamId() == MANUAL_ADD_PID) {
                	if (0 == dlParam.GetLastValueDsi()) {
                	    uint16_t dlValue = dlParam.GetLastGoodValue<uint16_t>();
                	    bool currentlyDepressed = (dlValue == 0x001D) ? true : false;

                	    if ((!LpsJobMgrJobTrackerInfoTbl.ManualAddPIDPreviouslyDepressed) && currentlyDepressed) {
                	        LpsJobMgrJobTrackerInfoTbl.ManualAddPIDActive = TRUE;
                            AIS_LOG_INFO("Manual Add PID Depressed");
                	    }

                	    LpsJobMgrJobTrackerInfoTbl.ManualAddPIDPreviouslyDepressed = currentlyDepressed;
                	}
                	else {
                		//DSI
                	    LpsJobMgrJobTrackerInfoTbl.ManualAddPIDPreviouslyDepressed = false;
                	}
                }
            }
        }
    }
}

/******************************************************************************
FUNCTION LpsSaJobMgrApp::LpsSaWeighScsTxParamRead( )
DESCRIPTION:It will get invoked once JobManager receives the parameters from weighing App
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaWeighScsTxParamRead( )
{
    bool dataReceived = weighAppTxDataReceived_;
    cpm_common_interfaces::msg::LpsSaWeighTxChannel rxParam;
    bool newData = weighAppInf_.waitForTxData(rxParam);

    if (newData) {
        // Indicate that we have received data at least once.
        //  This will remain true from now on.
        dataReceived = true;
    }
    else {
        AIS_LOG_ERROR("Timeout waiting for weigh app tx data, continuing...");
    }

    // Detect when the LFT system becomes sealed and clear the truck
    if (!LpsJobMgrJobTrackerInfoTbl.lftSealed && // Previously not sealed
            rxParam.lft_seal_status.sealed && // Now we are sealed
            weighAppTxDataReceived_ && dataReceived) { // Previous and current data is valid
        LpsSaJobMgrWmInput.clear_request_status = TRUE;
    }

    // Whether new data was received or not, the rxParam contains the latest data.
    LpsJobMgrJobTrackerInfoTbl.DigStat = static_cast<LpsWeighBktDigStat_t>(rxParam.dig_stat);
    LpsJobMgrJobTrackerInfoTbl.CalStat = static_cast<LpsWeighCalStatus_t>(rxParam.cal_stat);
    LpsJobMgrJobTrackerInfoTbl.DumpStat = static_cast<LpsWeighBktDumpStat_t>(rxParam.dump_stat);
    LpsSaJobMgrWmInput.current_weight = rxParam.best_bkt_wt_in_tonnes;
    LpsSaJobMgrWmInput.calc_method = (unsigned int)rxParam.payload_calc_meth;
    LpsSaJobMgrWmInput.current_bucket_weight_latched = rxParam.bkt_wt_latched_flag;
    LpsSaJobMgrWmInput.payload_latch_conditions_ok = rxParam.latch_conditions_met;
    LpsJobMgrJobTrackerInfoTbl.zeroWeight = rxParam.zero_weight;
    LpsJobMgrJobTrackerInfoTbl.simpleCalAdjust = rxParam.simple_cal_adjust;
    LpsJobMgrJobTrackerInfoTbl.lftSealed = rxParam.lft_seal_status.sealed;
    LpsSaJobMgrWmInput.lift_stalled = rxParam.lift_stalled;

    // Remember that we have received data at least once.
    weighAppTxDataReceived_ = dataReceived;
}
 
/******************************************************************************
FUNCTION NAME:LpsSaJobMgrApp::LpsSaJobMgrScsRx()
DESCRIPTION:It will read the Tx parameters from weighing app through SCS channel
PARAMETER DESCRIPTION:
RETURN VALUE:boolean
*******************************************************************************/
boolean LpsSaJobMgrApp::LpsSaJobMgrScsRx()
{
    /*Read the SHM object from ACD */
    LpsSaJobMgrScsSHMRead();

    /*Read the requests from Weighing App*/
    LpsSaWeighScsTxParamRead();

    /*Read DataLinkData */
    LpsSaJobMgrScsDataLinkDataRead();

    /*Read the requests from UI*/
    LpsSaJobMgrScsChkForReqst();

    /*Read the Simple cal data from data server. */
    AisJhmDataServerTxRead();

    // Read events and diagnostics
    LpsSaJobMgrScsEddtRead();

    return SUCCESS;
}

/******************************************************************************
FUNCTION NAME:LpsSaJobMgrApp::LpsSaJobMgrHornOnStoreAction()
DESCRIPTION:It will blow horn when Action Flag is ture;
PARAMETER DESCRIPTION:
RETURN VALUE:boolean
*******************************************************************************/
bool LpsSaJobMgrApp::LpsSaJobMgrHornOnStoreAction()
{
    OutputChannel outputChannel;
    OutputChannel::OutputCmd command;
    command.OutputPort = HORN_PORT_NUMBER;
    command.InitialState = OutputChannel::State::PORT_ON;
    command.StateChangeDuration = OutputChannel::ChangeDuration::NO_FLASH;
    command.TotalDuration = 2;
    command.FinalState = OutputChannel::State::PORT_OFF;
    outputChannel.AddOutputAppCmd(command);
    if (nullptr != LpsSaOutputChannelOut) {
        return LpsSaOutputChannelOut->publish(outputChannel);
    }
    return false;
}

/******************************************************************************
FUNCTION NAME:LpsSaJobMgrApp::LpsSaJobMgrScsTx()
DESCRIPTION:It will write the tx parameter to UI App
PARAMETER DESCRIPTION:
RETURN VALUE:boolean
*******************************************************************************/
boolean LpsSaJobMgrApp::LpsSaJobMgrScsTx()
{
    bool scsCmdRet=false;
    LpsSaJobMgrTxChannel txOut;
    /* frame SCS channel data to UI*/
    if (LpsSaJobMgrScsTxOut) {

        { // Update current task load information
            const LpsSaLoadRecordChannelStorage& loadRecord = tasks_.getCurrentTaskLoad();
            const LpsSaLoadRecordSubtotal& subtotal = loadRecord.getCurrentSubtotal();
            txOut.taskNumber = tasks_.getCurrentTaskNumber();
            txOut.materialId = subtotal.materialId;
            txOut.materialName = subtotal.materialName;
            txOut.materialDensity = subtotal.materialDensity;
            txOut.truckId = subtotal.truckId;
            txOut.truckName = subtotal.truckName;
            txOut.truckTargetWeight = subtotal.truckTargetWeightTonnes;

            if (0.0f != subtotal.truckTargetWeightTonnes) {
                txOut.remainingWeight = subtotal.truckTargetWeightTonnes - LpsSaJobMgrWmOutput.truck_weight;
            }
            else {
                txOut.remainingWeight = 0.0f;
            }

            txOut.tag1 = subtotal.tag1;
            txOut.tag2 = subtotal.tag2;
            txOut.tag3 = subtotal.tag3;
            txOut.tag4 = subtotal.tag4;
            txOut.customListName1 = subtotal.customListName1;
            txOut.customListName2 = subtotal.customListName2;
            txOut.customListName3 = subtotal.customListName3;
            txOut.customListName4 = subtotal.customListName4;

            txOut.passCount               = LpsSaJobMgrWmOutput.passcount;
            txOut.TruckStartWeight        = LpsSaJobMgrWmOutput.truck_start_weight;

            LpsWeighBktWtAccuracy_t subtotalWeightAccuracy;
            float subtotalWeight = subtotal.weightTonnes(subtotalWeightAccuracy);

            LpsWeighBktWtAccuracy_t totalWeightAccuracy;
            float totalWeight = loadRecord.weightTonnes(totalWeightAccuracy);

            if (TRUE == LpsSaJobMgrWmOutput.truck_pass_active) {
                /*
                 * Total weight (split weights) including active pass
                 * Truck Weight needs to include the active pass and the sub-total does not include that yet.
                 * Need to subtract off the current sub-total and add in the currently active truck weight
                 */
                totalWeight = totalWeight - subtotalWeight + LpsSaJobMgrWmOutput.truck_weight;
                totalWeightAccuracy = std::min(totalWeightAccuracy, LpsSaJobMgrWmOutput.display_bucket_weight_accuracy);

                // accuracy needs to include the current bucket weight
                subtotalWeight = LpsSaJobMgrWmOutput.truck_weight;
                subtotalWeightAccuracy = std::min(subtotalWeightAccuracy, LpsSaJobMgrWmOutput.display_bucket_weight_accuracy);
            }
            else if (subtotal.passCount() == 0) {
                // No active pass and no completed passes for the current subtotal
                subtotalWeightAccuracy = LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_NONE;

                if (loadRecord.passCount() == 0) {
                    // No active pass and no completed passes for the grand total
                    totalWeightAccuracy = LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_NONE;
                }
            }

            txOut.truckWeight = subtotalWeight;
            txOut.truckWeightAccuracy = subtotalWeightAccuracy;

            txOut.totalWeight = totalWeight;
            txOut.totalWeightAccuracy = totalWeightAccuracy;

            totalWeightAccuracy_ = totalWeightAccuracy;

            // current load subtotal count
            txOut.subtotalCount = loadRecord.subtotalCount();

            txOut.targetType = (uint8_t)loadRecord.targetType();
            if (loadRecord.targetType() == LpsSaLoadRecordTargetType::SINGLE) {
                txOut.splitModeEnabled = false;
            }
            else {
                txOut.splitModeEnabled = true;
            }

            txOut.stepNumber = loadRecord.getCurrentSubtotalIndex();

            txOut.iconType = subtotal.iconType;

            txOut.targetPasses = subtotal.targetPasses;
        }

        txOut.OperationMode = LpsJobMgrJobTrackerInfoTbl.OperationMode;

        txOut.ManualTipOffState       = LpsJobMgrJobTrackerInfoTbl.ManualTipOffState;
        txOut.TipOffState             = LpsJobMgrJobTrackerInfoTbl.TipOffState;

        // Standby State
        if (LpsSaJobMgrWmOutput.standby_active) {
            txOut.StandbyState = LPS_SA_JOB_MGR_STANDBY_ACTIVATED;
        }
        else {
            txOut.StandbyState = LPS_SA_JOB_MGR_STANDBY_DEACTIVATED;
        }

        // Clear or -1 Button is Showing?
        if (LpsSaJobMgrWmOutput.show_clear_not_minus_one) {
            txOut.ClearMinusOneEnableStat = LPS_SA_JOB_MGR_CLEAR_BTN_ENABLED;
        }
        else {
            txOut.ClearMinusOneEnableStat = LPS_SA_JOB_MGR_MINUS_ONE_BTN_ENABLED;
        }

        txOut.DispBestBktWt = LpsJobMgrJobTrackerInfoTbl.DispBestBktWt;

        txOut.TipOffTriggerType = (LpsSaTipOffTriggerType_t)config_.tipOffTriggerType;
        txOut.TipOffStateCfg = (LpsSaJobMgrTipOffState_t)config_.tipOffMode;

        if (SEALegalForTradeInstalled_) {
            txOut.AutoStorePassCount = LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MAX;
        }
        else {
            txOut.AutoStorePassCount = config_.autoStorePassCount;
        }

        /* SEA Level2 (Pro) interlock */
        if (SEALevel2ProInstalled_) {
            txOut.AutoTruckIdEnabled = config_.autoTruckIdEnabled;
            txOut.AutoMaterialIdEnabled = config_.autoMaterialIdEnabled;
            txOut.manualAddEnabled = config_.manualAddEnabled;
            txOut.multiTaskEnabled = config_.multiTaskEnabled;
            txOut.multiTaskCount = tasks_.getNumberOfTasks();
            txOut.truckListEnabled = config_.truckListEnabled;
            txOut.materialListEnabled = config_.materialListEnabled;
            txOut.tag1Enabled = config_.tag1Enabled;
            txOut.tag2Enabled = config_.tag2Enabled;
            txOut.tag3Enabled = config_.tag3Enabled;
            txOut.tag4Enabled = config_.tag4Enabled;
        }
        else {
            /* level2 (Pro) not installed, disable Pro features */
            txOut.AutoTruckIdEnabled = false;
            txOut.AutoMaterialIdEnabled = false;
            txOut.manualAddEnabled = false;
            txOut.multiTaskEnabled = false;
            txOut.multiTaskCount = 0;
            txOut.truckListEnabled = false;
            txOut.materialListEnabled = false;
            txOut.tag1Enabled = false;
            txOut.tag2Enabled = false;
            txOut.tag3Enabled = false;
            txOut.tag4Enabled = false;
            txOut.splitModeEnabled = false;
        }

        if (LpsSaJobMgrWmOutput.tip_off_active) {
            txOut.TipoffActive = true;
        }
        else {
            txOut.TipoffActive = false;
        }

        txOut.tipoffAssistActive = LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive;
        txOut.tipoffAssistActiveEid = LpsJobMgrJobTrackerInfoTbl.TipoffAssistActiveEid;

        txOut.ReqPloadCtrlSysStat = LpsJobMgrJobTrackerInfoTbl.ReqPloadCtrlSysStat;

        simpleCal_.getSimpleCalData(txOut.simpleCalData);

        txOut.storeCount = LpsJobMgrJobTrackerInfoTbl.storePressCount;

        // Show the "Payload Store:Not Available" info pop-up until at least this time is met.
        if (storeRejectedExpireTime > std::chrono::steady_clock::now()) {
            txOut.storeRejected = true;
        }
        else {
            txOut.storeRejected = false;
        }

        if (LpsSaJobMgrWmOutput.manual_add_available) {
            txOut.manualAddAvailable = true;
        }
        else {
            txOut.manualAddAvailable = false;
        }

        txOut.HornStoreState = config_.hornStoreEnable ? SOUND_HORN : NOT_SOUND_HORN;

        // LFT disabled state for current task
        txOut.lftDisabled = tasks_.currentTaskGetLFTDisable();

        scsCmdRet=LpsSaJobMgrScsTxOut->publish( txOut );
    }

    if(!scsCmdRet)
    {
        AIS_LOG_ERROR( "\n Line no = %d,'LpsSaJobMgrScsTx' function return code = %d\n",__LINE__,scsCmdRet);

        return FAIL;
    }

    return SUCCESS;
}

/******************************************************************************
FUNCTION LpsSaJobMgrApp::LpsSaJobMgrScsSendCmd
DESCRIPTION: It will send the cmd to weighing App through SCS channel by polling method
PARAMETER DESCRIPTION:                        
RETURN VALUE:             
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrScsSendCmd(LpsSaWeighReqstChannel::Command command)
{
    cpm_common_interfaces::msg::LpsSaWeighReqstChannel request;

    switch (command) {
    case (LpsSaWeighReqstChannel::Command::RESET_BEST_BUCKET_WEIGHT):
    case (LpsSaWeighReqstChannel::Command::CAPTURE_CYLINDER_EXTENSION_REFERENCE):
    case (LpsSaWeighReqstChannel::Command::CLEAR_REWEIGH_WARNING): {
        request.command.value = static_cast<uint8_t>(command);
        break;
    }
    default: {
        AIS_LOG_ERROR("Unsupported weigh app request command.");
        return;
    }
    }

    if (weighAppInf_.sendRequest(request)) {
        AIS_LOG_ERROR("[ROS2] Published weigh app request, command=%d", static_cast<int>(command));
    } else {
        AIS_LOG_ERROR("[ROS2] Failed to send weigh app request.");
    }

    // FIX: dual-publish the same request onto the real SCS channel.
    if (nullptr != LpsSaWeighScsReqstOut) {
        LpsSaWeighReqstChannel requestScs;
        requestScs.appName = getTaskName();
        requestScs.appRequestId = LpsSaWeighReqstChannelStorage::getNextAppRequestId();
        requestScs.command = command;

        if (LpsSaWeighScsReqstOut->publish(requestScs)) {
            AIS_LOG_ERROR("[SCS] Published weigh app request, command=%d", static_cast<int>(command));
        } else {
            AIS_LOG_ERROR("[SCS] Failed to publish weigh app request over raw SCS.");
        }
    }
}

/******************************************************************************
FUNCTION NAME: CheckActiveButton
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
ReqPloadCtrlSysStat_t LpsSaJobMgrApp::GetActiveButtonStatus (void)
{
    if(TRUE == LpsSaJobMgrWmInput.clear_request_status)
    {
        return ReqPloadCtrlSysStat_t::CLEAR;
    }
    else if(TRUE == LpsSaJobMgrWmInput.reweigh_request_status)
    {
        return ReqPloadCtrlSysStat_t::REWEIGH;
    }
    else if(TRUE == LpsSaJobMgrWmInput.zero_request_status)
    {
        return ReqPloadCtrlSysStat_t::ZERO;
    }
    else if(TRUE == LpsSaJobMgrWmInput.store_request_status)
    {
        return ReqPloadCtrlSysStat_t::STORE;
    }
    else if(TRUE == LpsSaJobMgrWmInput.tipoff_toggle_request_status)
    {
        return ReqPloadCtrlSysStat_t::TIPOFFTOGGLE;
    }
    else if(TRUE == LpsSaJobMgrWmInput.minus_one_request_status)
    {
        return ReqPloadCtrlSysStat_t::MINUS_ONE;
    }
    else
    {
        return ReqPloadCtrlSysStat_t::NONE;
    }
}
