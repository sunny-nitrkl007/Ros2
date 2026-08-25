#ifndef AUTONOMYCONDITIONDIAGNOSTICS_TXINTERFACE_HPP
#define AUTONOMYCONDITIONDIAGNOSTICS_TXINTERFACE_HPP

#include <cstdint>
#include <chrono>

#include <scl_prmsw.h>
#include <boost_helpers/boost_serialization_chrono.hpp>
#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>
#include <interfaces/SEAStatus/SEAStatus.h>
/*
 * The interface storage class.
 */
class AutonomyConditionDiagnosticsTxInterfaceStorage : public csvable {
public:
    std::chrono::steady_clock::time_point timePoint;
    bool displayEthernetBad;
    bool productLinkEthernetBad;
    std::vector<SEA> seaList;

    AutonomyConditionDiagnosticsTxInterfaceStorage() :
        timePoint(std::chrono::steady_clock::now()),
        displayEthernetBad(false),
        productLinkEthernetBad(false),
        seaList() {}

    inline void setTimePoint(const std::chrono::steady_clock::time_point& tp = std::chrono::steady_clock::now()) {
        timePoint = tp;
    }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    static bool checkSEAInstallStatus(const uint16_t& status) {
        bool seaInstalled = false;

        switch(status)
        {
        case SCL_PRMSW_FEAT_STATUS_INSTALLED_ENABLED:
        case SCL_PRMSW_FEAT_STATUS_INSTALLED_DISABLED:
        case SCL_PRMSW_FEAT_STATUS_FREE_USE_INSTALLED:
            //Installed
            seaInstalled = true;
            break;
        case SCL_PRMSW_FEAT_STATUS_UNINSTALLED:
        case SCL_PRMSW_FEAT_STATUS_NONE:
        default:
            //Uninstalled
            seaInstalled = false;
            break;
        }

        return seaInstalled;
    }

    static bool checkSEAEnableStatus(const uint16_t& status) {
        bool seaEnabled = false;

        switch(status)
        {
        case SCL_PRMSW_FEAT_STATUS_INSTALLED_ENABLED:
        case SCL_PRMSW_FEAT_STATUS_FREE_USE_INSTALLED:
            //enabled
            seaEnabled = true;
            break;
        case SCL_PRMSW_FEAT_STATUS_UNINSTALLED:
        case SCL_PRMSW_FEAT_STATUS_INSTALLED_DISABLED:
        case SCL_PRMSW_FEAT_STATUS_NONE:
        default:
            //disabled
            seaEnabled = false;
            break;
        }

        return seaEnabled;
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & timePoint;

        ar & displayEthernetBad;
        ar & productLinkEthernetBad;
        ar & seaList;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<AutonomyConditionDiagnosticsTxInterfaceStorage> AutonomyConditionDiagnosticsTxInterface;

BOOST_CLASS_VERSION(AutonomyConditionDiagnosticsTxInterfaceStorage, 0);

#endif
