#ifndef WORKORDERASSIST_TXINTERFACE_HPP
#define WORKORDERASSIST_TXINTERFACE_HPP

#include <cstdint>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

/*
 * The interface storage class.
 */
class WorkOrderAssistTxInterfaceStorage : public csvable {
public:
    bool activityDetected;
    int32_t activeSiteId;
    int64_t activeJobId;
    uint16_t numberOfJobs;

    bool hubServiceCommsActive;
    uint16_t hubServiceTxErrorCount;
    uint16_t hubServiceTxSuccessCount;
    uint16_t hubServiceRxCount;

    bool commsWarningActive;

    WorkOrderAssistTxInterfaceStorage() :
        activityDetected(false),
        activeSiteId(-1),
        activeJobId(-1),
        numberOfJobs(0),
        hubServiceCommsActive(false),
        hubServiceTxErrorCount(0),
        hubServiceTxSuccessCount(0),
        hubServiceRxCount(0),
        commsWarningActive(false) {}

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & activityDetected;
        ar & activeSiteId;
        ar & activeJobId;
        ar & numberOfJobs;

        ar & hubServiceCommsActive;
        ar & hubServiceTxErrorCount;
        ar & hubServiceTxSuccessCount;
        ar & hubServiceRxCount;

        ar & commsWarningActive;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<WorkOrderAssistTxInterfaceStorage> WorkOrderAssistTxInterface;

BOOST_CLASS_VERSION(WorkOrderAssistTxInterfaceStorage, 1);

#endif
