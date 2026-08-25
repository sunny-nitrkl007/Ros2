#ifndef LPSSALOADRECORD_INTERFACECOMMON_H
#define LPSSALOADRECORD_INTERFACECOMMON_H

enum class LpsSaLoadRecordStoreAction : uint8_t {
    AUTO = 0, // Stored automatically, not a ticket
    EXTERNAL, // Not used
    UI, // Not used
    TICKET_NOT_FOR_TRADE, // Stored manually and not for trade
    TICKET_LEGAL_FOR_TRADE // Stored manually and legal for trade
};


enum LpsSaLoadRecordTargetType : uint8_t {
    SINGLE = 0,
    SPLIT = 1,
    BATCH = 2,
    MIX = 3,
    BLEND_LOADS = 4,
    BLEND_WEIGHT = 5,
};


#define LpsSaLoadRecordStoreAction_Base_t(val) (static_cast<typename std::underlying_type<LpsSaLoadRecordStoreAction>::type>(val))


#endif /* LPSSALOADRECORD_INTERFACECOMMON_H */
