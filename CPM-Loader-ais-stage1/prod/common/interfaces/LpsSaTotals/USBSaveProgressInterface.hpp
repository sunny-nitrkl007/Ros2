#ifndef LPSSATOTALS_USBSAVEPROGRESS_INTERFACE_HPP
#define LPSSATOTALS_USBSAVEPROGRESS_INTERFACE_HPP

#include <cstdint>
#include <type_traits>
#include <string>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>


/*
 * The interface storage class.
 */
class LpsSaTotalsUSBSaveProgressInterfaceStorage : public csvable {
public:
    LpsSaTotalsUSBSaveProgressInterfaceStorage() :
        state(),
        totalFiles(),
        filesTransfered() {}

    uint8_t state;
    int totalFiles;
    int filesTransfered;

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & state;
        ar & totalFiles;
        ar & filesTransfered;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaTotalsUSBSaveProgressInterfaceStorage> LpsSaTotalsUSBSaveProgressInterface;

BOOST_CLASS_VERSION(LpsSaTotalsUSBSaveProgressInterfaceStorage, 1);

#endif /* LPSSATOTALS_USBSAVEPROGRESS_INTERFACE_HPP */
