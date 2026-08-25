#ifndef LPSSAUI_DISPLAYSTATE_HPP
#define LPSSAUI_DISPLAYSTATE_HPP

#include "DisplaySettings.hpp"

/*
 * The interface storage class.
 */
struct LpsSaUIDisplayState {
public:
    LpsSaUIDisplayState() :
        legalForTradeDescriptiveMarkingCache_(),
        serviceModeEnableCodeEntered_(false),
        inServiceMode_(false),
        inVerificationMode_(false),
        heartbeatCount_(0),
        documentId_(0),
        weightCapacity_(10.f),
        weightDecimalPrecision_(1),
        weightInterval_(0.1f),
        settings_() {}

    // Enter service mode enable code to allow entering service mode.
    inline bool enterServiceModeEnableCode(uint32_t code) {
        if (code == settings_.serviceModeEnableCode) {
            serviceModeEnableCodeEntered_ = true;
        }
        else {
            serviceModeEnableCodeEntered_ = false;
            inServiceMode_ = false;
        }
        return serviceModeEnableCodeEntered_;
    }

    // Enter service mode, if the code has been entered (or if forced).
    inline bool enterServiceMode(bool force = false) {
        if (serviceModeEnableCodeEntered_ || force) {
            serviceModeEnableCodeEntered_ = true;
            inServiceMode_ = true;
        }
        return inServiceMode_;
    }

    inline void exitServiceMode() {
        serviceModeEnableCodeEntered_ = false;
        inServiceMode_ = false;
    }

    // Is the service mode enable code entered?
    inline bool isServiceModeEnableCodeEntered() const { return serviceModeEnableCodeEntered_; }

    // Are we in service mode?
    inline bool isInServiceMode() const { return inServiceMode_; }

    // Set verification mode
    inline void setVerificationMode(bool inVerificationMode) {
        inVerificationMode_ = inVerificationMode;
        initWeightInterval();
    }

    // Are we in verification mode?
    inline bool isInVerificationMode() const { return inVerificationMode_; }

    // Report receipt of the display heartbeat message
    inline void reportHeartbeat() { ++heartbeatCount_; }

    // Get display heartbeat count.
    inline uint32_t getHeartbeatCount() const { return heartbeatCount_; }

    // Set document id, used to identify the current UI page.
    inline void setDocumentId(uint16_t documentId) { documentId_ = documentId; }

    // Get document id, used to identify the current UI page.
    inline uint16_t getDocumentId() const { return documentId_; }

    // Set the weight capacity and updates the weight interval based on new capacity and units.
    inline void setWeightCapacity(float capacity) {
        weightCapacity_ = capacity;
        initWeightInterval();
    }

    // Get the weight capacity in tonnes.
    inline float getWeightCapacity() const { return weightCapacity_; }

    // Get the weight decimal precision (number of decimal points).
    inline int getWeightDecimalPrecision() const { return weightDecimalPrecision_; }

    // Get the weight interval (distance from one weight to next higher or lower weight).
    inline float getWeightInterval() const { return weightInterval_; }

    inline bool legalForTradeDescriptionMarkingChanged() const { return legalForTradeDescriptiveMarkingCache_.empty(); }

    // Get Legal For Trade Descriptive Markings
    inline const std::string& getLegalForTradeDescriptiveMarking() {
        if (legalForTradeDescriptiveMarkingCache_.empty()) {

            std::string unitStr;
            switch (settings_.weightUnits) {
            case (LpsCommonWeightUnits::TONNE): {
                unitStr = "t";
                break;
            }
            case (LpsCommonWeightUnits::KG): {
                unitStr = "kg";
                break;
            }
            case (LpsCommonWeightUnits::TON): {
                unitStr = "T";
                break;
            }
            case (LpsCommonWeightUnits::LB): {
                unitStr = "lb";
                break;
            }
            case (LpsCommonWeightUnits::KLB): {
                unitStr = "klb";
                break;
            }
            default: {
                unitStr = "";
                break;
            }
            }

            std::string maxStr = "Max " + lps_common_weight_to_string(lps_common_weight_from_tonnes(weightCapacity_, settings_.weightUnits), weightDecimalPrecision_, weightInterval_) + unitStr;
            std::string minStr = "Min " + lps_common_weight_to_string(10 * weightInterval_, weightDecimalPrecision_, weightInterval_) + unitStr;
            std::string intervalStr;
            if (inVerificationMode_) {
                // In verification mode, show the actual scale interval, d, which differs from the verification scale interval, e
                intervalStr = "d=" + lps_common_weight_to_string(weightInterval_, weightDecimalPrecision_, weightInterval_) + unitStr;
            }
            else {
                // Show the verification scale interval, e
                intervalStr = "e=" + lps_common_weight_to_string(weightInterval_, weightDecimalPrecision_, weightInterval_) + unitStr;
            }
            std::string classStr = "Y(b)";
            legalForTradeDescriptiveMarkingCache_ = maxStr + ", " + minStr + ", " + intervalStr + " " + classStr;
        }
        return legalForTradeDescriptiveMarkingCache_;
    }

    // Set units.
    inline void setUnits(LpsSaUIDisplaySettingsUnits units) {
        settings_.units = units;
        settings_.makeUnitsConsistent();
        initWeightInterval();
    }

    // Set language.
    inline void setLanguage(LpsSaUIDisplaySettingsLanguage language) { settings_.language = language; }

    // Set brightness.
    inline void setBrightness(uint8_t brightness) { settings_.brightness = brightness; }

    // Set time format.
    inline void setTimeFormat(LpsSaUIDisplaySettingsTimeFormat timeFormat) { settings_.timeFormat = timeFormat; }

    // Set date format.
    inline void setDateFormat(LpsSaUIDisplaySettingsDateFormat dateFormat) { settings_.dateFormat = dateFormat; }

    // Set keyboard layout.
    inline void setKeyboardLayoutSetting(LpsSaUIKeyboardLayoutSetting keyboardLayoutSetting) { settings_.keyboardLayoutSetting = keyboardLayoutSetting; }

    // Set weight units.
    inline void setWeightUnits(LpsCommonWeightUnits weightUnits) {
        settings_.weightUnits = weightUnits;
        settings_.makeUnitsConsistent();
        initWeightInterval();
    }

    // Set weight precision.
    inline void setWeightPrecision(LpsCommonWeightPrecision weightPrecision) {
        settings_.weightPrecision = weightPrecision;
        initWeightInterval();
    }

    // Set service mode enable code
    inline void setServiceModeEnableCode(uint16_t serviceModeEnableCode) { settings_.serviceModeEnableCode = serviceModeEnableCode; }

    // Set display settings.
    inline void setSettings(const LpsSaUIDisplaySettings& settings) {
        settings_ = settings;
        settings_.makeUnitsConsistent();
        initWeightInterval();
    }

    // Get the display settings.
    inline const LpsSaUIDisplaySettings& getSettings() const { return settings_; }

private:
    mutable std::string legalForTradeDescriptiveMarkingCache_; // Do not serialize
    bool serviceModeEnableCodeEntered_;
    bool inServiceMode_;
    bool inVerificationMode_;
    uint32_t heartbeatCount_;
    uint16_t documentId_;
    float weightCapacity_; // Weight capacity in tonnes.
    int weightDecimalPrecision_; // Weight decimal precision (number of decimal points).
    float weightInterval_; // Weight interval (distance from one weight to next higher or lower weight)
    LpsSaUIDisplaySettings settings_;

    bool initWeightInterval() {
        bool success = settings_.getWeightInterval(weightCapacity_, inVerificationMode_, weightDecimalPrecision_, weightInterval_);
        if (!success) {
            weightDecimalPrecision_ = 1;
            weightInterval_ = 0.1f;
        }

        legalForTradeDescriptiveMarkingCache_.clear();

        return success;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        if (version >= 1) {
            ar & serviceModeEnableCodeEntered_;
        }

        ar & inServiceMode_;

        if (version >= 1) {
            ar & heartbeatCount_;
        }

        if (version >= 2) {
        	ar & documentId_;
        }

        if (version >= 3) {
            ar & weightCapacity_;
            ar & weightDecimalPrecision_;
            ar & weightInterval_;
        }

        ar & settings_;

        if (version >= 4) {
            ar & inVerificationMode_;
        }
    }
};

BOOST_CLASS_VERSION(LpsSaUIDisplayState, 4);

#endif /* LPSSAUI_DISPLAYSTATE_HPP */

