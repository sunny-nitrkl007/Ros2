#ifndef LPSSALISTMGR_INTERFACECOMMON_H
#define LPSSALISTMGR_INTERFACECOMMON_H

#include <cstdint>
#include <type_traits>

enum class LpsSaListMgrInterfaceListType : uint8_t {
    TRUCK,
    MATERIAL,
    OPERATOR_ID,
    CUSTOM1,
    CUSTOM2,
    CUSTOM3,
    CUSTOM4,
    NONE
};

#define LpsSaListMgrInterfaceListType_Base_t(val) (static_cast<typename std::underlying_type<LpsSaListMgrInterfaceListType>::type>(val))

inline const char* LpsSaListMgrInterfaceListType_string(LpsSaListMgrInterfaceListType type) {
    switch (type) {
    case (LpsSaListMgrInterfaceListType::TRUCK): return "Truck";
    case (LpsSaListMgrInterfaceListType::MATERIAL): return "Material";
    case (LpsSaListMgrInterfaceListType::OPERATOR_ID): return "OPERATOR_ID";
    case (LpsSaListMgrInterfaceListType::CUSTOM1): return "CustomList1";
    case (LpsSaListMgrInterfaceListType::CUSTOM2): return "CustomList2";
    case (LpsSaListMgrInterfaceListType::CUSTOM3): return "CustomList3";
    case (LpsSaListMgrInterfaceListType::CUSTOM4): return "CustomList4";
    case (LpsSaListMgrInterfaceListType::NONE): return "None";
    default: return "Unknown";
    }
}

enum class LpsSaListMgrInterfaceListFormat : uint8_t {
    JSON,
    CSV,
    BDT,
    PROTOBUF,
    NONE
};

#define LpsSaListMgrInterfaceListFormat_Base_t(val) (static_cast<typename std::underlying_type<LpsSaListMgrInterfaceListFormat>::type>(val))

inline const char* LpsSaListMgrInterfaceListFormat_string(LpsSaListMgrInterfaceListFormat format) {
    switch (format) {
    case (LpsSaListMgrInterfaceListFormat::JSON): return "Json";
    case (LpsSaListMgrInterfaceListFormat::CSV): return "Csv";
    case (LpsSaListMgrInterfaceListFormat::BDT): return "Bdt";
    case (LpsSaListMgrInterfaceListFormat::PROTOBUF): return "Protobuf";
    case (LpsSaListMgrInterfaceListFormat::NONE): return "None";
    default: return "Unknown";
    }
}

#endif /* LPSSALISTMGR_INTERFACECOMMON_H */
