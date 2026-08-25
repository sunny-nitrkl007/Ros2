#include <cstdint>
#include <chrono>

#include <boost_helpers/boost_serialization_chrono.hpp>
#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#include <time.h>
#include <deque>
#include <boost/serialization/deque.hpp>

#include <LpsPtPublic.h>

#ifndef _LpsSaJobMgrTxChannel_h_
#define _LpsSaJobMgrTxChannel_h_

/* Tip Off Trigger Type */
typedef enum
{
	LPS_SA_JOB_MGR_TIP_OFF_TRIGGER_AUTO = TIP_OFF_TRIGGER_AUTO,
	LPS_SA_JOB_MGR_TIP_OFF_TRIGGER_MANUAL = TIP_OFF_TRIGGER_MANUAL,
	LPS_SA_JOB_MGR_TIP_OFF_TRIGGER_DISABLED = TIP_OFF_TRIGGER_DISABLED
}LpsSaTipOffTriggerType_t;

typedef enum
{
	LPS_SA_JOB_MGR_TIP_OFF_PILE_ENABLE = TIP_OFF_MODE_PILE, /* To highlight Pile tip off button on display */
	LPS_SA_JOB_MGR_TIP_OFF_TRUCK_ENABLE = TIP_OFF_MODE_TRUCK, /* To highlight Truck tip off button on display */
	LPS_SA_JOB_MGR_TIP_OFF_UNAVAILABLE = 2, /* Tip off buttons (Pile/Truck) not available to operator on display,
	                                       Tip off buttons will be replaced by Zero button on display */
} LpsSaJobMgrTipOffState_t;

typedef enum
{
    LPS_SA_JOB_MGR_MAN_TIP_OFF_AVAILABLE = 0, /* Status to show manual tip off button on display */
    LPS_SA_JOB_MGR_MAN_TIP_OFF_UNAVAILABLE, /* Status to no show manual tip off button on display */
    LPS_SA_JOB_MGR_MAN_TIP_OFF_ACTIVE,/* Status to show active status of manual tip off button on display */
    LPS_SA_JOB_MGR_MAN_TIP_OFF_INACTIVE,/* Status to show inactive status of manual tip off button on display */
}LpsSaJobMgrManualTipOffState_t;

// $D10748  Payload Operating Mode
typedef enum
{
	LPS_SA_JOB_MGR_UNCALIBRATED = 0x0346,
	LPS_SA_JOB_MGR_WEIGH_MODE = 0x0009,
	//LPS_SA_JOB_MGR_ADJUST_CAL_MODE = 0x034F, // Not for new design
	LPS_SA_JOB_MGR_STANDBY_MODE = 0x00AF,
	LPS_SA_JOB_MGR_EXCESS_MODE = 0x034E
} LpsSaJobMgrOperationMode_t;

typedef enum
{
	LPS_SA_JOB_MGR_STANDBY_ACTIVATED = 0,
	LPS_SA_JOB_MGR_STANDBY_DEACTIVATED
} LpsSaJobMgrStandbyState_t;

typedef enum
{
	LPS_SA_JOB_MGR_CLEAR_BTN_ENABLED = 0,
	LPS_SA_JOB_MGR_MINUS_ONE_BTN_ENABLED
} LpsSaJobMgrClearMinusOneEnableStat_t;

typedef struct {
    float val;
    bool isOk;
} DispBestBktWt_t;

typedef struct
{
	std::string	timeStamp; // contains sec,min,hrs,mm,dd,yy
	float truckWt;
	float zeroedTruckWt;
	
	template <class Archive>
	void serialize(Archive& ar, unsigned int version)
	{
		ar & timeStamp;
		ar & truckWt;
		ar & zeroedTruckWt;
	}
} SimpleCalData_t;

enum class ReqPloadCtrlSysStat_t: uint16_t
{
   NONE = 0x003A,
   CLEAR = 0x0227,
   REWEIGH = 0x02FB,
   ZERO =0x035E,
   STORE = 0x035F,
   TIPOFFTOGGLE = 0x059E,
   MINUS_ONE = 0x0630
};

enum class ReqPloadOpMode_t: uint16_t
{
   NORMAL = 0x0009,
   STANDBY = 0x00AF,
   NOT_CALIBRATED = 0x0346,
   EXCESS =0x034E,
   TRUCK_WEIGHT_ADJ = 0x034F
};

typedef enum
{
   SOUND_HORN      = 0x031A,
   NOT_SOUND_HORN  = 0x031B
}StorePloadHornStat_t;

class LpsSaJobMgrTxChannelStorage: public csvable
{
public:
    LpsSaJobMgrTxChannelStorage() :
        timePoint(std::chrono::steady_clock::now()),
        taskNumber(1),
        passCount(0),
        truckWeight(0.f),
        truckWeightAccuracy(LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_NONE),
        TruckStartWeight(0.f),
        remainingWeight(0.f),
        totalWeight(0.f),
        totalWeightAccuracy(LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_NONE),
        subtotalCount(0),
        ticketId(),
        materialId(0),
        materialName(),
        materialDensity(0.f),
        truckId(0),
        truckName(),
        truckTargetWeight(0.f),
        truckListEnabled(true),
        materialListEnabled(true),
        tag1Enabled(false), customListName1(), tag1(),
        tag2Enabled(false), customListName2(), tag2(),
        tag3Enabled(false), customListName3(), tag3(),
        tag4Enabled(false), customListName4(), tag4(),
        ManualTipOffState(LpsSaJobMgrManualTipOffState_t::LPS_SA_JOB_MGR_MAN_TIP_OFF_UNAVAILABLE),
        TipOffTriggerType(LpsSaTipOffTriggerType_t::LPS_SA_JOB_MGR_TIP_OFF_TRIGGER_DISABLED),
        TipOffState(LpsSaJobMgrTipOffState_t::LPS_SA_JOB_MGR_TIP_OFF_UNAVAILABLE),
        OperationMode(LpsSaJobMgrOperationMode_t::LPS_SA_JOB_MGR_UNCALIBRATED),
        StandbyState(LpsSaJobMgrStandbyState_t::LPS_SA_JOB_MGR_STANDBY_DEACTIVATED),
        ClearMinusOneEnableStat(LpsSaJobMgrClearMinusOneEnableStat_t::LPS_SA_JOB_MGR_MINUS_ONE_BTN_ENABLED),
        DispBestBktWt{ 0.f, false },
        TipOffStateCfg(LpsSaJobMgrTipOffState_t::LPS_SA_JOB_MGR_TIP_OFF_UNAVAILABLE),
        simpleCalData(),
        storeCount(0),
        storeRejected(false),
        ReqPloadCtrlSysStat(ReqPloadCtrlSysStat_t::NONE),
        HornStoreState(StorePloadHornStat_t::NOT_SOUND_HORN),
        AutoStorePassCount(0),
        AutoTruckIdEnabled(false),
        AutoMaterialIdEnabled(false),
        manualAddEnabled(false),
        multiTaskEnabled(false),
        multiTaskCount(0),
        TipoffActive(false),
        tipoffAssistActive(false),
        tipoffAssistActiveEid(0),
        manualAddAvailable(false),
        splitModeEnabled(false),
        lftDisabled(false),
        targetType(0),
        stepNumber(1),
        iconType(0),
		targetPasses(0) {}

    /* SCS Job Manager App Interfaces */
    std::chrono::steady_clock::time_point timePoint;

    uint8_t taskNumber;
    uint16_t passCount;
    float truckWeight;
    LpsWeighBktWtAccuracy_t truckWeightAccuracy;
    float TruckStartWeight;
    float remainingWeight;
    float totalWeight;
    LpsWeighBktWtAccuracy_t totalWeightAccuracy;
    uint32_t subtotalCount;
    std::string ticketId;

    uint32_t materialId;
    std::string materialName;
    float materialDensity;

    uint32_t truckId;
    std::string truckName;
    float truckTargetWeight;

    bool truckListEnabled;
    bool materialListEnabled;

    bool tag1Enabled;
    std::string customListName1;
    std::string tag1;
    bool tag2Enabled;
    std::string customListName2;
    std::string tag2;
    bool tag3Enabled;
    std::string customListName3;
    std::string tag3;
    bool tag4Enabled;
    std::string customListName4;
    std::string tag4;

    LpsSaJobMgrManualTipOffState_t ManualTipOffState; /* Status variable to indicate manual tip off state on display */
    LpsSaTipOffTriggerType_t TipOffTriggerType;
    LpsSaJobMgrTipOffState_t TipOffState; /* Status variable to indicates the tip off button state on display */
    LpsSaJobMgrOperationMode_t OperationMode;
    LpsSaJobMgrStandbyState_t StandbyState;
    LpsSaJobMgrClearMinusOneEnableStat_t ClearMinusOneEnableStat;
    DispBestBktWt_t DispBestBktWt;
    LpsSaJobMgrTipOffState_t TipOffStateCfg; /* Tip Off of State Configuration in NVM */
    std::deque<SimpleCalData_t> simpleCalData;
    unsigned int storeCount;
    bool storeRejected;
    ReqPloadCtrlSysStat_t ReqPloadCtrlSysStat;
    StorePloadHornStat_t HornStoreState;
    uint16_t AutoStorePassCount;
    bool AutoTruckIdEnabled;
    bool AutoMaterialIdEnabled;
    bool manualAddEnabled;
    bool multiTaskEnabled;
    uint8_t multiTaskCount;
    bool TipoffActive;
    bool tipoffAssistActive;
    uint_least16_t tipoffAssistActiveEid;
    bool manualAddAvailable;
    bool splitModeEnabled;
    bool lftDisabled;
    uint8_t targetType;
    uint16_t stepNumber;
    uint8_t iconType;
    uint16_t targetPasses;

    template <class Archive>

	void serialize(Archive &ar, unsigned int version)
    {
		/* Archive Fields Here */
        if (version < 15) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & timePoint;

        ar & taskNumber;
        ar & passCount;
        ar & truckWeight;
        ar & truckWeightAccuracy;
        ar & TruckStartWeight;
        ar & remainingWeight;
        ar & totalWeight;
        ar & totalWeightAccuracy;
        ar & subtotalCount;
        ar & ticketId;

        ar & materialId;
        ar & materialName;
        ar & materialDensity;

        ar & truckId;
        ar & truckName;
        ar & truckTargetWeight;

        ar & truckListEnabled;
        ar & materialListEnabled;

        ar & tag1Enabled;
        ar & customListName1;
        ar & tag1;
        ar & tag2Enabled;
        ar & customListName2;
        ar & tag2;
        ar & tag3Enabled;
        ar & customListName3;
        ar & tag3;
        ar & tag4Enabled;
        ar & customListName4;
        ar & tag4;

        ar & ManualTipOffState;
        ar & TipOffTriggerType;
        ar & TipOffState;
        ar & OperationMode;
        ar & StandbyState;
        ar & ClearMinusOneEnableStat;
        ar & DispBestBktWt.val;
        ar & DispBestBktWt.isOk;
        ar & TipOffStateCfg;
        ar & simpleCalData;
        ar & storeCount;
        ar & storeRejected;
        ar & ReqPloadCtrlSysStat;
        ar & HornStoreState;
        ar & AutoStorePassCount;
        ar & AutoTruckIdEnabled;
        ar & AutoMaterialIdEnabled;
        ar & manualAddEnabled;
        ar & multiTaskEnabled;
        ar & multiTaskCount;
        ar & TipoffActive;
        ar & tipoffAssistActive;
        ar & tipoffAssistActiveEid;
        ar & manualAddAvailable;
        ar & splitModeEnabled;
        ar & lftDisabled;
        ar & targetType;
        ar & stepNumber;
        ar & iconType;
        ar & targetPasses;
    }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }


     inline bool ReadHornStoreEnable( void ){ return HornStoreState == SOUND_HORN? true : false;}

 private:  
     /* Add Fields Here */
};  

typedef Datum<LpsSaJobMgrTxChannelStorage> LpsSaJobMgrTxChannel;

BOOST_CLASS_VERSION(LpsSaJobMgrTxChannelStorage, 15);
#endif

