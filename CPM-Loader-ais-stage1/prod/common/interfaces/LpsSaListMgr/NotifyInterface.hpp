#ifndef LPSSALISTMGR_NOTIFYINTERFACE_HPP
#define LPSSALISTMGR_NOTIFYINTERFACE_HPP

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>
#include <boost_helpers/boost_serialization_chrono.hpp>
#include <cstdint>
#include <chrono>
#include <type_traits>
#include <string>
#include "InterfaceCommon.h"

/*
 * The interface storage class.
 */
class LpsSaListMgrNotifyInterfaceStorage : public csvable {
public:
    LpsSaListMgrNotifyInterfaceStorage() :
        appName(), appRequestId(0), timePoint(std::chrono::steady_clock::now()),
        type(LpsSaListMgrInterfaceListType::NONE),
        format(LpsSaListMgrInterfaceListFormat::NONE),
        name(),
        filePath() {}

    std::string appName;
    uint32_t appRequestId;
    std::chrono::steady_clock::time_point timePoint;
    LpsSaListMgrInterfaceListType type;
    LpsSaListMgrInterfaceListFormat format;
    std::string name;
    std::string filePath;

    void toCsv(CsvOutStream& out) const {
        out("type", LpsSaListMgrInterfaceListType_Base_t(type));
        out("format", LpsSaListMgrInterfaceListFormat_Base_t(format));
        out("name", name);
        out("filePath", filePath);
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        if (version < 2) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & appName;
        ar & appRequestId;
        ar & timePoint;
        ar & type;
        ar & format;
        ar & name;
        ar & filePath;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaListMgrNotifyInterfaceStorage> LpsSaListMgrNotifyInterface;

BOOST_CLASS_VERSION(LpsSaListMgrNotifyInterfaceStorage, 2);

#endif /* LPSSALISTMGR_NOTIFYINTERFACE_HPP */
