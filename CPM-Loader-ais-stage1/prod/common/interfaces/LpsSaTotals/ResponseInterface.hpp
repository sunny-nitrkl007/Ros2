#ifndef LPSSATOTALS_RESPONSEINTERFACE_HPP
#define LPSSATOTALS_RESPONSEINTERFACE_HPP

#include <cstdint>
#include <type_traits>
#include <string>

#include <boost/archive/archive_exception.hpp>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include "RequestInterface.hpp"

/*
 * The interface storage class.
 */
class LpsSaTotalsResponseInterfaceStorage : public csvable {
public:
    LpsSaTotalsResponseInterfaceStorage() :
        appName(),
        appRequestId(0),
        command(LpsSaTotalsRequestInterfaceCommand::NONE),
        success(false),
        arg1() {};

    std::string appName;
    uint32_t appRequestId;
    LpsSaTotalsRequestInterfaceCommand command;
    bool success;
    std::string arg1;

    inline const std::string& filePath() const { return arg1; }
    inline void filePath(const std::string& filePath) { arg1 = filePath; }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        if (version < 2) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & appName;
        ar & appRequestId;
        ar & command;
        ar & success;
        ar & arg1;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaTotalsResponseInterfaceStorage> LpsSaTotalsResponseInterface;

BOOST_CLASS_VERSION(LpsSaTotalsResponseInterfaceStorage, 2);

#endif /* LPSSATOTALS_RESPONSEINTERFACE_HPP */
