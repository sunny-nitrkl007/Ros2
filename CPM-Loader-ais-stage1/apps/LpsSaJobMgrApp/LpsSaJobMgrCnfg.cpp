#include <boost/filesystem.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>

#include <ais/log/Logger.h>

#include <fileio/sha1_fstream.hpp>

#include "LpsSaJobMgrCnfg.h"

namespace fs = boost::filesystem;

/*
 * Load configuration from storage
 */
bool LpsSaJobMgrCnfg::load(const fs::path& filePath) {
    bool success = false;
    tes_common_ais::sha1_ifstream ifs{ true }; // two_copy
    ifs.open(filePath.string(), std::ios_base::in | std::ios_base::binary);
    if (ifs) {
        // Read the file in
        try {
            boost::archive::binary_iarchive ia(ifs);
            ia >> *this;
            AIS_LOG_INFO("Loaded job manager configuration from storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not deserialize job manager configuration from storage.");
            AIS_LOG_ERROR(e.what());
            reset();
        }
        catch (const std::exception& e) {
            AIS_LOG_ERROR("Could not deserialize job manager configuration from storage.");
            AIS_LOG_ERROR(e.what());
            reset();
        }
        catch (...) {
            AIS_LOG_ERROR("Could not deserialize job manager configuration from storage, unexpected error.");
            reset();
        }

        ifs.close();

        if (ifs.is_corrupt()) {
            if (ifs.fix_it()) {
                AIS_LOG_WARN("Job manager configuration file was corrupt... fixed it.");
            }
            else {
                AIS_LOG_ERROR("Job manager configuration file was corrupt... could not fix it.");
            }
        }
    }
    else {
        AIS_LOG_ERROR("Job manager configuration file could not be opened from storage.");
    }

    return success;
}

/*
 * Save configuration to storage
 */
bool LpsSaJobMgrCnfg::save(const fs::path& filePath) const {
    bool success = false;
    tes_common_ais::sha1_ofstream ofs{ true }; // two_copy
    ofs.open(filePath.string(), std::ios_base::out | std::ios_base::binary);
    if (ofs) {
        // Read the file in
        try {
            boost::archive::binary_oarchive oa(ofs);
            oa << *this;
            AIS_LOG_INFO("Saved job manager configuration to storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not serialize job manager configuration to storage.");
            AIS_LOG_ERROR(e.what());
        }

        ofs.close();
    }
    else {
        AIS_LOG_ERROR("Job manager configuration could not be opened from storage.");
    }

    return success;
}
