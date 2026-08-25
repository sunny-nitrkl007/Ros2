/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaLinkageCal.cpp
DESCRIPTION:This file provides the update routines for the application software
            for LPS library.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef  _LPS_SA_WEIGHAPP_H_
#include "LpsSaWeighApp.h"
#endif

/*
**  --- Globals ---
*/
CAL_Linkage_Work_t         cal_linkage_workspace;


/*
**  --- Global Functions ---
*/
CAL_MGR_MR_E LpsSaWeighApp::LpsSaTiltRotaryLinkageSensorCalibration(
        CAL_MGR_MC_E Cmd, unsigned_16 CalId, unsigned_8* StepNo, unsigned_16* CalError)
{
    (void)cmc;
    *CalError = 0;

    switch (Cmd) {
        /********************/
        /* Control Commands */
        /********************/
        case (CAL_MGR_MC_INIT_LAST_STAT): {
            /*
            **  This command is sent when the application has requested the
            **  CAL_MGR to enable the calibration from the application with
            **  cal_mgr_enable_cal( CAL_NEUTSETPT_ID ) call.  This command
            **  is not sent during the actual execution of the calibration
            **  through a service tool.
            **  The possible responses are as follows:
            **      OK - the last status report is supported and
            **          the last status report cache has been initialized
            **          correctly.
            **      NOT_OK - the last status resport is required or
            **          the last status report cache has not been initialized
            **          correctly.
            **      NA - Last status report not used (not applicable) and
            **          allow the enabling of the calibration
            */
            return CAL_MGR_MR_INIT_LAST_STAT_OK;
        }

        case (CAL_MGR_MC_GET_INPUTS): {
            /*
            **  This "Control" command is intended to be used to acquire the calibration
            **  inputs required by the calibration from the application.  This
            **  is the first command sent on an execution loop.  Validation
            **  checks on the inputs are to be handled within the CAL_ADIAGCHK
            **  command.
            **
            **  The possible responses are as follows:
            **      OK - Inputs were processed/acquired successfully
            */
            return CAL_MGR_MR_GET_INPUTS_OK;
        }

        case (CAL_MGR_MC_SET_OUTPUTS): {
            /*
            **  This "Control" command is intended to be used to send the calibration
            **  outputs of the calibration to the application.  Validation
            **  checks on the outputs are to be handled within the CAL_ADIAGCHK
            **  command.  This is the last command sent on an execution loop.
            **
            **  The possible responses are as follows:
            **      OK - Outputs were processed successfully
            */
            return CAL_MGR_MR_SET_OUTPUTS_OK;
        }

        case (CAL_MGR_MC_INITIALIZE): {
            /*
            **  This "Control" command is sent when the user has selected the calibration
            **  from the service tool assuming cal_mgr_enable_cal( CAL_NEUTSETPT_ID )
            **  was successful.  This "Control" command is a high-level intialization of the
            **  calibration intended to be used to notify the application that the
            **  calibration has begun, presenting an opportunity to begin actively
            **  overriding certain components of the application or initializing
            **  qualified read parameters.  It follows GET_INPUTS command on an
            **  execution loop until response is OK.
            **
            **  The possible responses are as follows:
            **      OK - initialization was completed successfully, after returning
            **          OK, the CAL_MGR will wait for the user to start the calibration
            **          by pressing the (Next>>) button the first time.  This
            **          command will not get called again until the calibration
            **          has ended its execution.
            **      AGAIN - if initialization requires more than one loop
            **          to complete
            **      NOT_OK - intialization was not completed successfully
            */

            /* Fail calibrations if machine/model is not selected */
            if (TRUE == LpsSaWeighInfoTbl.MachineModelNotSet) {
                *CalError = CAL_MACHINE_MODEL_NOT_SET;
                return CAL_MGR_MR_INITIALIZE_NOT_OK;
            }

            /* Fail calibrations if Legal for Trade is Sealed */
            if (weighUpdtTbl.LegalForTradeSealed) {
                *CalError = CAL_PAYLOAD_LEGAL_FOR_TRADE_SEALED;
                return CAL_MGR_MR_INITIALIZE_NOT_OK;
            }

            if (TILT_SENSOR_TYPE_ROTARY != linkage_table_cnfg.tiltSensorType) {
                *CalError = 0x000B;
                return CAL_MGR_MR_INITIALIZE_NOT_OK;
            }

            /* Set linkage cal is in progress */
            LinkageCalInProgress = true;

            return CAL_MGR_MR_INITIALIZE_OK;
        }

        case (CAL_MGR_MC_ADIAGCHK): {
            /*
            **  This "Control" command is intended to be used to check active diagnostics
            **  for calibration inputs, outputs and any other high-level
            **  requirements of the calibration.  This "Control" command is sent just prior
            **  to SETUPCHK and FLOW commands only.
            **
            **  The possible responses are as follows:
            **      OK - No "Active Diagnostics" are present that will effect the
            **          execution of the calibration.
            **      NOT_OK - an "Active Diagnostic" is present that will effect the
            **          execution of the calibration.  The generic
            **          CAL_MGR_EW_ACTIVE_DIAGNOSTIC will be automatically issued by
            **          CAL_MGR if NOT_OK is the response.  Further
            **          cal_add_error() may be required here for the calibration.
            **          QUIT or TERMINATE command will be sent on the next
            **          execution loop.  SETUPCHK and FLOW commands will not
            **          be called on this execution loop due to this response.
            */
            if ((LPS_STATUS_BAD == LpsSaWeighInfoTbl.LiftPosition.status) ||
                    (LPS_STATUS_BAD == LpsSaWeighInfoTbl.TiltPosition.status)) {
                *CalError = CAL_LINKAGE_ABORTED_BY_ECM;
                return CAL_MGR_MR_ADIAGCHK_NOT_OK;
            }
            else {
                return CAL_MGR_MR_ADIAGCHK_OK;
            }
        }

        case (CAL_MGR_MC_SETUPCHK): {
            /*
            **  This "Control" command is intended to be used to check the setup conditions
            **  for the calibration prior to moving to the FLOW portion of the
            **  calibration.  This "Control" command follows the ADIAGCHK command when response
            **  from ADIAGCHK is OK.
            **
            **  The possible responses are as follows:
            **      OK - All setup conditions have been met and CAL_MGR_STEPNUMBER(cmw)
            **          has been set to the first FLOW step number.
            **      NOT_OK - All setup conditions have not been met and
            **          LpsSaWeighInfoTbl.stepNo has been set to the step number
            **          associated with the setup condition not being met.
            */
            LpsSaWeighInfoTbl.stepNo = CAL_TILT_LINKAGE_SETUP_COMPLETE;
            return CAL_MGR_MR_SETUPCHK_OK;
        }

        case (CAL_MGR_MC_INIT_FLOW): {
            /*
            **  This "Control" command is intended to be used to intialize the FLOW portion
            **  of a calibration.  This "Control" command is a lower-level intialization of the
            **  flow portion of the calibration which can be used to notify the
            **  application that the calibration flow portion has begun,
            **  presenting an opportunity to begin actively overriding additional
            **  components of the application.  It may also be used to set
            **  LpsSaWeighInfoTbl.stepNo, in the event that calibration has been
            **  "restarted internally", (i.e. not "restarted by service tool").
            **
            **  The possible responses are as follows:
            **      OK - Calibration Flow parameters have been initialized.
            */
            AIS_LOG_INFO("CAL_MGR_MC_INIT_FLOW");
            LpsSaWeighInfoTbl.stepNo = CAL_TILT_LINKAGE_FULL_RACK;
            return CAL_MGR_MR_INIT_FLOW_OK;
        }

        case (CAL_MGR_MC_FLOW): {
            /*
            **  This "Control" command is intended to be used to check conditions required
            **  by a FLOW step.
            **
            **  The possible responses are as follows:
            **      OK      - Flow conditions are valid, FLOW commands will follow.
            **      RESTART - A Flow condition is not valid and a "restart by calibration"
            **          is required.  The FLOW step may be recorded internally
            **          for "return to FLOW" capability.
            **      ABORT   - A Flow condition is not valid and will require
            **          the calibration to TERMINATE.  cal_add_error()'s may
            **          be called here to explicitly provide which Flow condition
            **          was not met.
            **
            */
            return CAL_MGR_MR_FLOW_OK;
        }

        case (CAL_MGR_MC_RESTART_FLOW): {
            /*
            **  This "Control" command is intended to be used to internally restart a
            **  calibration for some reason.  ADIAGCHK command is not sent during
            **  a restart.  LpsSaWeighInfoTbl.stepNo has been set to 1 on restart.
            **
            **  The possible responses are as follows:
            **      OK      - The internal restarting process has completed, optional
            **          setting of LpsSaWeighInfoTbl.stepNo can be applied to start
            **          at a specific step number
            **      AGAIN   - The internal restarting process requires more than one
            **          execution loop.
            */
            return CAL_MGR_MR_RESTART_FLOW_OK;
        }

        case (CAL_MGR_MC_STOP): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) before
            **  the calibration has "started" (meaning the user has yet to press
            **  the Next>> button to start the calibration)
            **  The application may choose to set the status of the calibration to
            **  NOT_CALIBRATED since the calibration was stopped.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible prior to "started"
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated INITIALIZE command. The EXIT_FLOW
            **          command will follow with this response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_STOP_EXIT_REQ;
        }

        case (CAL_MGR_MC_QUIT): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) before
            **  the calibration setup conditions have been met (prior to executing
            **  the flow portion of the calibration, TERMINATE sent on restarts);
            **  The application may choose to set the status of the calibration to
            **  NOT_CALIBRATED since the calibration was quit.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible prior to the
            **          INIT_FLOW command.
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated INITIALIZE command.
            **          The EXIT_FLOW command will follow with this response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_QUIT_EXIT_REQ;
        }

        case (CAL_MGR_MC_TERMINATE): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) after the
            **  INIT_FLOW command had been sent at least once.  The application may
            **  choose to set the status of the calibration to NOT_CALIBRATED since
            **  the calibration was terminated.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated by INITIALIZE, INIT_FLOW, and/or FLOW
            **          commands. The EXIT_FLOW command will follow with this
            **          response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_TERMINATE_EXIT_REQ;
        }

        case (CAL_MGR_MC_EXIT_FLOW): {
            /*
            **  This "Control" command is intended to be used to check if any calibration
            **  overrides are active that require a cleanup of more than one
            **  execution loop.  This command will be called at most once per
            **  calibration execution.
            **
            **  The possible responses are as follows:
            **      DONE        - No cleanup is required (no active overrides exist)
            **      CLEANUP_REQ - Cleanup is required to clear overrides.
            **          CLEANUP command will follow with this response.
            */
            return CAL_MGR_MR_EXIT_FLOW_CLEANUP_REQ;
        }
        case (CAL_MGR_MC_CLEANUP): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user).  The
            **  CLEANUP_REQ response to the EXIT_FLOW command indicates that an
            **  active overrides exist that will possibly take more than one
            **  execution loop to clear.  The cleanup procedure is executed within
            **  this command.
            **
            **  The possible responses are as follows:
            **      DONE        - No active overrides exist
            **      NOT_DONE    - More execution loops are required to clear
            **          active overrides.
            */
            return CAL_MGR_MR_CLEANUP_DONE;
        }

        /*****************/
        /* Flow Commands */
        /*****************/
        case (CAL_MGR_MC_READY): {
            /*
            **  This "Flow" command is sent to evaluate whether or not the
            **  calibration is "READY" to collect a sample.  The sample/s that
            **  is/are to be collected is/are most likely associated with a
            **  flow step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      YES - The calibration is ready to begin the sampling
            **          process associated with the current flow step.
            **      NO  - The calibration is not ready to begine the sampling
            **          process associated with the current flow step.
            */
            return CAL_MGR_MR_READY_YES;
        }

        case (CAL_MGR_MC_SAMPLE_INIT): {
            /*
            **  This "Flow" command is sent to intialize the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      OK  - Initialization for the sampling process associated with
            **          the current flow step (LpsSaWeighInfoTbl.stepNo) is complete.
            */
            AIS_LOG_INFO("CAL_MGR_MC_SAMPLE_INIT");
            LpsSaWeighInfoTbl.stepNo = CAL_TILT_LINKAGE_FULL_RACK;
            return CAL_MGR_MR_SAMPLE_INIT_OK;
        }

        case (CAL_MGR_MC_SAMPLE): {
            /*
            **  This "Flow" command is sent during the execution of the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      PT_COLLECTED - Sample point/s was/were collected that require
            **          validation after RESET
            **      WAIT - Sample Process is executing and requires another
            **          execution loop.
            **      SKIP - Sampling process has completed, no validation is required
            **          following RESET.
            */
            bool done = false;

            AIS_LOG_INFO("CAL_MGR_MC_SAMPLE");

            if (CAL_MGR_KEY_CONT) {
                AIS_LOG_INFO("CAL_MGR_KEY_CONT : STEPNO %d", LpsSaWeighInfoTbl.stepNo);

                switch (LpsSaWeighInfoTbl.stepNo) {
                    case (CAL_TILT_LINKAGE_FULL_RACK): {
                        AIS_LOG_INFO("CAL_TILT_LINKAGE_FULL_RACK");
                        cal_linkage_workspace.tilt_full_rack_dc = WeighPidTbl.TiltLinkageSensorDc;
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_LINKAGE_FULL_LOWER;
                        break;
                    }

                    case (CAL_TILT_LINKAGE_FULL_LOWER): {
                        AIS_LOG_INFO("CAL_TILT_LINKAGE_FULL_LOWER");
                        cal_linkage_workspace.lift_full_lower_dc = WeighPidTbl.LiftLinkageSensorDc; // Not Used
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_LINKAGE_FULL_RAISE;
                        break;
                    }

                    case (CAL_TILT_LINKAGE_FULL_RAISE): {
                        AIS_LOG_INFO("CAL_TILT_LINKAGE_FULL_RAISE");
                        cal_linkage_workspace.lift_full_raise_dc = WeighPidTbl.LiftLinkageSensorDc; // Not Used
                        LpsSaWeighInfoTbl.stepNo =CAL_TILT_LINKAGE_FULL_DUMP;
                        break;
                    }

                    case (CAL_TILT_LINKAGE_FULL_DUMP): {
                        AIS_LOG_INFO("CAL_TILT_LINKAGE_FULL_DUMP");
                        cal_linkage_workspace.tilt_full_dump_dc = WeighPidTbl.TiltLinkageSensorDc;
                        done = true;
                        break;
                    }

                    default: {
                        AIS_LOG_WARN("DEF - CAL_TILT_LINKAGE_FULL_RACK \n");
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_LINKAGE_FULL_RACK;
                        break;
                    }
                }
            }

            return (done ? CAL_MGR_MR_SAMPLE_PT_COLLECTED : CAL_MGR_MR_SAMPLE_WAIT);
        }

        case (CAL_MGR_MC_RESET_INIT): {
            /*
            **  This "Flow" command is sent to provide the calibration an
            **  opportunity to intialize any RESET parameters used within the
            **  RESET process after execution of the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      OK - Initialization of RESET parameters is complete and a
            **          RESET process exists and should be executed
            **      NONE - No RESET process exists for the current step number.
            */
            return CAL_MGR_MR_RESET_INIT_NONE;
        }

        case (CAL_MGR_MC_RESET): {
            /*
            **  This "Flow" command is sent during the execution of the RESET process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      DONE - RESET process is complete
            **      NOT_DONE - RESET process requires more execution loops to complete
            */
            return CAL_MGR_MR_RESET_DONE;
        }

        case (CAL_MGR_MC_VALIDATE): {
            /*
            **  This "Flow" command is sent to provide the calibration with the
            **  opportunity to validate the sample point/s associated with the
            **  current step number (LpsSaWeighInfoTbl.stepNo).  NOTE: This
            **  "Flow" command will not be sent if the SAMPLE command response is
            **  SKIP.
            **
            **  The possible responses are as follows:
            **      OK      - Sample Point/s is/are valid
            **      NOT_OK  - Sample Point/s is/are not valid.  cal_add_error()
            **          the reason for NOT_OK response.  The calibration will
            **          TERMINATE on next execution loop.
            **      RESTART - Sample Point/s is/are not valid.  cal_add_warning()
            **          the reason for RESTART response.  The calibration
            **          will RESTART_FLOW on next execution loop.
            */
            if (cal_linkage_workspace.tilt_full_dump_dc > cal_linkage_workspace.tilt_full_rack_dc) {
                *CalError = 0x0103;
                AIS_LOG_INFO("TILT CAL_MGR_MR_VALIDATE_NOT_OK");
                return( CAL_MGR_MR_VALIDATE_NOT_OK );
            }
            else {
                AIS_LOG_INFO("TILT CAL_MGR_MR_VALIDATE_OK");
                return CAL_MGR_MR_VALIDATE_OK;
            }
        }

        case (CAL_MGR_MC_NEXT_PT): {
            /*
            **  This "Flow" command is sent to provide the calibration with the
            **  opportunity to change the current step number
            **  (LpsSaWeighInfoTbl.stepNo) to the next step number.
            **
            **  The possible responses are as follows:
            **      MORE        - More Sample Point/s to collect
            **      NO_MORE     - No more Sample Point/s to collect
            */
            return CAL_MGR_MR_NEXT_PT_NO_MORE;
        }

        case (CAL_MGR_MC_SUCCESS): {
            /*
            **  This "Flow" command is sent following a NO_MORE response to the
            **  NEXT_PT command.  This "Flow" Command should be used to save the
            **  sampled points collected.  The status of the calibration points
            **  should be noted as CALIBRATED as appropriate just prior to a
            **  SAVED response.
            **
            **  The possible responses are as follows:
            **      SAVED   - Sampled Points (Data) were saved successfully
            **      WAIT    - Sampled Points (Data) are being saved or the
            **          SAVING process is waiting for User input (i.e. revert to
            **          previous, revert to default, save current, etc.)
            */

            // Set the calibration values (at the same time, save to nvm)
            AIS_LOG_INFO("CAL_MGR_MC_SUCCESS TILT: %f, %f", cal_linkage_workspace.tilt_full_dump_dc,
                    cal_linkage_workspace.tilt_full_rack_dc);

            // Set the new values
            setTiltLinkageSensorCalibrated(cal_linkage_workspace.tilt_full_dump_dc,
                    cal_linkage_workspace.tilt_full_rack_dc);

            // On successful tilt linkage cal, resetPayloads Cals
            setPayloadNotCalibrated();

            /* Set linkage cal is in progress to false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_SUCCESS_SAVED;
        }

        case (CAL_MGR_MC_DONE): {
            /*
            **  This "Flow" command is sent following a SAVED response to the
            **  SUCCESS command.  The EXIT_FLOW Command will follow on the next
            **  execution loop.
            **
            **  The possible responses are as follows:
            **      OK      - no more sets of Sampled Points (Data) to be
            **          collected.
            **      RESTART - (Advanced Option) another set of Sampled Points
            **          (Data) are to be collected.
            */
            AIS_LOG_INFO("CAL_MGR_MC_DONE");
            return CAL_MGR_MR_DONE_OK;
        }

        case( CAL_MGR_MC_FAIL ): {
            /*
            **  This "Flow" command is sent following a NOT_OK response to the
            **  VALIDATE command.  The TERMINATE command will follow on the next
            **  execution loop.
            **
            **  The possible responses are as follows:
            **      OK      - Set of Sampled Points (Data) were noted as being
            **          NOT_CALIBRATED
            */
            AIS_LOG_INFO("CAL_MGR_MC_FAIL");
            return CAL_MGR_MR_FAIL_OK;
        }

        default: {
           // Do nothing for now
           break;
        }
    }

    /* This should never happen */
    return CAL_MGR_MR_NOT_SUPPORTED;
}


CAL_MGR_MR_E LpsSaWeighApp::LpsSaTiltInlineLinkageSensorCalibration(
        CAL_MGR_MC_E Cmd, unsigned_16 CalId, unsigned_8* StepNo, unsigned_16* CalError)
{
    static bool cancel = false;

    (void)cmc;
    *CalError = 0;

    switch (Cmd) {
        /********************/
        /* Control Commands */
        /********************/
        case (CAL_MGR_MC_INIT_LAST_STAT): {
            /*
            **  This command is sent when the application has requested the
            **  CAL_MGR to enable the calibration from the application with
            **  cal_mgr_enable_cal( CAL_NEUTSETPT_ID ) call.  This command
            **  is not sent during the actual execution of the calibration
            **  through a service tool.
            **  The possible responses are as follows:
            **      OK - the last status report is supported and
            **          the last status report cache has been initialized
            **          correctly.
            **      NOT_OK - the last status resport is required or
            **          the last status report cache has not been initialized
            **          correctly.
            **      NA - Last status report not used (not applicable) and
            **          allow the enabling of the calibration
            */
            return CAL_MGR_MR_INIT_LAST_STAT_OK;
        }

        case (CAL_MGR_MC_GET_INPUTS): {
            /*
            **  This "Control" command is intended to be used to acquire the calibration
            **  inputs required by the calibration from the application.  This
            **  is the first command sent on an execution loop.  Validation
            **  checks on the inputs are to be handled within the CAL_ADIAGCHK
            **  command.
            **
            **  The possible responses are as follows:
            **      OK - Inputs were processed/acquired successfully
            */
            return CAL_MGR_MR_GET_INPUTS_OK;
        }

        case (CAL_MGR_MC_SET_OUTPUTS): {
            /*
            **  This "Control" command is intended to be used to send the calibration
            **  outputs of the calibration to the application.  Validation
            **  checks on the outputs are to be handled within the CAL_ADIAGCHK
            **  command.  This is the last command sent on an execution loop.
            **
            **  The possible responses are as follows:
            **      OK - Outputs were processed successfully
            */
            return CAL_MGR_MR_SET_OUTPUTS_OK;
        }

        case (CAL_MGR_MC_INITIALIZE): {
            /*
            **  This "Control" command is sent when the user has selected the calibration
            **  from the service tool assuming cal_mgr_enable_cal( CAL_NEUTSETPT_ID )
            **  was successful.  This "Control" command is a high-level intialization of the
            **  calibration intended to be used to notify the application that the
            **  calibration has begun, presenting an opportunity to begin actively
            **  overriding certain components of the application or initializing
            **  qualified read parameters.  It follows GET_INPUTS command on an
            **  execution loop until response is OK.
            **
            **  The possible responses are as follows:
            **      OK - initialization was completed successfully, after returning
            **          OK, the CAL_MGR will wait for the user to start the calibration
            **          by pressing the (Next>>) button the first time.  This
            **          command will not get called again until the calibration
            **          has ended its execution.
            **      AGAIN - if initialization requires more than one loop
            **          to complete
            **      NOT_OK - intialization was not completed successfully
            */

            /* Fail calibrations if machine/model is not selected */
            if (TRUE == LpsSaWeighInfoTbl.MachineModelNotSet) {
                *CalError = CAL_MACHINE_MODEL_NOT_SET;
                return CAL_MGR_MR_INITIALIZE_NOT_OK;
            }

            /* Fail calibrations if Legal for Trade is Sealed */
            if (weighUpdtTbl.LegalForTradeSealed) {
                *CalError = CAL_PAYLOAD_LEGAL_FOR_TRADE_SEALED;
                return CAL_MGR_MR_INITIALIZE_NOT_OK;
            }

            if (TILT_SENSOR_TYPE_INLINE != linkage_table_cnfg.tiltSensorType) {
                *CalError = 0x000B;
                return CAL_MGR_MR_INITIALIZE_NOT_OK;
            }

            cancel = false;

            /* Set linkage cal is in progress */
            LinkageCalInProgress = true;

            return CAL_MGR_MR_INITIALIZE_OK;
        }

        case (CAL_MGR_MC_ADIAGCHK): {
            /*
            **  This "Control" command is intended to be used to check active diagnostics
            **  for calibration inputs, outputs and any other high-level
            **  requirements of the calibration.  This "Control" command is sent just prior
            **  to SETUPCHK and FLOW commands only.
            **
            **  The possible responses are as follows:
            **      OK - No "Active Diagnostics" are present that will effect the
            **          execution of the calibration.
            **      NOT_OK - an "Active Diagnostic" is present that will effect the
            **          execution of the calibration.  The generic
            **          CAL_MGR_EW_ACTIVE_DIAGNOSTIC will be automatically issued by
            **          CAL_MGR if NOT_OK is the response.  Further
            **          cal_add_error() may be required here for the calibration.
            **          QUIT or TERMINATE command will be sent on the next
            **          execution loop.  SETUPCHK and FLOW commands will not
            **          be called on this execution loop due to this response.
            */
            if ((LPS_STATUS_BAD == LpsSaWeighInfoTbl.LiftPosition.status) ||
                    (LPS_STATUS_BAD == LpsSaWeighInfoTbl.TiltPosition.status)) {
                *CalError = CAL_LINKAGE_ABORTED_BY_ECM;
                return CAL_MGR_MR_ADIAGCHK_NOT_OK;
            }
            else {
                return CAL_MGR_MR_ADIAGCHK_OK;
            }
        }

        case (CAL_MGR_MC_SETUPCHK): {
            /*
            **  This "Control" command is intended to be used to check the setup conditions
            **  for the calibration prior to moving to the FLOW portion of the
            **  calibration.  This "Control" command follows the ADIAGCHK command when response
            **  from ADIAGCHK is OK.
            **
            **  The possible responses are as follows:
            **      OK - All setup conditions have been met and CAL_MGR_STEPNUMBER(cmw)
            **          has been set to the first FLOW step number.
            **      NOT_OK - All setup conditions have not been met and
            **          LpsSaWeighInfoTbl.stepNo has been set to the step number
            **          associated with the setup condition not being met.
            */
            LpsSaWeighInfoTbl.stepNo = CAL_TILT_LINKAGE_SETUP_COMPLETE;
            return CAL_MGR_MR_SETUPCHK_OK;
        }

        case (CAL_MGR_MC_INIT_FLOW): {
            /*
            **  This "Control" command is intended to be used to intialize the FLOW portion
            **  of a calibration.  This "Control" command is a lower-level intialization of the
            **  flow portion of the calibration which can be used to notify the
            **  application that the calibration flow portion has begun,
            **  presenting an opportunity to begin actively overriding additional
            **  components of the application.  It may also be used to set
            **  LpsSaWeighInfoTbl.stepNo, in the event that calibration has been
            **  "restarted internally", (i.e. not "restarted by service tool").
            **
            **  The possible responses are as follows:
            **      OK - Calibration Flow parameters have been initialized.
            */
            AIS_LOG_INFO("CAL_MGR_MC_INIT_FLOW");
            LpsSaWeighInfoTbl.stepNo = CAL_TILT_LINKAGE_FULL_RACK;
            return CAL_MGR_MR_INIT_FLOW_OK;
        }

        case (CAL_MGR_MC_FLOW): {
            /*
            **  This "Control" command is intended to be used to check conditions required
            **  by a FLOW step.
            **
            **  The possible responses are as follows:
            **      OK      - Flow conditions are valid, FLOW commands will follow.
            **      RESTART - A Flow condition is not valid and a "restart by calibration"
            **          is required.  The FLOW step may be recorded internally
            **          for "return to FLOW" capability.
            **      ABORT   - A Flow condition is not valid and will require
            **          the calibration to TERMINATE.  cal_add_error()'s may
            **          be called here to explicitly provide which Flow condition
            **          was not met.
            **
            */
            return CAL_MGR_MR_FLOW_OK;
        }

        case (CAL_MGR_MC_RESTART_FLOW): {
            /*
            **  This "Control" command is intended to be used to internally restart a
            **  calibration for some reason.  ADIAGCHK command is not sent during
            **  a restart.  LpsSaWeighInfoTbl.stepNo has been set to 1 on restart.
            **
            **  The possible responses are as follows:
            **      OK      - The internal restarting process has completed, optional
            **          setting of LpsSaWeighInfoTbl.stepNo can be applied to start
            **          at a specific step number
            **      AGAIN   - The internal restarting process requires more than one
            **          execution loop.
            */
            return CAL_MGR_MR_RESTART_FLOW_OK;
        }

        case (CAL_MGR_MC_STOP): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) before
            **  the calibration has "started" (meaning the user has yet to press
            **  the Next>> button to start the calibration)
            **  The application may choose to set the status of the calibration to
            **  NOT_CALIBRATED since the calibration was stopped.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible prior to "started"
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated INITIALIZE command. The EXIT_FLOW
            **          command will follow with this response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_STOP_EXIT_REQ;
        }

        case (CAL_MGR_MC_QUIT): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) before
            **  the calibration setup conditions have been met (prior to executing
            **  the flow portion of the calibration, TERMINATE sent on restarts);
            **  The application may choose to set the status of the calibration to
            **  NOT_CALIBRATED since the calibration was quit.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible prior to the
            **          INIT_FLOW command.
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated INITIALIZE command.
            **          The EXIT_FLOW command will follow with this response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_QUIT_EXIT_REQ;
        }

        case (CAL_MGR_MC_TERMINATE): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) after the
            **  INIT_FLOW command had been sent at least once.  The application may
            **  choose to set the status of the calibration to NOT_CALIBRATED since
            **  the calibration was terminated.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated by INITIALIZE, INIT_FLOW, and/or FLOW
            **          commands. The EXIT_FLOW command will follow with this
            **          response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_TERMINATE_EXIT_REQ;
        }

        case (CAL_MGR_MC_EXIT_FLOW): {
            /*
            **  This "Control" command is intended to be used to check if any calibration
            **  overrides are active that require a cleanup of more than one
            **  execution loop.  This command will be called at most once per
            **  calibration execution.
            **
            **  The possible responses are as follows:
            **      DONE        - No cleanup is required (no active overrides exist)
            **      CLEANUP_REQ - Cleanup is required to clear overrides.
            **          CLEANUP command will follow with this response.
            */
            return CAL_MGR_MR_EXIT_FLOW_CLEANUP_REQ;
        }

        case (CAL_MGR_MC_CLEANUP): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user).  The
            **  CLEANUP_REQ response to the EXIT_FLOW command indicates that an
            **  active overrides exist that will possibly take more than one
            **  execution loop to clear.  The cleanup procedure is executed within
            **  this command.
            **
            **  The possible responses are as follows:
            **      DONE        - No active overrides exist
            **      NOT_DONE    - More execution loops are required to clear
            **          active overrides.
            */
            return CAL_MGR_MR_CLEANUP_DONE;
        }

        /*****************/
        /* Flow Commands */
        /*****************/
        case (CAL_MGR_MC_READY): {
            /*
            **  This "Flow" command is sent to evaluate whether or not the
            **  calibration is "READY" to collect a sample.  The sample/s that
            **  is/are to be collected is/are most likely associated with a
            **  flow step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      YES - The calibration is ready to begin the sampling
            **          process associated with the current flow step.
            **      NO  - The calibration is not ready to begine the sampling
            **          process associated with the current flow step.
            */
            return CAL_MGR_MR_READY_YES;
        }

        case (CAL_MGR_MC_SAMPLE_INIT): {
            /*
            **  This "Flow" command is sent to intialize the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      OK  - Initialization for the sampling process associated with
            **          the current flow step (LpsSaWeighInfoTbl.stepNo) is complete.
            */
            AIS_LOG_INFO("CAL_MGR_MC_SAMPLE_INIT");
            LpsSaWeighInfoTbl.stepNo = CAL_TILT_INLINE_LINKAGE_LIFT_INIT;
            return CAL_MGR_MR_SAMPLE_INIT_OK;
        }

        case (CAL_MGR_MC_SAMPLE): {
            /*
            **  This "Flow" command is sent during the execution of the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      PT_COLLECTED - Sample point/s was/were collected that require
            **          validation after RESET
            **      WAIT - Sample Process is executing and requires another
            **          execution loop.
            **      SKIP - Sampling process has completed, no validation is required
            **          following RESET.
            */
            bool done = false;

            AIS_LOG_INFO("CAL_MGR_MC_SAMPLE");

            if (CAL_MGR_KEY_CONT) {
                AIS_LOG_INFO("CAL_MGR_KEY_CONT : STEPNO %d", LpsSaWeighInfoTbl.stepNo);

                switch (LpsSaWeighInfoTbl.stepNo) {
                    case (CAL_TILT_INLINE_LINKAGE_LIFT_INIT): {
                        AIS_LOG_INFO("CAL_TILT_INLINE_LINKAGE_LIFT_INIT");
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_INLINE_LINKAGE_FULL_DUMP;
                        break;
                    }
                    case (CAL_TILT_INLINE_LINKAGE_FULL_DUMP): {
                        AIS_LOG_INFO("CAL_TILT_INLINE_LINKAGE_FULL_DUMP");
                        cal_linkage_workspace.tilt_full_dump_dc = WeighPidTbl.TiltLinkageSensorDc;
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_INLINE_LINKAGE_FULL_RACK;
                        break;
                    }

                    case (CAL_TILT_INLINE_LINKAGE_FULL_RACK): {
                        AIS_LOG_INFO("CAL_TILT_INLINE_LINKAGE_FULL_RACK");
                        cal_linkage_workspace.tilt_full_rack_dc = WeighPidTbl.TiltLinkageSensorDc;

                        /*
                         * At this point, we have collected the limits of cylinder travel.
                         * We need to update the sensor process so that it starts calculating
                         * angle for the remainder of the calibration.
                         */
                        if (cal_linkage_workspace.tilt_full_dump_dc > cal_linkage_workspace.tilt_full_rack_dc) {
                            cancel = true;
                        }
                        else {
                            // Set the calibration values (at the same time, save to nvm)
                            AIS_LOG_INFO("CAL_TILT_INLINE_LINKAGE_FULL_RACK TILT: %f, %f",
                                    cal_linkage_workspace.tilt_full_dump_dc, cal_linkage_workspace.tilt_full_rack_dc);

                            // Set the new values
                            setTiltLinkageSensorCalibrated(cal_linkage_workspace.tilt_full_dump_dc,
                                    cal_linkage_workspace.tilt_full_rack_dc);

                            // On successful tilt linkage cal, resetPayloads Cals
                            setPayloadNotCalibrated();
                        }
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_INLINE_LINKAGE_FULL_RAISE;
                        break;
                    }

                    case (CAL_TILT_INLINE_LINKAGE_FULL_RAISE): {
                        AIS_LOG_INFO("CAL_TILT_INLINE_LINKAGE_FULL_RAISE");
                        cal_linkage_workspace.lift_full_raise_dc = WeighPidTbl.LiftLinkageSensorDc; // Not Used
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_INLINE_LINKAGE_DUMP_STOP;
                        break;
                    }

                    case (CAL_TILT_INLINE_LINKAGE_DUMP_STOP): {
                        AIS_LOG_INFO("CAL_TILT_INLINE_LINKAGE_DUMP_STOP");

                        cal_linkage_workspace.tilt_dump_stop_angle = LpsSaWeighInfoTbl.TiltPosition.angle;
                        AIS_LOG_DEBUG("tilt_dump_stop_angle:%f", cal_linkage_workspace.tilt_dump_stop_angle);
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_INLINE_LINKAGE_RACK_STOP;
                        break;
                    }

                    case (CAL_TILT_INLINE_LINKAGE_RACK_STOP): {
                        AIS_LOG_INFO("CAL_TILT_INLINE_LINKAGE_RACK_STOP");
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_INLINE_LINKAGE_FULL_LOWER;
                        break;
                    }

                    case (CAL_TILT_INLINE_LINKAGE_FULL_LOWER): {
                        AIS_LOG_INFO("CAL_TILT_INLINE_LINKAGE_FULL_LOWER");
                        cal_linkage_workspace.lift_full_lower_dc = WeighPidTbl.LiftLinkageSensorDc; // Not Used

                        // Rack stop is met when fully rack AND fully lowered.
                        cal_linkage_workspace.tilt_rack_stop_angle = LpsSaWeighInfoTbl.TiltPosition.angle;
                        AIS_LOG_DEBUG("tilt_rack_stop_angle:%f", cal_linkage_workspace.tilt_rack_stop_angle);
                        done = true;
                        break;
                    }

                    default: {
                        AIS_LOG_WARN("DEF - CAL_TILT_INLINE_LINKAGE_FULL_DUMP");
                        LpsSaWeighInfoTbl.stepNo = CAL_TILT_INLINE_LINKAGE_FULL_DUMP;
                        break;
                    }
                }
            }

            return (done || cancel ? CAL_MGR_MR_SAMPLE_PT_COLLECTED : CAL_MGR_MR_SAMPLE_WAIT);
        }

        case (CAL_MGR_MC_RESET_INIT): {
            /*
            **  This "Flow" command is sent to provide the calibration an
            **  opportunity to intialize any RESET parameters used within the
            **  RESET process after execution of the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      OK - Initialization of RESET parameters is complete and a
            **          RESET process exists and should be executed
            **      NONE - No RESET process exists for the current step number.
            */
            return CAL_MGR_MR_RESET_INIT_NONE;
        }

        case (CAL_MGR_MC_RESET): {
            /*
            **  This "Flow" command is sent during the execution of the RESET process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      DONE - RESET process is complete
            **      NOT_DONE - RESET process requires more execution loops to complete
            */
            return CAL_MGR_MR_RESET_DONE;
        }

        case (CAL_MGR_MC_VALIDATE): {
            /*
            **  This "Flow" command is sent to provide the calibration with the
            **  opportunity to validate the sample point/s associated with the
            **  current step number (LpsSaWeighInfoTbl.stepNo).  NOTE: This
            **  "Flow" command will not be sent if the SAMPLE command response is
            **  SKIP.
            **
            **  The possible responses are as follows:
            **      OK      - Sample Point/s is/are valid
            **      NOT_OK  - Sample Point/s is/are not valid.  cal_add_error()
            **          the reason for NOT_OK response.  The calibration will
            **          TERMINATE on next execution loop.
            **      RESTART - Sample Point/s is/are not valid.  cal_add_warning()
            **          the reason for RESTART response.  The calibration
            **          will RESTART_FLOW on next execution loop.
            */
            if (cancel ||
                    (cal_linkage_workspace.tilt_full_dump_dc > cal_linkage_workspace.tilt_full_rack_dc) ||
                    (cal_linkage_workspace.tilt_dump_stop_angle > cal_linkage_workspace.tilt_rack_stop_angle)) {
                *CalError =0x0103;
                AIS_LOG_INFO("TILT CAL_MGR_MR_VALIDATE_NOT_OK");
                return CAL_MGR_MR_VALIDATE_NOT_OK;
            }
            else {
                AIS_LOG_INFO("TILT CAL_MGR_MR_VALIDATE_OK");
                return CAL_MGR_MR_VALIDATE_OK;
            }
        }

        case (CAL_MGR_MC_NEXT_PT): {
            /*
            **  This "Flow" command is sent to provide the calibration with the
            **  opportunity to change the current step number
            **  (LpsSaWeighInfoTbl.stepNo) to the next step number.
            **
            **  The possible responses are as follows:
            **      MORE        - More Sample Point/s to collect
            **      NO_MORE     - No more Sample Point/s to collect
            */
            return CAL_MGR_MR_NEXT_PT_NO_MORE;
        }

        case (CAL_MGR_MC_SUCCESS): {
            /*
            **  This "Flow" command is sent following a NO_MORE response to the
            **  NEXT_PT command.  This "Flow" Command should be used to save the
            **  sampled points collected.  The status of the calibration points
            **  should be noted as CALIBRATED as appropriate just prior to a
            **  SAVED response.
            **
            **  The possible responses are as follows:
            **      SAVED   - Sampled Points (Data) were saved successfully
            **      WAIT    - Sampled Points (Data) are being saved or the
            **          SAVING process is waiting for User input (i.e. revert to
            **          previous, revert to default, save current, etc.)
            */
            // Set the calibration values (at the same time, save to nvm)
            AIS_LOG_INFO("CAL_MGR_MC_SUCCESS TILT: %f, %f", cal_linkage_workspace.tilt_dump_stop_angle,
                    cal_linkage_workspace.tilt_rack_stop_angle);

            // Set the new values
            setTiltLinkageSensorCalAngleLimits(cal_linkage_workspace.tilt_dump_stop_angle,
                    cal_linkage_workspace.tilt_rack_stop_angle);

            /* Set linkage cal is in progress to false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_SUCCESS_SAVED;
        }

        case (CAL_MGR_MC_DONE): {
            /*
            **  This "Flow" command is sent following a SAVED response to the
            **  SUCCESS command.  The EXIT_FLOW Command will follow on the next
            **  execution loop.
            **
            **  The possible responses are as follows:
            **      OK      - no more sets of Sampled Points (Data) to be
            **          collected.
            **      RESTART - (Advanced Option) another set of Sampled Points
            **          (Data) are to be collected.
            */
            AIS_LOG_INFO("CAL_MGR_MC_DONE");
            return CAL_MGR_MR_DONE_OK;
        }

        case (CAL_MGR_MC_FAIL): {
            /*
            **  This "Flow" command is sent following a NOT_OK response to the
            **  VALIDATE command.  The TERMINATE command will follow on the next
            **  execution loop.
            **
            **  The possible responses are as follows:
            **      OK      - Set of Sampled Points (Data) were noted as being
            **          NOT_CALIBRATED
            */
            AIS_LOG_INFO("CAL_MGR_MC_FAIL");
            return CAL_MGR_MR_FAIL_OK;
        }

        default: {
           // Do nothing for now
           break;
        }
    }

    /* This should never happen */
    return CAL_MGR_MR_NOT_SUPPORTED;
}


CAL_MGR_MR_E LpsSaWeighApp:: LpsSaLiftLinkageSensorCalibration(
        CAL_MGR_MC_E Cmd, unsigned_16 CalId, unsigned_8* StepNo, unsigned_16* CalError)
{
    (void)cmc;
    *CalError = 0;

    switch (Cmd) {
        /********************/
        /* Control Commands */
        /********************/
        case (CAL_MGR_MC_INIT_LAST_STAT): {
            /*
            **  This command is sent when the application has requested the
            **  CAL_MGR to enable the calibration from the application with
            **  cal_mgr_enable_cal( CAL_NEUTSETPT_ID ) call.  This command
            **  is not sent during the actual execution of the calibration
            **  through a service tool.
            **  The possible responses are as follows:
            **      OK - the last status report is supported and
            **          the last status report cache has been initialized
            **          correctly.
            **      NOT_OK - the last status resport is required or
            **          the last status report cache has not been initialized
            **          correctly.
            **      NA - Last status report not used (not applicable) and
            **          allow the enabling of the calibration
            */
            return CAL_MGR_MR_INIT_LAST_STAT_OK;
        }

        case (CAL_MGR_MC_GET_INPUTS): {
            /*
            **  This "Control" command is intended to be used to acquire the calibration
            **  inputs required by the calibration from the application.  This
            **  is the first command sent on an execution loop.  Validation
            **  checks on the inputs are to be handled within the CAL_ADIAGCHK
            **  command.
            **
            **  The possible responses are as follows:
            **      OK - Inputs were processed/acquired successfully
            */
            return CAL_MGR_MR_GET_INPUTS_OK;
        }

        case (CAL_MGR_MC_SET_OUTPUTS): {
            /*
            **  This "Control" command is intended to be used to send the calibration
            **  outputs of the calibration to the application.  Validation
            **  checks on the outputs are to be handled within the CAL_ADIAGCHK
            **  command.  This is the last command sent on an execution loop.
            **
            **  The possible responses are as follows:
            **      OK - Outputs were processed successfully
            */
            return CAL_MGR_MR_SET_OUTPUTS_OK;
        }

        case (CAL_MGR_MC_INITIALIZE): {
            /*
            **  This "Control" command is sent when the user has selected the calibration
            **  from the service tool assuming cal_mgr_enable_cal( CAL_NEUTSETPT_ID )
            **  was successful.  This "Control" command is a high-level intialization of the
            **  calibration intended to be used to notify the application that the
            **  calibration has begun, presenting an opportunity to begin actively
            **  overriding certain components of the application or initializing
            **  qualified read parameters.  It follows GET_INPUTS command on an
            **  execution loop until response is OK.
            **
            **  The possible responses are as follows:
            **      OK - initialization was completed successfully, after returning
            **          OK, the CAL_MGR will wait for the user to start the calibration
            **          by pressing the (Next>>) button the first time.  This
            **          command will not get called again until the calibration
            **          has ended its execution.
            **      AGAIN - if initialization requires more than one loop
            **          to complete
            **      NOT_OK - intialization was not completed successfully
            */

            /* Fail calibrations if machine/model is not selected */
            if (TRUE == LpsSaWeighInfoTbl.MachineModelNotSet) {
                *CalError = CAL_MACHINE_MODEL_NOT_SET;
                return CAL_MGR_MR_INITIALIZE_NOT_OK;
            }

            /* Fail calibrations if Legal for Trade is Sealed */
            if (weighUpdtTbl.LegalForTradeSealed) {
                *CalError = CAL_PAYLOAD_LEGAL_FOR_TRADE_SEALED;
                return CAL_MGR_MR_INITIALIZE_NOT_OK;
            }

            /* Set linkage cal is in progress */
            LinkageCalInProgress = true;

            return CAL_MGR_MR_INITIALIZE_OK;
        }

        case (CAL_MGR_MC_ADIAGCHK): {
            /*
            **  This "Control" command is intended to be used to check active diagnostics
            **  for calibration inputs, outputs and any other high-level
            **  requirements of the calibration.  This "Control" command is sent just prior
            **  to SETUPCHK and FLOW commands only.
            **
            **  The possible responses are as follows:
            **      OK - No "Active Diagnostics" are present that will effect the
            **          execution of the calibration.
            **      NOT_OK - an "Active Diagnostic" is present that will effect the
            **          execution of the calibration.  The generic
            **          CAL_MGR_EW_ACTIVE_DIAGNOSTIC will be automatically issued by
            **          CAL_MGR if NOT_OK is the response.  Further
            **          cal_add_error() may be required here for the calibration.
            **          QUIT or TERMINATE command will be sent on the next
            **          execution loop.  SETUPCHK and FLOW commands will not
            **          be called on this execution loop due to this response.
            */
            if ((LPS_STATUS_BAD == LpsSaWeighInfoTbl.LiftPosition.status) ||
                    (LPS_STATUS_BAD == LpsSaWeighInfoTbl.TiltPosition.status)) {
                *CalError = CAL_LINKAGE_ABORTED_BY_ECM;
                return CAL_MGR_MR_ADIAGCHK_NOT_OK;
            }
            else {
                return CAL_MGR_MR_ADIAGCHK_OK;
            }
        }

        case (CAL_MGR_MC_SETUPCHK): {
            /*
            **  This "Control" command is intended to be used to check the setup conditions
            **  for the calibration prior to moving to the FLOW portion of the
            **  calibration.  This "Control" command follows the ADIAGCHK command when response
            **  from ADIAGCHK is OK.
            **
            **  The possible responses are as follows:
            **      OK - All setup conditions have been met and CAL_MGR_STEPNUMBER(cmw)
            **          has been set to the first FLOW step number.
            **      NOT_OK - All setup conditions have not been met and
            **          LpsSaWeighInfoTbl.stepNo has been set to the step number
            **          associated with the setup condition not being met.
            */
            LpsSaWeighInfoTbl.stepNo = CAL_LIFT_LINKAGE_SETUP_COMPLETE;
            return CAL_MGR_MR_SETUPCHK_OK;
        }

        case (CAL_MGR_MC_INIT_FLOW): {
            /*
            **  This "Control" command is intended to be used to intialize the FLOW portion
            **  of a calibration.  This "Control" command is a lower-level intialization of the
            **  flow portion of the calibration which can be used to notify the
            **  application that the calibration flow portion has begun,
            **  presenting an opportunity to begin actively overriding additional
            **  components of the application.  It may also be used to set
            **  LpsSaWeighInfoTbl.stepNo, in the event that calibration has been
            **  "restarted internally", (i.e. not "restarted by service tool").
            **
            **  The possible responses are as follows:
            **      OK - Calibration Flow parameters have been initialized.
            */
            AIS_LOG_INFO("CAL_MGR_MC_INIT_FLOW");
            LpsSaWeighInfoTbl.stepNo = CAL_LIFT_LINKAGE_FULL_RAISE;
            return CAL_MGR_MR_INIT_FLOW_OK;
        }

        case (CAL_MGR_MC_FLOW): {
            /*
            **  This "Control" command is intended to be used to check conditions required
            **  by a FLOW step.
            **
            **  The possible responses are as follows:
            **      OK      - Flow conditions are valid, FLOW commands will follow.
            **      RESTART - A Flow condition is not valid and a "restart by calibration"
            **          is required.  The FLOW step may be recorded internally
            **          for "return to FLOW" capability.
            **      ABORT   - A Flow condition is not valid and will require
            **          the calibration to TERMINATE.  cal_add_error()'s may
            **          be called here to explicitly provide which Flow condition
            **          was not met.
            **
            */
            return CAL_MGR_MR_FLOW_OK;
        }

        case (CAL_MGR_MC_RESTART_FLOW): {
            /*
            **  This "Control" command is intended to be used to internally restart a
            **  calibration for some reason.  ADIAGCHK command is not sent during
            **  a restart.  LpsSaWeighInfoTbl.stepNo has been set to 1 on restart.
            **
            **  The possible responses are as follows:
            **      OK      - The internal restarting process has completed, optional
            **          setting of LpsSaWeighInfoTbl.stepNo can be applied to start
            **          at a specific step number
            **      AGAIN   - The internal restarting process requires more than one
            **          execution loop.
            */
            return CAL_MGR_MR_RESTART_FLOW_OK;
        }

        case (CAL_MGR_MC_STOP): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) before
            **  the calibration has "started" (meaning the user has yet to press
            **  the Next>> button to start the calibration)
            **  The application may choose to set the status of the calibration to
            **  NOT_CALIBRATED since the calibration was stopped.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible prior to "started"
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated INITIALIZE command. The EXIT_FLOW
            **          command will follow with this response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_STOP_EXIT_REQ;
        }

        case (CAL_MGR_MC_QUIT): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) before
            **  the calibration setup conditions have been met (prior to executing
            **  the flow portion of the calibration, TERMINATE sent on restarts);
            **  The application may choose to set the status of the calibration to
            **  NOT_CALIBRATED since the calibration was quit.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible prior to the
            **          INIT_FLOW command.
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated INITIALIZE command.
            **          The EXIT_FLOW command will follow with this response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_QUIT_EXIT_REQ;
        }

        case (CAL_MGR_MC_TERMINATE): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user) after the
            **  INIT_FLOW command had been sent at least once.  The application may
            **  choose to set the status of the calibration to NOT_CALIBRATED since
            **  the calibration was terminated.
            **
            **  The possible responses are as follows:
            **      OK          - No active overrides are possible
            **      EXIT_REQ    - It is possible that active overrides exist,
            **          most likely generated by INITIALIZE, INIT_FLOW, and/or FLOW
            **          commands. The EXIT_FLOW command will follow with this
            **          response.
            */

            /* Set linkage cal is in progress is false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_TERMINATE_EXIT_REQ;
        }

        case (CAL_MGR_MC_EXIT_FLOW): {
            /*
            **  This "Control" command is intended to be used to check if any calibration
            **  overrides are active that require a cleanup of more than one
            **  execution loop.  This command will be called at most once per
            **  calibration execution.
            **
            **  The possible responses are as follows:
            **      DONE        - No cleanup is required (no active overrides exist)
            **      CLEANUP_REQ - Cleanup is required to clear overrides.
            **          CLEANUP command will follow with this response.
            */
            return CAL_MGR_MR_EXIT_FLOW_CLEANUP_REQ;
        }

        case (CAL_MGR_MC_CLEANUP): {
            /*
            **  This "Control" command is sent to notify the calibration that the calibration
            **  has been requested to abort (internally or by the user).  The
            **  CLEANUP_REQ response to the EXIT_FLOW command indicates that an
            **  active overrides exist that will possibly take more than one
            **  execution loop to clear.  The cleanup procedure is executed within
            **  this command.
            **
            **  The possible responses are as follows:
            **      DONE        - No active overrides exist
            **      NOT_DONE    - More execution loops are required to clear
            **          active overrides.
            */
            return CAL_MGR_MR_CLEANUP_DONE;
        }

        /*****************/
        /* Flow Commands */
        /*****************/
        case (CAL_MGR_MC_READY): {
            /*
            **  This "Flow" command is sent to evaluate whether or not the
            **  calibration is "READY" to collect a sample.  The sample/s that
            **  is/are to be collected is/are most likely associated with a
            **  flow step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      YES - The calibration is ready to begin the sampling
            **          process associated with the current flow step.
            **      NO  - The calibration is not ready to begine the sampling
            **          process associated with the current flow step.
            */
            return CAL_MGR_MR_READY_YES;
        }

        case (CAL_MGR_MC_SAMPLE_INIT): {
            /*
            **  This "Flow" command is sent to intialize the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      OK  - Initialization for the sampling process associated with
            **          the current flow step (LpsSaWeighInfoTbl.stepNo) is complete.
            */
            AIS_LOG_INFO("CAL_MGR_MC_SAMPLE_INIT");
            LpsSaWeighInfoTbl.stepNo = CAL_LIFT_LINKAGE_FULL_RAISE;
            return CAL_MGR_MR_SAMPLE_INIT_OK;
        }

        case (CAL_MGR_MC_SAMPLE): {
            /*
            **  This "Flow" command is sent during the execution of the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      PT_COLLECTED - Sample point/s was/were collected that require
            **          validation after RESET
            **      WAIT - Sample Process is executing and requires another
            **          execution loop.
            **      SKIP - Sampling process has completed, no validation is required
            **          following RESET.
            */
            bool done = false;

            AIS_LOG_INFO("CAL_MGR_MC_SAMPLE");

            if (CAL_MGR_KEY_CONT) {
                AIS_LOG_INFO("CAL_MGR_KEY_CONT : STEPNO %d", LpsSaWeighInfoTbl.stepNo);

                switch (LpsSaWeighInfoTbl.stepNo) {
                    case (CAL_LIFT_LINKAGE_FULL_RAISE): {
                        AIS_LOG_INFO("CAL_LIFT_LINKAGE_FULL_RAISE");
                        cal_linkage_workspace.lift_full_raise_dc = WeighPidTbl.LiftLinkageSensorDc;
                        LpsSaWeighInfoTbl.stepNo = CAL_LIFT_LINKAGE_FULL_RACK;
                        break;
                    }

                    case (CAL_LIFT_LINKAGE_FULL_RACK): {
                        AIS_LOG_INFO("CAL_LIFT_LINKAGE_FULL_RACK");
                        cal_linkage_workspace.tilt_full_rack_dc = WeighPidTbl.TiltLinkageSensorDc; // Not Used
                        LpsSaWeighInfoTbl.stepNo = CAL_LIFT_LINKAGE_FULL_LOWER;
                        break;
                    }

                    case (CAL_LIFT_LINKAGE_FULL_LOWER): {
                        AIS_LOG_INFO("CAL_LIFT_LINKAGE_FULL_LOWER");
                        cal_linkage_workspace.lift_full_lower_dc = WeighPidTbl.LiftLinkageSensorDc;
                        done = true;
                        break;
                    }

                    default: {
                        AIS_LOG_WARN("DEF - CAL_LIFT_LINKAGE_FULL_RAISE");
                        LpsSaWeighInfoTbl.stepNo = CAL_LIFT_LINKAGE_FULL_RAISE;
                        break;
                    }
                }
            }

            return (done ? CAL_MGR_MR_SAMPLE_PT_COLLECTED : CAL_MGR_MR_SAMPLE_WAIT);
        }

        case (CAL_MGR_MC_RESET_INIT): {
            /*
            **  This "Flow" command is sent to provide the calibration an
            **  opportunity to intialize any RESET parameters used within the
            **  RESET process after execution of the sampling process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      OK - Initialization of RESET parameters is complete and a
            **          RESET process exists and should be executed
            **      NONE - No RESET process exists for the current step number.
            */
            return CAL_MGR_MR_RESET_INIT_NONE;
        }

        case (CAL_MGR_MC_RESET): {
            /*
            **  This "Flow" command is sent during the execution of the RESET process
            **  associated with the current step number (LpsSaWeighInfoTbl.stepNo).
            **
            **  The possible responses are as follows:
            **      DONE - RESET process is complete
            **      NOT_DONE - RESET process requires more execution loops to complete
            */
            return CAL_MGR_MR_RESET_DONE;
        }

        case (CAL_MGR_MC_VALIDATE): {
            /*
            **  This "Flow" command is sent to provide the calibration with the
            **  opportunity to validate the sample point/s associated with the
            **  current step number (LpsSaWeighInfoTbl.stepNo).  NOTE: This
            **  "Flow" command will not be sent if the SAMPLE command response is
            **  SKIP.
            **
            **  The possible responses are as follows:
            **      OK      - Sample Point/s is/are valid
            **      NOT_OK  - Sample Point/s is/are not valid.  cal_add_error()
            **          the reason for NOT_OK response.  The calibration will
            **          TERMINATE on next execution loop.
            **      RESTART - Sample Point/s is/are not valid.  cal_add_warning()
            **          the reason for RESTART response.  The calibration
            **          will RESTART_FLOW on next execution loop.
            */
            if (cal_linkage_workspace.lift_full_lower_dc > cal_linkage_workspace.lift_full_raise_dc) {
                *CalError = 0x0103;
                AIS_LOG_INFO("LIFT CAL_MGR_MR_VALIDATE_NOT_OK");
                return CAL_MGR_MR_VALIDATE_NOT_OK;
            }
            else {
                AIS_LOG_INFO("LIFT CAL_MGR_MR_VALIDATE_OK");
                return CAL_MGR_MR_VALIDATE_OK;
            }
        }

        case (CAL_MGR_MC_NEXT_PT): {
            /*
            **  This "Flow" command is sent to provide the calibration with the
            **  opportunity to change the current step number
            **  (LpsSaWeighInfoTbl.stepNo) to the next step number.
            **
            **  The possible responses are as follows:
            **      MORE        - More Sample Point/s to collect
            **      NO_MORE     - No more Sample Point/s to collect
            */
            return CAL_MGR_MR_NEXT_PT_NO_MORE;
        }

        case (CAL_MGR_MC_SUCCESS): {
            /*
            **  This "Flow" command is sent following a NO_MORE response to the
            **  NEXT_PT command.  This "Flow" Command should be used to save the
            **  sampled points collected.  The status of the calibration points
            **  should be noted as CALIBRATED as appropriate just prior to a
            **  SAVED response.
            **
            **  The possible responses are as follows:
            **      SAVED   - Sampled Points (Data) were saved successfully
            **      WAIT    - Sampled Points (Data) are being saved or the
            **          SAVING process is waiting for User input (i.e. revert to
            **          previous, revert to default, save current, etc.)
            */

            // Set the calibration values (at the same time, save to nvm)
            AIS_LOG_INFO("CAL_MGR_MC_SUCCESS LIFT: %f, %f", cal_linkage_workspace.lift_full_lower_dc,
                    cal_linkage_workspace.lift_full_raise_dc);

            // Set the new values
            setLiftLinkageSensorCalibrated(cal_linkage_workspace.lift_full_lower_dc,
                    cal_linkage_workspace.lift_full_raise_dc);

            if (TILT_SENSOR_TYPE_INLINE == linkage_table_cnfg.tiltSensorType) {
                // Position sensing cylinders rely on lift calibration to perform tilt calibration
                setTiltLinkageSensorNotCalibrated();
            }

            // On successful lift linkage cal, resetPayloads Cals
            setPayloadNotCalibrated();

            /* Set linkage cal is in progress to false */
            LinkageCalInProgress = false;

            return CAL_MGR_MR_SUCCESS_SAVED;
        }

        case (CAL_MGR_MC_DONE): {
            /*
            **  This "Flow" command is sent following a SAVED response to the
            **  SUCCESS command.  The EXIT_FLOW Command will follow on the next
            **  execution loop.
            **
            **  The possible responses are as follows:
            **      OK      - no more sets of Sampled Points (Data) to be
            **          collected.
            **      RESTART - (Advanced Option) another set of Sampled Points
            **          (Data) are to be collected.
            */
            AIS_LOG_INFO("CAL_MGR_MC_DONE");
            return CAL_MGR_MR_DONE_OK;
        }

        case (CAL_MGR_MC_FAIL): {
            /*
            **  This "Flow" command is sent following a NOT_OK response to the
            **  VALIDATE command.  The TERMINATE command will follow on the next
            **  execution loop.
            **
            **  The possible responses are as follows:
            **      OK      - Set of Sampled Points (Data) were noted as being
            **          NOT_CALIBRATED
            */
            AIS_LOG_INFO("CAL_MGR_MC_FAIL");
            return CAL_MGR_MR_FAIL_OK;
        }

        default: {
           // Do nothing for now
           break;
        }
    }

    /* This should never happen */
    return CAL_MGR_MR_NOT_SUPPORTED;
}
