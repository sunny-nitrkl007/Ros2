#ifndef LPSSAUI_DISPLAYSTATEINTERFACE_HPP
#define LPSSAUI_DISPLAYSTATEINTERFACE_HPP

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include "DisplayState.hpp"

/*
 * The interface storage class.
 */
class LpsSaUIDisplayStateInterfaceStorage : public csvable {
public:
    LpsSaUIDisplayStateInterfaceStorage() :
        state() {}

    LpsSaUIDisplayState state;

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & state;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaUIDisplayStateInterfaceStorage> LpsSaUIDisplayStateInterface;

BOOST_CLASS_VERSION(LpsSaUIDisplayStateInterfaceStorage, 0);

#endif /* LPSSAUI_DISPLAYSTATEINTERFACE_HPP */
