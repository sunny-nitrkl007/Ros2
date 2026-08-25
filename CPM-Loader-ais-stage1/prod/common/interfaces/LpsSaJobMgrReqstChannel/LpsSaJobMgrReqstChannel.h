#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#ifndef _LpsSaJobMgrReqstChannel_h_
#define _LpsSaJobMgrReqstChannel_h_

#include <cstdint>
#include <type_traits>
#include <string>
#include <vector>
#include <atomic>

#include <boost/serialization/vector.hpp>
#include <boost/archive/archive_exception.hpp>

#include <interfaces/LpsSaJobMgrTxChannel/InterfaceTypes.h>


enum class LpsSaJobMgrReqstCommand : uint8_t {
    WRITE_MATERIAL_ID,
    WRITE_MATERIAL_NAME,
    WRITE_MATERIAL_DENSITY,
    WRITE_TRUCK_ID,
    WRITE_TRUCK_NAME,
    WRITE_TRUCK_TARGET_WEIGHT,
    WRITE_TAG1,
    WRITE_TAG2,
    WRITE_TAG3,
    WRITE_TAG4,
    NONE
};

#define LpsSaJobMgrReqstCommand_Base_t(val) (static_cast<typename std::underlying_type<LpsSaJobMgrReqstCommand>::type>(val))

class LpsSaJobMgrReqst {
public:
    LpsSaJobMgrReqst() :
        command(LpsSaJobMgrReqstCommand::NONE),
        arg1(), arg2(0.f), arg3(0), arg4() {}

    inline uint32_t materialId() const { return arg3; }
    inline void materialId(uint32_t u) { arg3 = u; }

    inline const std::string& materialName() const { return arg1; }
    inline void materialName(const std::string& n) { arg1 = n; }

    inline float materialDensity() const { return arg2; }
    inline void materialDensity(float f) { arg2 = f; }

    inline uint32_t truckId() const { return arg3; }
    inline void truckId(uint32_t u) { arg3 = u; }

    inline const std::string& truckName() const { return arg1; }
    inline void truckName(const std::string& n) { arg1 = n; }

    inline float truckTargetWeight() const { return arg2; }
    inline void truckTargetWeight(float f) { arg2 = f; }

    inline const std::string& tagValue() const { return arg1; }
    inline const std::string& tagName() const {return arg4; }
    inline void tag(const std::string& tagValue, const std::string& tagName) { arg1 = tagValue; arg4 = tagName; }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        if (version < 1) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & command;
        ar & arg1;
        ar & arg2;
        ar & arg3;
        ar & arg4;
    }

public:
    LpsSaJobMgrReqstCommand command;

private:
    std::string arg1;
    float arg2;
    uint32_t arg3;
    std::string arg4;
};


class LpsSaJobMgrReqstChannelStorage: public csvable
{
public:
    enum class Command : uint8_t {
        NONE,
        ZERO,
        MINUS_ONE,
        CLEAR,
        STORE,
        REWEIGH,
        MANUAL_ADD,
        STANDBY_ACTIVATE,
        STANDBY_DEACTIVATE,
        TIPOFF_MODE_TOGGLE,
        MANUAL_TIPOFF_ACTIVATE,
        MANUAL_TIPOFF_DEACTIVATE,
        WRITE_TIPOFF_TRIGGER_TYPE,
        WRITE_TIPOFF_MODE,
        WRITE_HORN_ON_STORE_ENABLED,
        WRITE_AUTO_STORE_PASS_COUNT,
        WRITE_AUTO_TRUCK_ID_ENABLED,
        WRITE_AUTO_MATERIAL_ID_ENABLED,
        WRITE_MANUAL_ADD_ENABLED,
        WRITE_MULTI_TASK_ENABLED,
        WRITE_MULTI_TASK_COUNT,
        WRITE_SPLIT_MODE_ENABLED,
        SELECT_TASK,
        SELECT_NEXT_TASK,
        SELECT_PREVIOUS_TASK,
        WRITE_TRUCK_LIST_ENABLED,
        WRITE_MATERIAL_LIST_ENABLED,
        WRITE_TAG1_ENABLED,
        WRITE_TAG2_ENABLED,
        WRITE_TAG3_ENABLED,
        WRITE_TAG4_ENABLED,
        WRITE_PAYLOAD_NEXT_SUBTOTAL,
        WRITE_LFT_DISABLED,
        WRITE_TARGET_TYPE,
        WRITE_TOTAL_TARGET_WEIGHT,
        SELECT_SUBTOTAL,
        PAYLOAD_DETAILS_FILE_REQUEST,
        WRITE_SUBTOTAL_INFO,
    };

    std::string appName;
    uint32_t appRequestId;

    Command command;

    typedef struct {
        std::string command;
        uint16_t currentStepNumber;
        uint16_t newStepNumber;
        float targetWeight;
        uint16_t targetProportion;
        uint16_t targetPasses;
        uint32_t materialId;
        std::string materialName;
        float materialDensity;
        uint8_t iconType;
    }LpsSaJobMgrSubtotalInfo_t;

    struct {
        LpsSaTipOffTriggerType_t tipoffTriggerType;
        LpsSaJobMgrTipOffState_t tipoffMode;
        uint16_t autoStorePassCount;
        uint8_t taskNumber;
        bool enabled;
        uint8_t targetType;
        uint16_t subtotalIndex;
        float totalTargetWeight;
        LpsSaJobMgrSubtotalInfo_t subtotalInfo;
    } data;

    std::vector<LpsSaJobMgrReqst> requests;

    LpsSaJobMgrReqstChannelStorage() : /* Initialize Fields Here */
        appName(), appRequestId(0),
        command(Command::NONE),
        data {
            LpsSaTipOffTriggerType_t::LPS_SA_JOB_MGR_TIP_OFF_TRIGGER_DISABLED,
            LpsSaJobMgrTipOffState_t::LPS_SA_JOB_MGR_TIP_OFF_UNAVAILABLE,
            0, 1, false, 0, 0, 0.f, {}
        },
        requests() {}

    void reInit() {
        *this = LpsSaJobMgrReqstChannelStorage();
    }

    template <class Archive>
    void serialize(Archive &ar, unsigned int version) {
        /* Archive Fields Here */
        if (version < 9) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & appName;
        ar & appRequestId;

        ar & command;

        ar & data.tipoffTriggerType;
        ar & data.tipoffMode;
        ar & data.autoStorePassCount;
        ar & data.taskNumber;
        ar & data.enabled;

        ar & requests;

        ar & data.targetType;
        ar & data.subtotalIndex;
        ar & data.totalTargetWeight;

        ar & data.subtotalInfo.command;
        ar & data.subtotalInfo.currentStepNumber;
        ar & data.subtotalInfo.newStepNumber;
        ar & data.subtotalInfo.targetWeight;
        ar & data.subtotalInfo.targetProportion;
        ar & data.subtotalInfo.targetPasses;
        ar & data.subtotalInfo.materialId;
        ar & data.subtotalInfo.materialName;
        ar & data.subtotalInfo.materialDensity;
        ar & data.subtotalInfo.iconType;
    }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    static uint32_t getNextAppRequestId() {
        static std::atomic<uint32_t> nextAppRequestId(0);
        return nextAppRequestId++;
    }

private:
    /* Add Fields Here */
};  

typedef Datum<LpsSaJobMgrReqstChannelStorage> LpsSaJobMgrReqstChannel;

BOOST_CLASS_VERSION(LpsSaJobMgrReqst, 1);
BOOST_CLASS_VERSION(LpsSaJobMgrReqstChannelStorage, 9);
#endif

