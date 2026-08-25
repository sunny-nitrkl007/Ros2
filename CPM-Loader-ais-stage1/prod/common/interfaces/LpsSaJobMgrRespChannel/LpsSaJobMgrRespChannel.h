#include <cstdint>
#include <type_traits>
#include <chrono>

#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>

#include <boost_helpers/boost_serialization_chrono.hpp>

#include "../LpsSaJobMgrReqstChannel/LpsSaJobMgrReqstChannel.h"

#ifndef LPSSAJOBMGRRESPCHANNEL_H
#define LPSSAJOBMGRRESPCHANNEL_H

class LpsSaJobMgrRespChannelStorage: public csvable
{  
public:
    std::string appName;
    uint32_t appRequestId;
    std::chrono::steady_clock::time_point timePoint;
    LpsSaJobMgrReqstChannel::Command command;
    bool success;

    LpsSaJobMgrRespChannelStorage() :
        appName(),
        appRequestId(0),
        timePoint(std::chrono::steady_clock::now()),
        command(LpsSaJobMgrReqstChannel::Command::NONE),
        success(false) {}

    template <class Archive>
    void serialize(Archive &ar, unsigned int version) {
        if (version < 2) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & appName;
        ar & appRequestId;
        ar & timePoint;
        ar & command;
        ar & success;
    }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

private:
    /* Add Fields Here */
};  

typedef Datum<LpsSaJobMgrRespChannelStorage> LpsSaJobMgrRespChannel;

BOOST_CLASS_VERSION(LpsSaJobMgrRespChannelStorage, 2);
#endif

