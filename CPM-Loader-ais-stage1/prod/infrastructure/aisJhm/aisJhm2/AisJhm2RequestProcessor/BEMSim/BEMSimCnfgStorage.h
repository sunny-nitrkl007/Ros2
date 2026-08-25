/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: BEMSimCnfgStorage.h
--------------------------------------------------------------------------------
DESCRIPTION:This file provides the routines for Batter Electric Machine (BEM) Simulator.
The BEM simulator is an applet used to estimate energy used by the engine and
convert it to battery discharge rate.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef AISJHMBEMSIMCNFGSTOR_H
#define AISJHMBEMSIMCNFGSTOR_H

#include <string>
#include <chrono>
#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/filesystem.hpp>
#include <boost_helpers/boost_serialization_chrono.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/algorithm/string.hpp>
#include <fileio/sha1_fstream.hpp>
#include <fileio/directory.hpp>
#include <ais/log/Logger.h>
#include <ais/serialization/Datum.h>

static const int BEM_SIM_ENGINE_DERATE_LIMIT_RPM = 1300;

/*
 * BEMSim Saved Configuration Storage
 */
class BEMSimCnfgStorage
{
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

	BEMSimCnfgStorage() :
	    chargeMode(true),
	    startCharge(0.0f),
	    chargeModeStartTime(TimePoint::min()),
	    startFuelUsed(0.0f),
	    lastDischargeRate(34.1f),
	    bemSimEnabled(false),
	    chargerCapacity(22.0f),
	    engineDerateEnabled(false),
	    newMachine(false),
        savePeriodCount_(0),
        savePeriodSubCycles_(300)  {}

    void reset() {
         *this = BEMSimCnfgStorage();
    }

    bool load(const boost::filesystem::path& file_path);
    bool save(const boost::filesystem::path& file_path) const;

    void saveCnfgScheduler(const boost::filesystem::path& file_path);

    void logStorage();

    // Derive savePeriodSubCycle_ value from update cycle in seconds.
    void setSaveConfigPeriod(float update_period) {
        savePeriodSubCycles_ = static_cast<uint32_t>(savePeriodTime_/update_period);
    }

    // BEM States
    bool   chargeMode;             // charge mode true is charging false is discharging
    float  startCharge;            // charge level at start of mode change
    TimePoint chargeModeStartTime; // TimePoint at mode change
    double startFuelUsed;          // fuel used at start of mode change in Liters
    float  lastDischargeRate;      // last discharge rate at mode change (minutes)
    // UI Setting Inputs/Outputs
    bool   bemSimEnabled;        // BEM Simulator feature enabled
    float  chargerCapacity;      // charger Energy capacity (kWh/h)
    bool   engineDerateEnabled;  // Enable engine de-rate threshold

    bool newMachine; // Assumed new Machine

private:

    // period variables to run save info less frequently in host save thread
    // savePeriodTime_ = (savePeriodSubCycles_)/(host thread period)
    float savePeriodTime_ = 600.0f; // period Time in Seconds (10 Minutes - 600 seconds)
    uint32_t savePeriodCount_;      // period running loop count to subdivide the host thread period
    uint32_t savePeriodSubCycles_;  // period subdivided cycle value

	// STORED to NVM serial data file
	friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar & chargeMode;
        ar & startCharge;
        ar & chargeModeStartTime;
        ar & startFuelUsed;
        ar & lastDischargeRate;
        ar & bemSimEnabled;
        ar & chargerCapacity;
        ar & engineDerateEnabled;
    }
};

BOOST_CLASS_VERSION(BEMSimCnfgStorage, 1);

#endif // AISJHMBEMSIMCNFGSTOR_H
