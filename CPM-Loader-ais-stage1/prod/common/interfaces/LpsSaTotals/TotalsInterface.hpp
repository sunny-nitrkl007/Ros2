#ifndef LPSSATOTALS_TOTALSINTERFACE_HPP
#define LPSSATOTALS_TOTALSINTERFACE_HPP

#include <iostream>

#include <boost/array.hpp>
#include <boost/serialization/array.hpp>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

class LpsSaTotalsTripInterfaceStorage {
public:
    uint32_t loadCount;
    uint32_t passCount;
    double weightTonnes;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & loadCount;
        ar & passCount;
        ar & weightTonnes;
    }
};

BOOST_CLASS_VERSION(LpsSaTotalsTripInterfaceStorage, 1);

/*
 * The interface storage class.
 */
class LpsSaTotalsInterfaceStorage : public csvable {
public:
    LpsSaTotalsInterfaceStorage() = default;

    uint32_t loadCount;
    uint32_t passCount;
    double weightTonnes;
    std::array<LpsSaTotalsTripInterfaceStorage, 2> trips;
    float memoryRemainingPct; // Percent Memory Remaining for Tickets

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        if (version < 2) {
            // Too old, go away
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & loadCount;
        ar & passCount;
        ar & weightTonnes;
        ar & trips;
        ar & memoryRemainingPct;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaTotalsInterfaceStorage> LpsSaTotalsInterface;

BOOST_CLASS_VERSION(LpsSaTotalsInterfaceStorage, 2);

#endif /* LPSSATOTALS_TOTALSINTERFACE_HPP */
