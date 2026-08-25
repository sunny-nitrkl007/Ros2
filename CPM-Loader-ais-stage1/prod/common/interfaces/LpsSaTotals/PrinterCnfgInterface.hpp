#ifndef LPSSATOTALS_PRINTERCNFGINTERFACE_HPP
#define LPSSATOTALS_PRINTERCNFGINTERFACE_HPP

#include <cstdint>
#include <type_traits>
#include <string>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include "PrinterCnfg.hpp"

/*
 * The interface storage class.
 */
class LpsSaTotalsPrinterCnfgInterfaceStorage : public csvable {
public:
    LpsSaTotalsPrinterCnfgInterfaceStorage() :
        config() {}

    LpsSaTotalsPrinterCnfg config;

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & config;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaTotalsPrinterCnfgInterfaceStorage> LpsSaTotalsPrinterCnfgInterface;

BOOST_CLASS_VERSION(LpsSaTotalsPrinterCnfgInterfaceStorage, 1);

#endif /* LPSSATOTALS_PRINTERCNFGINTERFACE_HPP */
