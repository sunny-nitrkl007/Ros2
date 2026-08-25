#include <boost/filesystem.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/serialization/vector.hpp>
#include <boost_helpers/boost_serialization_chrono.hpp>

#include <ais/log/Logger.h>
#include <fileio/sha1_fstream.hpp>

#include "AisJhm2FrequentList.h"

namespace fs = boost::filesystem;

/*
 * Load target weights from storage
 */
bool AisJhm2FrequentList::loadList(const fs::path& filePath) {
    bool success = false;
    tes_common_ais::sha1_ifstream ifs{ false };
    ifs.open(filePath.string(), std::ios_base::in | std::ios_base::binary);
    if (ifs) {
        // Read the file in
        try {
            boost::archive::binary_iarchive ia(ifs);
            ia >> listVec_;
            AIS_LOG_INFO("Loaded short list from storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not deserialize short list from storage.");
            AIS_LOG_ERROR(e.what());
            reset();
        }
        catch (const std::exception& e) {
            AIS_LOG_ERROR("Could not deserialize short list from storage.");
            AIS_LOG_ERROR(e.what());
            reset();
        }
        catch (...) {
            AIS_LOG_ERROR("Could not deserialize short list from storage, unexpected error.");
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
        AIS_LOG_ERROR("Short List could not be opened from storage.");
    }

    return success;
}

/*
 * Save target weights to storage
 */
bool AisJhm2FrequentList::saveList(const fs::path& filePath) const {
    bool success = false;
    if (saveNeeded_) {
        tes_common_ais::sha1_ofstream ofs{ false };
        ofs.open(filePath.string(), std::ios_base::out | std::ios_base::binary);
        if (ofs) {
            // Read the file in
            try {
                boost::archive::binary_oarchive oa(ofs);
                oa << listVec_;
                AIS_LOG_INFO("Saved short vector to storage.");
                success = true;
            }
            catch (const boost::archive::archive_exception& e) {
                AIS_LOG_ERROR("Could not serialize short list vector to storage.");
                AIS_LOG_ERROR(e.what());
            }

            ofs.close();
        }
        else {
            AIS_LOG_ERROR("Short List could not be opened from storage.");
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
void AisJhm2FrequentList::addNewListItem(const std::string& name, uint32_t id, float value) {
    // find and update if already exists
    TimePoint now = Clock::now();
    auto oldest = listVec_.begin();
    bool found = false;

    if (listVec_.size()) {
        auto temp = listVec_.begin()->timestamp;
        for(auto iter = listVec_.begin(); iter < listVec_.end(); iter++) {
            if (iter->item.name == name) {
                // found it, increment the use count and update timestamp
                iter->count++;
                iter->timestamp = now;
                found = true;
                break;
            }

            if (iter->timestamp < temp) {
                temp = iter->timestamp;
                oldest = iter;
            }
        }
    }

    // add new entry
    if (!found) {
        // find oldest and remove if size is greater than 2x of actual list used in UI
        if (listVec_.size() >= (NUMBER_OF_SHORT_LISTS_ITEMS*2)) {
            listVec_.erase(oldest);
        }

        // add
        listVec_.emplace(listVec_.end(), list_item_t{ now, 1, {name, id, value} });
    }

    // reset the index
    resetCurrentIndex();

    saveNeeded_ = true;
}

/*
 * Get vector<float> of Target Weights in tonnes
 */
std::vector<AisJhm2FrequentList::list_item_data_t> AisJhm2FrequentList::getFrequentListVector() const {
    std::vector<list_item_data_t> sorted_short_list;
    std::vector<std::pair<list_item_data_t, int>> temp_short_pair_vec;

    // copy short list vec to vector of pairs
    for (auto const& x : listVec_) {
        temp_short_pair_vec.push_back(std::make_pair(x.item, x.count));
    }

    // sort the vector by increasing order of its pair's second value (the count)
    sort(temp_short_pair_vec.begin(), temp_short_pair_vec.end(),
            [](const std::pair<list_item_data_t, int>& a, const std::pair<list_item_data_t, int>& b) -> bool {
        return (a.second > b.second);
    });

    // create the vector of the short list
    for (auto const& x : temp_short_pair_vec) {
        sorted_short_list.push_back(x.first);
    }

    return sorted_short_list;
}

/*
 * Reset current index
 */
void AisJhm2FrequentList::resetCurrentIndex() {
    currentIndex_ = 0;
}

/*
 * Get next item in the list and return size of list
 */
bool AisJhm2FrequentList::getNextListItem(int increment, std::string& name) {
    uint32_t id = 0;
    float value = 0;
    return getNextListItem(increment, name, id, value);
}

/*
 * Get next item in the list along with val and return size of list
 */
bool AisJhm2FrequentList::getNextListItem(int increment, std::string& name, uint32_t& id, float& value) {

    std::vector<list_item_data_t> sorted_vec = getFrequentListVector();

    // reset list if we are at max shown in UI or have reached size, whatever comes first
    if ((currentIndex_ >= NUMBER_OF_SHORT_LISTS_ITEMS) || (currentIndex_ >= sorted_vec.size())) {
        resetCurrentIndex();
    }

    for (int i = 1; i < increment; ++i) {
        ++currentIndex_;
        if ((currentIndex_ >= NUMBER_OF_SHORT_LISTS_ITEMS) || (currentIndex_ >= sorted_vec.size())) {
            resetCurrentIndex();
        }
    }

    if (sorted_vec.size()) {
        name = sorted_vec[currentIndex_].name;
        id = sorted_vec[currentIndex_].id;
        value = sorted_vec[currentIndex_].value;
        ++currentIndex_;
        return true;
    }

    return false;
}

