#ifndef LPSSATOTALS_PRINTERCNFG_HPP
#define LPSSATOTALS_PRINTERCNFG_HPP

#include <cstdint>
#include <type_traits>
#include <string>

#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>

enum class LpsSaTotalsPrinterCnfgDateFormat : uint16_t {
    YYYY_MM_DD = 0x0670,
    MM_DD_YYYY = 0x0671,
    DD_MM_YYYY = 0x0672,
    YY_MM_DD = 0x0673,
    MM_DD_YY = 0x0674,
    DD_MM_YY = 0x0675,
    MM_DD = 0x0676,
    DD_MM = 0x0677,
};

#define LpsSaTotalsPrinterCnfgDateFormat_Base_t(val) (static_cast<typename std::underlying_type<LpsSaTotalsPrinterCnfgDateFormat>::type>(val))

enum class LpsSaTotalsPrinterCnfgDateSeparator : uint16_t {
    SLASH = 0x0678,
    HYPHEN = 0x0679
};

#define LpsSaTotalsPrinterCnfgDateSeparator_Base_t(val) (static_cast<typename std::underlying_type<LpsSaTotalsPrinterCnfgDateSeparator>::type>(val))

struct LpsSaTotalsPrinterCnfg {
public:
    LpsSaTotalsPrinterCnfg() :
        installed(false),
        leadingBlankLines(0),
        trailingBlankLines(0),
        dateFormat(LpsSaTotalsPrinterCnfgDateFormat::YYYY_MM_DD),
        dateSeparator(LpsSaTotalsPrinterCnfgDateSeparator::HYPHEN),
        truckTicket(),
        truckReport(),
        materialReport(),
        udpTransferIp(""),
        udpTransferPort(0),
        udpTransferEnabled(false) {}

    bool installed;
    uint8_t leadingBlankLines;
    uint8_t trailingBlankLines;
    LpsSaTotalsPrinterCnfgDateFormat dateFormat;
    LpsSaTotalsPrinterCnfgDateSeparator dateSeparator;

    struct TruckTicket {
        static constexpr uint16_t RETENTION_PERIOD_DAYS_DEFAULT = 366*2; // Default to 2 years

        TruckTicket() :
            numCopies(1),
            headerEnabled(true),
            headerLine1(),
            headerLine2(),
            headerLine3(),
            footerLine1(),
            bucketWeightsEnabled(true),
            truckWeightEnabled(true),
            truckIdEnabled(true),
            materialIdEnabled(true),
            storeTimeEnabled(true),
            ticketNumber(1),
            retentionPeriod(RETENTION_PERIOD_DAYS_DEFAULT) {}

        uint8_t numCopies;
        bool headerEnabled;
        std::string headerLine1;
        std::string headerLine2;
        std::string headerLine3;
        std::string footerLine1;
        bool bucketWeightsEnabled;
        bool truckWeightEnabled;
        bool truckIdEnabled;
        bool materialIdEnabled;
        bool storeTimeEnabled;
        uint32_t ticketNumber;
        uint16_t retentionPeriod;
    } truckTicket;

    struct TruckReport {
        TruckReport() : resetTimeEnabled(true) {}
        bool resetTimeEnabled;
    } truckReport;

    struct MaterialReport {
        MaterialReport(): resetTimeEnabled(true) {}
        bool resetTimeEnabled;
    } materialReport;

    std::string udpTransferIp;
    uint32_t udpTransferPort;
    bool udpTransferEnabled;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & installed;
        ar & leadingBlankLines;
        ar & trailingBlankLines;
        ar & dateFormat;
        ar & dateSeparator;
        ar & truckTicket.numCopies;
        ar & truckTicket.headerEnabled;
        ar & truckTicket.headerLine1;
        ar & truckTicket.headerLine2;
        ar & truckTicket.headerLine3;
        ar & truckTicket.bucketWeightsEnabled;
        ar & truckTicket.truckWeightEnabled;
        ar & truckTicket.truckIdEnabled;
        ar & truckTicket.materialIdEnabled;
        ar & truckTicket.storeTimeEnabled;
        ar & truckTicket.ticketNumber;

        ar & truckReport.resetTimeEnabled;

        ar & materialReport.resetTimeEnabled;

        if (version >= 1) {
            ar & udpTransferIp;
            ar & udpTransferPort;
            ar & udpTransferEnabled;
        }

        if (version >= 2) {
            ar & truckTicket.footerLine1;
        }

        if (version >= 3) {
            ar & truckTicket.retentionPeriod;
        }
    }
};

BOOST_CLASS_VERSION(LpsSaTotalsPrinterCnfg, 3);

#endif /* LPSSATOTALS_PRINTERCNFG_HPP */
