/******************************Advanced********************************/
#include <chrono>
#include <toa_wl00_model_if.h>
#include "../LpsSaWeighApp.h"
#include "LpsAdvCANProcess.h"
#include "TipoffAssist.h"

#define CAN_SAMPLING_RATE_USEC                      20000  /* 20ms */
#define CAN_TIMEOUT_USEC_TO_SET_XCP                 100000 /* 100ms */
#define CAN_TIMEOUT_USEC_TO_DISABLE_PAYLOAD         1000000 /* 1000ms(1sec) */

static LpsAdvCANProcess CANB = {};
static TipoffAssist LpsTipoffAssist = {};

LpsUpdtErrorTypes_t LpsSaWeighApp::ProcessInputs()
{
    AIS_LOG_DEBUG("Process Inputs Adv");

    LpsUpdtErrorTypes_t weighRet = LPS_UPDT_SUCCESS;
    float liftLinkageDc = 0.f;
    float tiltLinkageDc = 0.f;
    int number_of_packets = 0;
    static bool tipoff_initialized = false;
    static bool can11_initialized = false;
    static auto lastCANRxTime = std::chrono::steady_clock::now();
    static auto lastCANRxTimeAll = lastCANRxTime;

    /* Initialize tipoff if not already initialized */
    if (!tipoff_initialized) {
        LpsTipoffAssist.initialize(machineProperties);
        tipoff_initialized = true;
    }

    /* Initialize can11 if not already initialized */
    if (!can11_initialized) {
        CANB.init();
        can11_initialized = true;
    }

    /* Sample tilt pressure sensor pwm inputs */
    std::chrono::steady_clock::time_point thisPWMSampleTime;
    if (/*inPwm_.samplePWMInputs()*/0) {
        thisPWMSampleTime = inPwm_.getLastSampleTime();
    }
    else {
        thisPWMSampleTime = std::chrono::steady_clock::now();
    }

    /*
     * How long has it been since we processed the last CAN message?
     * This can be used to estimate how many CAN messages we missed.
     */
    int timeSinceLastRx_usec = std::chrono::duration_cast<std::chrono::microseconds>(thisPWMSampleTime - lastCANRxTime).count();

    //DebugLpsSaXCPChannels.DebugLiftCylPeriod = inPwm_.getLastSamplePeriod();
    DebugLpsSaXCPChannels.DebugLiftCylWidth = timeSinceLastRx_usec;

    // Process all available CAN packets
    bool keepGoing = true;
    while (keepGoing) {
        LpsSaLiftPosition_t liftPosition;
        LpsSaTiltPosition_t tiltPosition;

        bool lift_linkage_dc_ok = false;
        bool tilt_linkage_dc_ok = false;
        bool lift_he_pressure_ok = false;
        bool lift_re_pressure_ok = false;
        bool tilt_he_pressure_ok = false;
        bool tilt_re_pressure_ok = false;
        bool lift_valve_cmd_ok = false;
        bool tilt_valve_cmd_ok = false;

        if (demo_mode) {
            // Keep up with the CAN processing, faking it out.
            CANB.purgeRxBuffer();
            lastCANRxTime = std::chrono::steady_clock::now();
            lastCANRxTimeAll = lastCANRxTime;
            keepGoing = false;

            WeighPidTbl.TiltLinkageSensorDc = demoInputs_.tiltposition;   // set tilt DC
            WeighPidTbl.LiftLinkageSensorDc = demoInputs_.liftposition;   // set lift DC
            lift_linkage_dc_ok = true;
            tilt_linkage_dc_ok = true;

            weighUpdtTbl.LiftCylHePres.Val = demoInputs_.pres_lift_he_demo;
            weighUpdtTbl.LiftCylRePres.Val = demoInputs_.pres_lift_re_demo;
            lift_he_pressure_ok = true;
            lift_re_pressure_ok = true;

            LpsSaWeighInfoTbl.LiftValveCommand.Val = 0.f;
            LpsSaWeighInfoTbl.TiltValveCommand.Val = 0.f;
            lift_valve_cmd_ok = true;
            tilt_valve_cmd_ok = true;

            LpsSaWeighInfoTbl.TiltCylHePres.Val = 0.0f;
            LpsSaWeighInfoTbl.TiltCylHePres.Stat = LPS_STATUS_OK;
            tilt_he_pressure_ok = true;

            LpsSaWeighInfoTbl.TiltCylRePres.Val = 0.f;
            LpsSaWeighInfoTbl.TiltCylRePres.Stat = LPS_STATUS_OK;
            tilt_re_pressure_ok = true;

            /* get lift lever info */
            LpsSaWeighInfoTbl.LiftLeverInfo.LeverInfoAvailable = false;
            LpsSaWeighInfoTbl.LiftLeverInfo.InCenter = false;
            LpsSaWeighInfoTbl.LiftLeverInfo.InLowerDetent = false;
            LpsSaWeighInfoTbl.LiftLeverInfo.InRaiseDetent = false;
            LpsSaWeighInfoTbl.LiftLeverInfo.Faulted = true;
            LpsSaWeighInfoTbl.LiftLeverInfo.ValveCmdPercent = 0.f;

            LpsSaWeighInfoTbl.CalOvrAcknowledge = LpsCalAppInfCalOvActive();

            liftPosition = lpsSaGetLiftPosition(demoInputs_.angle_lift_demo, true);
            tiltPosition = lpsSaGetTiltRotaryPosition(demoInputs_.angle_lift_demo, true, demoInputs_.angle_ABC_demo, true);
        }
        else {
            // We are not in demo mode, prepare to grab the next CAN message
            auto now = std::chrono::steady_clock::now();

            //  Check to make sure we are keeping up.
            if ((now - thisPWMSampleTime) > std::chrono::microseconds(CAN_SAMPLING_RATE_USEC)) {
                /*
                 * We are not keeping up.  We need to purge the Rx buffer and start over again.
                 * The other option is to slip more and more and let our app get killed.
                 * I don't like that option.
                 */
                //int count = CANB.purgeRxBuffer();
                int count =1;
                AIS_LOG_ERROR("Falling behind, purged CAN Rx buffer of %d packets.", count);
                //break;
            }

            // Grab the next pair of CAN messages
            bool allRxd = false;
            if (CANB.update(allRxd)) {
                // We received something that we want to process, keep track of the time we last received something.
                lastCANRxTime = std::chrono::steady_clock::now();

                if (allRxd) {
                    lastCANRxTimeAll = lastCANRxTime;
                }
            }
            else {
                // We didn't receive anything new, we're all caught up.
                break;
            }

            // increment number of packets processed
            number_of_packets++;

            /*
             * If the time between sets of CAN messages is > 20ms, then we can assume we missed some.
             * Since we know a pair is sent every 20ms, we can try to go back to what the PWM inputs
             * were when the CAN message was sent.
             *
             * For example, if 25ms has gone by, then we can expect the next CAN message can not be any newer
             * than 5ms old (25 - 20).  If 45ms has gone by, then the first CAN messages we receive
             * cannot be any newer than 25ms old (45 - 20), and the second set of CAN messages we receive
             * cannot be any newer than 5ms old (45 - 40).
             *
             * Applying this pattern, we can move forward in time, selecting the newest possible PWM data
             * to match to each set of CAN messages:
             *  now - (time since - 20ms)
             *  now - (time since - 40ms)
             *  now - (time since - 60ms)
             *  ...
             *  now
             */
            if (timeSinceLastRx_usec > CAN_SAMPLING_RATE_USEC) {
                /* Read pressure sensor pwm inputs at t=x  (does not sample the driver) */
                timeSinceLastRx_usec -= CAN_SAMPLING_RATE_USEC;
                //inPwm_.readPWMInputs(timeSinceLastRx_usec);
            }
            else {
                //inPwm_.readPWMInputs(0); /* current sample, no delay */
            }

            DebugLpsSaXCPChannels.DebugTiltCylHePeriod = inPwm_.PwmData.Period[TILT_CYL_HE_PRES_SENS_CH_NUM];
            DebugLpsSaXCPChannels.DebugTiltCylHeWidth  = inPwm_.PwmData.Width[TILT_CYL_HE_PRES_SENS_CH_NUM];
            DebugLpsSaXCPChannels.DebugTiltCylRePeriod = inPwm_.PwmData.Period[TILT_CYL_RE_PRES_SENS_CH_NUM];
            DebugLpsSaXCPChannels.DebugTiltCylReWidth  = inPwm_.PwmData.Width[TILT_CYL_RE_PRES_SENS_CH_NUM];

            // temp no tilt pressures at this time on gen7
            /*LpsSaWeighInfoTbl.TiltCylHePres = TiltHeadEndPresTransferFunc(
                    inPwm_.PwmData.Width[TILT_CYL_HE_PRES_SENS_CH_NUM],
                    inPwm_.PwmData.Period[TILT_CYL_HE_PRES_SENS_CH_NUM],
                    inPwm_.PwmData.SwitchStatus[TILT_CYL_HE_PRES_SENS_CH_NUM],
                    inPwm_.PwmData.Timeout[TILT_CYL_HE_PRES_SENS_CH_NUM]);

            LpsSaWeighInfoTbl.TiltCylRePres = TiltRodEndPresTransferFunc(
                    inPwm_.PwmData.Width[TILT_CYL_RE_PRES_SENS_CH_NUM],
                    inPwm_.PwmData.Period[TILT_CYL_RE_PRES_SENS_CH_NUM],
                    inPwm_.PwmData.SwitchStatus[TILT_CYL_RE_PRES_SENS_CH_NUM],
                    inPwm_.PwmData.Timeout[TILT_CYL_RE_PRES_SENS_CH_NUM]);*/

            /* get lift lever info */
            LpsSaWeighInfoTbl.LiftLeverInfo.LeverInfoAvailable = true;   /* always true for adv */
            LpsAdvCANProcess::LiftLeverStatus_t lift_lever_status;
            CANB.get_lift_lever_status(lift_lever_status);
            LpsSaWeighInfoTbl.LiftLeverInfo.InCenter = lift_lever_status.lift_lever_centered;
            LpsSaWeighInfoTbl.LiftLeverInfo.InLowerDetent = lift_lever_status.lift_lever_lower_detent;
            LpsSaWeighInfoTbl.LiftLeverInfo.InRaiseDetent = lift_lever_status.lift_lever_raise_detent;
            LpsSaWeighInfoTbl.LiftLeverInfo.Faulted = lift_lever_status.lift_lever_faulted;
            CANB.get_lift_valve_cmd(LpsSaWeighInfoTbl.LiftLeverInfo.ValveCmdPercent);

            /* get lift pressures and dc from can 11-bit */
            lift_linkage_dc_ok = CANB.get_lift_linkage_dc(liftLinkageDc);
            tilt_linkage_dc_ok = CANB.get_tilt_linkage_dc(tiltLinkageDc);

            lift_he_pressure_ok = CANB.get_lift_he_pressure(weighUpdtTbl.LiftCylHePres.Val);
            lift_re_pressure_ok = CANB.get_lift_re_pressure(weighUpdtTbl.LiftCylRePres.Val);

            // Override according to test plan
            if (testFixture_.tpLiftHeadEndPressure(weighUpdtTbl.LiftCylHePres.Val)) {
                lift_he_pressure_ok = true;
            }

            if (testFixture_.tpLiftRodEndPressure(weighUpdtTbl.LiftCylRePres.Val)) {
                lift_re_pressure_ok = true;
            }

            tilt_he_pressure_ok = (LpsSaWeighInfoTbl.TiltCylHePres.Stat != LPS_STATUS_BAD);
            tilt_re_pressure_ok = (LpsSaWeighInfoTbl.TiltCylRePres.Stat != LPS_STATUS_BAD);
            lift_valve_cmd_ok = CANB.get_lift_valve_cmd(LpsSaWeighInfoTbl.LiftValveCommand.Val);
            tilt_valve_cmd_ok = CANB.get_tilt_valve_cmd(LpsSaWeighInfoTbl.TiltValveCommand.Val);

            /* Get Cal Override Acknowledge Status */
            LpsSaWeighInfoTbl.CalOvrAcknowledge = CANB.isCalOverrideActive();

            // Override according to test plan
            if (testFixture_.tpLiftPositionSensorDutyCycle(liftLinkageDc)) {
                lift_linkage_dc_ok = true;
            }

            /* Conversion of dc to lift angle */
            float liftAngle = lpsSaGetLiftAngle(liftLinkageDc, lift_linkage_dc_ok);

            /* Conversion of Lift angle input to Lift cylinder extension */
            liftPosition = lpsSaGetLiftPosition(liftAngle, lift_linkage_dc_ok);

            if (testFixture_.tpTiltPositionSensorDutyCycle(tiltLinkageDc)) {
                tilt_linkage_dc_ok = true;
            }

            float normalizedTiltPosition = lpsSaGetNormalizedTiltPosition(tiltLinkageDc, tilt_linkage_dc_ok);

            float tiltAngle = lpsSaGetTiltAngle(normalizedTiltPosition, tilt_linkage_dc_ok);

            /* Conversion of Tilt angle input to Tilt cylinder extension */
            tiltPosition = lpsSaGetTiltRotaryPosition(liftAngle, lift_linkage_dc_ok, tiltAngle, tilt_linkage_dc_ok);
        }

        /* Handle faults - this is done to show exclamation on bucket in UI. No popups are displayed on Adv */
        if (!lift_linkage_dc_ok) {
            liftPosition.status = LPS_STATUS_BAD;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.lift_angle_bad = 1;
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL] = true;
        }
        else {
            liftPosition.status = LPS_STATUS_OK;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.lift_angle_bad = 0;
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL] = false;
        }

        if (!tilt_linkage_dc_ok || !lift_linkage_dc_ok) {
            tiltPosition.status = LPS_STATUS_BAD;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_extension_bad = 1;
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL] = true;
        }
        else {
            tiltPosition.status = LPS_STATUS_OK;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_extension_bad = 0;
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL] = false;
        }

        if (!lift_he_pressure_ok) {
            weighUpdtTbl.LiftCylHePres.Stat = LPS_STATUS_BAD;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.lift_he_press_bad = 1;
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL] = true;
        }
        else {
            weighUpdtTbl.LiftCylHePres.Stat = LPS_STATUS_OK;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.lift_he_press_bad = 0;
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL] = false;
        }

        if (!lift_re_pressure_ok) {
            weighUpdtTbl.LiftCylRePres.Stat = LPS_STATUS_BAD;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.lift_re_press_bad = 1;
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL] = true;
        }
        else {
            weighUpdtTbl.LiftCylRePres.Stat = LPS_STATUS_OK;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.lift_re_press_bad = 0;
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL] = false;
        }

        if (!tilt_he_pressure_ok) {
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_he_press_bad = 1;
        }
        else {
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_he_press_bad = 0;
        }

        if (!tilt_re_pressure_ok) {
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_re_press_bad = 1;
        }
        else {
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_re_press_bad = 0;
        }

        if (!lift_valve_cmd_ok) {
            LpsSaWeighInfoTbl.LiftValveCommand.Stat = LPS_STATUS_BAD;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.lift_valve_cmd_bad = 1;
        }
        else {
            LpsSaWeighInfoTbl.LiftValveCommand.Stat = LPS_STATUS_OK;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.lift_valve_cmd_bad = 0;
        }

        if (!tilt_valve_cmd_ok) {
            LpsSaWeighInfoTbl.TiltValveCommand.Stat = LPS_STATUS_BAD;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_valve_cmd_bad = 1;
        }
        else {
            LpsSaWeighInfoTbl.TiltValveCommand.Stat = LPS_STATUS_OK;
            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_valve_cmd_bad = 0;
        }

        LpsSaWeighInfoTbl.LiftPosition = liftPosition;

        /* Assigning lift cylinder extension as percentage of full cylinder extension to weighing library*/
        weighUpdtTbl.LiftCylLengthNorm.Val  = liftPosition.percentCylinderLength;
        weighUpdtTbl.LiftCylLengthNorm.Stat = liftPosition.status;

        weighUpdtTbl.LiftCylLength.Val = liftPosition.cylinderLength;
        weighUpdtTbl.LiftCylLength.Stat = liftPosition.status;

        // Update lift cylinder velocity
        LpsSaWeighInfoTbl.LiftCylVel = UpdateVelocity(LiftCylVelLpFilt, liftPosition.cylinderLength, liftPosition.status);
        weighUpdtTbl.LiftCylVel = LpsSaWeighInfoTbl.LiftCylVel;

        weighUpdtTbl.LiftValveCommand = LpsSaWeighInfoTbl.LiftValveCommand;

        weighUpdtTbl.LiftAngle.Val = liftPosition.angle;
        weighUpdtTbl.LiftAngle.Stat = liftPosition.status;

        // Update lift angular velocity
        LpsSaWeighInfoTbl.LiftAngVel = UpdateVelocity(LiftAngVelLpFilt, liftPosition.angle, liftPosition.status);

        LpsSaWeighInfoTbl.TiltPosition = tiltPosition;

        /* Assigning tilt cylinder extension as percentage of full cylinder extension to weighing library*/
        weighUpdtTbl.TiltCylLengthNorm.Val  = tiltPosition.percentCylinderLength;
        weighUpdtTbl.TiltCylLengthNorm.Stat = tiltPosition.status;

        // Update tilt cylinder velocity
        LpsSaWeighInfoTbl.TiltCylVel = UpdateVelocity(TiltCylVelLpFilt, tiltPosition.cylinderLength, tiltPosition.status);
        weighUpdtTbl.TiltCylVel = LpsSaWeighInfoTbl.TiltCylVel;

        weighUpdtTbl.TiltValveCommand = LpsSaWeighInfoTbl.TiltValveCommand;

        /* Assigning tilt angle ABC as percentage of full cylinder extension angle to weighing library*/
        weighUpdtTbl.TiltAngleABCPercent.Val = tiltPosition.percentAngle;
        weighUpdtTbl.TiltAngleABCPercent.Stat = tiltPosition.status;

        /* Assigning Bucket Angle to weighing library*/
        weighUpdtTbl.BktAngle.Val = tiltPosition.bucketAngle;
        weighUpdtTbl.BktAngle.Stat = tiltPosition.status;

        { // Provide chassis imu data to weighing library
            // Doing nothing special in demo mode for now.
            { // Linear Acceleration Vector
                auto v = chassisImu_.imu.calibratedLinAccelVector();
                weighUpdtTbl.ReferenceIMUData.LinAcc[0] = v.x();
                weighUpdtTbl.ReferenceIMUData.LinAcc[1] = v.y();
                weighUpdtTbl.ReferenceIMUData.LinAcc[2] = v.z();
            }

            { // Angular Velocity Vector
                auto v = chassisImu_.imu.calibratedAngVelVector();
                weighUpdtTbl.ReferenceIMUData.AngVel[0] = v.x();
                weighUpdtTbl.ReferenceIMUData.AngVel[1] = v.y();
                weighUpdtTbl.ReferenceIMUData.AngVel[2] = v.z();
            }

            weighUpdtTbl.ReferenceIMUData.Pitch = chassisImu_.imu.pitchDegrees();
            weighUpdtTbl.ReferenceIMUData.Roll = chassisImu_.imu.rollDegrees();
            weighUpdtTbl.ReferenceIMUData.Stat = chassisImu_.imuOk ? LPS_STATUS_OK : LPS_STATUS_BAD;
        }

        /**************************Tipoff Assist Update Start*******************************/
        LpsSaWeighInfoTbl.TipoffInputs.invalidate_outputs =
                (LpsSaWeighInfoTbl.TipoffInputs.input_status.data ? true:false );
        LpsSaWeighInfoTbl.TipoffInputs.lift_valve_cmd =
                LpsSaWeighInfoTbl.LiftValveCommand.Val;
        LpsSaWeighInfoTbl.TipoffInputs.tilt_valve_cmd =
                LpsSaWeighInfoTbl.TiltValveCommand.Val;
        LpsSaWeighInfoTbl.TipoffInputs.tilt_extension = tiltPosition.cylinderExtension;
        LpsSaWeighInfoTbl.TipoffInputs.lift_angle = liftPosition.angle;
        LpsSaWeighInfoTbl.TipoffInputs.lift_norm_angle = liftPosition.percentAngle/100;
        LpsSaWeighInfoTbl.TipoffInputs.lift_norm_length = liftPosition.percentCylinderLength/100;
        LpsSaWeighInfoTbl.TipoffInputs.tilt_norm_angle = tiltPosition.percentAngle/100;
        LpsSaWeighInfoTbl.TipoffInputs.tilt_norm_length = tiltPosition.percentCylinderLength/100;
        LpsSaWeighInfoTbl.TipoffInputs.pass_count = LpsSaWeighInfoTbl.passCount;
        LpsSaWeighInfoTbl.TipoffInputs.lift_he_pressure = weighUpdtTbl.LiftCylHePres.Val;
        LpsSaWeighInfoTbl.TipoffInputs.lift_re_pressure = weighUpdtTbl.LiftCylRePres.Val;
        LpsSaWeighInfoTbl.TipoffInputs.tilt_he_pressure = LpsSaWeighInfoTbl.TiltCylHePres.Val;
        LpsSaWeighInfoTbl.TipoffInputs.tilt_re_pressure = LpsSaWeighInfoTbl.TiltCylRePres.Val;
        LpsSaWeighInfoTbl.TipoffInputs.tool_mass = payloadCalNvmTbl_.data.EmptyBucketWeightEst;
        LpsSaWeighInfoTbl.TipoffInputs.truck_target_wt = LpsSaWeighInfoTbl.TruckTargetWeight;
        LpsSaWeighInfoTbl.TipoffInputs.truck_start_weight = LpsSaWeighInfoTbl.TruckStartWeight;
        LpsSaWeighInfoTbl.TipoffInputs.bucket_current_weight_accuracy =
                (unsigned_8)GET_ACCURACY(LpsSaWeighInfoTbl.PayloadCalcMeth);
        LpsSaWeighInfoTbl.TipoffInputs.bucket_current_weight = LpsSaWeighInfoTbl.BestBktWtInTonnes;
        LpsSaWeighInfoTbl.TipoffInputs.zero_offset = payloadCalNvmTbl_.data.ZeroWeight;
        if (payloadCalNvmTbl_.data.CalAdjust <= 0.f) {
            LpsSaWeighInfoTbl.TipoffInputs.simple_cal_factor = 1;
        }
        else {
            LpsSaWeighInfoTbl.TipoffInputs.simple_cal_factor = payloadCalNvmTbl_.data.CalAdjust;
        }
        LpsSaWeighInfoTbl.TipoffInputs.mach_pitch_cal_offset = cnfg_.tipoffPitchCalOffset;

        /*
         * If TOA is active, they are not digging.
         *  - Ignoring dig detect while in TOA triggered tipoff is fine.
         * If tip-off is active, but it is manually triggered, they *could* be digging.
         *  - Ignoring dig detect while in manually triggered tipoff is *NOT* fine.
         * If tip-off is active, but automatically triggered, digging will exit auto tip.
         *  - Ignoring dig detect while in automatically triggered tipoff is fine.
         */
        /*if (tipOffActive) {
            // They are not digging, ignore dig flag.
            // Oops, we are ignoring dig if in manual trigger and digging, we don't want to do that.
        }
        else */
        if (LPS_WEIGHT_BKT_DIGGING == LpsSaWeighInfoTbl.DigStat) {
            /*
             * If TOA is active, they are not digging, and false dig detect will unlatch TOA weight
             * If tip-off is active, but it is manually triggered, we will unlatch weight potentially too often, big deal?
             * If tip-off is active, but it is automatically triggered, digging will exit tipoff anyway.
             *
             * The main issue with this is that the weigh library dig detect does not detect
             * end of dig very well and tends to hold on to the "digging" state for too long.
             * This can be looked into as an improvement.
             */
            LpsSaWeighInfoTbl.TipoffInputs.unlatch_trigger = true;
        }

        LpsSaWeighInfoTbl.TipoffInputs.anchor_zero_offset = cnfg_.toaAnchoredZeroOffset;
        LpsSaWeighInfoTbl.TipoffInputs.anchor_factor = cnfg_.toaAnchoredFactor;
        LpsSaWeighInfoTbl.TipoffInputs.bucket_angle = weighUpdtTbl.BktAngle.Val;

        /* Use live tipoff mode is available, otherwise use configured tipoff mode */
        if (LpsSaWeighInfoTbl.TipOffState != LPS_SA_JOB_MGR_TIP_OFF_UNAVAILABLE) {
            LpsSaWeighInfoTbl.TipoffInputs.tipoff_mode =
                    (LpsSaWeighInfoTbl.TipOffState == LPS_SA_JOB_MGR_TIP_OFF_PILE_ENABLE ? 2:0);
        }
        else {
            LpsSaWeighInfoTbl.TipoffInputs.tipoff_mode =
                    (LpsSaWeighInfoTbl.TipOffStateCfg == LPS_SA_JOB_MGR_TIP_OFF_PILE_ENABLE ? 2:0);
        }

        /* Save snapshot of inputs to send to XCP */
        DebugLpsSaXCPChannels.m_TipoffInputs = LpsSaWeighInfoTbl.TipoffInputs;

        /* Periodic update call for Tipoff Assist */
        LpsSaWeighInfoTbl.TipoffOutputsPtr = LpsTipoffAssist.update(LpsSaWeighInfoTbl.TipoffInputs);

        /* Copy outputs for XCP */
        DebugLpsSaXCPChannels.m_TipoffOutputs = *(LpsSaWeighInfoTbl.TipoffOutputsPtr);

        /* only send CAN-11 bit to imple if Tipoff Assist is enabled */
        if ((LpsSaWeighInfoTbl.TipOffAssistEnable) && (!demo_mode)) {
            {
                /* Send out the CAN-11 bit messages - 0x730
                 *  PCS Bucket Current Weight Normalized Error - 0.1% per bit, 16-bits
                 *  PCS Bucket Current Weight Accuracy - Status, 8-bits
                 *  Tilt Sensitivity - mm/rad 1mm/rad/bit, 16-bits
                 *  Tilt HE Pressure - kPa, 1kPa/bit, 16-bits
                 *  padding - 0xFF, 8-bits
                 *  */
                uint8_t data[8] = {0,0,0,0,0,0,0,0xFF};;
                uint_least8_t *data_ptr = (uint_least8_t*)&data[0];
                OEL_PACK_BE_16(data_ptr, (uint16_t)(TOA_current_weight_norm_error_out * 10));
                OEL_PACK_BE_8(data_ptr, (uint8_t)TOA_bucket_current_weight_accuracy);
                OEL_PACK_BE_16(data_ptr, (uint16_t)TOA_tilt_sensitivity_out);
                uint16_t temp_tilt_he_press = 0;
                if (LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.tilt_he_press_bad == 1) {
                    temp_tilt_he_press = UNKNOWN2U + FMICNM;
                }
                else {
                    temp_tilt_he_press = TOA_tilt_he_pressure;
                }
                OEL_PACK_BE_16(data_ptr, (uint16_t)temp_tilt_he_press);
                CANB.sendTX(data,8,0x8730);
            }

            {
                /* Send out the CAN-11 bit messages - 0x731
                 * Tip-Off Assist Advanced ECM Error - Status, 8-bits
                 * Tip-Off Assist Bucket Arbitrated Payload Normalized Error - 0.1% per bit, 16-bits
                 * Tip-Off Assist Pseudo-Frictionless Weigh Status - Status, 8-bits
                 * Tip-Off Assist Pseudo-Frictionless Weigh Normalized Std. Dev. - 0.025%/bit, 16-bits
                 * Tip-Off Assist Bucket Spill Normalized Rate - 0.05%/sec/bit, 8-bits
                 *  */
                uint8_t data[8] = {0,0,0,0,0,0,0,0};;
                uint_least8_t *data_ptr = (uint_least8_t*)&data[0];
                OEL_PACK_BE_8(data_ptr, (uint8_t)TOA_error_code_out );
                OEL_PACK_BE_16(data_ptr, (uint16_t)(TOA_arbitrated_payload_norm_error_out * 10));
                OEL_PACK_BE_8(data_ptr, (uint8_t)TOA_weigh_status_out);
                OEL_PACK_BE_16(data_ptr, (uint16_t)(TOA_payload_norm_stdev_out * 40));
                OEL_PACK_BE_16(data_ptr, (uint16_t)(TOA_spill_rate_out * 20));
                CANB.sendTX(data,8,0x8731);
            }

            {
                /* Send out the CAN-11 bit messages - 0x728
                 * Unsecure Payload Upper Bound Norm Error  - 0.1% per bit, 16-bits
                 * Unsecure Payload Lower Bound Norm Error  - 0.1% per bit, 16-bits
                 * Unsecure Payload Status - Status, 8-bits
                 * Minimum Secure Bucket Angle - 0.01 degrees/bit - 16-bit
                 * padding - 0xFF, 8-bits
                 *  */
                uint8_t data[8] = {0,0,0,0,0,0,0,0xFF};;
                uint_least8_t *data_ptr = (uint_least8_t*)&data[0];
                OEL_PACK_BE_16(data_ptr, (uint16_t)(TOA_unsecured_payload_upper_bound_norm * 10));
                OEL_PACK_BE_16(data_ptr, (uint16_t)(TOA_unsecured_payload_lower_bound_norm * 10));
                OEL_PACK_BE_8(data_ptr, (uint8_t)TOA_unsecured_PFW_status);
                OEL_PACK_BE_16(data_ptr, (uint16_t)(TOA_min_secure_bucket_angle * 100));
                CANB.sendTX(data,8,0x8728);
            }
        }

        /* Update tipoff weight and status for weigh lib */
        weighUpdtTbl.TipoffWeight.Val = TOA_payload_send_to_CPM;
        weighUpdtTbl.TipoffWeight.Stat = (LpsTipoffWeightStat_t)TOA_payload_status_send_to_CPM;

        /* reset input for unlatching weight */
        LpsSaWeighInfoTbl.TipoffInputs.unlatch_trigger = false;

        /**************************Tipoff Assist Update End*********************************/

        /* Updating Weigh Library with Inputs */
        weighRet = LpsUpdt(&weighUpdtTbl);

        /* Update Calibration Library */
        CalLibUpdt();

        // Don't send overrides in demo mode.
        if (!demo_mode) {
            /* Send out the CAN-11 bit messages - 0x729
             * CPM Pro Calibration Active Flag, 8-bits
             * CPM Pro Calibration Override Active Flag, 8-bits
             * CPM Pro Calibration Lift Raise Command Limit (0 to 100), 8-bits
             * CPM Pro Calibration Lift Lower Command Limit (-100 to 0), 8-bits
             * CPM Pro Calibration Tilt Rack Command (0 to 100), 8-bits (RESERVED)
             * CPM Pro Calibration Tilt Dump Command (-100 to 0), 8-bits (RESERVED)
             * Tip Off Assist Bucket Payload Target (0.005 tonnes/bit), 16-bits
             *  */

            /* get lift override cmd */
            int8_t lift_raise_override_limit = LpsCalAppInfGetRaiseCmdLmt();
            int8_t lift_lower_override_limit = LpsCalAppInfGetLowerCmdLmt();
            int8_t tilt_rack_override_cmd = LpsCalAppInfGetRackCmd();
            int8_t tilt_dump_override_cmd = LpsCalAppInfGetDumpCmd();

            uint8_t data[8] = {0,0,0,0,0,0,0,0};
            uint_least8_t *data_ptr = (uint_least8_t*)&data[0];
            OEL_PACK_BE_8(data_ptr, (uint8_t)LpsCalCalInProgress());
            OEL_PACK_BE_8(data_ptr, (uint8_t)LpsCalAppInfCalOvActive());
            OEL_PACK_BE_8(data_ptr, (uint8_t)lift_raise_override_limit);
            OEL_PACK_BE_8(data_ptr, (uint8_t)lift_lower_override_limit);
            OEL_PACK_BE_8(data_ptr, (uint8_t)tilt_rack_override_cmd);
            OEL_PACK_BE_8(data_ptr, (uint8_t)tilt_dump_override_cmd);
            OEL_PACK_BE_16(data_ptr, (uint16_t)(TOA_bucket_payload_target * 200 ));

            CANB.sendTX(data,8,0x8729);
        }
    }

    /* set CAN-11 bit timeout flag if we have not received any messages within the time specified */
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>
                (std::chrono::steady_clock::now() - lastCANRxTimeAll).count();

    if (elapsed_us > CAN_TIMEOUT_USEC_TO_SET_XCP) {
        LpsSaWeighInfoTbl.CAN11MessageTimeoutFlag = true;
    }
    else {
        LpsSaWeighInfoTbl.CAN11MessageTimeoutFlag = false;
    }

    /* Make inputs bad if timeout exceeds the defined timeout and disable payload */
    if (elapsed_us > CAN_TIMEOUT_USEC_TO_DISABLE_PAYLOAD) {
        /* The following flags will set the appropriate PIDs to bad */
        LpsSaWeighInfoTbl.LiftPosition.status = LPS_STATUS_BAD;
        LpsSaWeighInfoTbl.TiltPosition.status = LPS_STATUS_BAD;
        weighUpdtTbl.LiftCylHePres.Stat = LPS_STATUS_BAD;
        weighUpdtTbl.LiftCylRePres.Stat = LPS_STATUS_BAD;
        weighUpdtTbl.LiftCylLengthNorm.Stat = LPS_STATUS_BAD;
        weighUpdtTbl.LiftCylLength.Stat = LPS_STATUS_BAD;
        weighUpdtTbl.TiltCylLengthNorm.Stat = LPS_STATUS_BAD;
        weighUpdtTbl.LiftCylVel.Stat = LPS_STATUS_BAD;
        LpsSaWeighInfoTbl.LiftCylVel.Stat = LPS_STATUS_BAD;
        weighUpdtTbl.TiltCylVel.Stat = LPS_STATUS_BAD;
        LpsSaWeighInfoTbl.TiltCylVel.Stat = LPS_STATUS_BAD;
        weighUpdtTbl.LiftAngle.Stat = LPS_STATUS_BAD;
        weighUpdtTbl.BktAngle.Stat = LPS_STATUS_BAD;

        /* The DiagState needs to be set to ensure that an exclamation shows */
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL] = true;
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL] = true;
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL] = true;
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL] = true;

        /* Set flag to unlatch tipoff weight, when things return back to working conditions */
        LpsSaWeighInfoTbl.TipoffInputs.unlatch_trigger = true;

        /* This does nothing except print log_error messages upstream */
        weighRet = LPS_UPDT_INPUT_ERROR;
    }

    // Choosing some unused debug channels to capture CAN stats.
    DebugLpsSaXCPChannels.DebugLiftCylPeriod = CANB.CAN734_count;
    DebugLpsSaXCPChannels.DebugTiltCylPeriod = CANB.CAN733_count;
    DebugLpsSaXCPChannels.DebugTiltCylWidth = CANB.dropped;

    /* Number of CAN packets processed during this loop */
    AIS_LOG_DEBUG("Number of CAN packets processed in weigh loop: %d", number_of_packets);
    if (number_of_packets == 0) {
        /* This could indicate a potential problem if it is always zero, so logging it */
        AIS_LOG_DEBUG("number of CAN packets is zero; potential problem if it is always zero");
    }

    return weighRet;
}

