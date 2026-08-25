#ifndef LPSSALOADRECORDSUBTOTAL_H
#define LPSSALOADRECORDSUBTOTAL_H

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

#include <cstdint>
#include <cmath>
#include <chrono>
#include <vector>
#include <boost/serialization/vector.hpp>
#include <boost_helpers/boost_serialization_chrono.hpp>
#include <ext/rapidjson/RapidJsonGuarded.h>

#include <LpsPublic.h>

#include "InterfaceCommon.h"

struct LpsSaLoadRecordTimeStamp {
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    LpsSaLoadRecordTimeStamp() :
        utcTime(TimePoint::min()),
        shmTime(0) {}

    LpsSaLoadRecordTimeStamp(const TimePoint& utc_,
            uint32_t shm_) :
        utcTime(utc_),
        shmTime(shm_) {}

    TimePoint utcTime;
    uint32_t shmTime; // seconds

    static const LpsSaLoadRecordTimeStamp& min() {
        static const LpsSaLoadRecordTimeStamp val = LpsSaLoadRecordTimeStamp();
        return val;
    }

    template <class Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar & utcTime;
        ar & shmTime;
    }
};

BOOST_CLASS_VERSION(LpsSaLoadRecordTimeStamp, 1);

struct LpsSaLoadRecordPass {
    LpsSaLoadRecordPass() :
        weightTonnes(0.f),
        calcMethod(0),
        time(LpsSaLoadRecordTimeStamp::min()) {}

    LpsSaLoadRecordPass(float weightTonnes_,
            uint16_t calcMethod_,
            const LpsSaLoadRecordTimeStamp::TimePoint& utc_,
            uint32_t shm_) :
        weightTonnes(weightTonnes_),
        calcMethod(calcMethod_),
        time(utc_, shm_) {}

    float weightTonnes;
    uint16_t calcMethod; // This should really be defined somewhere
    LpsSaLoadRecordTimeStamp time;

    inline LpsWeighBktWtAccuracy_t getAccuracy() const { return (calcMethod >> 8) & 0x0F; }

    template <class Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar & weightTonnes;
        ar & calcMethod;
        ar & time;
    }
};

BOOST_CLASS_VERSION(LpsSaLoadRecordPass, 1);

struct LpsSaLoadRecordSubtotal {

public:
    LpsSaLoadRecordSubtotal() :
        startTime(LpsSaLoadRecordTimeStamp::min()),
        endTime(LpsSaLoadRecordTimeStamp::min()),
        truckId(0),
        truckName(defaultTag()),
        truckTargetWeightTonnes(0.f),
        targetProportion(1),
        targetPasses(1),
        materialId(0),
        materialName(defaultTag()),
        materialDensity(0.f),
        customListName1(),
        customListName2(),
        customListName3(),
        customListName4(),
        tag1(defaultTag()),
        tag2(defaultTag()),
        tag3(defaultTag()),
        tag4(defaultTag()),
        iconType(0),
        zeroWeight(0.f),
        calAdjust(1.f),
        passes_(),
        weightTonnes_(NAN),
        accuracy_(0) {}

    LpsSaLoadRecordTimeStamp startTime;
    LpsSaLoadRecordTimeStamp endTime;
    uint32_t truckId;
    std::string truckName;
    float truckTargetWeightTonnes;
    uint16_t targetProportion;
    uint16_t targetPasses;
    uint32_t materialId;
    std::string materialName;
    float materialDensity;
    std::string customListName1;
    std::string customListName2;
    std::string customListName3;
    std::string customListName4;
    std::string tag1;
    std::string tag2;
    std::string tag3;
    std::string tag4;
    uint8_t iconType;

    /*
     * With these we can reconstruct the raw weight.
     * finalWeight = calAdjust * (rawWeight - zeroWeight)
     */
    float zeroWeight;
    float calAdjust;

    static constexpr const char* defaultTag() { return "--"; }

    inline uint32_t passCount() const {
        return passes_.size();
    }

    inline bool empty() const {
        return passes_.empty();
    }

    inline const std::vector<LpsSaLoadRecordPass>& passes() const {
        return passes_;
    }

    inline float weightTonnes(LpsWeighBktWtAccuracy_t& accuracy) const {
        if (std::isnan(weightTonnes_)) {
            updateWeightAndAccuracy();
        }

        accuracy = accuracy_;
        return weightTonnes_;
    }

    inline float weightTonnes() const {
        if (std::isnan(weightTonnes_)) {
            updateWeightAndAccuracy();
        }

        return weightTonnes_;
    }

    inline float lastPassWeightTonnes(LpsWeighBktWtAccuracy_t& accuracy) const {
        float w = 0.f;

        if (passes_.empty()) {
            // no passes found
            w = 0.f;
            accuracy = LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_NONE;
        }
        else {
            const auto& lastPass = passes_.back();
            w = lastPass.weightTonnes;
            accuracy = lastPass.getAccuracy();
        }

        return w;
    }

    inline void setZeroWeight(float zeroWeight_) {
        zeroWeight = zeroWeight_;
    }

    inline void setSimpleCalAdjust(float simpelCalAdjust) {
        calAdjust = simpelCalAdjust;
    }

    void addPass(float weight, uint8_t accuracy, uint32_t shmTime) {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        if (passes_.empty()) {
            startTime.utcTime = now;
            startTime.shmTime = shmTime;
        }

        // Add the pass
        LpsSaLoadRecordPass pass;
        pass.time.utcTime = now;
        pass.time.shmTime = shmTime;
        pass.weightTonnes = weight;
        pass.calcMethod = makeCalcMethod(accuracy);
        passes_.push_back(std::move(pass));
        updateWeightAndAccuracy();
    }

    bool removeLastPass() {
        bool success = false;

        if (passes_.size()) {
            passes_.pop_back();
            updateWeightAndAccuracy();
            success = true;
        }

        return success;
    }

    void clearSubtotal() {
        passes_.clear();
        updateWeightAndAccuracy();
        startTime = LpsSaLoadRecordTimeStamp();
        endTime = LpsSaLoadRecordTimeStamp();
    }

    void disableLists() {
        disableTruckList();
        disableMaterialList();
        disableTag1List();
        disableTag2List();
        disableTag3List();
        disableTag4List();
    }

    void disableTruckList() {
        truckName = defaultTag();
        truckId = 0;
        truckTargetWeightTonnes = 0;
    }

    void disableMaterialList() {
        materialName = defaultTag();
        materialId = 0;
        materialDensity = 0;
    }

    void disableTag1List() {
        customListName1.clear();
        tag1 = defaultTag();
    }

    void disableTag2List() {
        customListName2.clear();
        tag2 = defaultTag();
    }

    void disableTag3List() {
        customListName3.clear();
        tag3 = defaultTag();
    }

    void disableTag4List() {
        customListName4.clear();
        tag4 = defaultTag();
    }

    template <class Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar & startTime;
        ar & endTime;
        ar & truckId;
        ar & truckName;
        ar & truckTargetWeightTonnes;
        ar & materialId;
        ar & materialName;
        ar & materialDensity;

        ar & tag1;
        ar & tag2;
        ar & tag3;
        ar & tag4;

        ar & customListName1;
        ar & customListName2;
        ar & customListName3;
        ar & customListName4;
        ar & zeroWeight;
        ar & calAdjust;

        ar & passes_;

        if (version >= 2) {
            ar & weightTonnes_;
            ar & accuracy_;
        }

        if (version >= 3) {
            ar & targetProportion;
            ar & targetPasses;
        }

        if (version >= 4) {
            ar & iconType;
        }
    }

private:
    friend class LpsSaLoadRecordChannelStorage;
    std::vector<LpsSaLoadRecordPass> passes_;

    mutable float weightTonnes_;
    mutable LpsWeighBktWtAccuracy_t accuracy_;

    void updateWeightAndAccuracy() const {
        float w = 0.0f;
        LpsWeighBktWtAccuracy_t a = LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_HIGH;

        for (const auto& p : passes_) {
            w += p.weightTonnes;

            { // Update accuracy
                LpsWeighBktWtAccuracy_t pa = p.getAccuracy();
                if (pa < a) { a = pa; }
            }
        }

        weightTonnes_ = w;
        accuracy_ = a;
    }

    static inline uint16_t makeCalcMethod(LpsWeighBktWtAccuracy_t accuracy) {
        uint16_t calcMethod = (((uint16_t)accuracy) << 8) | ((uint16_t)LPS_WEIGH_PCM_SENSOR_TYPE_HPS_HE_RE);
        if (accuracy >= LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_HIGH) {
            calcMethod |= (((uint16_t)LPS_WEIGH_PCM_PRIMARY_FILTER_TYPE_DYNAMIC_AVG) << 12);
        }
        else {
            calcMethod |= (((uint16_t)LPS_WEIGH_PCM_PRIMARY_FILTER_TYPE_BUTTERWORTH_LP) << 12);
        }
        return calcMethod;
    }
};

BOOST_CLASS_VERSION(LpsSaLoadRecordSubtotal, 4);

#endif

