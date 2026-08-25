#ifndef LPSSAUI_BEMSIMSTATEINTERFACE_HPP
#define LPSSAUI_BEMSIMSTATEINTERFACE_HPP

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include "../../../infrastructure/aisJhm/aisJhm2/AisJhm2RequestProcessor/BEMSim/BEMSimCnfgStorage.h"

/*
 * The interface storage class.
 */
class LpsSaUIBEMSimStateInterfaceStorage : public csvable  {
public:
    LpsSaUIBEMSimStateInterfaceStorage() :
        cnfg(),
        chargeLevel(0.0f),
        dischargeRate(0.0f),
        totalFuelUsed(0.0f),
        derateEngineCmd(false),
        derateEngineRPM(BEM_SIM_ENGINE_DERATE_LIMIT_RPM),
        chargeRate(0.0f),
        timeToCharged(0.0f),
        timeToDischarged(0.0f) {}

    BEMSimCnfgStorage cnfg;
    float chargeLevel;
    float dischargeRate;
    float totalFuelUsed;
    bool  derateEngineCmd;
    int   derateEngineRPM;
    float chargeRate;
    float timeToCharged;
    float timeToDischarged;

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & cnfg;
        ar & chargeLevel;
        ar & dischargeRate;
        ar & totalFuelUsed;
        ar & derateEngineCmd;
        ar & derateEngineRPM;
        ar & chargeRate;
        ar & timeToCharged;
        ar & timeToDischarged;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<LpsSaUIBEMSimStateInterfaceStorage> LpsSaUIBEMSimStateInterface;

BOOST_CLASS_VERSION(LpsSaUIBEMSimStateInterfaceStorage, 0);

#endif /* LPSSAUI_BEMSIMSTATEINTERFACE_HPP */
