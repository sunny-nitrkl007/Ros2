#ifndef AUTONOMYCONDITIONDIAGNOSTICS_RESPONSEINTERFACE_H
#define AUTONOMYCONDITIONDIAGNOSTICS_RESPONSEINTERFACE_H

#include <cstdint>
#include <type_traits>
#include <string>
#include <chrono>

#include <boost/archive/archive_exception.hpp>

#include <boost_helpers/boost_serialization_chrono.hpp>
#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include "RequestInterface.hpp"

/*
 * The interface storage class.
 */
class AutonomyConditionDiagnosticsResponseInterfaceStorage : public csvable {
public:
    AutonomyConditionDiagnosticsResponseInterfaceStorage() :
        appName(),
        appRequestId(0),
        timePoint(std::chrono::steady_clock::now()),
        command(AutonomyConditionDiagnosticsRequestInterfaceCommand::NONE),
        success(false) {};

    std::string appName;
    uint32_t appRequestId;
    std::chrono::steady_clock::time_point timePoint;
    AutonomyConditionDiagnosticsRequestInterfaceCommand command;
    bool success;

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & appName;
        ar & appRequestId;
        ar & timePoint;
        ar & command;
        ar & success;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<AutonomyConditionDiagnosticsResponseInterfaceStorage> AutonomyConditionDiagnosticsResponseInterface;

BOOST_CLASS_VERSION(AutonomyConditionDiagnosticsResponseInterfaceStorage, 0);

#endif /* AUTONOMYCONDITIONDIAGNOSTICS_RESPONSEINTERFACE_H */
