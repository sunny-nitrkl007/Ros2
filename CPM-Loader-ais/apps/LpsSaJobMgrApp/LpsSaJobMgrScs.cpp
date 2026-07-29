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
// !!! UNVERIFIED PLACEHOLDER -- DO NOT HARDWARE-TEST UNTIL FIXED !!!
// The real numeric value of OutputChannel::Port::OUTPUT_SINK_3 is not
// found ANYWHERE in this checkout -- this #define was its only occurrence
// in the whole tree (confirmed by repo-wide grep). This drives a real
// GPIO pin (see original comment: PIN29_GPIO88), so the value below is
// deliberately an obviously-fake sentinel, not a guess dressed up as real.
// Confirm the true value against interfaces/OutputChannel/OutputChannel.h
// once that folder is sourced (it does not exist in this checkout either
// -- see Challenges-And-Decisions.txt 3.3), then replace 0xFF here.
#define HORN_PORT_NUMBER   0xFF    //OUTPUT_APP_PORT_SINK3_PIN29_GPIO88 -- UNVERIFIED, see comment above

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
        // SwitchInputScs.msg is a scoped reconstruction (real struct not in
        // this checkout -- see Challenges-And-Decisions.txt 3.3): the real
        // get_STG_value(STG4)==STG::CLOSED test collapses to a single
        // `closed` field, since that's the only comparison any consumer in
        // the whole tree ever makes against index STG4.
        job_mgr_interfaces::msg::SwitchInputScs STG_Input;
        bool storeRequest = false;

        /* The store button has higher priority compared to LPS Job Mgr request. */
        while (LpsSaSwitchInput->get(STG_Input)) {
            if (STG_Input.closed) {
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
        job_mgr_interfaces::msg::LpsSaUIDisplayStateInterface displayState;
        while (displayStateInput_->get(displayState)) {
            // isInVerificationMode() (DisplayState.hpp:62) is a trivial
            // accessor: `return inVerificationMode_;` -- direct field read.
            LpsJobMgrJobTrackerInfoTbl.inVerificationMode = displayState.state.in_verification_mode;
            if (LpsJobMgrJobTrackerInfoTbl.inVerificationMode &&
                    (LpsJobMgrJobTrackerInfoTbl.OperationMode != LPS_SA_JOB_MGR_STANDBY_MODE)) {
                LpsSaJobMgrWmInput.standby_request_status = TRUE;
                AIS_LOG_NOTICE("Enter standby when in verification mode.");
            }
        }
    }

    cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel reqIn;
    bool loadRecordChanged = false;
    bool configChanged = false;

    /* Retrieve SCS channel data */
    while (LpsSaJobMgrScsReqstIn->get(reqIn)) {
        bool success = true;

        // Select target type first because this sets the context for the sub-requests.
        if (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TARGET_TYPE == reqIn.command.value) {
            tasks_.setTargetType(reqIn.data_target_type);
            LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck
            AIS_LOG_NOTICE("Command::WRITE_TARGET_TYPE success: %d", reqIn.data_target_type);
            loadRecordChanged = true;
        }

        /*
         * Check the *new* style requests and handle them
         */
        for (const auto& r : reqIn.requests) {
            switch (r.command) {
            // Original accessors (materialId(), materialName(), tagValue(), etc.)
            // returned arg1-4 depending on command -- see LpsSaJobMgrReqst.msg
            // header for the full mapping. Direct field access now, same
            // underlying data.
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_MATERIAL_ID): {
                tasks_.getCurrentTaskLoad().setMaterialId(r.arg3);
                AIS_LOG_NOTICE("Load Change - materialId");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_MATERIAL_NAME): {
                tasks_.getCurrentTaskLoad().setMaterialName(r.arg1);
                AIS_LOG_NOTICE("Load Change - materialName");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_MATERIAL_DENSITY): {
                tasks_.getCurrentTaskLoad().setMaterialDensity(r.arg2);
                AIS_LOG_NOTICE("Load Change - materialDensity");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_TRUCK_ID): {
                tasks_.getCurrentTaskLoad().setTruckId(r.arg3);
                AIS_LOG_NOTICE("Load Change - truckId");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_TRUCK_NAME): {
                tasks_.getCurrentTaskLoad().setTruckName(r.arg1);
                AIS_LOG_NOTICE("Load Change - truckName");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_TRUCK_TARGET_WEIGHT): {
                tasks_.getCurrentTaskLoad().setTotalTargetWeight(r.arg2);
                AIS_LOG_NOTICE("Load Change - truckTargetWeight");
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_TAG1): {
                tasks_.getCurrentTaskLoad().setTag1(r.arg4, r.arg1);
                AIS_LOG_NOTICE("WRITE_TAG1 = %s (%s)", r.arg1.c_str(), r.arg4.c_str());
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_TAG2): {
                tasks_.getCurrentTaskLoad().setTag2(r.arg4, r.arg1);
                AIS_LOG_NOTICE("WRITE_TAG2 = %s (%s)", r.arg1.c_str(), r.arg4.c_str());
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_TAG3): {
                tasks_.getCurrentTaskLoad().setTag3(r.arg4, r.arg1);
                AIS_LOG_NOTICE("WRITE_TAG3 = %s (%s)", r.arg1.c_str(), r.arg4.c_str());
                loadRecordChanged = true;
                break; // out of switch-case
            }
            case (job_mgr_interfaces::msg::LpsSaJobMgrReqst::WRITE_TAG4): {
                tasks_.getCurrentTaskLoad().setTag4(r.arg4, r.arg1);
                AIS_LOG_NOTICE("WRITE_TAG4 = %s (%s)", r.arg1.c_str(), r.arg4.c_str());
                loadRecordChanged = true;
                break; // out of switch-case
            }
            default: {
                break; // out of switch-case
            }
            }
        }

        bool breakOut = false;

        switch (reqIn.command.value) {
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::ZERO): {
            /* Request received for Weighing App,set the request flag */
            LpsSaJobMgrWmInput.zero_request_status = TRUE;
            AIS_LOG_NOTICE("Command::ZERO");
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::MINUS_ONE): {
            /* Request received for Weighing App,set the request flag */
            LpsSaJobMgrWmInput.minus_one_request_status = TRUE;
            AIS_LOG_NOTICE("Command::MINUS_ONE");
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::CLEAR): {
            /* Request received for Weighing App,set the request flag */
            LpsSaJobMgrWmInput.clear_request_status = TRUE;
            AIS_LOG_NOTICE("Command::CLEAR");
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::STORE): {
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
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_PAYLOAD_NEXT_SUBTOTAL): {
            /* set the request flag */
            LpsJobMgrJobTrackerInfoTbl.splitModeNextPayloadCmd = true;

            //for lps_tracker, make it look like a store so the subtotal is closed
            LpsSaJobMgrWmInput.store_request_status = TRUE;

            AIS_LOG_NOTICE("Command::WRITE_PAYLOAD_NEXT_SUBTOTAL");
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TARGET_TYPE): {
            // This case is handled outside of the switch
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TOTAL_TARGET_WEIGHT): {

            tasks_.getCurrentTaskLoad().setTotalTargetWeight(reqIn.data_total_target_weight);
            AIS_LOG_NOTICE("Command::WRITE_TOTAL_TARGET_WEIGHT success: %f", reqIn.data_total_target_weight);
            loadRecordChanged = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::STANDBY_ACTIVATE): {
            /* set the request flag */
            // ONLY DO THIS IF...
            if (LpsJobMgrJobTrackerInfoTbl.OperationMode != LPS_SA_JOB_MGR_STANDBY_MODE) {
                LpsSaJobMgrWmInput.standby_request_status = TRUE;
                AIS_LOG_NOTICE("Command::STANDBY_ACTIVATE");
                breakOut = true;
            }
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::STANDBY_DEACTIVATE): {
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
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::TIPOFF_MODE_TOGGLE): {
            /* only allow toggle if Tip Assist is not Active */
            if (!LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive) {
                /* set the request flag */
                LpsSaJobMgrWmInput.tipoff_toggle_request_status = TRUE;
                breakOut = true;
            }
            AIS_LOG_NOTICE("Command::TIPOFF_MODE_TOGGLE");
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::MANUAL_TIPOFF_ACTIVATE): {
            /* set the request flag */
            LpsJobMgrJobTrackerInfoTbl.ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_ACTIVE;
            LpsSaJobMgrWmInput.change_mode_excess = TRUE;
            AIS_LOG_NOTICE("Command::MANUAL_TIPOFF_ACTIVATE");
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::MANUAL_TIPOFF_DEACTIVATE): {
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
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::MANUAL_ADD): {
            /* Set the request flag */
            LpsSaJobMgrWmInput.manual_add_request = TRUE;
            AIS_LOG_NOTICE("Command::MANUAL_ADD");
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TIPOFF_TRIGGER_TYPE): {
            /* Writing  Tip-off Trigger Data to NVM */
            config_.tipOffTriggerType = reqIn.data_tipoff_trigger_type.value;
            configChanged = true;

            AIS_LOG_NOTICE("Command::WRITE_TIPOFF_TRIGGER_TYPE = %d", reqIn.data_tipoff_trigger_type.value);
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TIPOFF_MODE): {
            LpsSaJobMgrWmInput.tip_off_mode = reqIn.data_tipoff_mode.value;
            LpsSaJobMgrWmInput.tip_off_mode_request_status = TRUE;

            /* Writing Tipoff State to NVM */
            config_.tipOffMode = reqIn.data_tipoff_mode.value;
            configChanged = true;

            AIS_LOG_NOTICE("Command::WRITE_TIPOFF_MODE = %d", reqIn.data_tipoff_mode.value);
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::REWEIGH): {
            /* set the request flag */
            LpsSaJobMgrWmInput.reweigh_request_status = TRUE;
            AIS_LOG_NOTICE("Command::REWEIGH");
            breakOut = true;
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_HORN_ON_STORE_ENABLED): {
            config_.hornStoreEnable = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_HORN_ON_STORE_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_AUTO_STORE_PASS_COUNT): {
            // range check 1..999
            if (reqIn.data_auto_store_pass_count < LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MIN) {
                reqIn.data_auto_store_pass_count = LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MIN;
            }
            else if (reqIn.data_auto_store_pass_count > LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MAX) {
                reqIn.data_auto_store_pass_count = LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MAX;
            }
            config_.autoStorePassCount = reqIn.data_auto_store_pass_count;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_AUTO_STORE_PASS_COUNT = %d", reqIn.data_auto_store_pass_count);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_AUTO_MATERIAL_ID_ENABLED): {
            config_.autoMaterialIdEnabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_AUTO_MATERIAL_ID_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_AUTO_TRUCK_ID_ENABLED): {
            config_.autoTruckIdEnabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_AUTO_TRUCK_ID_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_MANUAL_ADD_ENABLED): {
            config_.manualAddEnabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_MANUAL_ADD_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_MULTI_TASK_ENABLED): {
            config_.multiTaskEnabled = reqIn.data_enabled;
            configChanged = true;

            // since multitask has changed, we will reset the LFT disable state
            tasks_.allTasksResetLFTDisableState();
            loadRecordChanged = true;

            AIS_LOG_NOTICE("Command::WRITE_MULTI_TASK_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_MULTI_TASK_COUNT): {
            tasks_.setNumberOfTasks(reqIn.data_task_number);
            loadRecordChanged = true;

            AIS_LOG_NOTICE("Command::WRITE_MULTI_TASK_COUNT = %d", reqIn.data_task_number);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_SPLIT_MODE_ENABLED): {
            config_.splitModeEnabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_SPLIT_MODE_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_LFT_DISABLED): {
            tasks_.currentTaskSetLFTDisable(reqIn.data_enabled);
            AIS_LOG_NOTICE("Command::WRITE_LFT_DISABLED = %d", reqIn.data_enabled);
            loadRecordChanged = true;
            break; // out of switch-case
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::SELECT_TASK): {
            if (config_.multiTaskEnabled) {
                tasks_.setTaskNumber(reqIn.data_task_number);
                LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck
                AIS_LOG_NOTICE("Command::SELECT_TASK = %d", reqIn.data_task_number);
                loadRecordChanged = true;
            }
            else {
                AIS_LOG_WARN("Command::SELECT_TASK rejected, multi-task disabled.");
            }
            break; // out of switch-case
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::SELECT_SUBTOTAL): {
            /* set the request flag, if selected is different than current */
            if (reqIn.data_subtotal_index != tasks_.getCurrentTaskLoad().getCurrentSubtotalIndex()) {
                LpsJobMgrJobTrackerInfoTbl.selectSubtotalCmd = true;

                LpsJobMgrJobTrackerInfoTbl.subtotalIndex = reqIn.data_subtotal_index;

                //for lps_tracker, make it look like a store so the subtotal is closed
                LpsSaJobMgrWmInput.store_request_status = TRUE;

                AIS_LOG_NOTICE("Command::SELECT_SUBTOTAL = %d", reqIn.data_subtotal_index);
                loadRecordChanged = true;
            }
            break; // out of switch-case
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::PAYLOAD_DETAILS_FILE_REQUEST): {
            tes_common_ais::OFlocker ofl;
            ofl.open(PAYLOAD_DETAIL_JSON_FILENAME);
            tasks_.getCurrentTaskLoad().payloadDetailsToJson(ofl.ofstream());
            ofl.close();
            AIS_LOG_NOTICE("Command::PAYLOAD_DETAILS_FILE_REQUEST");
            break; // out of switch-case
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_SUBTOTAL_INFO): {
            // subtotal_command, not "command" -- LpsSaJobMgrSubtotalInfo.msg
            // renamed this field specifically to avoid confusion with the
            // channel's own top-level Command enum (different type entirely,
            // this one is a plain string).
            auto command = reqIn.data_subtotal_info.subtotal_command;
            auto newStepNumber = reqIn.data_subtotal_info.new_step_number;
            auto currentStepNumber = reqIn.data_subtotal_info.current_step_number;
            auto targetWeight = reqIn.data_subtotal_info.target_weight;
            auto targetPasses = reqIn.data_subtotal_info.target_passes;
            auto targetProportion = reqIn.data_subtotal_info.target_proportion;
            auto materialName = reqIn.data_subtotal_info.material_name;
            auto materialId = reqIn.data_subtotal_info.material_id;
            auto materialDensity = reqIn.data_subtotal_info.material_density;
            auto iconType = reqIn.data_subtotal_info.icon_type;

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
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::SELECT_NEXT_TASK): {
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
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::SELECT_PREVIOUS_TASK): {
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
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TRUCK_LIST_ENABLED): {
            config_.truckListEnabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TRUCK_LIST_ENABLED = %d", reqIn.data_enabled);
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_MATERIAL_LIST_ENABLED): {
            config_.materialListEnabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_MATERIAL_LIST_ENABLED = %d", reqIn.data_enabled);
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TAG1_ENABLED): {
            config_.tag1Enabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TAG1_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TAG2_ENABLED): {
            config_.tag2Enabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TAG2_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TAG3_ENABLED): {
            config_.tag3Enabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TAG3_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::WRITE_TAG4_ENABLED): {
            config_.tag4Enabled = reqIn.data_enabled;
            configChanged = true;
            AIS_LOG_NOTICE("Command::WRITE_TAG4_ENABLED = %d", reqIn.data_enabled);
            // no need to break out of the while loop for a simple config parameter update
            break;
        }
        case (cpm_common_interfaces::msg::JobMgrReqstChannelCommand::NONE):
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
bool LpsSaJobMgrApp::sendReqstResponse(const cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel& request, bool success) {
    // Build the response
    job_mgr_interfaces::msg::LpsSaJobMgrRespChannel response;
    response.time_point_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    response.app_name = request.app_name;
    response.app_request_id = request.app_request_id;
    response.command = request.command; // same nested message type both sides, whole-struct copy is correct
    response.success = success;

    if (LpsSaJobMgrRespChannelOutput_) {   /* Publish Response */
        if (LpsSaJobMgrRespChannelOutput_->publish(response)) {
            AIS_LOG_INFO("Published response, command=%d, success=%d", request.command.value, success);
            return true;
        }
    }

    AIS_LOG_ERROR("Failed to publish response, command=%d, success=%d", request.command.value, success);
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
    job_mgr_interfaces::msg::AisJhm2TxChannel AisJhm2TxIn;
    while (AisJhm2TxInputScs->get(AisJhm2TxIn)) {
        if (AisJhm2TxIn.simplecal_data.new_data_flag) {
            simpleCal_.eraseEntry(AisJhm2TxIn.simplecal_data.time_stamp);
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
        job_mgr_interfaces::msg::EventDiagnosticData data;

        // Clear TipoffAssistActiveEid
        LpsJobMgrJobTrackerInfoTbl.TipoffAssistActiveEid = 0;

        while (eddtInputChannel_->get(data)) {
            // getListOfEventDiagnostics() (EventDiagnosticData.h:129-131) is a
            // trivial `return &m_activeDiagnostics;` -- active_diagnostics is a
            // real vector member here, never null, so the pointer/null-check
            // dance collapses away.
            bool memoryFull = false;

            for (const auto& diag : data.active_diagnostics) {
                if (2 != diag.category) {
                    // only looking for events (not diagnostics)
                    continue;
                }

                if ((SCL_DCLI_FAULT_ACTIVE == diag.status) ||
                        (SCL_DCLI_FAULT_ACTIVE_AND_LOGGED == diag.status)) {
                    uint_fast16_t eid = diag.cid; // getCID() (Diagnostic.h:162-165) is `return cid;`
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
        job_mgr_interfaces::msg::ShmClockInput shmClock;
        while (ShmClockInputScs->get(shmClock)) {
            int32_t offset = shmClock.utc_offset_min;
            tzone_tx_comm_struct tzone;
            // Real get_tz_struct(tzone_tx_comm_struct&, ShmClock&) (ais_chrono/tz.hpp:287)
            // just extracts the raw 20-byte TZ blob via shmClock.get_TZ() and calls
            // the lower-level byte-array overload (tz.hpp:259) -- called that
            // overload directly instead, using the raw bytes already preserved
            // verbatim in ShmClockInput.msg's tzone_info field. No old-typed
            // ShmClock object needed at all.
            if (tes_common_ais::get_tz_struct(shmClock.tzone_info.data(), shmClock.tzone_info.size(), tzone)) {
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

            serviceHourMeter_ = shmClock.shm_sec;
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

    // !!! UNVERIFIED PLACEHOLDER -- DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PID
    // has no numeric value anywhere in this checkout either (see
    // DataLinkParam.msg header) -- same class of gap as HORN_PORT_NUMBER.
    constexpr uint8_t DATA_LINK_PARAM_IDENTIFIER_PID_UNVERIFIED = 0xF3;

    job_mgr_interfaces::msg::DataLinkData dlData;

    while (dataLinkDataInput_->get(dlData)) {
        for (auto& dlParam : dlData.params) {
            // Skip this if no new data is received.
            if (!dlParam.pid_data_received) {
                continue;
            }

            if (dlParam.identifier_type == DATA_LINK_PARAM_IDENTIFIER_PID_UNVERIFIED) {
                if (dlParam.param_id == STORE_BUTTON_PID) {
                    if (0 == dlParam.last_value_dsi) {
                        uint8_t dlValue = dlParam.last_good_value_u8;
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
                else if (dlParam.param_id == TIPOFF_ASSSIST_PID) {
                    if( 0 == dlParam.last_value_dsi) {
                        if (dlParam.last_value_eng == 0x000E) {
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
                        AIS_LOG_INFO("Rxed Data for PID 0x%X with Val =  %f   stat = %d ", dlParam.param_id, dlParam.last_value_eng, dlParam.last_value_dsi);
                    }
                    else {
                        /* Tipoff Assist not Active, if DSI */
                        LpsJobMgrJobTrackerInfoTbl.TipoffAssistPIDActive = false;
                    }
                }
                else if (dlParam.param_id == MANUAL_ADD_PID) {
                	if (0 == dlParam.last_value_dsi) {
                	    uint16_t dlValue = dlParam.last_good_value_u16;
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
    LpsSaWeighTxChannel rxParam;
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
            rxParam.LftSealStatus.sealed && // Now we are sealed
            weighAppTxDataReceived_ && dataReceived) { // Previous and current data is valid
        LpsSaJobMgrWmInput.clear_request_status = TRUE;
    }

    // Whether new data was received or not, the rxParam contains the latest data.
    LpsJobMgrJobTrackerInfoTbl.DigStat = rxParam.DigStat;
    LpsJobMgrJobTrackerInfoTbl.CalStat = rxParam.CalStat;
    LpsJobMgrJobTrackerInfoTbl.DumpStat = rxParam.DumpStat;
    LpsSaJobMgrWmInput.current_weight = rxParam.BestBktWtInTonnes;
    LpsSaJobMgrWmInput.calc_method = (unsigned int)rxParam.PayloadCalcMeth;
    LpsSaJobMgrWmInput.current_bucket_weight_latched = rxParam.bktWtLatchedFlag;
    LpsSaJobMgrWmInput.payload_latch_conditions_ok = rxParam.latchConditionsMet;
    LpsJobMgrJobTrackerInfoTbl.zeroWeight = rxParam.ZeroWeight;
    LpsJobMgrJobTrackerInfoTbl.simpleCalAdjust = rxParam.SimpleCalAdjust;
    LpsJobMgrJobTrackerInfoTbl.lftSealed = rxParam.LftSealStatus.sealed;
    LpsSaJobMgrWmInput.lift_stalled = rxParam.LiftStalled;

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
    // !!! UNVERIFIED PLACEHOLDERS -- DO NOT HARDWARE-TEST UNTIL FIXED !!!
    // OutputChannel::State::PORT_ON/PORT_OFF and
    // OutputChannel::ChangeDuration::NO_FLASH have no numeric value anywhere
    // in this checkout either (same class of gap as HORN_PORT_NUMBER above,
    // see Challenges-And-Decisions.txt 3.3). Sentinels below are
    // deliberately obviously-fake, not guesses -- confirm against
    // interfaces/OutputChannel/OutputChannel.h once sourced.
    constexpr uint8_t PORT_ON_UNVERIFIED  = 0xF0;
    constexpr uint8_t PORT_OFF_UNVERIFIED = 0xF1;
    constexpr uint8_t NO_FLASH_UNVERIFIED = 0xF2;

    job_mgr_interfaces::msg::OutputChannel outputChannel;
    job_mgr_interfaces::msg::OutputCmd command;
    command.output_port = HORN_PORT_NUMBER;
    command.initial_state = PORT_ON_UNVERIFIED;
    command.state_change_duration = NO_FLASH_UNVERIFIED;
    command.total_duration = 2;
    command.final_state = PORT_OFF_UNVERIFIED;
    outputChannel.commands.push_back(command);
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
    job_mgr_interfaces::msg::LpsSaJobMgrTxChannel txOut;
    // Original AIS Datum type set this in its constructor. The new message
    // type is plain data with no constructor side effects, so it's set
    // explicitly here instead, preserving the original "stamped at
    // construction" behavior.
    txOut.time_point_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    /* frame SCS channel data to UI*/
    if (LpsSaJobMgrScsTxOut) {

        { // Update current task load information
            const LpsSaLoadRecordChannelStorage& loadRecord = tasks_.getCurrentTaskLoad();
            const LpsSaLoadRecordSubtotal& subtotal = loadRecord.getCurrentSubtotal();
            txOut.task_number = tasks_.getCurrentTaskNumber();
            txOut.material_id = subtotal.materialId;
            txOut.material_name = subtotal.materialName;
            txOut.material_density = subtotal.materialDensity;
            txOut.truck_id = subtotal.truckId;
            txOut.truck_name = subtotal.truckName;
            txOut.truck_target_weight = subtotal.truckTargetWeightTonnes;

            if (0.0f != subtotal.truckTargetWeightTonnes) {
                txOut.remaining_weight = subtotal.truckTargetWeightTonnes - LpsSaJobMgrWmOutput.truck_weight;
            }
            else {
                txOut.remaining_weight = 0.0f;
            }

            txOut.tag1 = subtotal.tag1;
            txOut.tag2 = subtotal.tag2;
            txOut.tag3 = subtotal.tag3;
            txOut.tag4 = subtotal.tag4;
            txOut.custom_list_name1 = subtotal.customListName1;
            txOut.custom_list_name2 = subtotal.customListName2;
            txOut.custom_list_name3 = subtotal.customListName3;
            txOut.custom_list_name4 = subtotal.customListName4;

            txOut.pass_count               = LpsSaJobMgrWmOutput.passcount;
            txOut.truck_start_weight       = LpsSaJobMgrWmOutput.truck_start_weight;

            // Local computation still uses the real, untouched AIS enum type --
            // subtotal.weightTonnes()/loadRecord.weightTonnes() are business
            // logic (LpsSaLoadRecordSubtotal.h), not edited for this migration.
            // Converted into the new message's nested WeighBktWtAccuracy.value
            // only at the point of assignment below.
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

            txOut.truck_weight = subtotalWeight;
            txOut.truck_weight_accuracy.value = static_cast<uint8_t>(subtotalWeightAccuracy);

            txOut.total_weight = totalWeight;
            txOut.total_weight_accuracy.value = static_cast<uint8_t>(totalWeightAccuracy);

            totalWeightAccuracy_ = totalWeightAccuracy;

            // current load subtotal count
            txOut.subtotal_count = loadRecord.subtotalCount();

            txOut.target_type = (uint8_t)loadRecord.targetType();
            if (loadRecord.targetType() == LpsSaLoadRecordTargetType::SINGLE) {
                txOut.split_mode_enabled = false;
            }
            else {
                txOut.split_mode_enabled = true;
            }

            txOut.step_number = loadRecord.getCurrentSubtotalIndex();

            txOut.icon_type = subtotal.iconType;

            txOut.target_passes = subtotal.targetPasses;
        }

        txOut.operation_mode = static_cast<uint16_t>(LpsJobMgrJobTrackerInfoTbl.OperationMode);

        txOut.manual_tip_off_state    = static_cast<uint8_t>(LpsJobMgrJobTrackerInfoTbl.ManualTipOffState);
        txOut.tip_off_state.value     = static_cast<uint8_t>(LpsJobMgrJobTrackerInfoTbl.TipOffState);

        // Standby State
        if (LpsSaJobMgrWmOutput.standby_active) {
            txOut.standby_state.value = cpm_common_interfaces::msg::StandbyState::ACTIVATED;
        }
        else {
            txOut.standby_state.value = cpm_common_interfaces::msg::StandbyState::DEACTIVATED;
        }

        // Clear or -1 Button is Showing?
        if (LpsSaJobMgrWmOutput.show_clear_not_minus_one) {
            txOut.clear_minus_one_enable_stat = job_mgr_interfaces::msg::LpsSaJobMgrTxChannel::CLEAR_BTN_ENABLED;
        }
        else {
            txOut.clear_minus_one_enable_stat = job_mgr_interfaces::msg::LpsSaJobMgrTxChannel::MINUS_ONE_BTN_ENABLED;
        }

        txOut.disp_best_bkt_wt.val = LpsJobMgrJobTrackerInfoTbl.DispBestBktWt.val;
        txOut.disp_best_bkt_wt.is_ok = LpsJobMgrJobTrackerInfoTbl.DispBestBktWt.isOk;

        txOut.tip_off_trigger_type.value = static_cast<uint8_t>((LpsSaTipOffTriggerType_t)config_.tipOffTriggerType);
        txOut.tip_off_state_cfg.value = static_cast<uint8_t>((LpsSaJobMgrTipOffState_t)config_.tipOffMode);

        if (SEALegalForTradeInstalled_) {
            txOut.auto_store_pass_count = LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MAX;
        }
        else {
            txOut.auto_store_pass_count = config_.autoStorePassCount;
        }

        /* SEA Level2 (Pro) interlock */
        if (SEALevel2ProInstalled_) {
            txOut.auto_truck_id_enabled = config_.autoTruckIdEnabled;
            txOut.auto_material_id_enabled = config_.autoMaterialIdEnabled;
            txOut.manual_add_enabled = config_.manualAddEnabled;
            txOut.multi_task_enabled = config_.multiTaskEnabled;
            txOut.multi_task_count = tasks_.getNumberOfTasks();
            txOut.truck_list_enabled = config_.truckListEnabled;
            txOut.material_list_enabled = config_.materialListEnabled;
            txOut.tag1_enabled = config_.tag1Enabled;
            txOut.tag2_enabled = config_.tag2Enabled;
            txOut.tag3_enabled = config_.tag3Enabled;
            txOut.tag4_enabled = config_.tag4Enabled;
        }
        else {
            /* level2 (Pro) not installed, disable Pro features */
            txOut.auto_truck_id_enabled = false;
            txOut.auto_material_id_enabled = false;
            txOut.manual_add_enabled = false;
            txOut.multi_task_enabled = false;
            txOut.multi_task_count = 0;
            txOut.truck_list_enabled = false;
            txOut.material_list_enabled = false;
            txOut.tag1_enabled = false;
            txOut.tag2_enabled = false;
            txOut.tag3_enabled = false;
            txOut.tag4_enabled = false;
            txOut.split_mode_enabled = false;
        }

        if (LpsSaJobMgrWmOutput.tip_off_active) {
            txOut.tipoff_active = true;
        }
        else {
            txOut.tipoff_active = false;
        }

        txOut.tipoff_assist_active = LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive;
        txOut.tipoff_assist_active_eid = LpsJobMgrJobTrackerInfoTbl.TipoffAssistActiveEid;

        txOut.req_pload_ctrl_sys_stat = static_cast<uint16_t>(LpsJobMgrJobTrackerInfoTbl.ReqPloadCtrlSysStat);

        // simpleCal_.getSimpleCalData() is untouched business logic
        // (LpsSaJobMgrSimpleCal.h/.cpp) -- still fills the OLD deque<SimpleCalData_t>
        // type. Converted element-by-element into the new message's array,
        // since ROS2 arrays are std::vector, not std::deque, and the element
        // type itself is different too.
        {
            std::deque<SimpleCalData_t> simpleCalDataOld;
            simpleCal_.getSimpleCalData(simpleCalDataOld);
            txOut.simple_cal_data.clear();
            for (const auto& item : simpleCalDataOld) {
                job_mgr_interfaces::msg::SimpleCalData newItem;
                newItem.time_stamp = item.timeStamp;
                newItem.truck_wt = item.truckWt;
                newItem.zeroed_truck_wt = item.zeroedTruckWt;
                txOut.simple_cal_data.push_back(newItem);
            }
        }

        txOut.store_count = LpsJobMgrJobTrackerInfoTbl.storePressCount;

        // Show the "Payload Store:Not Available" info pop-up until at least this time is met.
        if (storeRejectedExpireTime > std::chrono::steady_clock::now()) {
            txOut.store_rejected = true;
        }
        else {
            txOut.store_rejected = false;
        }

        if (LpsSaJobMgrWmOutput.manual_add_available) {
            txOut.manual_add_available = true;
        }
        else {
            txOut.manual_add_available = false;
        }

        txOut.horn_store_state = config_.hornStoreEnable
            ? job_mgr_interfaces::msg::LpsSaJobMgrTxChannel::HORN_SOUND
            : job_mgr_interfaces::msg::LpsSaJobMgrTxChannel::HORN_NOT_SOUND;

        // LFT disabled state for current task
        txOut.lft_disabled = tasks_.currentTaskGetLFTDisable();

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
    LpsSaWeighReqstChannel request;

    switch (command) {
    case (LpsSaWeighReqstChannel::Command::RESET_BEST_BUCKET_WEIGHT):
    case (LpsSaWeighReqstChannel::Command::CAPTURE_CYLINDER_EXTENSION_REFERENCE):
    case (LpsSaWeighReqstChannel::Command::CLEAR_REWEIGH_WARNING): {
        request.command = command;
        break;
    }
    default: {
        AIS_LOG_ERROR("Unsupported weigh app request command.");
        return;
    }
    }

    if (!weighAppInf_.sendRequest(request)) {
        AIS_LOG_ERROR("Failed to send weigh app request.");
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
