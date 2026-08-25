#ifndef LPSSALOADRECORDCHANNEL_H
#define LPSSALOADRECORDCHANNEL_H

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>
#include <ais/log/Logger.h>

#include <cstdint>
#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>
#include <boost/serialization/vector.hpp>
#include <boost_helpers/boost_serialization_chrono.hpp>

#include <chrono/print.hpp>

#include <LpsCommonWeight.h>

#include "InterfaceCommon.h"
#include "LpsSaLoadRecordSubtotal.h"

#define MAXIMUM_NUMBER_OF_SUBTOTALS_PER_LOAD 99

class LpsSaLoadRecordChannelStorage: public csvable
{
public:
    LpsSaLoadRecordChannelStorage() :
    	storeTime(LpsSaLoadRecordTimeStamp::min()),
    	productId_(),
        equipmentId_(),
        weightInterval_(0.f),
        weightDecimalPrecision_(-1),
        weightUnits_(LpsCommonWeightUnits::TONNE),
        ticketNumber_(0xFFFF),
        ticketId_(),
        storeAction_(LpsSaLoadRecordStoreAction::AUTO),
        subtotal_(),
        subtotals_(),
        recipeName_(),
        targetType_(LpsSaLoadRecordTargetType::SINGLE),
        totalTargetWeight_(0.f),
        currentSubtotalIndex_(1) {}

public:

    LpsSaLoadRecordTimeStamp storeTime;

    inline const std::string& productId() const { return productId_; }
    inline void productId(const std::string& productId) { productId_ = productId; }

    inline const std::string& equipmentId() const { return equipmentId_; }
    inline void equipmentId(const std::string& equipmentId) { equipmentId_ = equipmentId; }

    inline float weightInterval() const { return weightInterval_; }
    inline void weightInterval(float interval) { weightInterval_ = interval; }

    inline int weightDecimalPrecision() const { return weightDecimalPrecision_; }
    inline void weightDecimalPrecision(int decimalPrecision) { weightDecimalPrecision_ = decimalPrecision; }

    inline LpsCommonWeightUnits weightUnits() const { return weightUnits_; }
    inline void weightUnits(const LpsCommonWeightUnits& uom) { weightUnits_ = uom; }

    inline uint32_t ticketNumber() const { return ticketNumber_; }

    // Setting the ticket number will cause the ticket id to be updated.
    inline void ticketNumber(uint32_t value) {
        if (value != ticketNumber_) {
            ticketNumber_ = value;
            updateTicketId();
        }
        else if (ticketId_.empty()) {
            updateTicketId();
        }
    }

    inline const std::string& ticketId() const {
        if (ticketId_.empty()) {
            updateTicketId();
        }
        return ticketId_;
    }

    inline LpsSaLoadRecordStoreAction storeAction() const { return storeAction_; }

    // Setting the store action clears the ticket id to force it to be updated anew.
    inline void storeAction(LpsSaLoadRecordStoreAction value) { storeAction_ = value; ticketId_.clear(); }

    inline uint32_t passCount() const {
        uint32_t totalPasses = subtotal_.passCount();
        for (const auto& s : subtotals_) {
            totalPasses += s.passCount();
        }
        return totalPasses;
    }

    inline float weightTonnes(LpsWeighBktWtAccuracy_t& accuracy) const {
        LpsWeighBktWtAccuracy_t a;
        float w = subtotal_.weightTonnes(a);

        for (const auto& s : subtotals_) {
            LpsWeighBktWtAccuracy_t sa;
            w += s.weightTonnes(sa);
            if (sa < a) { a = sa; }
        }

        accuracy = a;
        return w;
    }

    inline float weightTonnes() const {
        float w = subtotal_.weightTonnes();

        for (const auto& s : subtotals_) {
            w += s.weightTonnes();
        }

        return w;
    }

    inline uint32_t subtotalCount() const {
        uint32_t count = 1;

        count += subtotals_.size();

        return count;
    }

    inline float lastPassWeightTonnes(LpsWeighBktWtAccuracy_t& accuracy) const {
        float w;

        if (subtotals_.empty()) {
            w = subtotal_.lastPassWeightTonnes(accuracy);
        }
        else {
            w = subtotals_.back().lastPassWeightTonnes(accuracy);
        }

        return w;
    }

    inline void clear() {
        if (targetType_ != LpsSaLoadRecordTargetType::SINGLE) {
            for (auto& subtotal : subtotals_) {
                subtotal.clearSubtotal();
            }
        }
        else {
            subtotals_.clear();
        }

        subtotal_.clearSubtotal();

        if (LpsSaLoadRecordTargetType::MIX == targetType_) {
            setTotalTargetWeight(0.f);
        }
    }

    inline const std::string& recipeName() const { return recipeName_; }
    inline void recipeName(const std::string& recipeName) { recipeName_ = recipeName; }

    inline const LpsSaLoadRecordTargetType targetType() const { return targetType_; }
    void setTargetType(const LpsSaLoadRecordTargetType targetType) {
        targetType_ = targetType;
        setTotalTargetWeight(totalTargetWeight_);
    }

    inline float getTotalTargetWeight() const {
        float totalTargetWeight;

        switch (targetType_) {
        case LpsSaLoadRecordTargetType::SINGLE:
            totalTargetWeight = subtotal_.truckTargetWeightTonnes;
            break;

        case LpsSaLoadRecordTargetType::SPLIT:
            totalTargetWeight = subtotal_.truckTargetWeightTonnes;
            for (const auto& s : subtotals_) {
                totalTargetWeight += s.truckTargetWeightTonnes;
            }
            break;

        case LpsSaLoadRecordTargetType::BATCH:
            totalTargetWeight = totalTargetWeight_;
            break;

        case LpsSaLoadRecordTargetType::MIX:
            totalTargetWeight = totalTargetWeight_;
            break;

        case LpsSaLoadRecordTargetType::BLEND_LOADS:
            totalTargetWeight = 0;
            break;

        case LpsSaLoadRecordTargetType::BLEND_WEIGHT:
            totalTargetWeight = subtotal_.truckTargetWeightTonnes;
            for (const auto& s : subtotals_) {
                totalTargetWeight += s.truckTargetWeightTonnes;
            }
            break;

        default:
            totalTargetWeight = totalTargetWeight_;
            break;
        }

        return totalTargetWeight;
    }

    inline void setTotalTargetWeight(float totalTargetWeight) {
        totalTargetWeight_ = totalTargetWeight;

        switch (targetType_) {
        case LpsSaLoadRecordTargetType::SINGLE:
            subtotal_.truckTargetWeightTonnes = totalTargetWeight;
            break;

        case LpsSaLoadRecordTargetType::BATCH:
        case LpsSaLoadRecordTargetType::MIX: {
            // calculate the individual weights based on the grand target
            uint16_t totalProportions = subtotal_.targetProportion;
            for (const auto& s : subtotals_) {
                totalProportions += s.targetProportion;
            }

            float baseWeight = 0.f;
            if (totalProportions > 0) {
                baseWeight = totalTargetWeight / totalProportions;
            }

            // set the subtotal targets
            subtotal_.truckTargetWeightTonnes = baseWeight * subtotal_.targetProportion;
            for (auto& s : subtotals_) {
                s.truckTargetWeightTonnes = baseWeight * s.targetProportion;
            }
            break;
        }

        case LpsSaLoadRecordTargetType::SPLIT:
        case LpsSaLoadRecordTargetType::BLEND_LOADS:
        case LpsSaLoadRecordTargetType::BLEND_WEIGHT:
        default:
            // In these other modes, you cannot set the total target weight directly.
            // it is a result of the subtotal targets.
            totalTargetWeight_ = getTotalTargetWeight();
            break;
        }
    }

    inline uint32_t getTotalTargetPasses() const {
        uint32_t totalTargetPasses = subtotal_.targetPasses;
        for (auto& s : subtotals_) {
            totalTargetPasses += s.targetPasses;
        }
        return totalTargetPasses;
    }

    inline void setMaterialId(uint32_t id) {
        if ((LpsSaLoadRecordTargetType::SINGLE == targetType_) ||
                (LpsSaLoadRecordTargetType::SPLIT == targetType_)) {
            subtotal_.materialId = id;
            for (auto& s : subtotals_) {
                s.materialId = id;
            }
        }
        else {
            getCurrentSubtotal().materialId = id;
        }
    }

    inline void setMaterialName(const std::string& name) {
        if ((LpsSaLoadRecordTargetType::SINGLE == targetType_) ||
                (LpsSaLoadRecordTargetType::SPLIT == targetType_)) {
            subtotal_.materialName = name;
            for (auto& s : subtotals_) {
                s.materialName = name;
            }
        }
        else {
            getCurrentSubtotal().materialName = name;
        }
    }

    inline void setMaterialDensity(float density) {
        if ((LpsSaLoadRecordTargetType::SINGLE == targetType_) ||
                (LpsSaLoadRecordTargetType::SPLIT == targetType_)) {
            subtotal_.materialDensity = density;
            for (auto& s : subtotals_) {
                s.materialDensity = density;
            }
        }
        else {
            getCurrentSubtotal().materialDensity = density;
        }
    }

    inline void setTruckId(uint32_t id) {
        subtotal_.truckId = id;
        for (auto& s : subtotals_) {
            s.truckId = id;
        }
    }

    inline void setTruckName(const std::string& name) {
        subtotal_.truckName = name;
        for (auto& s : subtotals_) {
            s.truckName = name;
        }
    }

    inline void setTag1(const std::string& name, const std::string& value) {
        subtotal_.customListName1 = name;
        subtotal_.tag1 = value;
        for (auto& s : subtotals_) {
            s.customListName1 = name;
            s.tag1 = value;
        }
    }

    inline void setTag2(const std::string& name, const std::string& value) {
        subtotal_.customListName2 = name;
        subtotal_.tag2 = value;
        for (auto& s : subtotals_) {
            s.customListName2 = name;
            s.tag2 = value;
        }
    }

    inline void setTag3(const std::string& name, const std::string& value) {
        subtotal_.customListName3 = name;
        subtotal_.tag3 = value;
        for (auto& s : subtotals_) {
            s.customListName3 = name;
            s.tag3 = value;
        }
    }

    inline void setTag4(const std::string& name, const std::string& value) {
        subtotal_.customListName4 = name;
        subtotal_.tag4 = value;
        for (auto& s : subtotals_) {
            s.customListName4 = name;
            s.tag4 = value;
        }
    }

    static constexpr const char* defaultTag() { return LpsSaLoadRecordSubtotal::defaultTag(); };

    void disableLists() {
        //disable lists in the first subtotal
        subtotal_.disableLists();

        //disable for the rest
        for (auto& subtotal : subtotals_) {
            subtotal.disableLists();
        }
    }

    void disableTruckList() {
        //disable Truck lists in the first subtotal
        subtotal_.disableTruckList();

        //disable for the rest
        for (auto& subtotal : subtotals_) {
            subtotal.disableTruckList();
        }
    }

    void disableMaterialList() {
        //disable Material lists in the first subtotal
        subtotal_.disableMaterialList();

        //disable for the rest
        for (auto& subtotal : subtotals_) {
            subtotal.disableMaterialList();
        }
    }

    void disableTag1List() {
        //disable tag1 lists in the first subtotal
        subtotal_.disableTag1List();

        //disable for the rest
        for (auto& subtotal : subtotals_) {
            subtotal.disableTag1List();
        }
    }

    void disableTag2List() {
        //disable Material lists in the first subtotal
        subtotal_.disableTag2List();

        //disable for the rest
        for (auto& subtotal : subtotals_) {
            subtotal.disableTag2List();
        }
    }

    void disableTag3List() {
        //disable Material lists in the first subtotal
        subtotal_.disableTag3List();

        //disable for the rest
        for (auto& subtotal : subtotals_) {
            subtotal.disableTag3List();
        }
    }

    void disableTag4List() {
        //disable Material lists in the first subtotal
        subtotal_.disableTag4List();

        //disable for the rest
        for (auto& subtotal : subtotals_) {
            subtotal.disableTag4List();
        }
    }

    uint16_t getCurrentSubtotalIndex() const {
        return currentSubtotalIndex_;
    }

    LpsSaLoadRecordSubtotal& getCurrentSubtotal() {
        return const_cast<LpsSaLoadRecordSubtotal&>(static_cast<const LpsSaLoadRecordChannelStorage&>(*this).getCurrentSubtotal());
    }

    const LpsSaLoadRecordSubtotal& getCurrentSubtotal() const {
        if (currentSubtotalIndex_ == 0 || currentSubtotalIndex_ > subtotalCount()) {
            return subtotal_;
        }
        else if (currentSubtotalIndex_ == 1) {
            return subtotal_;
        }
        else {
            return subtotals_.at(currentSubtotalIndex_-2);
        }
    }

    // getSubtotalByIndex - index starts at 1
    const LpsSaLoadRecordSubtotal& getSubtotalByIndex(uint16_t x) const {
        if (x > 1 && x <= subtotalCount()) {
            return subtotals_.at(x-2);
        }
        else {
            // return first subtotal
            return subtotal_;
        }
    }

    // adds new but does not change currently selected subtotal. Index starting at 1
    void insertSubtotal(uint16_t index, float targetWeight, uint16_t targetPasses,
            uint16_t targetProportion, const std::string& materialName, uint32_t materialId, float materialDensity,
			uint8_t iconType) {

        // fail if we are already at max subtotals
        if (subtotalCount() >= MAXIMUM_NUMBER_OF_SUBTOTALS_PER_LOAD) {
            AIS_LOG_ALERT("Subtotal count is limited to %d. Current target type is:%d",
                    MAXIMUM_NUMBER_OF_SUBTOTALS_PER_LOAD, targetType_);
            return;
        }

        // fail for single target type
        if (LpsSaLoadRecordTargetType::SINGLE == targetType_) {
            AIS_LOG_ERROR("Cannot insert a subtotal for single target type.");
            return;
        }

        // Do not allow invalid target proportion
        if (0 == targetProportion) { targetProportion = 1; }

        // create new subtotal, initialize it from the current subtotal to inherit default.
        LpsSaLoadRecordSubtotal newSubtotal = getCurrentSubtotal();

        // Clear the measurement data from the inherited subtotal.
        newSubtotal.clearSubtotal();

        // Only makes sense for SPLIT and BLEND_WEIGHT, otherwise harmless
        newSubtotal.truckTargetWeightTonnes = targetWeight;

        // Set target passes, only makes sense for BLEND_LOADS, otherwise harmless
        newSubtotal.targetPasses = targetPasses;

        // Set target proportion, only makes sense for BATCH and MIX, otherwise harmless
        newSubtotal.targetProportion = targetProportion;

        // All target types besides SINGLE and SPLIT support unique materials per subtotal
        if (LpsSaLoadRecordTargetType::SPLIT != targetType_) {
            newSubtotal.materialName = materialName;
            newSubtotal.materialId = materialId;
            newSubtotal.materialDensity = materialDensity;
        }

        // Set icon type, only makes sense for SPLIT, otherwise harmless
        newSubtotal.iconType = iconType;

        if (index > subtotalCount() || index == 0) {
            // add at the end. index 0 is invalid, so adding at the end
            subtotals_.push_back(std::move(newSubtotal));
        }
        else {
            if (index == 1) {
                // we need to change the 1st subtotal and then insert the current 1st into the vector
                LpsSaLoadRecordSubtotal firstSubtotal = subtotal_;
                subtotal_ = std::move(newSubtotal);
                subtotals_.insert(subtotals_.begin(), std::move(firstSubtotal));
            }
            else {
                // insert at the desired location
                subtotals_.insert(subtotals_.begin()+index-2, std::move(newSubtotal));
            }

            if (index <= currentSubtotalIndex_) {
                currentSubtotalIndex_++;
            }
        }

        // Redistribute the total target weight.
        // Using the original target weight here to skip the MIX mode logic above
        // We decided we want to keep the total target unchanged when adding/editing subtotals
        setTotalTargetWeight(totalTargetWeight_);
    }

    void editSubtotal(uint16_t currentStepNumber, uint16_t newStepNumber, float targetWeight, uint16_t targetPasses,
            uint16_t targetProportion, const std::string& materialName, uint32_t materialId, float materialDensity,
			uint8_t iconType) {

        // validate the arguments based on the target type
        if (currentStepNumber < 1 || newStepNumber < 1 || currentStepNumber > subtotalCount() ||
                newStepNumber > subtotalCount()) {
            AIS_LOG_ALERT("Invalid step number; currentStepNumber: %d,newStepNumber: %d,subtotalCount:%d",
                    currentStepNumber, newStepNumber, subtotalCount());
            // invalid step number
            return;
        }

        // Do not allow invalid target proportion
        if (0 == targetProportion) { targetProportion = 1; }

        // check if index has been changed, if so reorder subtotals
        try {
            LpsSaLoadRecordSubtotal& editedSubtotal = (currentStepNumber > 1) ? subtotals_.at(currentStepNumber-2) : subtotal_;

            // Only makes sense for SPLIT and BLEND_WEIGHT, otherwise harmless
            editedSubtotal.truckTargetWeightTonnes = targetWeight;

            // Set target passes, only makes sense for BLEND_LOADS, otherwise harmless
            editedSubtotal.targetPasses = targetPasses;

            // Set target proportion, only makes sense for BATCH and MIX, otherwise harmless
            editedSubtotal.targetProportion = targetProportion;

            // All target types besides SINGLE and SPLIT support unique materials per subtotal
            if (LpsSaLoadRecordTargetType::SPLIT != targetType_) {
                editedSubtotal.materialName = materialName;
                editedSubtotal.materialId = materialId;
                editedSubtotal.materialDensity = materialDensity;
            }

            // Set icon type, only makes sense for SPLIT, otherwise harmless
            editedSubtotal.iconType = iconType;

            if (currentStepNumber != newStepNumber) {
                // Make a copy to move it to a new location.
                LpsSaLoadRecordSubtotal editedSubtotalCopy = editedSubtotal;

                // reorder
                if (newStepNumber == 1) {
                    LpsSaLoadRecordSubtotal firstSubtotalCopy = subtotal_;
                    subtotals_.erase(subtotals_.begin()+currentStepNumber-2);
                    subtotals_.insert(subtotals_.begin(), std::move(firstSubtotalCopy));
                    subtotal_ = editedSubtotalCopy;
                }
                else if (currentStepNumber == 1) {
                    LpsSaLoadRecordSubtotal secondSubtotalCopy = subtotals_.at(0);
                    subtotals_.erase(subtotals_.begin());
                    subtotal_ = secondSubtotalCopy;
                    subtotals_.insert(subtotals_.begin()+newStepNumber-2, std::move(editedSubtotalCopy));
                }
                else {
                    subtotals_.erase(subtotals_.begin()+currentStepNumber-2);
                    subtotals_.insert(subtotals_.begin() + newStepNumber-2, std::move(editedSubtotalCopy));
                }
            }

            // Update the currently active subtotal index
            if (currentStepNumber == currentSubtotalIndex_) {
                // Moved the current step to a new step
                currentSubtotalIndex_ = newStepNumber;
            }
            else if (currentStepNumber > currentSubtotalIndex_) {
                if (newStepNumber <= currentSubtotalIndex_) {
                    // Moved a step from after to before
                    currentSubtotalIndex_++;
                }
            }
            else {
                if (newStepNumber > currentSubtotalIndex_) {
                    // Moved a step from before to after
                    currentSubtotalIndex_--;
                }
            }
        }
        catch (const std::out_of_range& e) {
            AIS_LOG_ERROR("Index out of range.  currentStepNumber: %d, newStepNumber: %d", currentStepNumber,newStepNumber);
        }

        // Redistribute the total target weight.
        // Using the original target weight here to skip the MIX mode logic above
        // We decided we want to keep the total target unchanged when adding/editing subtotals
        setTotalTargetWeight(totalTargetWeight_);
    }

    void nextSubtotal(uint32_t shmTime) {
        // targetType dependent.
        //   1. If split target type, goes to next subtotal if available, otherwise creates a new one
        //   2. For other target types, it will only go to next one if available otherwise loop
        AIS_LOG_INFO("next subtotal cmd");

        if (targetType_ == LpsSaLoadRecordTargetType::SPLIT) {
            if (currentSubtotalIndex_ < subtotalCount()) {
                // go to next subtotal
                selectSubtotal(shmTime, currentSubtotalIndex_+1);
            }
            else {
                // create new at the end and select it
                auto& currentSubtotal = getCurrentSubtotal();
                insertSubtotal(0, currentSubtotal.truckTargetWeightTonnes,
                        currentSubtotal.targetPasses, currentSubtotal.targetProportion,
                        currentSubtotal.materialName, currentSubtotal.materialId,
                        currentSubtotal.materialDensity, currentSubtotal.iconType);

                selectSubtotal(shmTime, currentSubtotalIndex_+1);
            }
        }
        else {
            // for other target types, we will only loop through the subtotals when nextSubtotal is requested
            selectSubtotal(shmTime, currentSubtotalIndex_+1);
        }
    }

    void selectSubtotal(uint32_t shmTime, uint16_t subtotalIndex) {
        if ((subtotalIndex < 1) || (subtotalIndex > subtotalCount())) {
            // invalid index so loop back to first
            AIS_LOG_DEBUG("Invalid index: %d, count:%d", subtotalIndex, subtotalCount());
            subtotalIndex = 1;
        }

        if (subtotalIndex != currentSubtotalIndex_) {
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            LpsSaLoadRecordSubtotal& currentSubtotal = getCurrentSubtotal();

            // Update the closing information for the subtotal
            currentSubtotal.endTime.utcTime = now;
            currentSubtotal.endTime.shmTime = shmTime;

            // if in mix mode, calculate the targets if needed
            if (targetType_ == LpsSaLoadRecordTargetType::MIX) {
                auto currentPassCount = currentSubtotal.passCount();

                if ((currentPassCount > 0) &&
                        (currentPassCount == passCount())) {

                    // This was the first pass with non-zero passcount, cascade the target weight to other steps.
                    currentSubtotal.truckTargetWeightTonnes = currentSubtotal.weightTonnes();

                    // Calculate the base weight, how much each proportion is worth.
                    uint16_t baseProportion = currentSubtotal.targetProportion;
                    float baseWeight = 0.f;
                    if (baseProportion > 0) {
                        baseWeight = currentSubtotal.truckTargetWeightTonnes / baseProportion;
                    }

                    // Calculate the total number of proportions there are.
                    uint16_t totalProportions = subtotal_.targetProportion;
                    for (const auto& s : subtotals_) {
                        totalProportions += s.targetProportion;
                    }

                    // The total target weight is the base weight multiplied by the number of proportions
                    setTotalTargetWeight(baseWeight * totalProportions);
                }
            }

            currentSubtotalIndex_ = subtotalIndex;
        }
    }

    // index starts at 1
    void removeSubtotal(uint16_t subtotalIndex) {
        // remove subtotal by index, ignore remove if only one subtotal exists
        uint32_t sc = subtotalCount();

        if (sc <= 1) {
            // This is the last subtotal, can't remove it.
            return;
        }

        if ((subtotalIndex < 1) || (subtotalIndex > sc)) {
            // invalid index, 0 or greater than subtotal count
            AIS_LOG_ERROR("Attempt to remove an invalid subtotal with index(starts at 1): %d of total count: %d", subtotalIndex, subtotalCount());
            return;
        }

        try {
            if (1 == subtotalIndex) {
                // shift second subtotal to first and remove second
                subtotal_ = subtotals_[0];
                subtotals_.erase(subtotals_.begin());
            }
            else {
                subtotals_.erase(subtotals_.begin()+subtotalIndex-2);
            }

            // Decrement the subtotal count after erasing it.
            sc--;

            // One was removed before current index, shift back.
            if (subtotalIndex < currentSubtotalIndex_) {
                currentSubtotalIndex_--;
            }

            // If the subtotal index is not beyond the end, meaning we deleted the last item,
            // then set the current subtotal index to the end.  Otherwise leave it alone.
            if (currentSubtotalIndex_ > sc) {
                currentSubtotalIndex_ = sc;
            }
    	} catch (const std::out_of_range& e) {
    		AIS_LOG_ERROR("Index out of range.  subtotalIndex: %d", subtotalIndex);
    	}

        // Redistribute the total target weight.
        // Using the original target weight here to skip the MIX mode logic above
        // We decided we want to keep the total target unchanged when adding/editing subtotals
        setTotalTargetWeight(totalTargetWeight_);
    }

    template <class Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar & storeAction_;

        // support legacy LpsSaLoadRecordChannel for backwards compatibility
        ar & subtotal_.startTime;
        ar & subtotal_.endTime;
        ar & subtotal_.truckId;
        ar & subtotal_.truckName;
        ar & subtotal_.truckTargetWeightTonnes;
        ar & subtotal_.materialId;
        ar & subtotal_.materialName;
        ar & subtotal_.materialDensity;

        if (version >= 2) {
            // for backwards compatibility, the first subtotal must be serialized here.
            ar & subtotal_.tag1;
            ar & subtotal_.tag2;
            ar & subtotal_.tag3;
            ar & subtotal_.tag4;
        }

        if (version < 3) {
            // For version 3 and forward, we don't keep the security key
            // information here.

            // Read it and throw it out.
            std::string securityKeyDescription;
            ar & securityKeyDescription;

            // Read it and throw it out.
            int32_t securityKeyId = 0;
            ar & securityKeyId;
        }

        if (version >= 4) {
            // for backwards compatibility, the first subtotal must be serialized here.
            ar & subtotal_.customListName1;
            ar & subtotal_.customListName2;
            ar & subtotal_.customListName3;
            ar & subtotal_.customListName4;
        }

        if (version >= 5) {
            // for backwards compatibility, the first subtotal must be serialized here.
            ar & subtotal_.zeroWeight;
            ar & subtotal_.calAdjust;
        }

        // for backwards compatibility, the first subtotal must be serialized here.
        ar & subtotal_.passes_;

        // new load record type
        if (version >= 6) {
            ar & subtotals_;
        }

        if (version >= 7) {
            ar & ticketNumber_;
        }

        if (version >= 8) {
            // for backwards compatibility, the first subtotal must be serialized here.
            ar & subtotal_.weightTonnes_;
            ar & subtotal_.accuracy_;
        }

        if (version >= 9) {
            ar & ticketId_;
        }

        if (version >= 10) {
            ar & productId_;
            ar & equipmentId_;
            ar & weightInterval_;
            ar & weightDecimalPrecision_;
            ar & weightUnits_;
        }

        if (version >= 11) {
            ar & recipeName_;
            ar & targetType_;
            ar & totalTargetWeight_;
            ar & currentSubtotalIndex_;
        }

        if (version >= 12) {
            ar & storeTime;
        }
    }

    void payloadDetailsToJson(std::ostream& os) const {
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer{ sb };
        writer.SetMaxDecimalPlaces(3);
        writer.StartObject();
        writer.Key("PayloadDetails");
        writer.StartObject();
        writer.Key("recipeName"); writer.String(recipeName_);
        writer.Key("targetType"); writer.Uint(targetType_);
        writer.Key("totalTargetWeight"); writer.Double(getTotalTargetWeight());
        writer.Key("totalWeight"); writer.Double(weightTonnes());
        writer.Key("totalPasses"); writer.Uint(passCount());
        writer.Key("totalTargetPasses"); writer.Uint(getTotalTargetPasses());
        writer.Key("subtotals");
        writer.StartArray();
        for (uint16_t idx = 1; idx <= subtotalCount(); ++idx) {
            const LpsSaLoadRecordSubtotal& subtotal = getSubtotalByIndex(idx);
            writer.StartObject();
            writer.Key("step"); writer.Uint(idx);
            writer.Key("iconType"); writer.Uint(subtotal.iconType);
            writer.Key("targetWeight"); writer.Double(subtotal.truckTargetWeightTonnes);
            writer.Key("targetProportion"); writer.Uint(subtotal.targetProportion);
            writer.Key("targetPasses"); writer.Uint(subtotal.targetPasses);
            writer.Key("truckId"); writer.Uint(subtotal.truckId);
            writer.Key("truckName"); writer.String(subtotal.truckName);
            writer.Key("materialId"); writer.Uint(subtotal.materialId);
            writer.Key("materialName"); writer.String(subtotal.materialName);
            writer.Key("materialDensity"); writer.Double(subtotal.materialDensity);
            writer.Key("tag1"); writer.String(subtotal.tag1);
            writer.Key("tag2"); writer.String(subtotal.tag2);
            writer.Key("tag3"); writer.String(subtotal.tag3);
            writer.Key("tag4"); writer.String(subtotal.tag4);
            writer.Key("weightTonnes"); writer.Double(subtotal.weightTonnes());
            writer.Key("passCount"); writer.Uint(subtotal.passCount());
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        writer.EndObject();
        os << sb.GetString();
    }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

private:
    std::string productId_; // PID 0xF82D
    std::string equipmentId_; // PID 0xF81A
    float weightInterval_;
    int weightDecimalPrecision_;
    LpsCommonWeightUnits weightUnits_;
    uint32_t ticketNumber_;
    mutable std::string ticketId_; // Mutable because it is updated as needed
    LpsSaLoadRecordStoreAction storeAction_;
    LpsSaLoadRecordSubtotal subtotal_;
    std::vector<LpsSaLoadRecordSubtotal> subtotals_;
    std::string recipeName_;
    LpsSaLoadRecordTargetType targetType_;
    float totalTargetWeight_;
    uint16_t currentSubtotalIndex_;

    void updateTicketId() const {
        if (LpsSaLoadRecordStoreAction::AUTO == storeAction_) {
            ticketId_.clear();
        }
        else {
            std::stringstream ss;
            std::string timeStr = tes_common_ais::putTime(getCurrentSubtotal().endTime.utcTime, "%y-%m-%d-%H-%M-%S-");
            if (18 == timeStr.length()) {
                ss << timeStr << std::setw(5) << std::setfill('0') << ticketNumber_;
                ticketId_ = ss.str();
            }
            else {
                ticketId_.clear();
            }
        }
    }
};

typedef Datum<LpsSaLoadRecordChannelStorage> LpsSaLoadRecordChannel;

BOOST_CLASS_VERSION(LpsSaLoadRecordChannelStorage, 12);

#endif

