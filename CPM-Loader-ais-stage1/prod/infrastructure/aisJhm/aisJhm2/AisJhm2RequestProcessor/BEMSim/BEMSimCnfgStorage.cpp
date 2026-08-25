/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: BEMSimCnfgStorage.cpp
--------------------------------------------------------------------------------
DESCRIPTION:This file provides the routines for Batter Electric Machine (BEM) Simulator.
The BEM simulator is an applet used to estimate energy used by the engine and
convert it to battery discharge rate.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/

#include "BEMSimCnfgStorage.h"
#include <ctime>

/*
 * Load configuration from storage
 */
bool BEMSimCnfgStorage::load(const boost::filesystem::path& file_path) {
    bool success = false;

    tes_common_ais::sha1_ifstream ifs{ true }; // two_copy
    ifs.open(file_path.string(), std::ios_base::in | std::ios_base::binary);
    if (ifs) {
        // Read the file in
        try {
            boost::archive::binary_iarchive ia(ifs);
            ia >> *this;
            AIS_LOG_INFO("Loaded BEM Sim configuration from storage.");
            success = true;

            AIS_LOG_INFO("BEMSimCnfgStorage::load()"); 
            logStorage();
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not deserialize BEM Sim configuration from storage.");
            AIS_LOG_ERROR(e.what());
            reset();
        }
        catch (const std::exception& e) {
            AIS_LOG_ERROR("Could not deserialize BEM Sim configuration from storage.");
            AIS_LOG_ERROR(e.what());
            reset();
        }
        catch (...) {
            AIS_LOG_ERROR("Could not deserialize BEM Sim configuration from storage, unexpected error.");
            reset();
        }

        ifs.close();

        if (ifs.is_corrupt()) {
            if (ifs.fix_it()) {
            	AIS_LOG_ERROR("BEM Sim configuration file was corrupt... fixed it.");
            }
            else {
                AIS_LOG_ERROR("BEM Sim configuration file was corrupt... could not fix it.");
            }
        }
    }
    else {
        AIS_LOG_ERROR("BEM Sim configuration file could not be opened from storage.");

        // Assuming new Machine or corrupted NVM file.
        newMachine = true;
    }

    return success;
}

/*
 * Save configuration to storage
 */
bool BEMSimCnfgStorage::save(const boost::filesystem::path& file_path) const {

    bool success = false;
    tes_common_ais::sha1_ofstream ofs{ true }; // two_copy
    ofs.open(file_path.string(), std::ios_base::out | std::ios_base::binary);
    if (ofs) {
        // Write to the file
        try {
            boost::archive::binary_oarchive oa(ofs);
            oa << *this;
            AIS_LOG_INFO("Saved BEM Sim configuration to storage.");
            success = true;

        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not serialize BEM Sim configuration to storage.");
            AIS_LOG_ERROR(e.what());
        }

        ofs.close();
    }
    else {
        AIS_LOG_ERROR("BEM Sim configuration could not be opened from storage.");
    }

    return success;
}

// Private scheduler to run save info less frequently in host thread
// savePeriodTime_ = (savePeriodSubCycles_)*(update period)
void BEMSimCnfgStorage::saveCnfgScheduler(const boost::filesystem::path& file_path) {

    //INCREMENT period count for each host cycle
    ++savePeriodCount_;
    if ((savePeriodCount_ >= savePeriodSubCycles_) && (bemSimEnabled == true)) {

        // Save Config data.
        save(file_path);

        // RESET period count to start new subcycle
        savePeriodCount_ = 0;
    }
}

void BEMSimCnfgStorage::logStorage () {

    AIS_LOG_INFO("chargeMode =  %d",chargeMode);
    AIS_LOG_INFO("startCharge =  %4.2f kWh",startCharge);
    AIS_LOG_INFO("startFuelUsed =  %4.2f L",startFuelUsed);
    AIS_LOG_INFO("lastDischargeRate =  %4.2f kWh/h",lastDischargeRate);
    AIS_LOG_INFO("bemSimEnabled =  %d",bemSimEnabled);
    AIS_LOG_INFO("chargerCapacity = %4.2f kWh/h", chargerCapacity);
    AIS_LOG_INFO("engineDerateEnabled =  %d",engineDerateEnabled);
    std::time_t t_c =  Clock::to_time_t(chargeModeStartTime);
    AIS_LOG_INFO("chargeModeStartTime =  %s", ctime(&t_c));

}
