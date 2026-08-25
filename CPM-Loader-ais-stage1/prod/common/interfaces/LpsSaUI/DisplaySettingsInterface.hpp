#ifndef LPSSAUI_DISPLAYSETTINGSINTERFACE_HPP
#define LPSSAUI_DISPLAYSETTINGSINTERFACE_HPP

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include "DisplaySettings.hpp"

/*
 * The interface storage class.
 */
class LpsSaUIDisplaySettingsInterfaceStorage : public csvable {
public:
    LpsSaUIDisplaySettingsInterfaceStorage() :
        settings() {}

    LpsSaUIDisplaySettings settings;

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & settings;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaUIDisplaySettingsInterfaceStorage> LpsSaUIDisplaySettingsInterface;

BOOST_CLASS_VERSION(LpsSaUIDisplaySettingsInterfaceStorage, 0);

#endif /* LPSSAUI_DISPLAYSETTINGSINTERFACE_HPP */
