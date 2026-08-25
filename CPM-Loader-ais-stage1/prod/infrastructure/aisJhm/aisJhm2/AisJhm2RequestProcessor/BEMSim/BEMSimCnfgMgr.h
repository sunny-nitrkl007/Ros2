/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: BEMSimCnfgMgr.h

DESCRIPTION:This file provides the CLASS for Batter Electric Machine (BEM) Simulator.
The BEM simulator is an applet used to estimate energy used by the engine and
convert it to battery discharge rate.

- Object BEMSimCnfgStorage

The BEMSim logic simulates a battery powered machine with a fuel powered machine.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef AISJHMBEMSIMCNFGMGR_H
#define AISJHMBEMSIMCNFGMGR_H

#include "BEMSimCnfgStorage.h"
#include "BEMSimAlgorithm.h"

enum class BEMSimEventLatchState{
    UNKNOWN = 0,   // uninitialized
    SET,           // Event active latch
    CLEARED        // Event cleared latch
};

class BEMSimCnfgMgr
{
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

	BEMSimCnfgMgr() :
	    calc(),
	    cnfg(),
	    totalFuelUsedIsOK_(false),
		totalFuelUsed_(0.f),
		updatePeriodTime_(0.5f), // 500 ms
		updatePeriodCount_(0),
		updatePeriodSubCycles_(10),
		chargeLevelWarningTheshold_(20.5f), // 20.5% (half percent added to match rounding in UI)
		chargeLevelDerateThreshold_(10.5f), // 10.5% (half percent added to match rounding in UI)
		latchSetDerateEvent_(BEMSimEventLatchState::UNKNOWN),
		latchSetWarningEvent_(BEMSimEventLatchState::UNKNOWN) {}

    BEMSimAlgorithm   calc;

    BEMSimCnfgStorage cnfg;

    void initialize();

    void updateInfo();

    void updateEvents(bool &set_warning, bool &clr_warning, bool &set_below_threshold, bool &clr_below_threshold);

    void setConstInit(float bat_cap, float k, std::string bat_model);

    void setChargeMode(bool charge_mode, bool force=false);

    void setChargeLevel(float soc);

    void setBEMSimEnable(bool charge_mode);

    float getChargeLevel() const { return calc.getCurrentChargePercentUsable(); }

    float getDischargeRate() const { return calc.getEstDischargeRate(); }

    float getChargeRate() const { return calc.getChargeRate(); }

    float getTimeToDischarged() const { return calc.getTimeToDischarged(); }

    float getTimeToCharged() const { return calc.getTimeToCharged(); }

    void setChargerCapacity(float cap);

    float getDerateLevelWarning() const { return chargeLevelWarningTheshold_; }

    float getDerateLevelThreshold() const { return chargeLevelDerateThreshold_; }

    double getTotalFuelUsed() { return totalFuelUsed_; }

    void setTotalFuelUsed(double fuel) { totalFuelUsed_ = fuel; }

    void setTotalFuelUsedIsOK(bool is_ok) {totalFuelUsedIsOK_ = is_ok; }

    BEMSimCnfgStorage& getCnfg() { return cnfg; }

    double calcTestFuelUsed() {
        double fuel_used =  totalFuelUsed_ + 16.15f/3600;  // 11.0 L/h is average

        if (fuel_used < cnfg.startFuelUsed) {
            fuel_used = cnfg.startFuelUsed + fuel_used;
        }
        return fuel_used;
    }

    bool getDerateEngineCmd() {
        bool derate = false;

        if (cnfg.bemSimEnabled && cnfg.engineDerateEnabled) {
            if (BEMSimEventLatchState::SET == latchSetDerateEvent_) {
                // Discharged too much
                derate = true;
            }
            else if (cnfg.chargeMode) {
                // Charging
                derate = true;
            }
        }

        return derate;
    }

    inline int getDerateEngineRPM() {
        return (BEM_SIM_ENGINE_DERATE_LIMIT_RPM);
    }

    const std::vector<float>& getChargeCapacityList() const {return chargeCapacityList_; }

    // Derive updatePeriodSubCycle_ value from host thread rate in Hz.
    void setUpdateInfoPeriod(float rate_hz) {
        updatePeriodSubCycles_ = static_cast<uint32_t>((rate_hz)*updatePeriodTime_);
    }

    // Private scheduler to run update info less frequently in host update thread
    // updatePeriodTime_ = (updatePeriodSubCycles_)/(host thread period)
    inline bool updateInfoScheduler() {
        bool update = false;

        //INCREMENT period count for each host cycle
        ++updatePeriodCount_;
        if (updatePeriodCount_ >= updatePeriodSubCycles_) {

            // Publish State data.
            update = true;

            // RESET period count to start new subcycle
            updatePeriodCount_ = 0;
        }

        return(update);
    }

private:

    bool totalFuelUsedIsOK_;

    double totalFuelUsed_; // Total Fuel used in Liters

    const float chargerEfficiency_ = 0.88f; // % efficiency
    const std::vector<float> chargeCapacityList_{22.0f, 60.0f, 120.0f, 200.0f}; // (kWh/h)

    // period variables to run update info less frequently in host update thread
    // updatePeriodTime_ = (updatePeriodSubCycles_)/(host thread period)
    const float updatePeriodTime_; // period Time in Seconds (500ms)
    uint32_t updatePeriodCount_; // period running loop count to subdivide the host thread period
    uint32_t updatePeriodSubCycles_; // period subdivided cycle value

    const float chargeLevelWarningTheshold_; // Charge Level percentage Threshold to warn of de-rating engine
    const float chargeLevelDerateThreshold_; // Charge Level percentage Threshold to de-rate engine

    BEMSimEventLatchState latchSetDerateEvent_;
    BEMSimEventLatchState latchSetWarningEvent_;


};

#endif //end if AISJHMBEMSIMCNFGMGR_H
