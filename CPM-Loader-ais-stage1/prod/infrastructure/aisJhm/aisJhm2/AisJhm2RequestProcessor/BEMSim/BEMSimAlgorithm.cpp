/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: BEMSimAlgorithm.cpp LpsJobMgrJobTrackerInfoTbl
--------------------------------------------------------------------------------
FILE NAME: BEMSimAlgorithm.cpp
DESCRIPTION:This file provides the routines for Batter Electric Machine (BEM) Simulator.
The BEM simulator is an applet used to estimate energy used by the engine and
convert it to battery discharge rate.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <stdio.h>
#include <array>
#include <algorithm>
#include <tuple>

#include <LpsCommonTypeDef.h>
#include <LpsCommonLookUpTblUtility.h>

#include "BEMSimAlgorithm.h"

// Calculate the time delta in hours
static float timeDelta_(const BEMSimAlgorithm::TimePoint& t0, const BEMSimAlgorithm::TimePoint& t1) {
    auto delta_seconds = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count();
    return delta_seconds / 3600.f;
}

// static member definition...
constexpr std::size_t BEMSimAlgorithm::battery_map_size;
constexpr std::array<float, BEMSimAlgorithm::battery_map_size> BEMSimAlgorithm::battery_map_soc;
constexpr std::array<float, BEMSimAlgorithm::battery_map_size> BEMSimAlgorithm::battery_map_c_rate_linear;
constexpr std::array<float, BEMSimAlgorithm::battery_map_size> BEMSimAlgorithm::battery_map_c_rate_nmc;
constexpr float BEMSimAlgorithm::battery_max_usable_soc;
constexpr float BEMSimAlgorithm::battery_min_usable_soc;


void BEMSimAlgorithm::initChargingMode(bool c_mode,
                                       float start_q,
                                       float d_last,
                                       double start_fuel,
                                       TimePoint start_time) {

    C_mode = c_mode;

    D_last = d_last;
    D_current = d_last;

    if (start_q > (Q_max*(battery_max_usable_soc/100.0f))) {
        Q_start = (Q_max*(battery_max_usable_soc/100.0f));
    }
    else {
        Q_start = start_q;
    }

    Q_current = Q_start;

    Q_current_usable_pct = calcUsablePercentCharge(Q_current);

    T_start = start_time;
    TT_charge = 0.f;
    TT_discharge = 0.f;

    F_start = start_fuel;

    C_rate = C_cap;

    reset_discharge_rate_est = true;
    Q_discharge_rate_est_start = Q_start;
    T_discharge_rate_est_start = T_start;
}

void BEMSimAlgorithm::update(double total_fuel_used) {
    auto time_delta_hours = timeDelta_(T_start, Clock::now());

    if (init_charging_time_table) {
        initChargingTimeTable();
        init_charging_time_table = false;
    }

    if (C_mode) {
        // CHARGING

        // calcCurrentChargeLevel()
        float start_charge_pct = Q_start*100.f/Q_max;
        float start_time_ref_hours = LpsLookup(start_charge_pct, battery_map_soc.data(), battery_map_time_table.data(), battery_map_soc.size());
        float current_time_ref_hours = start_time_ref_hours + time_delta_hours;
        float charge_pct = LpsLookup(current_time_ref_hours, battery_map_time_table.data(), battery_map_soc.data(), battery_map_soc.size());

        C_rate = LpsLookup(charge_pct, battery_map_soc.data(), charge_rate_table.data(), charge_rate_table.size());

        Q_current = charge_pct*Q_max/100.f;

        if (Q_current <= (Q_max*(battery_min_usable_soc/100.0f))) {
            Q_current = (Q_max*(battery_min_usable_soc/100.0f));
        }
        else if (Q_current > (Q_max*(battery_max_usable_soc/100.0f))) {
            Q_current = (Q_max*(battery_max_usable_soc/100.0f));
        }

        // Calculate time to charge from current charge level.
        float end_time_ref_hours = LpsLookup(battery_max_usable_soc, battery_map_soc.data(), battery_map_time_table.data(), battery_map_soc.size());
        TT_charge = (end_time_ref_hours - current_time_ref_hours) * 60.f;

        if (TT_charge <= 0.f) {
            TT_charge = 0.f;
        }

        // calcEstDischargeRate()
        D_current = D_last;
    }
    else {
        // DISCHARGING

        if ((total_fuel_used > 0) || (total_fuel_used > F_start))
        {
            float charge_loss_kwh = K_sfc * (total_fuel_used - F_start);

            Q_current = Q_start - charge_loss_kwh;

            if (Q_current < (Q_max*(battery_min_usable_soc/100.0f))) {
                Q_current = Q_max*(battery_min_usable_soc/100.0f);
            }
            else if (Q_current > (Q_max*(battery_max_usable_soc/100.0f))) {
                Q_current = (Q_max*(battery_max_usable_soc/100.0f));
            }

            if (reset_discharge_rate_est) {
                // These are used to estimate the discharge rate during this power-up
                Q_discharge_rate_est_start = Q_current;
                T_discharge_rate_est_start = Clock::now();
                reset_discharge_rate_est = false;
            }

            { // Update estimated discharge rate.
                float charge_loss_discharge_rate_est = (Q_discharge_rate_est_start - Q_current);
                float time_delta_hours_discharge_rate_est = timeDelta_(T_discharge_rate_est_start, Clock::now());

                if ((Q_current <= 0.0f) || (charge_loss_discharge_rate_est < 0)) {
                    D_current = 0.0f;
                }
                else {
                    D_current = (charge_loss_discharge_rate_est / time_delta_hours_discharge_rate_est);
                }

                if (((charge_loss_discharge_rate_est / Q_max) < (0.05f)) || (D_current <= 0.0f)) {
                    D_current = D_last;
                }
                else {
                    D_last = D_current;
                }
            }
        }

        // Calculate time to discharge from current charge level
        if (D_current <= 0.f) {
            TT_discharge = 10000.f; // Invalid high
        }
        else {
            // Time to minimum usable SOC (State Of Charge)
            TT_discharge = ((Q_current - (Q_max * battery_min_usable_soc / 100.f)) * 60.f) / D_current;

            if (TT_discharge <= 0.f) {
                TT_discharge = 0.f;
            }
            else if (TT_discharge >= 9999.f) {
                TT_discharge = 9999.f; // Valid high
            }
        }
    }

    // Update usable state of charge
    Q_current_usable_pct = calcUsablePercentCharge(Q_current);
}

void BEMSimAlgorithm::initChargingTimeTable() {

    { // Calculate the charge rate table
        std::array<float, battery_map_size> temp_table;

        switch(battery_type) {
            case BatteryType::NMC:
                temp_table = battery_map_c_rate_nmc;
                break;
            case BatteryType::LINEAR:
            default:
                temp_table = battery_map_c_rate_linear;
                break;
        }

        for (auto& rate : temp_table) {
            float max_rate = rate * Q_max; // Max battery charge rate, kW
            rate = (max_rate >= C_cap) ? C_cap : max_rate; // Limit to charger capacity, kW
        }

        charge_rate_table = temp_table;
    }

    // Check to make sure the arrays are of expected sizes.
    static_assert(battery_map_soc.size() > 1, "SOC array must have more that 1 point.");
    static_assert(battery_map_soc.size() == std::tuple_size<decltype(battery_map_time_table)>::value, "SOC array size and time table array size must be equal.");

    // Make sure we start at 0,0
    static_assert(battery_map_soc[0] == 0.f, "First SOC array point must be 0.");
    battery_map_time_table[0] = 0;

    for (unsigned int ii = 1; ii < battery_map_soc.size(); ++ii) {
        float delta_time_hr;
        float delta_charge_kwh = (battery_map_soc[ii] - battery_map_soc[ii-1]) * Q_max / 100.f;

        if (ii < charge_rate_table.size()) {
            delta_time_hr = delta_charge_kwh * 2.f / (charge_rate_table[ii] + charge_rate_table[ii-1]);
        }
        else {
            delta_time_hr = delta_charge_kwh / C_cap;
        }

        battery_map_time_table[ii] = battery_map_time_table[ii-1] + delta_time_hr;
    }
}

float BEMSimAlgorithm::calcUsablePercentCharge(float q) const {
    float physical_soc_pct = q * 100.f / Q_max;

    float usable_soc_pct;

    if (physical_soc_pct <= battery_min_usable_soc) {
        usable_soc_pct = 0.f;
    }
    else if (physical_soc_pct >= battery_max_usable_soc) {
        usable_soc_pct = 100.f;
    }
    else {
        usable_soc_pct = (physical_soc_pct - battery_min_usable_soc) * 100.f / (battery_max_usable_soc - battery_min_usable_soc);
    }

    return usable_soc_pct;
}

float BEMSimAlgorithm::calcPhysicalPercentCharge(float usable_soc_pct) const {
    float physical_soc_pct;

    physical_soc_pct = (((usable_soc_pct * (battery_max_usable_soc - battery_min_usable_soc))/100.0f) + battery_min_usable_soc);

    return physical_soc_pct;

}

