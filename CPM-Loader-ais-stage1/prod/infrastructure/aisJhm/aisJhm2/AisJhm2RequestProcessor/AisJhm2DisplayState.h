#ifndef AISJHM2DISPLAYSTATE_H
#define AISJHM2DISPLAYSTATE_H

#include <cstdint>
#include <string>
#include <iostream>

#include <boost/filesystem.hpp>

#include <ais/config/ConfigSection.h>
#include <ais/log/Logger.h>

#include <interfaces/LpsSaUI/DisplayState.hpp>

/*
 * Ais Jhm2 Display Settings Storage
 */
struct AisJhm2DisplayState : LpsSaUIDisplayState {
public:
    AisJhm2DisplayState() : LpsSaUIDisplayState() {}

    inline void reset() {
        *this = AisJhm2DisplayState();
    }

    bool loadSettings(const boost::filesystem::path& filePath);
    bool saveSettings(const boost::filesystem::path& filePath) const;

    inline bool setWeightCapacity(ConfigSection& cs) {
        bool success;
        float capacity;

        if (cs.get("ScaleWeightCapacity", capacity)) {
            AIS_LOG_INFO("Scale weight capacity is %f tonnes.", capacity);
            success = true;
        }
        else if (cs.get("LoaderBktPayldTrgtWt", capacity)) {
            AIS_LOG_INFO("Scale weight capacity, %f tonnes, is calculated from LoaderBktPayldTrgtWt", capacity);
            capacity *= 1.4f; // 1.4 times target payload is the default scale capacity.
            success = true;
        }
        else {
            AIS_LOG_ERROR("ScaleWeightCapacity or LoaderBktPayldTrgtWt not found in MachineSpecificConfig");
            success = false;
        }

        if (success) {
            LpsSaUIDisplayState::setWeightCapacity(capacity);
            AIS_LOG_INFO("Weight interval, %f (units 0x%x), based on a capacity of %f tonnes.",
                    getWeightInterval(), getSettings().weightUnits, capacity);
        }

        return success;
    }
};

#endif // AISJHM2DISPLAYSTATE_H
