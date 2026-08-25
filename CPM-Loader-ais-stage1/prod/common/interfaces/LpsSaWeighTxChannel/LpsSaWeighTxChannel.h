#include <chrono>
#include <bitset>

#include <boost/archive/archive_exception.hpp>
#include <boost/serialization/bitset.hpp>

#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#include <cat_std_types.h>

#include <boost_helpers/boost_serialization_chrono.hpp>

#include <LpsPublic.h>

#ifndef LPSSAWEIGHTXCHANNEL_H
#define LPSSAWEIGHTXCHANNEL_H

typedef enum
{
    ZEROED = 0x0394,
    NOT_ZEROED = 0x0395
} PloadSysZeroStat_t;

typedef enum
{
    CAL_ENTRY_REQUIRED = 0x001A,
    CAL_ENTRY_NOT_REQUIRED = 0x001B
} PloadSysCalWtEntryReqStat_t;

typedef enum
{
    BUCKET_PAYLOAD_NOT_AVAILABLE = 0,
    BUCKET_PAYLOAD_AVAILABLE = 1,
    BUCKET_PAYLOAD_STORED = 2,
    BUCKET_PAYLOAD_NOT_INSTALLED = 0xF4
} BktPayloadData_t;

/*
 * Info Popup Flags
 * This is an example of a portable bitset instead of the non-portable union/bitfield pattern.
 */
namespace ACDInfoPopUp {
    enum {
        PAYLOAD_TOO_HEAVY_TO_ZERO,
        PAYLOAD_NOT_ZEROED,
        PAYLOAD_CAL_ACCEPTED,
        PAYLOAD_ZERO_ACCEPTED,
        PAYLOAD_REWEIGH_LIFT_TOO_SLOW,
        PAYLOAD_REWEIGH_STOPPED_IN_RANGE,
        PAYLOAD_REWEIGH_NOT_RACKED,
        PAYLOAD_REWEIGH_SPEED_CHANGING,
        PAYLOAD_REWEIGH_INCONSISTENT,
        PAYLOAD_REWEIGH_PRESSURE_CHANGING,
        PAYLOAD_REWEIGH_EXCESSIVE_PITCH,
        PAYLOAD_REWEIGH_WARMUP_LIFT,
        PAYLOAD_RAISE_STALL,
        PAYLOAD_LOWER_STALL,
        PAYLOAD_CAL_WT_ENTRY_REQUIRED,
        NUM_BITS,
    };
    static_assert(NUM_BITS <= 32, "ACDInfoPopUp::NUM_BITS > 32");

    typedef std::bitset<NUM_BITS> type;
}

/*
 * Payload Weight Status (PID 0xD00E9B)
 */
namespace ACDWeighStatus {
    enum {
        LOWER_STALL = 7,
        RAISE_STALL,
        INSUFFICIENT_DATA,
        REWEIGH_PRESSURE_CHANGING,
        REWEIGH_INCONSISTENT,
        REWEIGH_SPEED_CHANGING,
        REWEIGH_NOT_RACKED_EXCESSIVE_PITCH,
        REWEIGH_STOPPED_IN_RANGE,
        REWEIGH_LIFT_TOO_SLOW,
        NUM_BITS,
    };
    static_assert(NUM_BITS <= 16, "ACDWeighStatus::NUM_BITS > 16");

    typedef std::bitset<NUM_BITS> type;
}

/*
 * Diag Popup Flags
 */
namespace ACDDiagPopUp {
    enum {
        TILT_RE_FREQ_ABNORMAL = 8,
        TILT_RE_VOLTAGE_BELOW_NORMAL,
        TILT_RE_VOLTAGE_ABOVE_NORMAL,

        TILT_HE_FREQ_ABNORMAL,
        TILT_HE_VOLTAGE_BELOW_NORMAL,
        TILT_HE_VOLTAGE_ABOVE_NORMAL,

        HYDRAULIC_OIL_TEMP_BAD,
        MACHINE_MODEL_NOT_SET,
        PAYLOAD_SYSTEM_NOT_INSTALLED,
        PAYLOAD_SYSTEM_OUT_OF_CAL,

        LIFT_RE_FREQ_ABNORMAL,
        LIFT_RE_VOLTAGE_BELOW_NORMAL,
        LIFT_RE_VOLTAGE_ABOVE_NORMAL,

        LIFT_HE_FREQ_ABNORMAL,
        LIFT_HE_VOLTAGE_BELOW_NORMAL,
        LIFT_HE_VOLTAGE_ABOVE_NORMAL,

        LIFT_LINK_OUT_OF_CAL,
        LIFT_LINK_FREQ_ABNORMAL,
        LIFT_LINK_VOLTAGE_BELOW_NORMAL,
        LIFT_LINK_VOLTAGE_ABOVE_NORMAL,

        TILT_LINK_OUT_OF_CAL,
        TILT_LINK_FREQ_ABNORMAL,
        TILT_LINK_VOLTAGE_BELOW_NORMAL,
        TILT_LINK_VOLTAGE_ABOVE_NORMAL,

        NUM_BITS,
    };
    static_assert(NUM_BITS <= 32, "ACDDiagPopUp::NUM_BITS > 32");

    typedef std::bitset<NUM_BITS> type;
}

/*
 * Event Popup Flags
 */
namespace ACDEventPopUp {
    enum {
        BATTERY_HIGH = 28,
        BATTERY_LOW = 29,
        PAYLOAD_LFT_NOT_SEALED = 30,
        OVERLOAD_LIMIT_EXCEEDED = 31,
        NUM_BITS,
    };
    static_assert(NUM_BITS <= 32, "ACDEventPopUp::NUM_BITS > 32");

    typedef std::bitset<NUM_BITS> type;
}

struct LpsSaLiftPosition_t {
    float angle; // Lift Angle
    float percentAngle; // Percent Lift Angle
    float cylinderLength; /* Holds cylinder length value in millimeters */
    float percentCylinderLength; // Cylinder Length percentage of full cylinder extension
    float cylinderExtension; // Cylinder extension in millimeters
    LpsStat_t status;

    LpsSaLiftPosition_t() :
        angle(0.f),
        percentAngle(0.f),
        cylinderLength(0.f),
        percentCylinderLength(0.f),
        cylinderExtension(0.f),
        status(LPS_STATUS_BAD) {}
};

struct LpsSaTiltPosition_t {
    float angle; // Tilt Angle ABC'
    float percentAngle; // Percent angle ABC' where 0% is Full Dump and 100% is Full Rack
    float cylinderLength; // Tilt Cylinder Length
    float percentCylinderLength; // Percent cylinder extension where 0% is Full Dump and 100% is Full Rack
    float cylinderExtension; // Cylinder extension in millimeters
    float bucketAngle; // Angle of the floor of the bucket/worktool w.r.t. machine.
    LpsStat_t status;

    LpsSaTiltPosition_t() :
        angle(0.f),
        percentAngle(0.f),
        cylinderLength(0.f),
        percentCylinderLength(0.f),
        cylinderExtension(0.f),
        bucketAngle(0.f),
        status(LPS_STATUS_BAD) {}
};

typedef LpsInstWtRawStat_t LpsSaPayloadRatioStatus_t;
#define LPS_SA_PAYLOAD_RATIO_STATUS_OK LPS_INST_RAW_WT_STATUS_OK
#define LPS_SA_PAYLOAD_RATIO_STATUS_UNKNOWN LPS_INST_RAW_WT_STATUS_BAD
#define LPS_SA_PAYLOAD_RATIO_STATUS_RAISE_STALL LPS_INST_RAW_WT_STATUS_RAISE_STALL
#define LPS_SA_PAYLOAD_RATIO_STATUS_LOWER_STALL LPS_INST_RAW_WT_STATUS_LOWER_STALL

struct LpsSaPayload_t {
    float payloadRatioRaw;
    float payloadRatio;
    LpsSaPayloadRatioStatus_t payloadRatioStatus;

    LpsSaPayload_t() :
        payloadRatioRaw(0.f),
        payloadRatio(0.f),
        payloadRatioStatus(LPS_SA_PAYLOAD_RATIO_STATUS_UNKNOWN) {}
};

struct LpsSaLftSealStatus_t {
    bool sealed = false; // Sealed or not sealed
    std::chrono::system_clock::time_point sealTime; // The timepoint when the seal status changed
    uint_least32_t sealId; // Seal Identifier

    LpsSaLftSealStatus_t() :
        sealed(false),
        sealTime(std::chrono::system_clock::time_point::min()),
        sealId(0) {}
};

class LpsSaWeighTxChannelStorage: public csvable
{  
public:

    enum ToaAnchorStatus_t {
        UNCHANGED,
        RESET,
        ACCEPTED,
        REJECTED
    };

    enum TiltSensorConfig_t : uint16_t {
        ROTARY_POSITION_SENSOR = 0x01C3,
        IN_CYLINDER_POSITION_SENSOR = 0x01C4
        // BUCKER_POSITIONER_SWITCH = 0x01C5, // Not supported
    };

    enum AudibleAnnunciationPriority_t : uint16_t {
        TONE_NONE = 0,
        TONE_CONFIRMATION = 2,
        TONE_ATTENTION = 6
    };

    enum TestStatus_t : uint8_t {
        INACTIVE = 0,
        TESTING,
        RECORDING,
        FAILED
    };

    typedef struct
    {
        float WeighRangeBottom; /* weigh range Bottom in % */
        float WeighRangeSize; /* weigh range Size in % */
    } LpsSaWeighRange_t;

    typedef struct
    {
        float LiftLinkDC; /* 0xF13D */
        float LiftCylPos; /* 0xF5AE */
        float LiftCylHEPres; /* 0xF47B */
        float LiftCylREPres; /* 0xF5AF */
        TiltSensorConfig_t TiltSensorConfig; /* 0xD10046 */
        float TiltCylHEPres; /* 0xF4E0 */
        float TiltCylREPres; /* 0xF47C */
        float TiltLinkDC; /* 0xF13E */
        float HydOilTemp; /* 0x0045 */
    } ProdMeasureSensorStatus_t;

    typedef struct
    {
        float LiftPosSensorDC; /* 0xF13D */
        float LiftPosSensorFullRaiseDC; /* 0x00D010FA */
        float LiftPosSensorFullLowerDC; /* 0x00D010FB */
        float TiltPosSensorDC; /* 0xF13E */
        float TiltPosSensorFullRackDC; /* 0x00D010FC */
        float TiltPosSensorFullDumpDC; /* 0x00D010FD */
    } LinkSensorCalLim_t;

    typedef struct
    {
        PloadSysZeroStat_t PloadSysZeroStat = ZEROED; /* 0x00D10822 */
        uint16_t LdrPayloadStat = 0x01B5; /* 0x00D106DF */
        bool OverloadWarningEnabled = false;/* 0x00D10ACC */
        ProdMeasureSensorStatus_t ProdMeasureSensorStatus = { 0.f, 0.f, 0.f, 0.f, ROTARY_POSITION_SENSOR, 0.f, 0.f, 0.f, 0.f };
        LinkSensorCalLim_t LinkSensorCalLim = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
        float LoaderBktPloadTgtWt = 0.f;
        float LoaderBktPloadTgtWtPer = 0.f;
        uint16_t PloadSysZeroReqStat = 0x001B; /* 0xD10933 */
        PloadSysCalWtEntryReqStat_t PloadSysCalWtEntryReqStat = CAL_ENTRY_NOT_REQUIRED;/* 0xD109A5 */
        float LastPloadWt = 0.f;
        ACDWeighStatus::type ProdMeasureWeighStatus;
        BktPayloadData_t BktPayloadData = BUCKET_PAYLOAD_NOT_AVAILABLE; /* $F2C6 */
        int16_t QR_HydOilTempMin_C = 0; /* $45 */
        int16_t QR_LiftCylVelMin_mm_sec = 0; /* $D0 $1961 */
        int16_t QR_LiftCylVelMax_mm_sec = 0; /* $D0 $1961 */

        // Rear (EEF - Engine End Frame)
        int16_t MachineRearLateralAcceleration = 0; /* $D019A1 */
        int16_t MachineRearLongitudinalAcceleration = 0; /* $D019A3 */
        int16_t MachineRearVerticalAcceleration = 0; /* $D019A5 */

        // Rear (EEF - Engine End Frame)
        int16_t MachinePitch = 0; /* $F58B */
        int16_t MachineSlope = 0; /* $F533 */

        // Rear (EEF - Engine End Frame)
        int16_t MachineRearRoll = 0; /* $D01AFF */
        int16_t MachineRearSideSlope = 0; /* $D01FC9 */

        // Ambiguously defined, populating as Rear (EEF - Engine End Frame)
        int16_t MachineRoll = 0; /* $D00A64 */
        int16_t MachineSideSlope = 0; /* $D00409 */

        float TipoffPitchCalOffset = 0.f;

        bool HydOilTempEnabled = true;

        bool AudibleWeightEnabled = false; /* D10F96 - Payload Weight Calculation Acceptable Audible Alarm Configuration */

        AudibleAnnunciationPriority_t AudibleWeightCommand = TONE_NONE; /* $D02923 - Desired Audible Annunciation Priority */
    } WeighPidData_t;

    std::chrono::steady_clock::time_point timePoint;

    LpsWeighBktDigStat_t DigStat; // Machine Dig Status
    LpsWeighCalStatus_t CalStat; // Lps System Calibration Status
    LpsWeighBktDumpStat_t DumpStat; // Machine Dump Status
	float BestBktWtInTonnes; // Best Available Bucket Weight in metric tonne
	uint16_t WarmupLiftsRequired; // Number of warmup lifts that are currently required
	unsigned short int PayloadCalcMeth; // Payload Calculation method
	bool bktWtLatchedFlag;	// Indicates if we have a latched bucket weight
	bool latchConditionsMet; // Indicates if latch conditions are ok
    bool zeroAvailable; // true if the zero operation is available

	LpsSaPayload_t Payload; // Payload Information
	LpsSaLftSealStatus_t LftSealStatus; // Legal For Trade Seal Status
	bool flashEnabled; // Flash Enable Status

	// Sensor Identifiers for Legal For Trade
	std::string LiftPositionSensorId;
	std::string TiltPositionSensorId;
	std::string LiftHeadEndPressureSensorId;
	std::string LiftRodEndPressureSensorId;
	std::string HydraulicOilTemperatureSensorId;
        std::string ImuSensorId;
        std::string ImplementSerialNum;

	// Active Work Tool Identifier
	std::string WorkToolId;

    LpsSaLiftPosition_t LiftPosition; // Lift Position
    LpsFloatIO_t LiftCylVel; // Lift Cyclinder Velocity
	LpsSaTiltPosition_t TiltPosition; // Tilt Position

	LpsSaWeighRange_t WeighRange; /* weigh range */
	LpsWeighRangeIndicator_t Indicator;
	float CalWt;
	bool LiftStalled;

	LpsFloatIO_t LiftValveCommand;
	LpsFloatIO_t TiltValveCommand;

	ACDEventPopUp::type EventState;
	ACDDiagPopUp::type DiagState;
	ACDInfoPopUp::type InfoState;
    bool ShowExclamationPoint;
    bool ExcessivePitch;
    bool BucketFullyRacked;

	float ZeroWeight; // The zero weight.
	float SimpleCalAdjust; // The Simple Cal Adjustment Factor.

	int16_t EngineSpeedRPM;

	WeighPidData_t PidData; // Data needs to be sent to AutonomyconditionDiagnostics App

	bool CAN11MessageTimeoutFlag;

	float ToaAnchoredZeroOffset;
	float ToaAnchoredFactor;
	ToaAnchorStatus_t ToaAnchorStatus;

	bool PayloadCalInProgress;
	TestStatus_t TestStatus;

	LpsSaWeighTxChannelStorage() :
	    timePoint(std::chrono::steady_clock::now()),
	    DigStat(LPS_WEIGHT_BKT_DIG_STATE_UNKNOWN),
	    CalStat(LPS_WEIGH_SYSTEM_UNCALIBRATED),
	    DumpStat(LPS_WEIGHT_BKT_DUMP_STATE_UNKNOWN),
	    BestBktWtInTonnes(0.f),
	    WarmupLiftsRequired(0),
	    PayloadCalcMeth(0),
        bktWtLatchedFlag(false),
        latchConditionsMet(false),
        zeroAvailable(false),
        Payload(),
        LftSealStatus(),
        flashEnabled(true),
        LiftPositionSensorId(),
        TiltPositionSensorId(),
        LiftHeadEndPressureSensorId(),
        LiftRodEndPressureSensorId(),
        HydraulicOilTemperatureSensorId(),
        ImuSensorId(),
        ImplementSerialNum(),
        WorkToolId(),
        LiftPosition(),
        LiftCylVel{ LPS_STATUS_BAD, 0.f },
        TiltPosition(),
        WeighRange{ 0.f, 0.f },
        Indicator(LPS_BELOW_WEIGH_RANGE),
        CalWt(0.f),
        LiftStalled(false),
        LiftValveCommand{ LPS_STATUS_BAD, 0.f },
        TiltValveCommand{ LPS_STATUS_BAD, 0.f },
        EventState(),
        DiagState(),
        InfoState(),
        ShowExclamationPoint(false),
        ExcessivePitch(false),
        BucketFullyRacked(false),
        ZeroWeight(0.f),
        SimpleCalAdjust(0.f),
        EngineSpeedRPM(0),
        PidData(),
        CAN11MessageTimeoutFlag(false),
        ToaAnchoredZeroOffset(0.f),
        ToaAnchoredFactor(1.f),
        ToaAnchorStatus(UNCHANGED),
        PayloadCalInProgress(false),
        TestStatus(TestStatus_t::INACTIVE) {}

    inline void setTimePoint(const std::chrono::steady_clock::time_point& tp = std::chrono::steady_clock::now()) {
        timePoint = tp;
    }

    template <class Archive>
    void serialize(Archive &ar, unsigned int version) {
        if (version < 20) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & timePoint;

        ar & DigStat;
      	ar & CalStat;
      	ar & DumpStat;
      	ar & BestBktWtInTonnes;
      	ar & WarmupLiftsRequired;
        ar & PayloadCalcMeth;
        ar & bktWtLatchedFlag;
        ar & latchConditionsMet;
        ar & zeroAvailable;

        ar & Payload.payloadRatioRaw;
        ar & Payload.payloadRatio;
        ar & Payload.payloadRatioStatus;

        ar & LftSealStatus.sealed;
        ar & flashEnabled;
        ar & LftSealStatus.sealTime;
        ar & LftSealStatus.sealId;

        ar & LiftPositionSensorId;
        ar & TiltPositionSensorId;
        ar & LiftHeadEndPressureSensorId;
        ar & LiftRodEndPressureSensorId;
        ar & HydraulicOilTemperatureSensorId;
        ar & ImuSensorId;
        ar & WorkToolId;

        ar & LiftPosition.angle;
        ar & LiftPosition.percentAngle;
        ar & LiftPosition.cylinderLength;
        ar & LiftPosition.percentCylinderLength;
        ar & LiftPosition.cylinderExtension;
        ar & LiftPosition.status;

      	ar & LiftCylVel.Stat;
      	ar & LiftCylVel.Val;

      	ar & TiltPosition.angle;
        ar & TiltPosition.percentAngle;
        ar & TiltPosition.cylinderLength;
        ar & TiltPosition.percentCylinderLength;
        ar & TiltPosition.cylinderExtension;
        ar & TiltPosition.bucketAngle;
      	ar & TiltPosition.status;

      	ar & WeighRange.WeighRangeBottom;
      	ar & WeighRange.WeighRangeSize;

        ar & Indicator;
        ar & CalWt;
        ar & LiftStalled;

        ar & LiftValveCommand.Val;
        ar & TiltValveCommand.Val;
        ar & LiftValveCommand.Stat;
        ar & TiltValveCommand.Stat;

        ar & EventState;
        ar & DiagState;
        ar & InfoState;
        ar & ShowExclamationPoint;
        ar & ZeroWeight;
        ar & SimpleCalAdjust;

        ar & EngineSpeedRPM;

        ar & PidData.PloadSysZeroStat;
        ar & PidData.LdrPayloadStat;
        ar & PidData.OverloadWarningEnabled;
        ar & PidData.ProdMeasureSensorStatus.LiftCylHEPres;
        ar & PidData.ProdMeasureSensorStatus.LiftCylPos;
        ar & PidData.ProdMeasureSensorStatus.LiftCylREPres;
        ar & PidData.ProdMeasureSensorStatus.LiftLinkDC;
        ar & PidData.ProdMeasureSensorStatus.TiltLinkDC;
        ar & PidData.ProdMeasureSensorStatus.HydOilTemp;
        ar & PidData.ProdMeasureSensorStatus.TiltCylHEPres;
        ar & PidData.ProdMeasureSensorStatus.TiltCylREPres;
        ar & PidData.LinkSensorCalLim.LiftPosSensorDC;
        ar & PidData.LinkSensorCalLim.LiftPosSensorFullLowerDC;
        ar & PidData.LinkSensorCalLim.LiftPosSensorFullRaiseDC;
        ar & PidData.LinkSensorCalLim.TiltPosSensorDC;
        ar & PidData.LinkSensorCalLim.TiltPosSensorFullDumpDC;
        ar & PidData.LinkSensorCalLim.TiltPosSensorFullRackDC;
        ar & PidData.LoaderBktPloadTgtWt;
        ar & PidData.LoaderBktPloadTgtWtPer;
        ar & PidData.PloadSysZeroReqStat;
        ar & PidData.PloadSysCalWtEntryReqStat;
        ar & PidData.LastPloadWt;
        ar & PidData.ProdMeasureWeighStatus;
        ar & PidData.BktPayloadData;
        ar & PidData.QR_HydOilTempMin_C;
        ar & PidData.QR_LiftCylVelMin_mm_sec;
        ar & PidData.QR_LiftCylVelMax_mm_sec;
        ar & PidData.MachineRearLateralAcceleration;
        ar & PidData.MachineRearLongitudinalAcceleration;
        ar & PidData.MachineRearVerticalAcceleration;
        ar & PidData.TipoffPitchCalOffset;

        ar & PidData.HydOilTempEnabled;

        ar & CAN11MessageTimeoutFlag;

        ar & ToaAnchoredZeroOffset;
        ar & ToaAnchoredFactor;
        ar & ToaAnchorStatus;

        ar & PidData.ProdMeasureSensorStatus.TiltSensorConfig;

        ar & PayloadCalInProgress;

        ar & PidData.MachinePitch;
        ar & PidData.MachineSlope;
        ar & PidData.MachineRearRoll;
        ar & PidData.MachineRearSideSlope;
        ar & PidData.MachineRoll;
        ar & PidData.MachineSideSlope;

        ar & PidData.AudibleWeightEnabled;
        ar & PidData.AudibleWeightCommand;

        ar & ExcessivePitch;
        ar & BucketFullyRacked;

        ar & TestStatus;

        ar & ImplementSerialNum;
    }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    };

private:
    /* Add Fields Here */
};  

typedef Datum<LpsSaWeighTxChannelStorage> LpsSaWeighTxChannel;

BOOST_CLASS_VERSION(LpsSaWeighTxChannelStorage, 20);
#endif

