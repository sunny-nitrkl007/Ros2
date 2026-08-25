#include <boost/filesystem.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/serialization/map.hpp>
#include <boost_helpers/boost_serialization_chrono.hpp>

#include <ais/log/Logger.h>
#include <fileio/sha1_fstream.hpp>

#include "AisJhm2TargetWtSelection.h"

namespace fs = boost::filesystem;

/*
 * Load target weights from storage
 */
bool AisJhm2TargetWtSelection::loadTargetWeights(const fs::path& filePath) {
    bool success = false;
    tes_common_ais::sha1_ifstream ifs{ false };
    ifs.open(filePath.string(), std::ios_base::in | std::ios_base::binary);
    if (ifs) {
        // Read the file in
        try {
            boost::archive::binary_iarchive ia(ifs);
            ia >> targetWtsMap_;
            AIS_LOG_INFO("Loaded target weights from storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not deserialize target weights from storage.");
            AIS_LOG_ERROR(e.what());
            reset();
        }
        catch (const std::exception& e) {
            AIS_LOG_ERROR("Could not deserialize target weights from storage.");
            AIS_LOG_ERROR(e.what());
            reset();
        }
        catch (...) {
            AIS_LOG_ERROR("Could not deserialize target weights from storage, unexpected error.");
            reset();
        }

        ifs.close();

        if (ifs.is_corrupt()) {
            if (ifs.fix_it()) {
                AIS_LOG_WARN("target weights file was corrupt... fixed it.");
            }
            else {
                AIS_LOG_ERROR("Target Weights file was corrupt... could not fix it.");
            }
        }
    }
    else {
        AIS_LOG_ERROR("Target weights could not be opened from storage.");
    }

    return success;
}

/*
 * Save target weights to storage
 */
bool AisJhm2TargetWtSelection::saveTargetWeights(const fs::path& filePath) const {
    bool success = false;
    if (saveNeeded_) {
        tes_common_ais::sha1_ofstream ofs{ false };
        ofs.open(filePath.string(), std::ios_base::out | std::ios_base::binary);
        if (ofs) {
            // Read the file in
            try {
                boost::archive::binary_oarchive oa(ofs);
                oa << targetWtsMap_;
                AIS_LOG_INFO("Saved target weights map to storage.");
                success = true;
            }
            catch (const boost::archive::archive_exception& e) {
                AIS_LOG_ERROR("Could not serialize target weights map to storage.");
                AIS_LOG_ERROR(e.what());
            }

            ofs.close();
        }
        else {
            AIS_LOG_ERROR("Target Weights could not be opened from storage.");
        }

        saveNeeded_ = false;
    }
    else {
        // No save needed
        success = true;
    }

    return success;
}

/*
 * Add New Target Weight to list
 */
void AisJhm2TargetWtSelection::addNewTargetWt(const std::string& wt_string, const float wt_tonnes) {
    // find and update if already exists
    TimePoint now = Clock::now();
    auto it = targetWtsMap_.find(wt_string);
    if (it != targetWtsMap_.end()) {
        it->second.count++;
        it->second.timestamp = now;
    }
    else {
        // find oldest and remove if size is greater than 2x of actual list used in UI
        if (targetWtsMap_.size() >= (NUMBER_OF_WEIGHTS_SHOWN_IN_UI*2)) {
            auto temp = targetWtsMap_.begin()->second.timestamp;
            std::string oldest_key = targetWtsMap_.begin()->first;
            for (auto const& x : targetWtsMap_) {
                auto temp2 = x.second.timestamp;
                if (temp2 < temp) {
                    temp = temp2;
                    oldest_key = x.first;
                }
            }

            // delete the oldest entry using key
            targetWtsMap_.erase(oldest_key);
        }

        // add to map
        {
            targetWtsMap_.emplace(wt_string, target_weights_t{ now, 1, wt_tonnes });
        }
    }

    // reset the index
    resetCurrentIndex();

    saveNeeded_ = true;
}

/*
 * Get vector<float> of Target Weights in tonnes
 */
std::vector<float> AisJhm2TargetWtSelection::getTargetWeightsVector() const {

    std::vector<std::pair<float, int>> target_wts_vec;
    std::vector<float> sorted_wts;

    // copy key-value pairs from the map to the vector
    for (auto const& x : targetWtsMap_) {
        target_wts_vec.push_back(std::make_pair(x.second.wt_tonnes, x.second.count));
    }

    // sort the vector by increasing order of its pair's second value
    sort(target_wts_vec.begin(), target_wts_vec.end(),
            [](const std::pair<float, int> &a,const std::pair<float, int> &b) -> bool {
        return (a.second > b.second);
    });

    // create the vector of sorted weights
    for (auto const& x : target_wts_vec) {
        sorted_wts.push_back(x.first);
    }

    return sorted_wts;
}

/*
 * Reset current index
 */
void AisJhm2TargetWtSelection::resetCurrentIndex() {
    currentIndex_ = 0;
}

/*
 * Get Next Target Weight in the list and return size of list
 */
bool AisJhm2TargetWtSelection::getNextTargetWeight(int increment, float& targetWt) {

    std::vector<float> sorted_wts = getTargetWeightsVector();

    // reset list if we are at max shown in UI or have reached size, whatever comes first
    if ((currentIndex_ >= NUMBER_OF_WEIGHTS_SHOWN_IN_UI) || (currentIndex_ >= sorted_wts.size())) {
        resetCurrentIndex();
    }

    for (int i = 1; i < increment; ++i) {
        ++currentIndex_;
        if ((currentIndex_ >= NUMBER_OF_WEIGHTS_SHOWN_IN_UI) || (currentIndex_ >= sorted_wts.size())) {
            resetCurrentIndex();
        }
    }

    if (sorted_wts.size()) {
        targetWt = sorted_wts[currentIndex_];
        ++currentIndex_;
        return true;
    }

    return false;
}

