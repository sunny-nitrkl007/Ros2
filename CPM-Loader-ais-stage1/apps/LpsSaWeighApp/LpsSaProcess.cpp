/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaProcess.cpp
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef  _LPS_SA_WEIGHAPP_H_
#include "LpsSaWeighApp.h"
#endif

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/

// Period is in microseconds so we have to convert to Hz by multiplying by
//  1000000 microseconds per second
#define CONVERT_PWM_COUNTS_TO_FREQ(PERIOD) ((1.f/float(PERIOD)) * 1000000.f)

// Pulse Width divided by period is multiplied 100 to get it in that range
//  of 0% to 100% duty cycle
#define CONVERT_PWM_COUNTS_TO_DC(WIDTH, PERIOD) ((float(WIDTH)/float(PERIOD)) * 100.f)

/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/
static float sensorDutyCycleToPressure(float dc, float intercept, float slope);


/******************************************************************************
FUNCTION NAME: sensorDutyCycleToPressure
DESCRIPTION: Converts pressure sensor duty cycle to pressure in kPa.
PARAMETER DESCRIPTION:
 dc - sensor duty cycle 0 to 100
RETURN VALUE: pressure in kPa
*******************************************************************************/
static float sensorDutyCycleToPressure(float dc, float intercept, float slope) {
    /*
     * Computing sensor pressure in Kpa - limit at min/max pressures that the sensor supports
     *  This is done to avoid the pressures going negative after the scaling is applied
     */
    float pressure = (dc - intercept)/slope;

    if (pressure > HYD_PRES_SENSOR_MAX_VALUE_KPA) {
        pressure = HYD_PRES_SENSOR_MAX_VALUE_KPA;
    }
    else if (pressure < HYD_PRES_SENSOR_MIN_VALUE_KPA) {
        pressure = HYD_PRES_SENSOR_MIN_VALUE_KPA;
    }

    return pressure;
}

/******************************************************************************
FUNCTION NAME: liftHeadEndPresTransferFunc
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
LpsFloatIO_t LpsSaWeighApp::LiftHeadEndPresTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout) {
    LpsFloatIO_t presInfo;

    float sensorFreq = CONVERT_PWM_COUNTS_TO_FREQ(period);
    float sensorDutyCycle = CONVERT_PWM_COUNTS_TO_DC(width,period);

    bool good = true;

    // Override according to test plan
    if (testFixture_.tpLiftHeadEndPressure(sensorDutyCycle)) {
        Timeout = PwmInputTimeout::NOTIMEOUT;
        sensorFreq = (HYD_PRES_SENSOR_MAX_FREQ + HYD_PRES_SENSOR_MIN_FREQ) / 2.f;
    }

    DebugLpsSaXCPChannels.DebugLiftCylHeDc = sensorDutyCycle;

    if (Timeout == PwmInputTimeout::TIMEOUT) {
        good = false;

        if (SwitchStatus == PwmInputSwitchStatus::INACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(LiftHEVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftHEVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL] = false;
        }

        if (SwitchStatus == PwmInputSwitchStatus::ACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(LiftHEVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftHEVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL] = false;
        }

        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftHEFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL] = false;
        }
    }
    else {
        // Voltage Above Normal or Open Circuit
        if (good && (sensorDutyCycle > HYD_PRES_SENSOR_MAX_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(LiftHEVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftHEVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL] = false;
        }

        // Voltage Below Normal
        if (good && (sensorDutyCycle < HYD_PRES_SENSOR_MIN_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(LiftHEVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftHEVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL] = false;
        }

        // Abnormal Frequency
        if (good && ((sensorFreq > HYD_PRES_SENSOR_MAX_FREQ) || (sensorFreq < HYD_PRES_SENSOR_MIN_FREQ))) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL]) {
                setAutonomyCondition(LiftHEFreqAbnormal());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftHEFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_HE_FREQ_ABNORMAL] = false;
        }
    }

    if (good) {
        presInfo.Val = sensorDutyCycleToPressure(sensorDutyCycle, linkage_table_cnfg.hydPresSensorIntercept, linkage_table_cnfg.hydPresSensorSlope);
        presInfo.Stat = LPS_STATUS_OK;
    }
    else {
        presInfo.Val  = 0.0;
        presInfo.Stat = LPS_STATUS_BAD;
    }

    return presInfo;
}

/******************************************************************************
FUNCTION NAME: liftRodEndPresTransferFunc
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
LpsFloatIO_t LpsSaWeighApp::LiftRodEndPresTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout) {
    LpsFloatIO_t presInfo;

    float sensorFreq = CONVERT_PWM_COUNTS_TO_FREQ(period);
    float sensorDutyCycle = CONVERT_PWM_COUNTS_TO_DC(width,period);

    bool good = true;

    // Override according to test plan
    if (testFixture_.tpLiftRodEndPressure(sensorDutyCycle)) {
        Timeout = PwmInputTimeout::NOTIMEOUT;
        sensorFreq = (HYD_PRES_SENSOR_MAX_FREQ + HYD_PRES_SENSOR_MIN_FREQ) / 2.f;
    }

    DebugLpsSaXCPChannels.DebugLiftCylReDc = sensorDutyCycle;

    if (Timeout == PwmInputTimeout::TIMEOUT) {
        good = false;

        if (SwitchStatus == PwmInputSwitchStatus::INACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(LiftREVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftREVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL] = false;
        }

        if (SwitchStatus == PwmInputSwitchStatus::ACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(LiftREVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftREVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL] = false;
        }

        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftREFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL] = false;
        }
    }
    else {
        // Voltage Above Normal or Open Circuit
        if (good && (sensorDutyCycle > HYD_PRES_SENSOR_MAX_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(LiftREVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftREVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL] = false;
        }

        // Voltage Below Normal
        if (good && (sensorDutyCycle < HYD_PRES_SENSOR_MIN_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(LiftREVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftREVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL] = false;
        }

        // Abnormal Frequency
        if (good && ((sensorFreq > HYD_PRES_SENSOR_MAX_FREQ) || (sensorFreq < HYD_PRES_SENSOR_MIN_FREQ))) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL]) {
                setAutonomyCondition(LiftREFreqAbnormal());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftREFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_RE_FREQ_ABNORMAL] = false;
        }
    }

    if (good) {
        presInfo.Val = sensorDutyCycleToPressure(sensorDutyCycle, linkage_table_cnfg.hydPresSensorIntercept, linkage_table_cnfg.hydPresSensorSlope);
        presInfo.Stat = LPS_STATUS_OK;
    }
    else {
        presInfo.Val  = 0.0;
        presInfo.Stat = LPS_STATUS_BAD;
    }

    return presInfo;
}

/******************************************************************************
FUNCTION NAME: TiltHeadEndPresTransferFunc
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
LpsFloatIO_t LpsSaWeighApp::TiltHeadEndPresTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout) {
    LpsFloatIO_t presInfo;

    float sensorFreq = CONVERT_PWM_COUNTS_TO_FREQ(period);
    float sensorDutyCycle = CONVERT_PWM_COUNTS_TO_DC(width,period);

    bool good = true;

    DebugLpsSaXCPChannels.DebugTiltCylHeDc = sensorDutyCycle;

    if (Timeout == PwmInputTimeout::TIMEOUT) {
        good = false;

        if (SwitchStatus == PwmInputSwitchStatus::INACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(TiltHEVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<TiltHEVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_ABOVE_NORMAL] = false;
        }

        if (SwitchStatus == PwmInputSwitchStatus::ACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(TiltHEVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<TiltHEVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_BELOW_NORMAL] = false;
        }

        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<TiltHEFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_FREQ_ABNORMAL] = false;
        }
    }
    else {
        // Voltage Above Normal or Open Circuit
        if (good && (sensorDutyCycle > HYD_PRES_SENSOR_MAX_DC)) {
            good = false;
            if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(TiltHEVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<TiltHEVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_ABOVE_NORMAL] = false;
        }

        // Voltage Below Normal
        if (good && (sensorDutyCycle < HYD_PRES_SENSOR_MIN_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(TiltHEVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<TiltHEVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_VOLTAGE_BELOW_NORMAL] = false;
        }

        // Abnormal Frequency
        if (good && ((sensorFreq > HYD_PRES_SENSOR_MAX_FREQ) || (sensorFreq < HYD_PRES_SENSOR_MIN_FREQ))) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_FREQ_ABNORMAL]) {
                setAutonomyCondition(TiltHEFreqAbnormal());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_FREQ_ABNORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<TiltHEFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_HE_FREQ_ABNORMAL] = false;
        }
    }

    if (good) {
        presInfo.Val = sensorDutyCycleToPressure(sensorDutyCycle, linkage_table_cnfg.hydPresSensorIntercept, linkage_table_cnfg.hydPresSensorSlope);
        presInfo.Stat = LPS_STATUS_OK;

    }
    else {
        presInfo.Val  = 0.0;
        presInfo.Stat = LPS_STATUS_BAD;

    }

    return presInfo;
}

/******************************************************************************
FUNCTION NAME: TiltRodEndPresTransferFunc
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
LpsFloatIO_t LpsSaWeighApp::TiltRodEndPresTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout) {
    LpsFloatIO_t presInfo;

    float sensorFreq = CONVERT_PWM_COUNTS_TO_FREQ(period);
    float sensorDutyCycle = CONVERT_PWM_COUNTS_TO_DC(width,period);

    bool good = true;

    DebugLpsSaXCPChannels.DebugTiltCylReDc = sensorDutyCycle;

    if (Timeout == PwmInputTimeout::TIMEOUT) {
        good = false;

        if (SwitchStatus == PwmInputSwitchStatus::INACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(TiltREVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<TiltREVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_ABOVE_NORMAL] = false;
        }

        if (SwitchStatus == PwmInputSwitchStatus::ACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(TiltREVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<TiltREVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_BELOW_NORMAL] = false;
        }

        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<TiltREFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_FREQ_ABNORMAL] = false;
        }
    }
    else {
        // Voltage Above Normal or Open Circuit
        if (good && (sensorDutyCycle > HYD_PRES_SENSOR_MAX_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(TiltREVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<TiltREVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_ABOVE_NORMAL] = false;
        }

        // Voltage Below Normal
        if (good && (sensorDutyCycle < HYD_PRES_SENSOR_MIN_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(TiltREVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<TiltREVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_VOLTAGE_BELOW_NORMAL] = false;
        }

        // Abnormal Frequency
        if (good && ((sensorFreq > HYD_PRES_SENSOR_MAX_FREQ) || (sensorFreq < HYD_PRES_SENSOR_MIN_FREQ))) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_FREQ_ABNORMAL]) {
                setAutonomyCondition(TiltREFreqAbnormal());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_FREQ_ABNORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_FREQ_ABNORMAL]) {
            clearAutonomyCondition<TiltREFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_RE_FREQ_ABNORMAL] = false;
        }
    }

    if (good) {
        presInfo.Val = sensorDutyCycleToPressure(sensorDutyCycle, linkage_table_cnfg.hydPresSensorIntercept, linkage_table_cnfg.hydPresSensorSlope);
        presInfo.Stat = LPS_STATUS_OK;
    }
    else {
        presInfo.Val  = 0.0;
        presInfo.Stat = LPS_STATUS_BAD;
    }

    return presInfo;
}

/******************************************************************************
FUNCTION NAME: UpdateVelocity
DESCRIPTION: Update the velocity calculation
PARAMETER DESCRIPTION:
    DERFILT& filt - reference to filter workspace
    float value - the new input sample value
    LpsStat_t status - the new input sample status
RETURN VALUE:
*******************************************************************************/
LpsFloatIO_t LpsSaWeighApp::UpdateVelocity(DERFILT& filt, float value, LpsStat_t status)
{
    LpsFloatIO_t filteredValue;

    if (LPS_STATUS_BAD != status) {
        filteredValue.Val = cpm_filter_derivative_filter(&filt, value);
        filteredValue.Stat = LPS_STATUS_OK;
    }
    else {
        cpm_filter_derivative_filter_reset(&filt);
        filteredValue.Val = 0.0;
        filteredValue.Stat = LPS_STATUS_BAD;
    }

    return filteredValue;
}

/******************************************************************************
FUNCTION NAME: LpsSaLiftSensorTransferFunc
DESCRIPTION:            
PARAMETER DESCRIPTION:                        
RETURN VALUE:             
*******************************************************************************/
LpsSaLiftPosition_t LpsSaWeighApp::LpsSaLiftSensorTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout)
{
    LpsSaLiftPosition_t liftPosition;
    float sensorDutyCycle;
    float sensorFreq;

    /* Convert PWM inputs to duty cycle */
    sensorDutyCycle = CONVERT_PWM_COUNTS_TO_DC(width, period);
    if (0 != linkage_table_cnfg.invertLiftDc) {
        // Invert if needed.
        sensorDutyCycle = 100.f - sensorDutyCycle;
    }

    sensorFreq = CONVERT_PWM_COUNTS_TO_FREQ(period);

    bool good = true;

    if (testFixture_.tpLiftPositionSensorDutyCycle(sensorDutyCycle)) {
        // Override according to test plan
        Timeout = PwmInputTimeout::NOTIMEOUT;
        sensorFreq = (CYL_POS_SENSOR_MAX_FREQ + CYL_POS_SENSOR_MIN_FREQ) / 2.f;
    }

    if (Timeout == PwmInputTimeout::TIMEOUT) {
        good = false;

        if (SwitchStatus == PwmInputSwitchStatus::INACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(LiftLinkVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftLinkVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL] = false;
        }

        if (SwitchStatus == PwmInputSwitchStatus::ACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(LiftLinkVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftLinkVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL] = false;
        }

        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftLinkFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL] = false;
        }
    }
    else {
        // Voltage Above Normal or Open Circuit
        if (good && (sensorDutyCycle > LIFT_TILT_CYL_MAX_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(LiftLinkVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<LiftLinkVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL] = false;
        }

        // Voltage Below Normal
        if (good && (sensorDutyCycle < LIFT_TILT_CYL_MIN_DC)) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(LiftLinkVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<LiftLinkVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL] = false;
        }

        // Abnormal Frequency
        if (good && ((sensorFreq > CYL_POS_SENSOR_MAX_FREQ) || (sensorFreq < CYL_POS_SENSOR_MIN_FREQ))) {
            good = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL]) {
                setAutonomyCondition(LiftLinkFreqAbnormal());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL]) {
            clearAutonomyCondition<LiftLinkFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_FREQ_ABNORMAL] = false;
        }
    }

    float liftAngle = lpsSaGetLiftAngle(sensorDutyCycle, good);

    /* Convert angle to cylinder extension based on the map from selected application */
    liftPosition = lpsSaGetLiftPosition(liftAngle, good);

    return liftPosition;
}

/******************************************************************************
FUNCTION NAME: LpsSaTiltSensorTransferFunc
DESCRIPTION:            
PARAMETER DESCRIPTION:                        
RETURN VALUE:             
*******************************************************************************/
LpsSaTiltPosition_t LpsSaWeighApp::LpsSaTiltSensorTransferFunc(
        const LpsSaLiftPosition_t& liftPosition,
        int tiltSensorPulseWidth, int tiltSensorPeriod,
        PwmInputSwitchStatus tiltSwitchStatus, PwmInputTimeout tiltTimeout)
{
    LpsSaTiltPosition_t tiltPosition;
    float tiltSensorDutyCycle, tiltSensorFreq;

    /* Convert PWM inputs to duty cycle */
    tiltSensorDutyCycle = CONVERT_PWM_COUNTS_TO_DC(tiltSensorPulseWidth, tiltSensorPeriod);
    if (0 != linkage_table_cnfg.invertTiltDc) {
        // Invert if needed.
        tiltSensorDutyCycle = 100.f - tiltSensorDutyCycle;
    }

    /* Convert period to frequency */
    tiltSensorFreq = CONVERT_PWM_COUNTS_TO_FREQ(tiltSensorPeriod);

    /* If lift is bad, tilt can't be good */
    bool liftGood = (LPS_STATUS_OK == liftPosition.status);

    bool tiltGood = true;

    if (testFixture_.tpTiltPositionSensorDutyCycle(tiltSensorDutyCycle)) {
        // Override according to test plan
        tiltTimeout = PwmInputTimeout::NOTIMEOUT;
        tiltSensorFreq = (CYL_POS_SENSOR_MAX_FREQ + CYL_POS_SENSOR_MIN_FREQ) / 2.f;
    }

    if (tiltTimeout == PwmInputTimeout::TIMEOUT) {
        tiltGood = false;

        if (tiltSwitchStatus == PwmInputSwitchStatus::INACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(TiltLinkVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<TiltLinkVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL] = false;
        }

        if (tiltSwitchStatus == PwmInputSwitchStatus::ACTIVE) {
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(TiltLinkVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<TiltLinkVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL] = false;
        }

        if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL]) {
            clearAutonomyCondition<TiltLinkFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL] = false;
        }
    }
    else {
        // Voltage Above Normal or Open Circuit
        if (tiltGood && (tiltSensorDutyCycle > LIFT_TILT_CYL_MAX_DC)) {
            tiltGood = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL]) {
                setAutonomyCondition(TiltLinkVoltageAbove());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL]) {
            clearAutonomyCondition<TiltLinkVoltageAbove>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL] = false;
        }

        // Voltage Below Normal
        if (tiltGood && (tiltSensorDutyCycle < LIFT_TILT_CYL_MIN_DC)) {
            tiltGood = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL]) {
                setAutonomyCondition(TiltLinkVoltageBelow());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL]) {
            clearAutonomyCondition<TiltLinkVoltageBelow>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL] = false;
        }

        // Abnormal Frequency
        if (tiltGood && ((tiltSensorFreq > CYL_POS_SENSOR_MAX_FREQ) || (tiltSensorFreq < CYL_POS_SENSOR_MIN_FREQ))) {
            tiltGood = false;
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL]) {
                setAutonomyCondition(TiltLinkFreqAbnormal());
                LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL] = true;
            }
        }
        else if (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL]) {
            clearAutonomyCondition<TiltLinkFreqAbnormal>();
            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_FREQ_ABNORMAL] = false;
        }
    }

    float liftAngle = liftPosition.angle; // Get lift angle

    /* Normalize sensor duty cycle for tilt */
    float normalizedPosition = lpsSaGetNormalizedTiltPosition(tiltSensorDutyCycle, tiltGood);

    if (linkage_table_cnfg.tiltSensorType == TILT_SENSOR_TYPE_ROTARY) {

        float tiltAngle = lpsSaGetTiltAngle(normalizedPosition, tiltGood);

        /* Convert duty cycle to tilt cylinder extension(in percentage and millimeter) based on the machine specific maps */
        /* This function has side effects of updating a bunch of class variables */
        tiltPosition = lpsSaGetTiltRotaryPosition(liftAngle, liftGood, tiltAngle, tiltGood);
    }
    else { /* TILT_SENSOR_TYPE_INLINE */
        float liftCylinderLength = liftPosition.cylinderLength; // Get lift cylinder length

        float tiltCylinderLength = LpsLookup(normalizedPosition,
                linkage_table_cnfg.inTiltDc.data(),
                linkage_table_cnfg.outTiltCylLen.data(),
                linkage_table_cnfg.inTiltDc.size());

        /* Convert duty cycle to tilt angle based on the machine specific maps */
        /* This function has side effects of updating a bunch of class variables */
        tiltPosition = lpsSaGetTiltInlinePosition(liftAngle, liftCylinderLength, liftGood, tiltCylinderLength, tiltGood);
    }

    return tiltPosition;
}

/******************************************************************************
FUNCTION NAME:LpsSaHydOilTempTransferFunc
DESCRIPTION:            
PARAMETER DESCRIPTION:                        
RETURN VALUE:             
*******************************************************************************/
LpsHydOilTemp_t LpsSaWeighApp::LpsSaHydOilTempTransferFunc(void)
{
    LpsHydOilTemp_t hydOilTemp;

    if (testFixture_.tpHydaulicOilTemperature(WeighPidTbl.HydOilTemp)) {
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::HYDRAULIC_OIL_TEMP_BAD] = false;
    }

    hydOilTemp.Val  = WeighPidTbl.HydOilTemp;

    if ((machineProperties.hydOilTempOptional) && (!cnfg_.hydOilTempEnabled)) {
        // Hydraulic oil temperature is optional and disabled.
        hydOilTemp.Stat = LPS_HYD_OIL_TEMP_STATUS_DISABLED;
    }
    // Set the status based on diag state which is derived from dsi
    else if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::HYDRAULIC_OIL_TEMP_BAD]) {
        hydOilTemp.Stat = LPS_HYD_OIL_TEMP_STATUS_OK;
    }
    else {
        hydOilTemp.Stat = LPS_HYD_OIL_TEMP_STATUS_BAD;
    }

    return hydOilTemp;
}

/******************************************************************************
FUNCTION ; CheckCalibrationStatus
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::UpdateCalibrationDiagnostics(void)
{
    bool isTiltLinkCalibrated = GetTiltCylCalStatus();

    if ((!isTiltLinkCalibrated) && (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_OUT_OF_CAL])) {
        // If we are not ADVANCED or we are ADVANCED LFT, log this diagnostic
        if ((ADVANCED != getApplicationVariant()) || payloadCalNvmTbl_.legalForTradeInstalled) {
            setAutonomyCondition(TiltLinkCalibrationOut());
        }
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_OUT_OF_CAL] = true;
    }
    else if (isTiltLinkCalibrated && (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_OUT_OF_CAL])) {
        clearAutonomyCondition<TiltLinkCalibrationOut>();
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::TILT_LINK_OUT_OF_CAL] = false;
    }

    bool isLiftLinkCalibrated = GetLiftCylCalStatus();

    if ((!isLiftLinkCalibrated) && (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_OUT_OF_CAL])) {
        // If we are not ADVANCED or we are ADVANCED LFT, log this diagnostic
        if ((ADVANCED != getApplicationVariant()) || payloadCalNvmTbl_.legalForTradeInstalled) {
            setAutonomyCondition(LiftLinkCalibrationOut());
        }
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_OUT_OF_CAL] = true;
    }
    else if ((isLiftLinkCalibrated) && (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_OUT_OF_CAL])) {
        clearAutonomyCondition<LiftLinkCalibrationOut>();
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::LIFT_LINK_OUT_OF_CAL] = false;
    }

    bool isPayLoadMonSysCalibrated = GetPayloadMonSysCalStatus();

    if ((!isPayLoadMonSysCalibrated) && (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_OUT_OF_CAL])) {
        setAutonomyCondition(PayloadMonCalibrationOut());
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_OUT_OF_CAL] = true;
    }
    else if ((isPayLoadMonSysCalibrated) && (LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_OUT_OF_CAL])) {
        clearAutonomyCondition<PayloadMonCalibrationOut>();
        LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_OUT_OF_CAL] = false;
    }
}

/******************************************************************************
FUNCTION NAME:LpsSaUpdateWeighingLib
DESCRIPTION:            
PARAMETER DESCRIPTION:                        
RETURN VALUE:             
*******************************************************************************/
 LpsUpdtErrorTypes_t LpsSaWeighApp::LpsSaUpdateWeighingLib()
{
    LpsUpdtErrorTypes_t weighRet;

    /* Check if request for Best Bucket Weight reset is received */
    if (LpsSaWeighReqstChannelStorage::Command::RESET_BEST_BUCKET_WEIGHT == request_.command) {
        AIS_LOG_NOTICE("Reset Best Bucket Weight Request Received.");

        /* Reset Best Bucket Weight*/
        LpsWeighResetBestAvailableBktWt();

        /* Set input true for tipoff assist to unlatch weight if we are not in tipoff */
        if (!LpsSaWeighInfoTbl.TipoffActive) {
            LpsSaWeighInfoTbl.TipoffInputs.unlatch_trigger = true;
        }

        LpsSaScsSendReqstResponse(request_.command, true);
    }
    /* Check if request for Zero Weight Adjust is received */
    else if (LpsSaWeighReqstChannelStorage::Command::ZERO == request_.command) {
        AIS_LOG_NOTICE("Zero Request Received.");

        // Command Weigh Library for Zero Adjust
        if (LpsWeighZeroAdjust()) {
            float zeroWeight = LpsWeighGetZeroWeight();

            // Update the init table with new zero weight to keep it in sync.
            LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.CalibTbl.ZeroWeight = zeroWeight;

            // Zero was successful.
            // If the zero request was just made, at least show the
            // "Zero Accepted" info pop-up for minimum 5 seconds.
            ZeroAcceptedInfoExpireTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);

            // Tell job manager about the ZERO
            LpsSaScsSendZeroRqst();

            // Updating storage
            payloadCalNvmTbl_.data.ZeroWeight = zeroWeight;
            payloadCalNvmTbl_.setSaveNeeded();

            AIS_LOG_NOTICE("Zeroed  Weight = %f", zeroWeight);

            // As Zero Adjust was successful send positive response/acknowledgment
            LpsSaScsSendReqstResponse(request_.command, true);
            AIS_LOG_INFO("Sent Positive response for Zero Adjust request");
        }
        else {
            // Zero was not successful, why?
            LpsZeroErrorTypes_t zeroAdjStatus = LpsWeighGetZeroAdjStatus();

            if (LPS_ZERO_UPDATE_TOO_HEAVY_TO_ZERO == zeroAdjStatus) {
                // "Too Heavy to Zero" info pop-up for minimum 5 seconds.
                TooHeavyToZeroInfoExpireTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
            }

            // As Zero Adjust was unsuccessful send negative response/acknowledgment
            LpsSaScsSendReqstResponse(request_.command, false);
            AIS_LOG_NOTICE("Sent Negative response for Zero Adjust request, status = %d", zeroAdjStatus);
        }
    }
    else if (LpsSaWeighReqstChannelStorage::Command::CAPTURE_CYLINDER_EXTENSION_REFERENCE == request_.command) {
        AIS_LOG_NOTICE("Capture Tilt Cylinder Extension Request Received.");
        LpsWeighSetTiltExtensionThreshold();
        LpsSaScsSendReqstResponse(request_.command, true);
    }
    /*  reweigh warning clearing */
    else if (LpsSaWeighReqstChannelStorage::Command::CLEAR_REWEIGH_WARNING == request_.command) {
        weighUpdtTbl.StandbyActiveState = FALSE;
        LpsSaScsSendReqstResponse(request_.command, true);
    }

    CheckForFilterSettleTime();

    /* Update the Loader payload target weight from info table */
    weighUpdtTbl.LoaderBktPayldTrgtWt = cnfg_.bucketPayloadTargetWeight;
    weighUpdtTbl.LoadCheckValveInstalled = cnfg_.loadCheckValveInstalled;
    weighUpdtTbl.IMUCompEnabled = cnfg_.imuCompEnabled;
    weighUpdtTbl.HydOilTemp  = LpsSaHydOilTempTransferFunc();
    weighUpdtTbl.ClockKeyonSec = GetSysMonotonicTime();
    weighUpdtTbl.PassCount = LpsSaWeighInfoTbl.passCount;
    weighUpdtTbl.StandbyActiveState = (char)LpsSaWeighInfoTbl.StandbyState;

    /* Update legal for trade installation status */
    weighUpdtTbl.LegalForTradeInstalled = payloadCalNvmTbl_.legalForTradeInstalled;

    weighUpdtTbl.LegalForTradeSealed = sealTracker_.isSealed();

    weighUpdtTbl.InVerificationMode = LpsSaWeighInfoTbl.inVerificationMode;

    weighUpdtTbl.WeightInterval = lps_common_weight_to_tonnes(LpsSaWeighInfoTbl.weightInterval, LpsSaWeighInfoTbl.weightUnits);
    weighUpdtTbl.WeightCapacity = LpsSaWeighInfoTbl.weightCapacity;

    // make status bad if event is active
    if ((LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_LOW]) ||
            (LpsSaWeighInfoTbl.EventState[ACDEventPopUp::BATTERY_HIGH]) ) {
        weighUpdtTbl.VoltageEventActive = true;
    }
    else {
        weighUpdtTbl.VoltageEventActive = false;
    }

    // Weighing library needs to know if a calibration is active.
    weighUpdtTbl.CalInProgress = LpsCalCalInProgress();

    /*
     * Process inputs to calculate pressures and positions (cylinder extensions)
     * This also calls...
     *  LpsUpdt(), the weigh library update function, and
     *  CalLibUpdt(), the calibration library update function.
     */
    weighRet = ProcessInputs();

    // Update calibration diagnostics
    UpdateCalibrationDiagnostics();

    if (GetCalWtRequired()) {
        WeighPidTbl.PloadSysCalWtEntryReqStat = CAL_ENTRY_REQUIRED;
    }
    else {
        WeighPidTbl.PloadSysCalWtEntryReqStat = CAL_ENTRY_NOT_REQUIRED;
    }

    /* Read All Pop Up Info including Zeroing Status (If Applicable) Once the Weigh library Updated */
    LpsSaWeighGetAllInfoPopUp();

    return weighRet;
}

PloadSysZeroStat_t LpsSaWeighApp::GetZeroStat(void) {
    PloadSysZeroStat_t status;
    float weight = LpsWeighGetZeroWeight();

    // The worst way of determining whether or not we are zeroed
    if ((weight >= 0.01) || (weight <= -0.01)) {
        status = ZEROED;
    }
    else {
        status = NOT_ZEROED;
    }

    return status;
}

/******************************************************************************
 FUNCTION CheckForFilterSettleTime
 DESCRIPTION:
 PARAMETER DESCRIPTION:
 RETURN VALUE:
 *******************************************************************************/
void LpsSaWeighApp::CheckForFilterSettleTime(void) {
    weighUpdtTbl.UseFilterDataFlag = TRUE;
    unsigned int reqDelay;
    reqDelay = (LpsSaWeighInfoTbl.FilterTauDelay/(float)((float)(1000/LpsSaWeighInfoTbl.PwmcycleRate)/(float)1000))+1;

    if (LpsSaWeighInfoTbl.FilterSettleDelay < reqDelay) {
        LpsSaWeighInfoTbl.FilterSettleDelay++;
        weighUpdtTbl.UseFilterDataFlag = FALSE;
        AIS_LOG_DEBUG("requiredDelay = %i",reqDelay);
        AIS_LOG_DEBUG("UseFilterDataFlag == FALSE");
    }
}

/******************************************************************************
 FUNCTION NAME: LpsSaWeighGetAllInfoPopUp
 DESCRIPTION: Collects all info pop up information
 PARAMETER DESCRIPTION: boolean zeroReqstStat   (Zero Request by Operator)
 RETURN VALUE: None
 Author CWS ID: kathis2
 Last Modified: Nov 15, 2017
 *******************************************************************************/
void LpsSaWeighApp::LpsSaWeighGetAllInfoPopUp()
{
    LpsSaWeighInfoTbl.InfoState.reset();

    if (CAL_ENTRY_REQUIRED == WeighPidTbl.PloadSysCalWtEntryReqStat) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_CAL_WT_ENTRY_REQUIRED] = true;
    }

    // If we have any active diagnostics don't show any other info pop-ups
    if (LpsSaWeighInfoTbl.DiagState.any()) {
        return;
    }

    LpsWeighWrwReweighStatus_t wrwReweighRetStat = LpsGetWrwReweighWarning();

    if (TRUE == wrwReweighRetStat.TooSlow) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_LIFT_TOO_SLOW] = true;
    }

    if (TRUE == wrwReweighRetStat.Stopped) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_STOPPED_IN_RANGE] = true;
    }

    if (TRUE == wrwReweighRetStat.NotRacked) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_NOT_RACKED] = true;
    }

    if (TRUE == wrwReweighRetStat.ExcessivePitch) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_EXCESSIVE_PITCH] = true;
    }

    if (TRUE == wrwReweighRetStat.Jerky) { // Speed Change Too High
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_SPEED_CHANGING] = true;
    }

    if (TRUE == wrwReweighRetStat.Inconsistent) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_INCONSISTENT] = true;
    }

    if (TRUE == wrwReweighRetStat.RateOfChangeHi) { // Pressure Change Too High
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_PRESSURE_CHANGING] = true;
    }

    if (TRUE == wrwReweighRetStat.WarmUpLift) { // Warmup Lift Performed
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_WARMUP_LIFT] = true;
    }

    // Keep the "Zero Accepted" info pop-up until at least this time is met.
    if (ZeroAcceptedInfoExpireTime > std::chrono::steady_clock::now()) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_ZERO_ACCEPTED] = true;
    }

    // Keep the "Too Heavy to Zero" info pop-up until at least this time is met.
    if (TooHeavyToZeroInfoExpireTime > std::chrono::steady_clock::now()) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_TOO_HEAVY_TO_ZERO] = true;
    }

    { // Update Payload System Not Zeroed Info Pop Up
        LpsZeroNeededStatus_t status = LpsWeighGetAutoZeroUpdateStatus();

        bool zeroRequired = false;
        if (LPS_ZERO_UPDATE_TEMPERATURE_CHANGE_ZERO_REQUIRED == status) {
            zeroRequired = true;
        }
        else if (LPS_ZERO_UPDATE_TIMER_EXPIRED_ZERO_REQUIRED == status) {
            zeroRequired = true;
        }

        if (zeroRequired) {
            LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_NOT_ZEROED] = true;
        }
    }

    if (TRUE == LpsWeighRaiseStallWarningActive()) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_RAISE_STALL] = true;
    }

    if (TRUE == LpsWeighLowerStallWarningActive()) {
        LpsSaWeighInfoTbl.InfoState[ACDInfoPopUp::PAYLOAD_LOWER_STALL] = true;
    }
}

unsigned int LpsSaWeighApp::GetSysMonotonicTime() {
    struct timespec tempTime;
    unsigned int currentTime;

    if (clock_gettime(CLOCK_MONOTONIC,&tempTime) != -1) {
        currentTime = tempTime.tv_sec;
    }
    else {
        AIS_LOG_DEBUG("FetchingcurrentTime FAILED");
        currentTime = 0;
    }
    return currentTime;
}
