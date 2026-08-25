#ifndef LPSSALISTMGR_POSTINTERFACE_HPP
#define LPSSALISTMGR_POSTINTERFACE_HPP

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include "InterfaceCommon.h"
#include <cstdint>
#include <type_traits>
#include <string>
#include <atomic>

/*
 * The interface storage class.
 */
class LpsSaListMgrPostInterfaceStorage : public csvable {
public:
    LpsSaListMgrPostInterfaceStorage() :
        appName(), appRequestId(0),
        type(LpsSaListMgrInterfaceListType::NONE),
        format(LpsSaListMgrInterfaceListFormat::NONE),
        name(),
        filePath() {}

    std::string appName;
    uint32_t appRequestId;
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
        ar & type;
        ar & format;
        ar & name;
        ar & filePath;
    }

    static uint32_t getNextAppRequestId() {
        static std::atomic<uint32_t> nextAppRequestId(0);
        return nextAppRequestId++;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaListMgrPostInterfaceStorage> LpsSaListMgrPostInterface;

BOOST_CLASS_VERSION(LpsSaListMgrPostInterfaceStorage, 2);

#endif /* LPSSALISTMGR_POSTINTERFACE_HPP */
