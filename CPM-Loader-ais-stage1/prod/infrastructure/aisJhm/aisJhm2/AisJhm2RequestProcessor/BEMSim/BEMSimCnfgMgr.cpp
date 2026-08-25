/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: BEMSimCnfgMgr.cpp
--------------------------------------------------------------------------------
DESCRIPTION:This file provides the routines for Batter Electric Machine (BEM) Simulator.
The BEM simulator is an applet used to estimate energy used by the engine and
convert it to battery discharge rate.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include "BEMSimCnfgMgr.h"
/*
 *  CALLED on reset condition
 *  IMPORTANT: Order dependent function calls
 *
 *  - Initialize NVM files and sync storage
 *  - Get static config data from Ruby file.
 *  - Get NVM parameters and set Algorithm  initial condition
 *  - UPDATE ALL internal states and output levels
 */
void BEMSimCnfgMgr::initialize()
{
    // Default to 100% SOC max battery capacity for brand new Machine
    if (cnfg.newMachine) {
        calc.setCurrentChargePercentUsable(100.f);
        setChargeMode(true /* Charging */, true /* Force Mode Change */);
    }
    else {
        // Initialize the charging mode
        calc.initChargingMode(cnfg.chargeMode,
                cnfg.startCharge,
                cnfg.lastDischargeRate,
                cnfg.startFuelUsed,
                cnfg.chargeModeStartTime);
    }

    // Set the charger capacity
    calc.setChargerCapacity(cnfg.chargerCapacity, chargerEfficiency_);

    // Update ALL internal states and output levels
    updateInfo();

    // SET saveCnfg cycle period.
    cnfg.setSaveConfigPeriod(updatePeriodTime_);

}

void BEMSimCnfgMgr::setConstInit(float bat_cap, float k, std::string bat_model) {

    if (bat_cap == 0.0f) {
        calc.setBatteryCapacity(250.0f);
    }
    else {
        calc.setBatteryCapacity(bat_cap);
    }

    if (k == 0.0f) {
        calc.setSpecificFuelConsuption(3.1f);
    }
    else {
        calc.setSpecificFuelConsuption(k);
    }

    boost::algorithm::to_lower(bat_model);
    if (bat_model == "nmc") {
        calc.setBatteryType(BEMSimAlgorithm::NMC);
    }
    else {  // (bat_model == "linear") and Default
        calc.setBatteryType(BEMSimAlgorithm::LINEAR);
    }

    AIS_LOG_INFO("Init Battery Cap=%f, K=%f, Type = %s",bat_cap, k, bat_model.c_str());

}

void BEMSimCnfgMgr::setChargeMode(bool charge_mode, bool force) {

    if((!charge_mode) && (!totalFuelUsedIsOK_)) {
		//DISCHARGING and fule is not OK!
        return;
    }

	if ((charge_mode != cnfg.chargeMode) || (force)) {

	    // SET stored Charge Mode
	    cnfg.chargeMode = charge_mode;

        // SET stored Charge Start
	    float start_charge = calc.getCurrentCharge();
        cnfg.startCharge = start_charge;

	    // SET stored last discharge Rate
        float last_discharge_rate = calc.getLastDischargeRate();
		cnfg.lastDischargeRate = last_discharge_rate;

        // Set Fuel used from storage
		cnfg.startFuelUsed = totalFuelUsed_;
        double start_fuel = totalFuelUsed_;

		// GET current time
		TimePoint start_tp = Clock::now();

		// SET  Start time in storage
		cnfg.chargeModeStartTime = start_tp;

		// INIT Algorithm
		calc.initChargingMode(charge_mode,
                              start_charge,
                              last_discharge_rate,
                              start_fuel,
                              start_tp);
	}
}

// Allows test access to set physical charge level.
void BEMSimCnfgMgr::setChargeLevel(float soc) {
    calc.setCurrentChargePercentUsable(soc);
    bool force = true;
    setChargeMode(cnfg.chargeMode, force);
}

/*
 * Set Charge Rate --
 *
 * Need to reset start charge level with last charger capacity.
 * This avoids a drop in current charge since start charge and
 * charge capacity are used to calculate current charge when
 * charging. If discharging let mode change update start charge.
 *
 * IMPORTNAT order dependent function calls
 *
 */
void BEMSimCnfgMgr::setChargerCapacity(float capacity) {

    if (capacity != cnfg.chargerCapacity) {

        // SET stored charger capacity
        cnfg.chargerCapacity = capacity;

        // SET new charger capacity for next current charge update
        calc.setChargerCapacity(capacity, chargerEfficiency_);

        // IF charging reset start charge in algorithm and storage
        if (cnfg.chargeMode == true) {
            // CHARGING

            // RESET start from current charge with last charger capacity
            calc.resetStartCharge();

            // RESET stored start charge
            cnfg.startCharge = calc.getCurrentCharge();
            cnfg.chargeModeStartTime = calc.getStartTime();
        }
    }
}

void BEMSimCnfgMgr::setBEMSimEnable(bool enable) {
    cnfg.bemSimEnabled = enable;

    if(enable) {
        calc.setCurrentChargePercentUsable(100.f);
        setChargeMode(true /* Charging */, true /* Force Mode Change */);
    }
}

/*
 *  Update Internal BEM Simulator states
 *  and UI Info request parameters.
 *
 *  IMPORTANT: Order dependent function calls
 *
 */
void BEMSimCnfgMgr::updateInfo() {

    if (cnfg.bemSimEnabled) {
        // Update the algorithm
        calc.update(totalFuelUsed_);

        // SET stored last discharge Rate
        cnfg.lastDischargeRate = calc.getLastDischargeRate();
    }
}

/*
 *  Update Event BEM Simulator states
 *  and UI Info request parameters.
 *
 *  CHARGING always CLEARS below derate threshold.
 *  Event will re-set when discharging and threshold is crossed.
 *
 */

void BEMSimCnfgMgr::updateEvents(bool &set_warning, bool &clr_warning, bool &set_derate, bool &clr_derate) {

    if (cnfg.bemSimEnabled && !cnfg.chargeMode) {
        // BEM SIM ENABLED
        // and DERATE ENABLED
        // and DISCHARGING
        float charge_level = calc.getCurrentChargePercentUsable();

        // SET Events when CLEARED or UNKNOWN (uninitialized)
        // and in below threshold
        if (charge_level < chargeLevelDerateThreshold_) {
            // Recharge Now
            if (latchSetDerateEvent_ != BEMSimEventLatchState::SET) {
                set_derate = true;
                latchSetDerateEvent_ = BEMSimEventLatchState::SET;
            }

            if (latchSetWarningEvent_ != BEMSimEventLatchState::CLEARED) {
                clr_warning = true;
                latchSetWarningEvent_ = BEMSimEventLatchState::CLEARED;
            }
        }
        else if (charge_level <  chargeLevelWarningTheshold_)  {
            // Recharge Soon
            if (latchSetWarningEvent_ != BEMSimEventLatchState::SET) {
                set_warning = true;
                latchSetWarningEvent_ = BEMSimEventLatchState::SET;
            }
        }
        else {
            // CLEAR Events when SET or UNKNOWN (uninitialized)
            if(latchSetWarningEvent_ != BEMSimEventLatchState::CLEARED) {
                clr_warning = true;
                latchSetWarningEvent_ = BEMSimEventLatchState::CLEARED;
            }

            if(latchSetDerateEvent_ != BEMSimEventLatchState::CLEARED) {
                clr_derate = true;
                latchSetDerateEvent_ = BEMSimEventLatchState::CLEARED;
            }
        }
    }
    else
    {
        // CHARGING 
        // OR DERATE DISABLED
        // OR BEM SIM DISABLED

        // CLEAR Events when SET or UNKNOWN (uninitialized)
        if(latchSetWarningEvent_ != BEMSimEventLatchState::CLEARED) {
            clr_warning = true;
            latchSetWarningEvent_ = BEMSimEventLatchState::CLEARED;
        }

        if(latchSetDerateEvent_ != BEMSimEventLatchState::CLEARED) {
            clr_derate = true;
            latchSetDerateEvent_ = BEMSimEventLatchState::CLEARED;
        }
    }
}
