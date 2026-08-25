#ifndef AUTONOMYCONDITIONDIAGNOSTICS_REQUESTINTERFACE_H
#define AUTONOMYCONDITIONDIAGNOSTICS_REQUESTINTERFACE_H

#include <cstdint>
#include <type_traits>
#include <string>
#include <atomic>

#include <boost/archive/archive_exception.hpp>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

enum class AutonomyConditionDiagnosticsRequestInterfaceCommand : uint8_t {
    SEA_LEVEL2_TEMP_INSTALL,
    NONE,
};

#define AutonomyConditionDiagnosticsRequestInterfaceCommand_Base_t(val) (static_cast<typename std::underlying_type<AutonomyConditionDiagnosticsRequestInterfaceCommand>::type>(val))

/*
 * The interface storage class.
 */
class AutonomyConditionDiagnosticsRequestInterfaceStorage : public csvable {
public:
    AutonomyConditionDiagnosticsRequestInterfaceStorage() :
        appName(),
        appRequestId(0),
        command(AutonomyConditionDiagnosticsRequestInterfaceCommand::NONE),
        data() {}

    std::string appName; // Who is sending the message?
    uint32_t appRequestId; // This gets sent back in the "Return" interface.
    AutonomyConditionDiagnosticsRequestInterfaceCommand command;

    struct {
        bool installed;
    } data;

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & appName;
        ar & appRequestId;
        ar & command;
        ar & data.installed;
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
typedef Datum<AutonomyConditionDiagnosticsRequestInterfaceStorage> AutonomyConditionDiagnosticsRequestInterface;

BOOST_CLASS_VERSION(AutonomyConditionDiagnosticsRequestInterfaceStorage, 0);

#endif /* AUTONOMYCONDITIONDIAGNOSTICS_REQUESTINTERFACE_H */
