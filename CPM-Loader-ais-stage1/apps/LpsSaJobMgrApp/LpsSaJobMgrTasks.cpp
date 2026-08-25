#include <boost/filesystem.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>

#include <ais/log/Logger.h>

#include <fileio/sha1_fstream.hpp>

#include "LpsSaJobMgrTasks.h"

namespace fs = boost::filesystem;

bool LpsSaJobMgrTasks::load() {
    bool success = false;
    tes_common_ais::sha1_ifstream ifs{ true }; // two_copy
    ifs.open(filePath_, std::ios_base::in | std::ios_base::binary);
    if (ifs) {
        // Read the file in
        try {
            boost::archive::binary_iarchive ia(ifs);
            ia >> *this;
            AIS_LOG_INFO("Loaded job manager tasks from storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not deserialize job manager tasks from storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (const std::exception& e) {
            AIS_LOG_ERROR("Could not deserialize job manager tasks from storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (...) {
            AIS_LOG_ERROR("Could not deserialize job manager tasks from storage, unexpected error.");
        }

        ifs.close();

        if (ifs.is_corrupt()) {
            if (ifs.fix_it()) {
                AIS_LOG_WARN("Job manager tasks file was corrupt... fixed it.");
            }
            else {
                AIS_LOG_ERROR("Job manager tasks file was corrupt... could not fix it.");
            }
        }
    }
    else {
        AIS_LOG_ERROR("Job manager configuration file could not be opened from storage.");
    }

    if (!success) {
        *this = LpsSaJobMgrTasks(filePath_, defaultTargetWeight_);
    }

    return success;
}

bool LpsSaJobMgrTasks::save() {
    bool success = false;

    // update the current targetType for the task before saving
    try {
        targetTypes_.at(loadIndex_) = getCurrentTaskLoad().targetType();
    } catch (const std::out_of_range& e) {
        AIS_LOG_ERROR("Index out of range");
    }

    tes_common_ais::sha1_ofstream ofs{ true }; // two_copy
    ofs.open(filePath_, std::ios_base::out | std::ios_base::binary);
    if (ofs) {
        // Read the file in
        try {
            boost::archive::binary_oarchive oa(ofs);
            oa << *this;
            AIS_LOG_INFO("Saved job manager tasks to storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not serialize job manager tasks to storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (const std::exception &e) {
            AIS_LOG_ERROR("Could not serialize job manager tasks to storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (...) {
            AIS_LOG_ERROR("Could not serialize job manager tasks to storage, unexpected error.");
        }

        ofs.close();
    }
    else {
        AIS_LOG_ERROR("Job manager tasks could not be opened from storage.");
    }

    return success;
}
