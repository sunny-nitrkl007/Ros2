#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>

#include <ais/log/Logger.h>

#include <fileio/sha1_fstream.hpp>

#include "LpsSaJobMgrStats.h"

/*
 * Load statistics from storage
 */
bool LpsSaJobMgrStats::load() {
    bool success = false;
    tes_common_ais::sha1_ifstream ifs{ true }; // two_copy
    ifs.open(filePath_, std::ios_base::in | std::ios_base::binary);
    if (ifs) {
        // Read the file in
        try {
            boost::archive::binary_iarchive ia(ifs);
            ia >> *this;
            saveNeeded_ = false;
            AIS_LOG_INFO("Loaded job manager statistics from storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not deserialize job manager statistics from storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (const std::exception& e) {
            AIS_LOG_ERROR("Could not deserialize job manager statistics from storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (...) {
            AIS_LOG_ERROR("Could not deserialize job manager statistics from storage, unexpected error.");
        }

        ifs.close();

        if (ifs.is_corrupt()) {
            if (ifs.fix_it()) {
                AIS_LOG_WARN("Job manager statistics file was corrupt... fixed it.");
            }
            else {
                AIS_LOG_ERROR("Job manager statistics file was corrupt... could not fix it.");
            }
        }
    }
    else {
        AIS_LOG_ERROR("Job manager statistics file could not be opened from storage.");
    }

    if (!success) {
        // Reinitialize to default values.
        *this = LpsSaJobMgrStats(filePath_);
    }

    return success;
}

/*
 * Reset statistics in storage
 */
bool LpsSaJobMgrStats::reset() {
    bool success = false;

    try {
        success = tes_common_ais::sha1_fstream::remove_files(filePath_);
    }
    catch (const boost::archive::archive_exception& e) {
        AIS_LOG_ERROR("Could not delete from storage.");
        AIS_LOG_ERROR(e.what());
    }

    // Reinitialize to default values.
    *this = LpsSaJobMgrStats(filePath_);

    return success;
}

/*
 * Save statistics to storage
 */
bool LpsSaJobMgrStats::save(bool force) const {
    bool success = false;

    if (force || saveNeeded_) {
        tes_common_ais::sha1_ofstream ofs{ true }; // two_copy
        ofs.open(filePath_, std::ios_base::out | std::ios_base::binary);
        if (ofs) {
            // Read the file in
            try {
                boost::archive::binary_oarchive oa(ofs);
                oa << *this;
                saveNeeded_ = false;
                AIS_LOG_INFO("Saved job manager statistics to storage.");
                success = true;
            }
            catch (const boost::archive::archive_exception& e) {
                AIS_LOG_ERROR("Could not serialize job manager statistics to storage.");
                AIS_LOG_ERROR(e.what());
            }

            ofs.close();
        }
        else {
            AIS_LOG_ERROR("Job manager statistics could not be opened from storage.");
        }
    }
    else {
        success = true;
    }

    return success;
}
