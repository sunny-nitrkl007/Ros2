#ifndef LPSSALISTMGR_TXINTERFACE_HPP
#define LPSSALISTMGR_TXINTERFACE_HPP

#include <boost/filesystem.hpp>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>
#include <chrono>
#include <boost_helpers/boost_serialization_chrono.hpp>

#include "InterfaceCommon.h"

/*
 * The interface storage class.
 */
class LpsSaListMgrTxInterfaceStorage : public csvable {
public:
    LpsSaListMgrTxInterfaceStorage() :
        timePoint(std::chrono::steady_clock::now()) {}

    std::string basePath;
    std::string customList1Name;
    std::string customList2Name;
    std::string customList3Name;
    std::string customList4Name;
    std::chrono::steady_clock::time_point timePoint;

    static constexpr auto truckListJsonFileName = "TruckList.json";
    static constexpr auto truckListCsvFileName = "TruckList.csv";
    static constexpr auto materialListJsonFileName = "MaterialList.json";
    static constexpr auto materialListCsvFileName = "MaterialList.csv";
    static constexpr auto operatorIdListJsonFileName = "OperatorID.json";
    static constexpr auto operatorIdListCsvFileName = "OperatorID.csv";

    static std::string getCustomListJsonFileName(LpsSaListMgrInterfaceListType listType) {
        std::string prefix(LpsSaListMgrInterfaceListType_string(listType));
        return prefix + ".json";
    }

    static std::string getCustomListCsvFileName(LpsSaListMgrInterfaceListType listType) {
        std::string prefix(LpsSaListMgrInterfaceListType_string(listType));
        return prefix + ".csv";
    }

    std::string getTruckListJsonPath() const {
        return (boost::filesystem::path{ basePath } / truckListJsonFileName).string();
    }

    std::string getTruckListCsvPath() const {
        return (boost::filesystem::path{ basePath } / truckListCsvFileName).string();
    }

    std::string getMaterialListJsonPath() const {
        return (boost::filesystem::path{ basePath } / materialListJsonFileName).string();
    }

    std::string getMaterialListCsvPath() const {
        return (boost::filesystem::path{ basePath } / materialListCsvFileName).string();
    }

    std::string getOperatorIdListJsonPath() const {
        return (boost::filesystem::path{ basePath } / operatorIdListJsonFileName).string();
    }

    std::string getOperatorIdListCsvPath() const {
        return (boost::filesystem::path{ basePath } / operatorIdListCsvFileName).string();
    }

    std::string getCustomListJsonPath(LpsSaListMgrInterfaceListType listType) {
        return (boost::filesystem::path{ basePath } / getCustomListJsonFileName(listType)).string();
    }

    std::string getCustomListCsvPath(LpsSaListMgrInterfaceListType listType) {
        return (boost::filesystem::path{ basePath } / getCustomListCsvFileName(listType)).string();
    }

    void toCsv(CsvOutStream& out) const {
        out("basePath", basePath);
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & basePath;

        if (version >= 3) {
            ar & customList1Name;
            ar & customList2Name;
            ar & customList3Name;
            ar & customList4Name;
        }
        if (version >= 4) {
            ar & timePoint;
        }
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaListMgrTxInterfaceStorage> LpsSaListMgrTxInterface;

BOOST_CLASS_VERSION(LpsSaListMgrTxInterfaceStorage, 4);

#endif /* LPSSALISTMGR_TXINTERFACE_HPP */
