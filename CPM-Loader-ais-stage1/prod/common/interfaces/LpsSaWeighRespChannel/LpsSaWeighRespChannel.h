#include <cstdint>
#include <type_traits>
#include <chrono>

#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>

#include <boost_helpers/boost_serialization_chrono.hpp>

#include "../LpsSaWeighReqstChannel/LpsSaWeighReqstChannel.h"

#ifndef LPSSAWEIGHRESPCHANNEL_H
#define LPSSAWEIGHRESPCHANNEL_H

class LpsSaWeighRespChannelStorage: public csvable
{  
public:
    std::string appName;
    uint32_t appRequestId;
    std::chrono::steady_clock::time_point timePoint;
    LpsSaWeighReqstChannel::Command command;
    bool success;
    std::string arg1;

    LpsSaWeighRespChannelStorage() :
        appName(),
        appRequestId(0),
        timePoint(std::chrono::steady_clock::now()),
        command(LpsSaWeighReqstChannel::Command::NONE),
        success(false),
        arg1() {}


    inline const std::string& filePath() const { return arg1; }
    inline void filePath(const std::string& filePath) { arg1 = filePath; }

    inline void setTimePoint(const std::chrono::steady_clock::time_point& tp = std::chrono::steady_clock::now()) {
        timePoint = tp;
    }

    template <class Archive>
    void serialize(Archive &ar, unsigned int version) {
        if (version < 4) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & appName;
        ar & appRequestId;
        ar & timePoint;
        ar & command;
        ar & success;
        ar & arg1;
    }

    void toCsv(CsvOutStream& out) const
    {
        out("Not", "Supported");
    }

private:
    /* Add Fields Here */
};  

typedef Datum<LpsSaWeighRespChannelStorage> LpsSaWeighRespChannel;

BOOST_CLASS_VERSION(LpsSaWeighRespChannelStorage, 4);

#endif

