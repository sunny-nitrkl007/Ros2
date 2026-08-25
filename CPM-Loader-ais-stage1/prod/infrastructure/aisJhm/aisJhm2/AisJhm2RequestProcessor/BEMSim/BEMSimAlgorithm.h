/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: BEMSimAlgorithm.h
--------------------------------------------------------------------------------
DESCRIPTION:This file provides the routines for Batter Electric Machine (BEM) Simulator.
The BEM simulator is an applet used to estimate energy used by the engine and
convert it to battery discharge rate.

Note: Attempt to make BEM Simulator platform independent
	  to allow unit testing off target.  i.e. using only standard libs
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef AISJHMBEMSIMALG_H
#define AISJHMBEMSIMALG_H

#include <array>
#include <chrono>
#include "BEMSimCnfgStorage.h"

#include <ctime> 

class BEMSimAlgorithm {
public:
	using Clock = std::chrono::system_clock;
	using TimePoint = Clock::time_point;

    typedef enum {
        LINEAR = 0,
        NMC
    } BatteryType;

	BEMSimAlgorithm():
	    init_charging_time_table(true),
		F_start(0.0f), // liters
		K_sfc(3.1f), // kWh/L
		Q_start(0.0f), // kWh
		Q_current(0.0f), // kWh
		Q_current_usable_pct(0.f), // %
		Q_max(0.0f), // kWh
		D_current(0.0f), // kW
		D_last(11.0f), // kW
		T_start(TimePoint::min()),
		TT_charge(0.f), // minutes
		TT_discharge(0.f), // minutes
		C_cap(22.0f), // kW
		C_rate(0.0f), // kW
		C_mode(true), // Charging
		battery_type(BatteryType::NMC),
		reset_discharge_rate_est(true),
        Q_discharge_rate_est_start(0.f),
        T_discharge_rate_est_start(TimePoint::min()),
        charge_rate_table{0.f},
        battery_map_time_table{0.f} {}


    void initChargingMode(bool c_mode,
                          float start_q,
                          float d_last,
                          double start_fuel,
                          TimePoint start_time);

    void update(double total_fuel_used);

    void setChargerCapacity(float rate, float efficiency=1.f) {
        float actualRate = rate * efficiency;
        if (C_cap != actualRate) {
            C_cap = actualRate;
            init_charging_time_table = true;
        }
    }

    void setBatteryType(BatteryType type) {
        if (battery_type != type) {
            battery_type = type;
            init_charging_time_table = true;
        }
    }

    void setBatteryCapacity(float battery_cap) {
        if (Q_max != battery_cap) {
            Q_max = battery_cap;
            init_charging_time_table = true;
        }
    }

    float getBatteryCapacity() { return Q_max;}

    void resetStartCharge() {
        Q_start = Q_current;
        T_start = Clock::now();
    }

    void setSpecificFuelConsuption(float k) { K_sfc = k; }

    TimePoint getStartTime() const { return T_start; }

    float getLastDischargeRate() const { return D_last; }

    float getCurrentCharge() const {return Q_current;}

    void setCurrentChargePercentUsable(float soc) { Q_current = ((calcPhysicalPercentCharge(soc)/100.0)*Q_max); }

    float getCurrentChargePercentUsable() const { return Q_current_usable_pct; }

    float getEstDischargeRate() const { return D_current; }

    float getTimeToDischarged() const { return TT_discharge; }

    float getChargeRate() const { return C_rate; }

    float getTimeToCharged() const { return TT_charge; }

private:
    bool init_charging_time_table;
	double F_start; // Fuel Used Start (liters)
	float K_sfc; // Specific Fuel Consumption (kWh/L)
	float Q_start; // Charge Level start (kWh)
	float Q_current; // Physical Charge Level current (kWh)
	float Q_current_usable_pct; // Usable State of Charge (%)
	float Q_max; // Battery Capacity (kWh)
	float D_current; // Discharge Rate current (kWh/hour = kW)
	float D_last; // Discharge Rate last (kWh/hour = kW)
	TimePoint T_start; // Time at start of Charge Mode Change (seconds)
	float TT_charge; // Time to charged (minutes)
	float TT_discharge; // Time to discharged (minutes)
	float C_cap; // Charger Capacity (kWh/h = kW)
	float C_rate; // Charger Rate Current (kWh/h = kW)
	bool C_mode; // Charge Mode (CHARGING = true or DISCHARGING = false)
	BatteryType battery_type; // Battery Type NMC (curved) LINEAR (linear)

	bool reset_discharge_rate_est; // On initialization or mode change, the discharge rate estimate should be reset
	float Q_discharge_rate_est_start; // Start charge for the discharge rate estimate (kWh)
	TimePoint T_discharge_rate_est_start; // Start time for the discharge rate estimate

	static constexpr std::size_t battery_map_size = 21;
	static constexpr std::array<float, battery_map_size> battery_map_soc = { 0.f, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100 };
	static constexpr std::array<float, battery_map_size> battery_map_c_rate_linear = { 1.f, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	static constexpr std::array<float, battery_map_size> battery_map_c_rate_nmc = { 1.273, 1.273, 1.273, 1.273, 1.273, 1.273, 1.273, 1.273, 1.273, 1.273, 1.273, 1.212, 1.152, 1.091, 0.822, 0.552, 0.283, 0.256, 0.229, 0.202, 0 };

	static constexpr float battery_max_usable_soc = 95.f; // Represents 100% usable battery
	static constexpr float battery_min_usable_soc = 10.f; // Represents 0% usable battery

	std::array<float, battery_map_size> charge_rate_table;
	std::array<float, battery_map_size> battery_map_time_table;

	void initChargingTimeTable();

	float calcUsablePercentCharge(float q) const;
	float calcPhysicalPercentCharge(float usable_soc_pct) const;
};

#endif //AISJHMBEMSIMALG_H
