#include <algorithm>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>

#include <ais/log/Logger.h>

#include <fileio/sha1_fstream.hpp>
#include <chrono/print.hpp>

#include "LpsSaJobMgrSimpleCal.h"

/*
 * Add a new simple cal entry from a load record.
 */
void LpsSaJobMgrSimpleCal::addEntry(const LpsSaLoadRecordChannelStorage& loadRecord) {
    const auto& lastSubtotal = loadRecord.getCurrentSubtotal();
    Entry entry;

    //add entire load as a single line item for simple cal instead of split when in split mode
    entry.simpleCalData.truckWt = loadRecord.weightTonnes();

    /*
     * Un-adjust display weight to get back to pre-span-adjusted weight (a.k.a. zeroed weight)
     * But don't divide by 0. It is okay to use the calAdjust from the last subtotal because
     * they are all have the same adjust factor
     */
    if (lastSubtotal.calAdjust > 0) {
        entry.simpleCalData.zeroedTruckWt = entry.simpleCalData.truckWt / lastSubtotal.calAdjust;
    }
    else {
        entry.simpleCalData.zeroedTruckWt = entry.simpleCalData.truckWt;
    }

    { // Convert to local time string
        std::string str = tes_common_ais::putLocalTime(lastSubtotal.endTime.utcTime, tes_common_ais::date_time_formats::ISO_DATE_TIME_LOCAL);
        entry.simpleCalData.timeStamp = str;
        AIS_LOG_DEBUG("Simple Cal Timestamp: %s", str.c_str());
    }

    entry.loadRecord = loadRecord;

    // Limit the size of the queue.
    if (queue_.size() >= maxQueueSize_) {
        queue_.pop_back();
    }

    queue_.push_front(entry);

    saveNeeded_ = true;
}

/*
 * Find and erase a simple cal data entry
 */
void LpsSaJobMgrSimpleCal::eraseEntry(const std::string& timestamp) {
    for (unsigned int idx = 0; idx < queue_.size(); ++idx) {
        if (0 == queue_[idx].simpleCalData.timeStamp.compare(timestamp)) {
            queue_.erase(queue_.cbegin()+idx);
            break;
        }
    }

    saveNeeded_ = true;
}

/*
 * Fill in a deque with all of the simple cal data
 */
void LpsSaJobMgrSimpleCal::getSimpleCalData(std::deque<SimpleCalData_t>& simpleCalData) const {
    simpleCalData.clear();
    std::transform(queue_.cbegin(), queue_.cend(), std::back_inserter(simpleCalData),
            [](const Entry& e) -> const SimpleCalData_t& { return e.simpleCalData; });
}

/*
 * Fill in a vector with all of the simple cal data
 */
void LpsSaJobMgrSimpleCal::getSimpleCalData(std::vector<SimpleCalData_t>& simpleCalData) const {
    simpleCalData.clear();
    std::transform(queue_.cbegin(), queue_.cend(), std::back_inserter(simpleCalData),
            [](const Entry& e) -> const SimpleCalData_t& { return e.simpleCalData; });
}

/*
 * Load simple cal data from storage
 */
bool LpsSaJobMgrSimpleCal::load() {
    bool success = false;
    tes_common_ais::sha1_ifstream ifs{ false }; // one_copy
    ifs.open(filePath_, std::ios_base::in | std::ios_base::binary);
    if (ifs) {
        // Read the file in
        try {
            boost::archive::binary_iarchive ia(ifs);
            ia >> *this;
            saveNeeded_ = false;
            AIS_LOG_INFO("Loaded job manager simple cal data from storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not deserialize job manager simple cal data from storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (const std::exception& e) {
            AIS_LOG_ERROR("Could not deserialize job manager simple cal data from storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (...) {
            AIS_LOG_ERROR("Could not deserialize job manager simple cal data from storage, unexpected error.");
        }

        ifs.close();

        if (ifs.is_corrupt()) {
            if (ifs.fix_it()) {
                AIS_LOG_WARN("Job manager simple cal data file was corrupt... fixed it.");
            }
            else {
                AIS_LOG_ERROR("Job manager simple cal data file was corrupt... could not fix it.");
            }
        }
    }
    else {
        AIS_LOG_ERROR("Job manager simple cal data file could not be opened from storage.");
    }

    if (success) {
        // Reapply max queue size to make sure the stuff loaded from the file does not exceed.
        setMaxQueueSize(this->maxQueueSize_);
    }
    else {
        // Reinitialize to default values.
        *this = LpsSaJobMgrSimpleCal(filePath_, maxQueueSize_);
    }

    return success;
}

/*
 * Reset simple cal data in storage
 */
bool LpsSaJobMgrSimpleCal::reset() {
    bool success = false;

    try {
        success = tes_common_ais::sha1_fstream::remove_files(filePath_);
    }
    catch (const boost::archive::archive_exception& e) {
        AIS_LOG_ERROR("Could not delete from storage.");
        AIS_LOG_ERROR(e.what());
    }

    // Reinitialize to default values.
    *this = LpsSaJobMgrSimpleCal(filePath_, maxQueueSize_);

    return success;
}

/*
 * Save simple cal data to storage
 */
bool LpsSaJobMgrSimpleCal::save(bool force) const {
    bool success = false;

    if (force || saveNeeded_) {
        tes_common_ais::sha1_ofstream ofs{ false }; // one_copy
        ofs.open(filePath_, std::ios_base::out | std::ios_base::binary);
        if (ofs) {
            // Read the file in
            try {
                boost::archive::binary_oarchive oa(ofs);
                oa << *this;
                saveNeeded_ = false;
                AIS_LOG_INFO("Saved job manager simple cal data to storage.");
                success = true;
            }
            catch (const boost::archive::archive_exception& e) {
                AIS_LOG_ERROR("Could not serialize job manager simple cal data to storage.");
                AIS_LOG_ERROR(e.what());
            }

            ofs.close();
        }
        else {
            AIS_LOG_ERROR("Job manager simple cal data could not be opened from storage.");
        }
    }
    else {
        success = true;
    }

    return success;
}

