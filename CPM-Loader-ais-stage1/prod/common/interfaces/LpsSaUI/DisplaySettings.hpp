#ifndef LPSSAUI_DISPLAYSETTINGS_HPP
#define LPSSAUI_DISPLAYSETTINGS_HPP

#include <cstdint>
#include <type_traits>
#include <string>

#include <boost/serialization/version.hpp>

#include <LpsCommonWeight.h>

enum class LpsSaUIDisplaySettingsUnits : uint8_t {
    METRIC = 0,
    ENGLISH = 1
};

#define LpsSaUIDisplaySettingsUnits_Base_t(val) (static_cast<typename std::underlying_type<LpsSaUIDisplaySettingsUnits>::type>(val))

enum class LpsSaUIDisplaySettingsLanguage : uint16_t {
    English = 0x0000,
    Spanish = 0x0001,
    French = 0x0002,
    Portuguese = 0x0003,
    German = 0x0004,
    Norwegian = 0x0005,
    Swedish = 0x0006,
    Finnish = 0x0007,
    Dutch = 0x0008,
    Danish = 0x0009,
    Italian = 0x000A,
    Greek = 0x000B,
    Turkish = 0x000C,
    Indonesian = 0x000D,
    Russian = 0x000E,
    Japanese = 0x000F,
    Hungarian = 0x0010,
    Icelandic = 0x0011,
    Arabic = 0x0012,
    Traditional_Chinese = 0x0013,
    Czech = 0x0014,
    Polish = 0x0015,
    Simplified_Chinese = 0x0016,
    Thai = 0x0017,
    Slovenian = 0x0018,
    Slovakian = 0x0019,
    Estonian = 0x001A,
    Latvian = 0x001B,
    Lithuanian = 0x001C,
    Malaysian = 0x001D,
    Korean = 0x001E,
    Portuguese_Brazil = 0x001F,
    Farsi = 0x0020,
    Hebrew = 0x0021,
    Serbian = 0x0022,
    Croatian = 0x0023,
    Bulgarian = 0x0024,
    Romanian = 0x0025,
    Vietnamese = 0x0026,
    Mongolian = 0x0027,
    Macedonian = 0x0028,
    Burmese = 0x0029,
    Khmer = 0x002A,
    Ukrainian = 0x002B,
    Spanish_Castilian_EAME = 0x002C,
    French_EAME = 0x002D,
    Portuguese_EAME = 0x002E,
    Dutch_Netherlands = 0x002F,
    Burmese_Myanmasa = 0x0030,
    Afrikaans = 0x0031,
    Albanian = 0x0032,
    Armenian = 0x0033,
    Bosnian = 0x0034,
    Dari = 0x0035,
    Dutch_Belgium = 0x0036,
    French_Canadian = 0x0037,
    Hindi = 0x0038,
    Kazakh = 0x0039,
    Laotian = 0x003A,
    Spanish_LACD = 0x003B,
    Tamil = 0x003C,
    Turkmen = 0x003D,
    Urdu = 0x003E
};

#define LpsSaUIDisplaySettingsLanguage_Base_t(val) (static_cast<typename std::underlying_type<LpsSaUIDisplaySettingsLanguage>::type>(val))

enum class LpsSaUIDisplaySettingsTimeFormat : uint8_t {
    TWENTY_FOUR_HOUR = 0,
    TWELVE_HOUR = 1
};

#define LpsSaUIDisplaySettingsTimeFormat_Base_t(val) (static_cast<typename std::underlying_type<LpsSaUIDisplaySettingsTimeFormat>::type>(val))

enum class LpsSaUIDisplaySettingsDateFormat : uint16_t {
    YYYY_MM_DD = 0x0670,
    MM_DD_YYYY = 0x0671,
    DD_MM_YYYY = 0x0672
};

#define LpsSaUIDisplaySettingsDateFormat_Base_t(val) (static_cast<typename std::underlying_type<LpsSaUIDisplaySettingsDateFormat>::type>(val))

enum class LpsSaUIKeyboardLayoutSetting : uint8_t {
    QWERTY = 0,
    Native = 1,
    Mixed = 2
};

#define LpsSaUIKeyboardLayoutSetting_Base_t(val) (static_cast<typename std::underlying_type<LpsSaUIKeyboardLayoutSetting>::type>(val))

struct LpsSaUIDisplaySettings {
public:
    LpsSaUIDisplaySettings() :
        units(LpsSaUIDisplaySettingsUnits::METRIC),
        language(LpsSaUIDisplaySettingsLanguage::English),
        brightness(80),
        timeFormat(LpsSaUIDisplaySettingsTimeFormat::TWENTY_FOUR_HOUR),
        dateFormat(LpsSaUIDisplaySettingsDateFormat::DD_MM_YYYY),
        keyboardLayoutSetting(LpsSaUIKeyboardLayoutSetting::QWERTY),
        weightUnits(LpsCommonWeightUnits::TONNE),
        weightPrecision(LpsCommonWeightPrecision::STANDARD),
        serviceModeEnableCode(1234) {}

    LpsSaUIDisplaySettingsUnits units;
    LpsSaUIDisplaySettingsLanguage language;
    uint8_t brightness; // in percent 0-100
    LpsSaUIDisplaySettingsTimeFormat timeFormat;
    LpsSaUIDisplaySettingsDateFormat dateFormat;
    LpsSaUIKeyboardLayoutSetting keyboardLayoutSetting;
    LpsCommonWeightUnits weightUnits;
    LpsCommonWeightPrecision weightPrecision;
    uint16_t serviceModeEnableCode;

    // Get weight decimal precision and interval in the currently configured units, given the scale capacity in tonnes.
    inline bool getWeightInterval(float capacity, bool inVerificationMode, int& decimalPrecision, float& interval) const {
        bool success;
        capacity = lps_common_weight_from_tonnes(capacity, weightUnits);
        if (inVerificationMode) {
            if ((LpsCommonWeightUnits::KG == weightUnits) ||
                    (LpsCommonWeightUnits::LB == weightUnits)) {
                // In verification mode, if it configured at kg or lb, then use 1.0 scale interval.
                // This was required when doing European Certification (metric) with US test weights (pounds) in order
                // to reduce uncertainty due to units conversion and rounding.
                decimalPrecision = 0;
                interval = 1.f;
                success = true;
            }
            else if (weightPrecision == LpsCommonWeightPrecision::HIGH) {
                // If already high precision, leave it there.
                success = lps_common_weight_get_interval(capacity, weightPrecision, decimalPrecision, interval);
            }
            else {
                success = lps_common_weight_get_interval(capacity, LpsCommonWeightPrecision::MEDIUM, decimalPrecision, interval);
            }
        }
        else {
            success = lps_common_weight_get_interval(capacity, weightPrecision, decimalPrecision, interval);
        }
        return success;
    }

    inline void makeUnitsConsistent() {
        if (LpsSaUIDisplaySettingsUnits::ENGLISH == units) {
            switch (weightUnits) {
            case (LpsCommonWeightUnits::TON):
            case (LpsCommonWeightUnits::KLB):
            case (LpsCommonWeightUnits::LB): {
                // Already consistent
                break;
            }
            default: {
                weightUnits = LpsCommonWeightUnits::TON;
                break;
            }
            }
        }
        else {
            if (LpsSaUIDisplaySettingsUnits::METRIC != units) {
                units = LpsSaUIDisplaySettingsUnits::METRIC;
            }

            switch (weightUnits) {
            case (LpsCommonWeightUnits::TONNE):
            case (LpsCommonWeightUnits::KG): {
                // Already consistent
                break;
            }
            default: {
                weightUnits = LpsCommonWeightUnits::TONNE;
                break;
            }
            }
        }
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & units;
        ar & language;
        ar & brightness;
        ar & timeFormat;
        ar & dateFormat;
        ar & weightUnits;

        { // Older versions had weight resolution in this place.
            int8_t weightResolution = 1;
            ar & weightResolution;
        }

        ar & serviceModeEnableCode;

        if (version >= 2) {
            ar & weightPrecision;
        }
        if (version >= 3) {
            ar & keyboardLayoutSetting;
        }
    }
};

BOOST_CLASS_VERSION(LpsSaUIDisplaySettings, 3);

#endif /* LPSSAUI_DISPLAYSETTINGS_HPP */

