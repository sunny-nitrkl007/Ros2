#ifndef AISJHM2FREQUENTLIST_H
#define AISJHM2FREQUENTLIST_H

#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <array>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <algorithm> // for sort function
#include <utility>      // std::pair, std::make_pair

#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/filesystem.hpp>

#include <ais/log/Logger.h>

#define NUMBER_OF_SHORT_LISTS_ITEMS   6

/*
 * Ais Jhm2 Frequent List Storage
 */
class AisJhm2FrequentList {
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    struct list_item_data_t {
        std::string name;
        uint32_t id;
        float value;
    };

    struct list_item_t {
        TimePoint timestamp;
        int count;
        list_item_data_t item;

    private:
        // boost serialize
        friend class boost::serialization::access;
        template <typename Archive> void serialize(Archive &ar, const unsigned int version) {
            ar & timestamp;
            ar & count;
            ar & item.name;
            ar & item.id;
            ar & item.value;
        }
    };

public:

    AisJhm2FrequentList() :
        listVec_(),
        currentIndex_(0),
        saveNeeded_(false)
    {}

    inline void reset() {
        *this = AisJhm2FrequentList();
    }

    bool loadList(const boost::filesystem::path& filePath);
    bool saveList(const boost::filesystem::path& filePath) const;
    void addNewListItem(const std::string& name, uint32_t id = 0, float val = 0.f);
    bool getNextListItem(int increment, std::string& name);
    bool getNextListItem(int increment, std::string& name, uint32_t& id, float& value);
    void resetCurrentIndex();

private:
    std::vector<list_item_data_t> getFrequentListVector() const;

    std::vector<list_item_t> listVec_;
    unsigned int currentIndex_;
    mutable bool saveNeeded_;
};

BOOST_CLASS_VERSION(AisJhm2FrequentList::list_item_t, 1);

#endif // AISJHM2FREQUENTLIST_H
