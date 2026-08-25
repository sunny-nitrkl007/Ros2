/******************************Standalone********************************/
#include "../LpsSaWeighApp.h"


LpsUpdtErrorTypes_t LpsSaWeighApp::ProcessInputs()
{
    AIS_LOG_DEBUG("Process Inputs Standalone");

    LpsSaLiftPosition_t  liftPosition;
    LpsSaTiltPosition_t  tiltPosition;
    LpsUpdtErrorTypes_t weighRet;

    if (demo_mode) {
        WeighPidTbl.TiltLinkageSensorDc = demoInputs_.tiltposition;   // set tilt DC
        WeighPidTbl.LiftLinkageSensorDc = demoInputs_.liftposition;   // set lift DC

        weighUpdtTbl.LiftCylHePres.Val = demoInputs_.pres_lift_he_demo;
        weighUpdtTbl.LiftCylHePres.Stat = LPS_STATUS_OK;
        weighUpdtTbl.LiftCylRePres.Val = demoInputs_.pres_lift_re_demo;
        weighUpdtTbl.LiftCylRePres.Stat = LPS_STATUS_OK;
        liftPosition = lpsSaGetLiftPosition(demoInputs_.angle_lift_demo, true);
        tiltPosition = lpsSaGetTiltRotaryPosition(demoInputs_.angle_lift_demo, true, demoInputs_.angle_ABC_demo, true);

        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftLinkVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftLinkVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftLinkFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<TiltLinkVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<TiltLinkVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL]) {
            clearAutonomyCondition<TiltLinkFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftREVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftREVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftREFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftHEVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftHEVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL] = false;
        }
        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftHEFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL] = false;
        }

    }
    else {
        /* sample and read PWM inputs */
        inPwm_.samplePWMInputs();

        DebugLpsSaXCPChannels.DebugLiftCylHePeriod = inPwm_.PwmData.Period[LIFT_CYL_HE_PRES_SENS_CH_NUM];
        DebugLpsSaXCPChannels.DebugLiftCylHeWidth  = inPwm_.PwmData.Width[LIFT_CYL_HE_PRES_SENS_CH_NUM];

        DebugLpsSaXCPChannels.DebugLiftCylRePeriod = inPwm_.PwmData.Period[LIFT_CYL_RE_PRES_SENS_CH_NUM];
        DebugLpsSaXCPChannels.DebugLiftCylReWidth  = inPwm_.PwmData.Width[LIFT_CYL_RE_PRES_SENS_CH_NUM];

        DebugLpsSaXCPChannels.DebugLiftCylPeriod = inPwm_.PwmData.Period[LIFT_CYL_POS_SENS_CH_NUM];
        DebugLpsSaXCPChannels.DebugLiftCylWidth  = inPwm_.PwmData.Width[LIFT_CYL_POS_SENS_CH_NUM];

        DebugLpsSaXCPChannels.DebugTiltCylPeriod = inPwm_.PwmData.Period[TILT_CYL_POS_SENS_CH_NUM];
        DebugLpsSaXCPChannels.DebugTiltCylWidth  = inPwm_.PwmData.Width[TILT_CYL_POS_SENS_CH_NUM];

        weighUpdtTbl.LiftCylHePres = LiftHeadEndPresTransferFunc(
                inPwm_.PwmData.Width[LIFT_CYL_HE_PRES_SENS_CH_NUM],
                inPwm_.PwmData.Period[LIFT_CYL_HE_PRES_SENS_CH_NUM],
                inPwm_.PwmData.SwitchStatus[LIFT_CYL_HE_PRES_SENS_CH_NUM],
                inPwm_.PwmData.Timeout[LIFT_CYL_HE_PRES_SENS_CH_NUM]);

        weighUpdtTbl.LiftCylRePres = LiftRodEndPresTransferFunc(
                inPwm_.PwmData.Width[LIFT_CYL_RE_PRES_SENS_CH_NUM],
                inPwm_.PwmData.Period[LIFT_CYL_RE_PRES_SENS_CH_NUM],
                inPwm_.PwmData.SwitchStatus[LIFT_CYL_RE_PRES_SENS_CH_NUM],
                inPwm_.PwmData.Timeout[LIFT_CYL_RE_PRES_SENS_CH_NUM]);

        /* Conversion of Lift position sensor input to Lift cylinder extension */
        liftPosition = LpsSaLiftSensorTransferFunc(
                inPwm_.PwmData.Width[LIFT_CYL_POS_SENS_CH_NUM],
                inPwm_.PwmData.Period[LIFT_CYL_POS_SENS_CH_NUM],
                inPwm_.PwmData.SwitchStatus[LIFT_CYL_POS_SENS_CH_NUM],
                inPwm_.PwmData.Timeout[LIFT_CYL_POS_SENS_CH_NUM]);

        /* Conversion of Tilt position sensor input to Tilt cylinder extension */
        tiltPosition = LpsSaTiltSensorTransferFunc(liftPosition,
                inPwm_.PwmData.Width[TILT_CYL_POS_SENS_CH_NUM],
                inPwm_.PwmData.Period[TILT_CYL_POS_SENS_CH_NUM],
                inPwm_.PwmData.SwitchStatus[TILT_CYL_POS_SENS_CH_NUM],
                inPwm_.PwmData.Timeout[TILT_CYL_POS_SENS_CH_NUM]);
    }

    LpsSaWeighInfoTbl.LiftPosition = liftPosition;

    /* Assigning lift cylinder extension as percentage of full cylinder extension to weighing library*/
    weighUpdtTbl.LiftCylLengthNorm.Val  = liftPosition.percentCylinderLength;
    weighUpdtTbl.LiftCylLengthNorm.Stat = liftPosition.status;

    weighUpdtTbl.LiftCylLength.Val  = liftPosition.cylinderLength;
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

    /* Update tipoff weight and status for weigh lib - Disabled for Standalone */
    weighUpdtTbl.TipoffWeight.Val = 0.f;
    weighUpdtTbl.TipoffWeight.Stat = LPS_TIPOFF_WEIGHT_INVALID;

    /* Updating Weigh Library with Inputs */
    weighRet = LpsUpdt(&weighUpdtTbl);

    /* Update Calibration Library */
    CalLibUpdt();

    return weighRet;
}

