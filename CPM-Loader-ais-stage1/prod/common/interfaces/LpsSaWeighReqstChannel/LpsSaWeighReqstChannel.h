#include <cstdint>
#include <atomic>
#include <string>
#include <vector>
#include <utility>

#include <cstdint>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/archive/archive_exception.hpp>

#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#include <interfaces/LpsSaWeighTxChannel/InterfaceTypes.h>

#ifndef LPSSAWEIGHREQSTCHANNEL_H
#define LPSSAWEIGHREQSTCHANNEL_H

class LpsSaWeighReqstChannelStorage: public csvable
{
public:
    enum class Command : uint8_t {
        NONE,
        WRITE_OVERLOAD_WARNING_ENABLE,
        WRITE_BUCKET_PAYLOAD_TARGET_WEIGHT,
        ZERO,
        RESET_BEST_BUCKET_WEIGHT,
        CAPTURE_CYLINDER_EXTENSION_REFERENCE,
        CLEAR_REWEIGH_WARNING,
        WRITE_WEIGH_RANGE,
        WRITE_CALIBRATION_WEIGHT,
        WRITE_MACHINE_PITCH_CAL_OFFSET,
        WRITE_HYD_OIL_TEMP_ENABLE,
        WRITE_AUDIBLE_WEIGHT_ENABLE,
        WRITE_LFT_SEALED,
        WRITE_LFT_SEALED_FLASH_ENABLE,
        WRITE_LIFT_POSITION_SENSOR_ID,
        WRITE_TILT_POSITION_SENSOR_ID,
        WRITE_LIFT_HE_PRESSURE_SENSOR_ID,
        WRITE_LIFT_RE_PRESSURE_SENSOR_ID,
        WRITE_HYDRAULIC_OIL_TEMP_SENSOR_ID,
        WRITE_PAYLOAD_OUT_OF_CAL,
        NOTIFY_TICKET_NUMBER_WRITE,
        PUBLISH_SERVICE_HISTORY,
        WRITE_WORK_TOOL_ID,
        PUBLISH_LIFT_SENSOR_CALIBRATION,
        PUBLISH_TILT_SENSOR_CALIBRATION,
        PUBLISH_WEIGH_CALIBRATION,
        PUBLISH_WEIGH_CONFIGURATION,
        PUBLISH_RECENT_WEIGH_RESULTS,
        WRITE_REWEIGH_MAX_PITCH,
        WRITE_REWEIGH_MIN_PITCH,
        WRITE_REWEIGH_MAX_ABS_ROLL,
        WRITE_REWEIGH_MIN_LIFT_CYL_VEL,
        WRITE_ADVANCED_CALIBRATION_ADJUSTMENT,
        RUN_TEST,
        RECORD_TEST,
        WRITE_IMU_COMP_ENABLE,
    };

    std::string appName; // Who is sending the message?
    uint32_t appRequestId; // This gets sent back in the "Return" interface.

    Command command;

    struct {
        bool b;
        float f1;
        float f2;
        std::string s;
        uint32_t u;
        std::vector<std::pair<float, float>> map;
    } arg;

    /* Initialize Fields Here */
    LpsSaWeighReqstChannelStorage() :
        appName(),
        appRequestId(0),
        command(Command::NONE),
        arg{ false, 0.f, 0.f, "", 0 } {}

    void reInit() {
        *this = LpsSaWeighReqstChannelStorage();
    }

    template <class Archive>
    void serialize(Archive &ar, unsigned int version)
    {
        if (version < 7) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & appName;
        ar & appRequestId;

        ar & command;

        ar & arg.b;
        ar & arg.f1;
        ar & arg.f2;
        ar & arg.s;
        ar & arg.u;
        ar & arg.map;
    }

    void toCsv(CsvOutStream& out) const
    {
        out("Not", "Supported");
    }

    static uint32_t getNextAppRequestId() {
        static std::atomic<uint32_t> nextAppRequestId(0);
        return nextAppRequestId++;
    }

private:
     /* Add Fields Here */
};

typedef Datum<LpsSaWeighReqstChannelStorage> LpsSaWeighReqstChannel;

BOOST_CLASS_VERSION(LpsSaWeighReqstChannelStorage, 7);

#endif




