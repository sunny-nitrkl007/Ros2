#ifndef LPSSATOTALS_REQUESTINTERFACE_HPP
#define LPSSATOTALS_REQUESTINTERFACE_HPP

#include <cstdint>
#include <type_traits>
#include <string>
#include <atomic>

#include <boost/archive/archive_exception.hpp>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

enum class LpsSaTotalsRequestInterfaceCommand : uint8_t {
    NONE,

    PUBLISH_MATERIAL_TOTALS,
    PUBLISH_TRUCK_TOTALS,
    PUBLISH_TAG_TOTALS,
    PUBLISH_TICKET_HISTORY,
    PUBLISH_TICKET_HISTORY_NEXT,
    PUBLISH_TICKET_HISTORY_PREVIOUS,
    PUBLISH_TICKET_DETAILS,

    PRINT_MATERIAL_TOTALS_ALL,
    PRINT_MATERIAL_TOTAL,
    PRINT_TRUCK_REPORT_ALL,
    PRINT_TRUCK_REPORT_TODAY,
    PRINT_TRUCK_REPORT_PREVIOUS_DAY,
    PRINT_TRUCK_TICKET,
    PRINT_TRUCK_TOTALS_ALL,
    PRINT_TAG_TOTALS_ALL,

    CLEAR_TRUCK_REPORT_ALL,
    CLEAR_MATERIAL_TOTALS_ALL,
    CLEAR_MATERIAL_TOTAL,
    CLEAR_TRUCK_TOTALS_ALL,
    CLEAR_TRUCK_TOTAL,
    CLEAR_TAG_TOTALS_ALL,
    CLEAR_TAG_TOTAL,

    CLEAR_TRIP_TOTAL_A,
    CLEAR_TRIP_TOTAL_B,

    WRITE_LIFETIME_TOTAL_WEIGHT,
    WRITE_LIFETIME_TOTAL_PASS_COUNT,
    WRITE_LIFETIME_TOTAL_LOAD_COUNT,

    CLEAR_ALL_PRINTED_REPORTS,

    USB_CMD_SAVE_NEW,
    USB_CMD_SAVE_ALL,
    USB_CMD_SAVE_CANCEL,
    USB_SAVE_PROGRESS,
    PUBLISH_MATERIAL_TOTALS_TODAY,
    PUBLISH_TRUCK_TOTALS_TODAY,
    PUBLISH_TAG_TOTALS_TODAY,

    PUBLISH_MATERIAL_TOTALS_PREVIOUS_DAY,
    PUBLISH_TRUCK_TOTALS_PREVIOUS_DAY,
    PUBLISH_TAG_TOTALS_PREVIOUS_DAY,

    PRINT_MATERIAL_TOTALS_TODAY,
    PRINT_TRUCK_TOTALS_TODAY,
    PRINT_TAG_TOTALS_TODAY,

    PRINT_MATERIAL_TOTALS_PREVIOUS_DAY,
    PRINT_TRUCK_TOTALS_PREVIOUS_DAY,
    PRINT_TAG_TOTALS_PREVIOUS_DAY,
};

#define LpsSaTotalsRequestInterfaceCommand_Base_t(val) (static_cast<typename std::underlying_type<LpsSaTotalsRequestInterfaceCommand>::type>(val))

/*
 * The interface storage class.
 */
class LpsSaTotalsRequestInterfaceStorage : public csvable {
public:
    LpsSaTotalsRequestInterfaceStorage() :
        appName(),
        appRequestId(0),
        command(LpsSaTotalsRequestInterfaceCommand::NONE),
        arg1(),
        arg2(0),
        arg3(0.0) {}

    std::string appName; // Who is sending the message?
    uint32_t appRequestId; // This gets sent back in the "Return" interface.
    LpsSaTotalsRequestInterfaceCommand command;
    std::string arg1;
    uint32_t arg2;
    double arg3;

    inline const std::string& materialName() const { return arg1; }
    inline void materialName(const std::string& name) { arg1 = name; }

    inline const std::string& truckName() const { return arg1; }
    inline void truckName(const std::string& name) { arg1 = name; }

    inline const std::string& tagItemName() const { return arg1; }
    inline void tagItemName(const std::string& name) { arg1 = name; }

    inline const std::string& localDateStr() const { return arg1; }
    inline void localDateStr(const std::string& str) { arg1 = str; }

    inline const std::string& ticketId() const { return arg1; }
    inline void ticketId(const std::string& str) { arg1 = str; }

    inline const uint32_t& tagTotalsIndex() const { return arg2; }
    inline void tagTotalsIndex(const uint32_t& index) { arg2 = index; }

    inline const uint32_t& loadCount() const { return arg2; }
    inline void loadCount(const uint32_t& count) { arg2 = count; }

    inline const uint32_t& passCount() const { return arg2; }
    inline void passCount(const uint32_t& count) { arg2 = count; }

    inline const double& weight() const { return arg3; }
    inline void weight(const double& value) { arg3 = value; }

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
        ar & arg1;
        ar & arg2;
        ar & arg3;
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
typedef Datum<LpsSaTotalsRequestInterfaceStorage> LpsSaTotalsRequestInterface;

BOOST_CLASS_VERSION(LpsSaTotalsRequestInterfaceStorage, 2);

#endif /* LPSSATOTALS_REQUESTINTERFACE_HPP */
