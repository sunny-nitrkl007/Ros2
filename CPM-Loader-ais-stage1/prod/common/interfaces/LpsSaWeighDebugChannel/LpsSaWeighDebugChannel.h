#include <std_t.h>

#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#include <string.h>

#ifndef __LPS_COMMON_TYPE_DEF_H__
#include <LpsCommonTypeDef.h>
#endif

#ifndef __CPM_FILTER_H__
#include <cpm_filter.h>
#endif

#ifndef __LPS_COMMON_STRUCTURES_H__
#include <LpsCommonStructures.h>
#endif

#ifndef __LPS_PUBLIC_H__
#include <LpsPublic.h>
#endif

#ifndef __LPS_PRIVATE_H__
#include <LpsPrivate.h>
#endif

#ifndef __LPS_SA_INCLUDES_H__
#include <LpsSaWeighApp/LpsSaIncludes.h>
#endif

#ifndef _TIPOFFASSSIST_H_
#include <LpsSaWeighApp/adv/TipoffAssist.h>
#endif

#ifndef _LpsSaWeighDebugChannel_h_
#define _LpsSaWeighDebugChannel_h_
class LpsSaWeighDebugChannelStorage: public csvable
{  
public:  
    LpsSaWeighDebugChannelStorage() :
        m_LpsWrk(),
        m_TipoffInputs(),
        m_TipoffOutputs() {
        m_LpsWrk.DigDetectData.DigDetected = false;
        m_LpsWrk.DigDetectData.WtStabilized = false;
        m_LpsWrk.WrwTbl.WkTbl.BeenBelowRange = false;
        m_LpsWrk.OverloadWarnTbl.DigReverseFlag = false;
        m_LpsWrk.OverloadWarnTbl.InOverloadRange = false;
        m_LpsWrk.OverloadWarnTbl.Level = LPS_WEIGH_OVERLOAD_LEVEL_NONE;
    }

    template <class Archive>
    void serialize(Archive &ar, unsigned int version)
    {
        /* Archive Fields Here */
        if (version < 2) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        /* Lift Cylinder Head End Pressure */
        ar & DebugLiftCylHePeriod;
        ar & DebugLiftCylHeWidth;
        ar & DebugLiftCylHeDc;
        ar & DebugLiftCylHePress;
        ar & DebugLiftCylHePressStatus;

        /* Lift Cylinder Rod End Pressure */
        ar & DebugLiftCylRePeriod;
        ar & DebugLiftCylReWidth;
        ar & DebugLiftCylReDc;
        ar & DebugLiftCylRePress;
        ar & DebugLiftCylRePressStatus;

        /* Tilt Cylinder Head End Pressure */
        ar & DebugTiltCylHePeriod;
        ar & DebugTiltCylHeWidth;
        ar & DebugTiltCylHeDc;
        ar & DebugTiltCylHePress;
        ar & DebugTiltCylHePressStatus;

        /* Tilt Cylinder Rod End Pressure */
        ar & DebugTiltCylRePeriod;
        ar & DebugTiltCylReWidth;
        ar & DebugTiltCylReDc;
        ar & DebugTiltCylRePress;
        ar & DebugTiltCylRePressStatus;

        /* Lift Cylinder */
        ar & DebugLiftCylPeriod;
        ar & DebugLiftCylWidth;
        ar & DebugLiftCylDc;
        ar & DebugLiftCylRawExt;
        ar & DebugLiftCylFiltExt;
        ar & DebugLiftCylNormLen;
        ar & DebugLiftCylVel;
        ar & DebugLiftAngle;
        ar & DebugLiftAngVel;
        ar & DebugLiftPositionStatus;

        /* Tilt Cylinder */
        ar & DebugTiltCylPeriod;
        ar & DebugTiltCylWidth;
        ar & DebugTiltCylDc;
        ar & DebugTiltCylRawLen;
        ar & DebugTiltCylNormLen;
        ar & DebugTiltCylVel;
        ar & DebugTiltAngle;
        ar & DebugTiltAngleABC;
        ar & DebugTiltPositionStatus;

        /* LpsUpdtTbl_t m_weighUpdtTbl */
        ar & DebugBucketAngle;
        ar & DebugBucketAngleStatus;
        ar & DebugHydOilTemp;
        ar & DebugHydOilTempStatus;
        ar & DebugRequestedGear;
        ar & DebugRequestedGearStatus;
        ar & DebugWeighUpdtLoaderBktPayldTrgtWt;
        ar & DebugWeighUpdtClockKeyonSec;
        ar & DebugWeighUpdtPassCount;

        /* LpsWrkTbl_t m_LpsWrk */
        ar & m_LpsWrk.BestBktWt.PayloadCalcMeth;
        ar & m_LpsWrk.BestBktWt.Wt;
        ar & m_LpsWrk.BestBktWt.TipoffLiveWeightInUse;
        ar & m_LpsWrk.CylFlowConst.LiftHeFlowConst;
        ar & m_LpsWrk.CylFlowConst.LiftReToHeFlowRatio;
        ar & m_LpsWrk.DigDetectData.DigDetected;
        ar & m_LpsWrk.DigDetectData.DigDuration;
        ar & m_LpsWrk.DigDetectData.DigStarted;
        ar & m_LpsWrk.DigDetectData.DigState;
        ar & m_LpsWrk.DigDetectData.WtStabilized;
        ar & m_LpsWrk.DumpWtChangeData.DumpCumulativeWtChange;
        ar & m_LpsWrk.InstWtDer;
        ar & m_LpsWrk.InstWtFilt;
        ar & m_LpsWrk.InstWtLlwFilt;
        ar & m_LpsWrk.InstWtRaw.Stat;
        ar & m_LpsWrk.InstWtRaw.Val;
        ar & m_LpsWrk.InstWtRawPreTiltComp;
        ar & m_LpsWrk.InstWtRawPostTiltComp;
        ar & m_LpsWrk.LiftCylPressure;
        ar & m_LpsWrk.LiftCylPressureV0;
        ar & m_LpsWrk.LiftCylPressureIMUAdjusted;
        ar & m_LpsWrk.LiveWeighTbl.Stat;
        ar & m_LpsWrk.LiveWeighTbl.Wt;
        ar & m_LpsWrk.LiveWeighTbl.UseFastFilter;
        ar & m_LpsWrk.LowLiftWt.Wt;
        ar & m_LpsWrk.LowLiftWt.Confidence;
        ar & m_LpsWrk.LowLiftWt.ConfidenceEst;
        ar & m_LpsWrk.LowLiftWt.ConfidenceTimer;
        ar & m_LpsWrk.LowLiftWt.LastInput;
        ar & m_LpsWrk.LowLiftWt.MeanEst;
        ar & m_LpsWrk.LowLiftWt.MeanEstComp;
        ar & m_LpsWrk.LowLiftWt.Stat;
        ar & m_LpsWrk.LowLiftWt.StdevEst;
        ar & m_LpsWrk.LowLiftWt.VarianceEst;
        ar & m_LpsWrk.DumpStateStatus;
        ar & m_LpsWrk.LpsStallDetect.liftStalled;
        ar & m_LpsWrk.LinkageMovement.lift;
        ar & m_LpsWrk.LinkageMovement.tilt;
        ar & m_LpsWrk.WrwTbl.WkTbl.result.weightAveRaw;
        ar & m_LpsWrk.WrwTbl.WkTbl.result.weightAve;
        ar & m_LpsWrk.WrwTbl.OpTbl.Wt;
        ar & m_LpsWrk.WrwTbl.OpTbl.Warning;
        ar & m_LpsWrk.WrwTbl.OpTbl.Indicator;
        ar & m_LpsWrk.WrwTbl.OpTbl.Status;
        ar & m_LpsWrk.WrwTbl.WkTbl.result.weightAveRawIMUComp;

        // Have to shuffle the bitfield into an unsigned_16 for serialization
        // On the way out, m_LpsWrk.WrwTbl.ReweighWarn will not be correct.
        memcpy(&reweighWarn, &m_LpsWrk.WrwTbl.ReweighWarn, sizeof(reweighWarn));
        ar & reweighWarn;

        ar & m_LpsWrk.WrwTbl.WkTbl.BeenBelowRange;
        ar & m_LpsWrk.ZeroAdjTbl.ZeroAdjStatus;
        ar & m_LpsWrk.OverloadWarnTbl.BucketLoadFactor;
        ar & m_LpsWrk.OverloadWarnTbl.DigReverseFlag;
        ar & m_LpsWrk.OverloadWarnTbl.InOverloadRange;
        ar & m_LpsWrk.OverloadWarnTbl.Level;
        ar & m_LpsWrk.OverloadWarnTbl.PrevDigState;
        ar & m_LpsWrk.LiftPositionFilt.length;
        ar & m_LpsWrk.LiftPositionFilt.velocity;
        ar & m_LpsWrk.LiftPositionFilt.angle;

        /* tipoff assists channels */
        ar & m_LpsWrk.BestBktWt.TipoffLiveWeightInUse;
        ar & m_TipoffInputs.bucket_current_weight;
        ar & m_TipoffInputs.bucket_current_weight_accuracy;
        ar & m_TipoffInputs.eef_imu_accelX;
        ar & m_TipoffInputs.eef_imu_accelY;
        ar & m_TipoffInputs.eef_imu_accelZ;
        ar & m_TipoffInputs.input_status.data;
        ar & m_TipoffInputs.invalidate_outputs;
        ar & m_TipoffInputs.lift_angle;
        ar & m_TipoffInputs.lift_he_pressure;
        ar & m_TipoffInputs.lift_he_pressure;
        ar & m_TipoffInputs.lift_re_pressure;
        ar & m_TipoffInputs.lift_valve_cmd;
        ar & m_TipoffInputs.steering_angle;
        ar & m_TipoffInputs.tilt_extension;
        ar & m_TipoffInputs.tilt_he_pressure;
        ar & m_TipoffInputs.tilt_re_pressure;
        ar & m_TipoffInputs.tilt_valve_cmd;
        ar & m_TipoffInputs.tipoff_mode;
        ar & m_TipoffInputs.tool_mass;
        ar & m_TipoffInputs.truck_start_weight;
        ar & m_TipoffInputs.truck_target_wt;
        ar & m_TipoffInputs.zero_offset;
        ar & m_TipoffInputs.simple_cal_factor;
        ar & m_TipoffInputs.bucket_angle;
        ar & m_TipoffInputs.unlatch_trigger;
        ar & m_TipoffInputs.anchor_factor;
        ar & m_TipoffInputs.anchor_zero_offset;
        ar & m_TipoffInputs.lift_norm_angle;
        ar & m_TipoffInputs.lift_norm_length;
        ar & m_TipoffInputs.tilt_norm_angle;
        ar & m_TipoffInputs.tilt_norm_length;
        ar & m_TipoffInputs.pass_count;
        ar & m_TipoffInputs.friction_mu;
        ar & m_TipoffInputs.friction_offset;

        ar & m_TipoffOutputs.arbitrated_payload_norm_error_out;
        ar & m_TipoffOutputs.current_weight_norm_error_out;
        ar & m_TipoffOutputs.error_code_out;
        ar & m_TipoffOutputs.payload_norm_stdev_out;
        ar & m_TipoffOutputs.payload_send_to_CPM;
        ar & m_TipoffOutputs.payload_status_send_to_CPM;
        ar & m_TipoffOutputs.pcs_weight_accuracy_out;
        ar & m_TipoffOutputs.spill_rate_out;
        ar & m_TipoffOutputs.tilt_pressure_out;
        ar & m_TipoffOutputs.tilt_sensitivity_out;
        ar & m_TipoffOutputs.weigh_status_out;
        ar & m_TipoffOutputs.bucket_payload_target;
        ar & m_TipoffOutputs.unsecured_PFW_status;
        ar & m_TipoffOutputs.unsecured_payload_lower_bound_norm;
        ar & m_TipoffOutputs.unsecured_payload_upper_bound_norm;
        ar & m_TipoffOutputs.min_secure_bucket_angle;

        // Chassis IMU
        ar & chassisImuGravityX;
        ar & chassisImuGravityY;
        ar & chassisImuGravityZ;
        ar & chassisImuVelocityX;
        ar & chassisImuVelocityY;
        ar & chassisImuVelocityZ;
        ar & chassisImuBiasMagJerk;
        ar & chassisImuBiasMagAngAccel;
        ar & chassisImuBiasMaxAngVel;
        ar & chassisImuBiasAngVelX;
        ar & chassisImuBiasAngVelY;
        ar & chassisImuBiasAngVelZ;
        ar & chassisImuPitch;
        ar & chassisImuRoll;
        ar & chassisImuSensorLinAccStatus;
        ar & chassisImuSensorAngVelStatus;
        ar & chassisImuLinAccX;
        ar & chassisImuLinAccY;
        ar & chassisImuLinAccZ;
        ar & chassisImuAngVelX;
        ar & chassisImuAngVelY;
        ar & chassisImuAngVelZ;
    }

    void toCsv(CsvOutStream& out) const
    {
        //out("Minus One JobMgr Response Code", MinusOneResp);
    }

    LpsWrkTbl_t m_LpsWrk;
    TipoffAssistInputs m_TipoffInputs;
    TipoffAssistOutputs m_TipoffOutputs;

    /* bitwise to unsigned_16 */
    unsigned_16 reweighWarn = 0;

    /* Lift Cylinder Head End Pressure */
    int DebugLiftCylHePeriod = 0;
    int DebugLiftCylHeWidth = 0;
    float DebugLiftCylHeDc = 0.f;
    float DebugLiftCylHePress = 0.f;
    LpsStat_t DebugLiftCylHePressStatus = LPS_STATUS_BAD;

    /* Lift Cylinder Rod End Pressure */
    int DebugLiftCylRePeriod = 0;
    int DebugLiftCylReWidth = 0;
    float DebugLiftCylReDc = 0.f;
    float DebugLiftCylRePress = 0.f;
    LpsStat_t DebugLiftCylRePressStatus = LPS_STATUS_BAD;

    /* Tilt Cylinder Head End Pressure */
    int DebugTiltCylHePeriod = 0;
    int DebugTiltCylHeWidth = 0;
    float DebugTiltCylHeDc = 0.f;
    float DebugTiltCylHePress = 0.f;
    LpsStat_t DebugTiltCylHePressStatus = LPS_STATUS_BAD;

    /* Tilt Cylinder Rod End Pressure */
    int DebugTiltCylRePeriod = 0;
    int DebugTiltCylReWidth = 0;
    float DebugTiltCylReDc = 0.f;
    float DebugTiltCylRePress = 0.f;
    LpsStat_t DebugTiltCylRePressStatus = LPS_STATUS_BAD;

    /* Lift Cylinder */
    int DebugLiftCylPeriod = 0;
    int DebugLiftCylWidth = 0;
    float DebugLiftCylDc = 0.f;
    float DebugLiftCylRawExt = 0.f;
    float DebugLiftCylFiltExt = 0.f;
    float DebugLiftCylNormLen = 0.f;
    float DebugLiftCylVel = 0.f;
    float DebugLiftAngle = 0.f;
    float DebugLiftAngVel = 0.f;
    LpsStat_t DebugLiftPositionStatus = LPS_STATUS_BAD;

    /* Tilt Cylinder */
    int DebugTiltCylPeriod = 0;
    int DebugTiltCylWidth = 0;
    float DebugTiltCylDc = 0.f;
    float DebugTiltCylRawLen = 0.f;
    float DebugTiltCylNormLen = 0.f;
    float DebugTiltCylVel = 0.f;
    float DebugTiltAngle = 0.f;
    float DebugTiltAngleABC = 0.f;
    LpsStat_t DebugTiltPositionStatus = LPS_STATUS_BAD;

    float DebugBucketAngle = 0.f;
    LpsStat_t DebugBucketAngleStatus = LPS_STATUS_BAD;
    float DebugHydOilTemp = 0.f;
    LpsHydOilTempSensorStat_t DebugHydOilTempStatus = LPS_HYD_OIL_TEMP_STATUS_BAD;
    uint16_t DebugRequestedGear = 0;
    LpsStat_t DebugRequestedGearStatus = LPS_STATUS_BAD;
    float DebugWeighUpdtLoaderBktPayldTrgtWt = 0.f;
    uint32_t DebugWeighUpdtClockKeyonSec = 0;
    uint32_t DebugWeighUpdtPassCount = 0;

    // Chassis IMU
    float chassisImuGravityX = 0.f;
    float chassisImuGravityY = 0.f;
    float chassisImuGravityZ = 0.f;
    float chassisImuVelocityX = 0.f;
    float chassisImuVelocityY = 0.f;
    float chassisImuVelocityZ = 0.f;
    float chassisImuBiasMagJerk = 0.f;
    float chassisImuBiasMagAngAccel = 0.f;
    float chassisImuBiasMaxAngVel = 0.f;
    float chassisImuBiasAngVelX = 0.f;
    float chassisImuBiasAngVelY = 0.f;
    float chassisImuBiasAngVelZ = 0.f;
    float chassisImuPitch = 0.f;
    float chassisImuRoll = 0.f;
    uint8_t chassisImuSensorLinAccStatus = 0;
    uint8_t chassisImuSensorAngVelStatus = 0;
    float chassisImuLinAccX = 0.f;
    float chassisImuLinAccY = 0.f;
    float chassisImuLinAccZ = 0.f;
    float chassisImuAngVelX = 0.f;
    float chassisImuAngVelY = 0.f;
    float chassisImuAngVelZ = 0.f;

private:
    /* Add Fields Here */
};  

typedef Datum<LpsSaWeighDebugChannelStorage> LpsSaWeighDebugChannel;

BOOST_CLASS_VERSION(LpsSaWeighDebugChannelStorage, 2);
#endif

