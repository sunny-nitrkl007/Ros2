#ifndef AISJHM2TARGETWTSELECTION_H
#define AISJHM2TARGETWTSELECTION_H

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

#define NUMBER_OF_WEIGHTS_SHOWN_IN_UI   6

/*
 * Ais Jhm2 Target Weight Selection Storage
 */
class AisJhm2TargetWtSelection {
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    struct target_weights_t {
        TimePoint timestamp;
        int count;
        float wt_tonnes;

    private:
        // boost serialize
        friend class boost::serialization::access;
        template <typename Archive> void serialize(Archive &ar, const unsigned int version) {
            ar & timestamp;
            ar & count;
            ar & wt_tonnes;
        }
    };

public:

    AisJhm2TargetWtSelection() :
        targetWtsMap_(),
        currentIndex_(0),
        saveNeeded_(false)
    {}

    inline void reset() {
        *this = AisJhm2TargetWtSelection();
    }

    bool loadTargetWeights(const boost::filesystem::path& filePath);
    bool saveTargetWeights(const boost::filesystem::path& filePath) const;
    void addNewTargetWt(const std::string& wt_string, const float wt_tonnes);
    std::vector<float> getTargetWeightsVector() const;
    bool getNextTargetWeight(int increment, float& targetWt);
    void resetCurrentIndex();

private:

    std::map<std::string, target_weights_t> targetWtsMap_;
    unsigned int currentIndex_;
    mutable bool saveNeeded_;
};


BOOST_CLASS_VERSION(AisJhm2TargetWtSelection::target_weights_t, 1);


#endif // AISJHM2TARGETWTSELECTION_H
