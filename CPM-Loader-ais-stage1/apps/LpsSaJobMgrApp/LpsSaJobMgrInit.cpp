/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaJobMgrInit.cpp
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include "LpsSaJobMgrApp.h"

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/

/******************************************************************************
FUNCTION: resetLpsPtInputFlags
DESCRIPTION: Resets some of the input flags.
PARAMETER DESCRIPTION:
RETURN VALUE:void
*******************************************************************************/
static void resetLpsPtInputFlags(LpsPtInputs_t& in) {
    in.cpm_is_calibrated = FALSE;
    in.dump_state_is_partial_dump = FALSE;
    in.dump_state_is_full_dump = FALSE;
    in.dig_detected = FALSE;
    in.tip_off_mode_request_status = FALSE;
    in.store_request_status = FALSE;
    in.reweigh_request_status = FALSE;
    in.zero_request_status = FALSE;
    in.minus_one_request_status = FALSE;
    in.clear_request_status = FALSE;
    in.tipoff_toggle_request_status = FALSE;
    in.standby_request_status = FALSE;
    in.change_mode_weigh = FALSE;
    in.change_mode_excess = FALSE;
    in.manual_add_request = FALSE;
}

/******************************************************************************
FUNCTION: initLpsPtInput
DESCRIPTION: Initializes the entire LpsPtInputs_t structure
PARAMETER DESCRIPTION:
RETURN VALUE:void
*******************************************************************************/
static void initLpsPtInput(LpsPtInputs_t& in, const LpsPtInit_t& init) {
    resetLpsPtInputFlags(in);

    in.cpm_install_status = FALSE;
    // cpm_is_calibrated
    in.pt_time_step_us = 100000.00;

    in.weigh_range_is_weighing = FALSE;
    in.calc_method = 0;
    in.current_weight = 0.f;
    in.lift_stalled = FALSE;
    in.payload_latch_conditions_ok = FALSE;
    // dump_state_is_partial_dump
    // dump_state_is_full_dump
    in.current_bucket_weight_latched = FALSE;
    // dig_detected

    in.tip_off_trigger = TIP_OFF_TRIGGER_DISABLED;

    // tip_off_mode_request_status
    in.tip_off_mode = init.tip_off_mode;

    // store_request_status
    // reweigh_request_status
    // zero_request_status
    // minus_one_request_status
    // clear_request_status
    // tipoff_toggle_request_status
    // standby_request_status
    // change_mode_weigh
    // change_mode_excess

    in.maximum_pass_count = LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_DEFAULT;
    in.auto_store_enabled = FALSE;

    in.manual_add_enabled = FALSE;
    // manual_add_request
}

/******************************************************************************
FUNCTION NAME:LpsSaJobMgrApp::LpsSaJobMgrInit()
DESCRIPTION:Initialize the pass tracker Lib
PARAMETER DESCRIPTION:
RETURN VALUE:LpsSaJobMgrInitErrorType_t
*******************************************************************************/
void LpsSaJobMgrApp::LpsSaJobMgrPtInit()
{
    LpsPtInit_t init;

	init.tip_off_mode = config_.tipOffMode;

	LpsSaJobMgrWmOutput = LpsPtInit(&init);

	LpsSaJobMgrPtRestoreTruck();

    initLpsPtInput(LpsSaJobMgrWmInput, init);
}

void LpsSaJobMgrApp::LpsSaJobMgrPtRestoreTruck() {
    const LpsSaLoadRecordSubtotal& subtotal = tasks_.getCurrentTaskLoad().getCurrentSubtotal();
    if (0 == subtotal.passCount()) { //no passes
        LpsPtRestoreTruck(0.f, 0, FALSE, 0.f, LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_NONE);
    }
    else {
        LpsWeighBktWtAccuracy_t lastPassAccuracy;
        float lastPassWeight = subtotal.lastPassWeightTonnes(lastPassAccuracy);
        LpsPtRestoreTruck(subtotal.weightTonnes(), subtotal.passCount(), TRUE, lastPassWeight, lastPassAccuracy);
    }
}

/******************************************************************************
FUNCTION:resetLpsPtInputFlags
DESCRIPTION:Resets some of the input flags.
PARAMETER DESCRIPTION:
RETURN VALUE:void
*******************************************************************************/
void LpsSaJobMgrApp::resetLpsPtInputFlags()
{
    ::resetLpsPtInputFlags(LpsSaJobMgrWmInput);
}
