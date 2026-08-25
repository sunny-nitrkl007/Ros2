#ifndef WORKORDERASSIST_SETTINGSINTERFACE_HPP
#define WORKORDERASSIST_SETTINGSINTERFACE_HPP

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include "Settings.hpp"

/*
 * The interface storage class.
 */
class WorkOrderAssistSettingsInterfaceStorage : public WorkOrderAssistSettings, public csvable {
public:
    WorkOrderAssistSettingsInterfaceStorage() {}

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<WorkOrderAssistSettings>(*this);
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<WorkOrderAssistSettingsInterfaceStorage> WorkOrderAssistSettingsInterface;

BOOST_CLASS_VERSION(WorkOrderAssistSettingsInterfaceStorage, 1);

#endif
