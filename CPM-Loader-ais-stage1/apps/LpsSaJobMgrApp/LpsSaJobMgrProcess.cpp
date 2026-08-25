/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaJobMgrProcess.cpp
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <stdio.h>

#include "LpsSaJobMgrApp.h"
#include <clock_tm_zone_proto.h>
#include <chrono>

#include <chrono/print.hpp>

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
/*******************************************************************************
** -- Function Prototypes --
*******************************************************************************/
/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/


/******************************************************************************
FUNCTION NAME:LpsSaJobMgrPtUpdate
DESCRIPTION: Handles inputs and outputs of PT lib.           
PARAMETER DESCRIPTION:                        
RETURN VALUE:             
*******************************************************************************/
boolean LpsSaJobMgrApp::LpsSaJobMgrPtUpdate(void)
{
    LpsSaJobMgrWmInput.cpm_install_status = true;

    if(LpsJobMgrJobTrackerInfoTbl.CalStat == LPS_WEIGH_SYSTEM_CALIBRATED)
    {
        LpsSaJobMgrWmInput.cpm_is_calibrated = true;
    }

    LpsSaJobMgrWmInput.pt_time_step_us = 100000.00;

    if(LpsJobMgrJobTrackerInfoTbl.DumpStat == LPS_WEIGHT_BKT_PARTIALLY_DUMPED)
    {
        LpsSaJobMgrWmInput.dump_state_is_partial_dump = true;
    }
    else if(LpsJobMgrJobTrackerInfoTbl.DumpStat == LPS_WEIGHT_BKT_FULLY_DUMPED)
    {
        // Full dump is beyond partial dump, therefore partial dump is also true.
        LpsSaJobMgrWmInput.dump_state_is_partial_dump = true;
        LpsSaJobMgrWmInput.dump_state_is_full_dump = true;
    }

    if (LPS_WEIGHT_BKT_DIGGING == LpsJobMgrJobTrackerInfoTbl.DigStat)
    {
        LpsSaJobMgrWmInput.dig_detected = true;
    }

    /* SEA Level2 (Pro) Interlock */
    if (SEALevel2ProInstalled_) {
        LpsSaJobMgrWmInput.manual_add_enabled = config_.manualAddEnabled;
    }
    else {
        /* not installed, disable following Pro features */
        LpsSaJobMgrWmInput.manual_add_enabled = false;
    }

    LpsSaJobMgrWmInput.tip_off_trigger = config_.tipOffTriggerType;

    if (SEALegalForTradeInstalled_) {
        LpsSaJobMgrWmInput.auto_store_enabled = false;
        LpsSaJobMgrWmInput.maximum_pass_count = LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MAX;
    }
    else {
        LpsSaJobMgrWmInput.auto_store_enabled = true;
        LpsSaJobMgrWmInput.maximum_pass_count = config_.autoStorePassCount;
    }

    /*
     * Update the Pass Tracker State Machine (weigh_mode)
     *  But not unless we have received input from the weigh app.
     *  We cannot make any assumptions about the inputs from the weigh app
     *  prior to updating pass tracker.  For example, the calibration status is
     *  unknown until we receive it from the weigh app.
     */
    if (weighAppTxDataReceived_) {
        LpsSaJobMgrWmOutput = LpsPtUpdate(&LpsSaJobMgrWmInput);
    }

    /*
     * Handle Load Update
     */
    {
        bool changed = false;

        if (LpsSaJobMgrWmOutput.loadUpdate.addPass) {
            tasks_.currentTaskLoadAddPass(LpsSaJobMgrWmOutput.loadUpdate.addPassWeight,
                    LpsSaJobMgrWmOutput.loadUpdate.addPassAccuracy, serviceHourMeter_);
	        AIS_LOG_NOTICE("Load Change - addPass");
            changed = true;
        }

        if (LpsSaJobMgrWmOutput.loadUpdate.removePass) {
            bool removedPass = tasks_.currentTaskLoadRemoveLastPass(serviceHourMeter_);

            if (removedPass) {
                // restore truck with updated load
                const LpsSaLoadRecordSubtotal& subtotal = tasks_.getCurrentTaskLoad().getCurrentSubtotal();
                LpsWeighBktWtAccuracy_t lastPassAccuracy;
                float lastPassWeight = subtotal.lastPassWeightTonnes(lastPassAccuracy);
                LpsPtRestoreTruck(subtotal.weightTonnes(), subtotal.passCount(), FALSE, lastPassWeight, lastPassAccuracy);
            }
            else {
                LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck
            }

            AIS_LOG_NOTICE("Load Change - removePass");
            changed = true;
        }

        if (LpsSaJobMgrWmOutput.loadUpdate.clear) {
            tasks_.currentTaskLoadClearCurrentSubtotal();
            AIS_LOG_NOTICE("Load Change - clear");
            changed = true;
        }

        // Publish load records on store only if SEA level1 is installed, and if store was not due to split mode
        //  next subtotal cmd or select subtotal cmd
        if (LpsSaJobMgrWmOutput.loadUpdate.store
                && !LpsJobMgrJobTrackerInfoTbl.splitModeNextPayloadCmd
                && !LpsJobMgrJobTrackerInfoTbl.selectSubtotalCmd
                && SEALevel1EssentialsInstalled_) {
            // Honk the horn if it is not an auto store and SEA level2 (Pro) is installed
            if (!LpsSaJobMgrWmOutput.loadUpdate.isAutoStore
                    && SEALevel2ProInstalled_) {
                LpsSaJobMgrScsChkHornAction();
            }

            { // Close out the current tasks load and sent it on to downstream consumers.
                LpsSaLoadRecordChannel loadRecord;

                { // Get load record
                    LpsSaLoadRecordChannelStorage& loadRecordData = loadRecord;
                    loadRecordData = tasks_.currentTaskLoadClose(serviceHourMeter_);
                }

                if (loadRecord.passCount() > 0) {
                    { // remove subtotals with zero passes
                        // looping backwards from the end so we can remove items along the way without
                        //  ruining the indexing.
                        for (auto idx = loadRecord.subtotalCount(); idx > 0; idx--) {
                            if (0 == loadRecord.getSubtotalByIndex(idx).passCount()) {
                                // remove subtotal
                                loadRecord.removeSubtotal(idx);
                            }
                        }
                    }

                    LpsJobMgrJobTrackerInfoTbl.storePressCount++;

                    // Set the reason for store.
                    if (LpsSaJobMgrWmOutput.loadUpdate.isAutoStore) {
                        loadRecord.storeAction(LpsSaLoadRecordStoreAction::AUTO);
                    }
                    else if (SEALegalForTradeInstalled_) {
                        // If nobody disabled LFT, then it is LFT
                        if (loadRecord.storeAction() != LpsSaLoadRecordStoreAction::TICKET_NOT_FOR_TRADE) {
                            loadRecord.storeAction(LpsSaLoadRecordStoreAction::TICKET_LEGAL_FOR_TRADE);
                        }
                    }
                    else {
                        loadRecord.storeAction(LpsSaLoadRecordStoreAction::TICKET_NOT_FOR_TRADE);
                    }

                    // disable lists if SEA Level2 is not installed
                    if (!SEALevel2ProInstalled_) {
                        loadRecord.disableLists();
                    }

                    /*
                     * If user tags are not enabled, then clear them out of the published
                     * load record.  This means that the last set values for these tags
                     * will be retained if the tags get enabled again.  We need to clear
                     * them out here so that we don't have incorrect "orphaned" tags
                     * sent to the back office and/or accumulated in totals.
                     */

                    if (!config_.truckListEnabled) {
                        loadRecord.disableTruckList();
                    }

                    if (!config_.materialListEnabled) {
                        loadRecord.disableMaterialList();
                    }

                    if (!config_.tag1Enabled) {
                        loadRecord.disableTag1List();
                    }

                    if (!config_.tag2Enabled) {
                        loadRecord.disableTag2List();
                    }

                    if (!config_.tag3Enabled) {
                        loadRecord.disableTag3List();
                    }

                    if (!config_.tag4Enabled) {
                        loadRecord.disableTag4List();
                    }

                    // set zero/cal adjust for last subtotal
                    loadRecord.getCurrentSubtotal().setZeroWeight(LpsJobMgrJobTrackerInfoTbl.zeroWeight);
                    loadRecord.getCurrentSubtotal().setSimpleCalAdjust(LpsJobMgrJobTrackerInfoTbl.simpleCalAdjust);

                    // Add load to simple cal
                    simpleCal_.addEntry(loadRecord);

                    // Publish the load record
                    if (nullptr != loadRecordOutputChannel_) {
                        loadRecordOutputChannel_->publish(loadRecord);
                    }
                }
            }

            AIS_LOG_NOTICE("Load Change - store");
            changed = true;
        }

        if (LpsJobMgrJobTrackerInfoTbl.splitModeNextPayloadCmd) {
            LpsSaLoadRecordChannelStorage& load = tasks_.getCurrentTaskLoad();
            LpsSaLoadRecordSubtotal& subtotal = load.getCurrentSubtotal();

            // set the zero and cal adjust for subtotal being closed out
            subtotal.setZeroWeight(LpsJobMgrJobTrackerInfoTbl.zeroWeight);
            subtotal.setSimpleCalAdjust(LpsJobMgrJobTrackerInfoTbl.simpleCalAdjust);

            // add new subtotal
            load.nextSubtotal(serviceHourMeter_);

            LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck

            //reset command
            LpsJobMgrJobTrackerInfoTbl.splitModeNextPayloadCmd = false;

            AIS_LOG_NOTICE("Load Change - next payload subtotal");
            changed = true;
        }

        if (LpsJobMgrJobTrackerInfoTbl.selectSubtotalCmd) {
            LpsSaLoadRecordChannelStorage& load = tasks_.getCurrentTaskLoad();
            LpsSaLoadRecordSubtotal& subtotal = load.getCurrentSubtotal();

            // set the zero and cal adjust for subtotal being closed out
            subtotal.setZeroWeight(LpsJobMgrJobTrackerInfoTbl.zeroWeight);
            subtotal.setSimpleCalAdjust(LpsJobMgrJobTrackerInfoTbl.simpleCalAdjust);

            // select subtotal
            load.selectSubtotal(serviceHourMeter_, LpsJobMgrJobTrackerInfoTbl.subtotalIndex);

            LpsSaJobMgrPtRestoreTruck(); // Make sure the information gets reflected in the pass tracker truck

            //reset command
            LpsJobMgrJobTrackerInfoTbl.selectSubtotalCmd = false;

            AIS_LOG_NOTICE("Load Change - select subtotal");
            changed = true;
        }

        if (changed) {
            tasks_.save();
        }
    }

    // Payload Operating Mode (D10748)
    if(!LpsSaJobMgrWmInput.cpm_is_calibrated) {
        LpsJobMgrJobTrackerInfoTbl.OperationMode = LPS_SA_JOB_MGR_UNCALIBRATED; // Not calibrated
    }
    else if (LpsSaJobMgrWmOutput.standby_active) {
        LpsJobMgrJobTrackerInfoTbl.OperationMode = LPS_SA_JOB_MGR_STANDBY_MODE;  // In Standby
    }
    else if (LpsSaJobMgrWmOutput.tip_off_active) {
        LpsJobMgrJobTrackerInfoTbl.OperationMode = LPS_SA_JOB_MGR_EXCESS_MODE;   // In tip off
    }
    else {
        LpsJobMgrJobTrackerInfoTbl.OperationMode = LPS_SA_JOB_MGR_WEIGH_MODE;   // Normal mode
    }

    LpsJobMgrJobTrackerInfoTbl.ReqPloadCtrlSysStat = GetActiveButtonStatus();

    LpsJobMgrJobTrackerInfoTbl.DispBestBktWt.val = LpsSaJobMgrWmOutput.display_bucket_weight;

    LpsSaJobMgrSendCmdToWeighApp();

    resetLpsPtInputFlags();   // Reset the flags and then handle tipoff.

    // Did tip-off assist become active or inactive like requested?
    if (!LpsSaJobMgrWmOutput.tip_off_active) {
        // If we are not in tip-off, we are definitely not in tip-off assist
        LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive = false;
    }
    else if (LpsJobMgrJobTrackerInfoTbl.TipoffAssistPIDActive != LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive) {
        // This is the result of the request to enter or exit tip-off.
        LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive = LpsSaJobMgrWmOutput.tip_off_active;
    }

    LpsSaJobMgrHandleTipOffBtnStates();

    //Set flags to show bucket weight on the display.
    if (TRUE == LpsSaJobMgrWmOutput.display_bucket_weight_available)
    {
        LpsJobMgrJobTrackerInfoTbl.DispBestBktWt.isOk = true;
    }
    else
    {
        /* Sending display status to UI to display *** at Bucket weight */
        LpsJobMgrJobTrackerInfoTbl.DispBestBktWt.isOk = false;
    }

    return SUCCESS;
}


/******************************************************************************
FUNCTION: LpsSaJobMgrScsChkHornAction
DESCRIPTION: Raise Action Flag when it fit the condition.
PARAMETER DESCRIPTION:
RETURN VALUE:void
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrScsChkHornAction()
{
    if (config_.hornStoreEnable) {
        AIS_LOG_INFO("Horn: ACTIVACTED");
        LpsSaJobMgrHornOnStoreAction(); // send scs object to OutputApp to drive Output Pin
    }
    else {
        AIS_LOG_INFO("Horn: NOT ACTIVATED");
    }
}


/******************************************************************************
FUNCTION:LpsSaJobMgrHandleTipOffBtnStates
DESCRIPTION:Handles whether to show or hide tip off buttons.
PARAMETER DESCRIPTION:
RETURN VALUE:void
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrHandleTipOffBtnStates()
{
    LpsJobMgrJobTrackerInfoTbl.TipOffState = LPS_SA_JOB_MGR_TIP_OFF_UNAVAILABLE;
    LpsJobMgrJobTrackerInfoTbl.ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_UNAVAILABLE;

    // Tip Off manual
    if (LpsSaJobMgrWmInput.tip_off_trigger == TIP_OFF_TRIGGER_MANUAL)
    {
    	if (LpsSaJobMgrWmOutput.tip_off_active)
    	{
    		LpsJobMgrJobTrackerInfoTbl.ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_ACTIVE;
    	}
    	else if (LpsSaJobMgrWmOutput.display_bucket_weight_available)
    	{
    		LpsJobMgrJobTrackerInfoTbl.ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_AVAILABLE;
    	}
    	else {
    		LpsJobMgrJobTrackerInfoTbl.ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_UNAVAILABLE;
    	}
    }
    else {
    	LpsJobMgrJobTrackerInfoTbl.ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_UNAVAILABLE;
    }
    // Tip Off Auto or Disabled or Tipoff Assist is Active
    if (LpsSaJobMgrWmOutput.tip_off_active && (!LpsJobMgrJobTrackerInfoTbl.TipoffAssistActive))
    {
    	LpsJobMgrJobTrackerInfoTbl.TipOffState = (LpsSaJobMgrTipOffState_t)LpsSaJobMgrWmOutput.tip_off_mode;
    }
    else {
    	LpsJobMgrJobTrackerInfoTbl.TipOffState = LPS_SA_JOB_MGR_TIP_OFF_UNAVAILABLE;

    }
}

/******************************************************************************
FUNCTION:LpsSaJobMgrSendCmdToWeighApp
DESCRIPTION:Send appropriate commands to weigh app depending on flags set. 
PARAMETER DESCRIPTION:
RETURN VALUE:void
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrSendCmdToWeighApp()
{
    if (LpsSaJobMgrWmOutput.unlatch_current_bucket_weight) {
        LpsSaJobMgrScsSendCmd(LpsSaWeighReqstChannel::Command::RESET_BEST_BUCKET_WEIGHT);
    }

    if (LpsSaJobMgrWmOutput.dump_detect_capture_cyl_ext_reference) {
        LpsSaJobMgrScsSendCmd(LpsSaWeighReqstChannel::Command::CAPTURE_CYLINDER_EXTENSION_REFERENCE);
    }

    if (LpsSaJobMgrWmOutput.clear_reweigh_warning_status) {
        LpsSaJobMgrScsSendCmd(LpsSaWeighReqstChannel::Command::CLEAR_REWEIGH_WARNING);
    }
}
