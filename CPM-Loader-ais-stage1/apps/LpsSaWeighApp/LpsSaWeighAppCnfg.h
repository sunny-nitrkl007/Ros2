#ifndef LPSSAWEIGHAPPCNFG_H
#define LPSSAWEIGHAPPCNFG_H

#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/filesystem.hpp>

/*
 * LpsSaWeighApp Configuration Storage
 */
struct LpsSaWeighAppCnfg {
public:
    LpsSaWeighAppCnfg() :
        tipoffPitchCalOffset(0.f),
        toaAnchoredZeroOffset(0.f),
        toaAnchoredFactor(1.f),
        hydOilTempEnabled(true),
        imuCompEnabled(false),
        loadCheckValveInstalled(false),
        audibleWeightEnabled(false),
        overloadWarningEnabled(false),
        bucketPayloadTargetWeight(0.f),
        lastSuggestedCalWeight(-1.f), // Invalid
        weighRangeStart(50.f),
        weighRangeSize(15.f),
        reweighMaxPitch(5.0), // 5 degrees pitch uphill
        reweighMinPitch(-5.0), // 5 degrees pitch downhill
        reweighMaxAbsRoll(5.0), // 5 degrees roll left/right
        reweighMinLiftCylVel(0.0), // default unlimited
        flashEnabled(true),
        filePath_(""),
        saveNeeded_(false) {}

    LpsSaWeighAppCnfg(const LpsSaWeighAppCnfg&) = default;

    float tipoffPitchCalOffset;
    float toaAnchoredZeroOffset;
    float toaAnchoredFactor;
    bool hydOilTempEnabled;
    bool imuCompEnabled;
    bool loadCheckValveInstalled;
    bool audibleWeightEnabled;
    bool overloadWarningEnabled;
    float bucketPayloadTargetWeight;
    float lastSuggestedCalWeight;
    float weighRangeStart;
    float weighRangeSize;
    float reweighMaxPitch; // Maximum allowed pitch up-hill for green box, in degrees
    float reweighMinPitch; // Minimum allowed pitch down-hill for green box, in degrees
    float reweighMaxAbsRoll; // Maximum allowed absolute value of roll for green box, in degrees
    float reweighMinLiftCylVel; // Minimum allowed lift cylinder velocity for green box, in mm/s
    bool flashEnabled;

    inline void setFilePath(const boost::filesystem::path& filePath) {
        filePath_ = filePath.string();
    }

    bool load();
    bool reset();
    bool save(bool force = false) const;

    void setSaveNeeded() { saveNeeded_ = true; }

    bool publish(const boost::filesystem::path& filePath) const;

private:
    std::string filePath_;
    mutable bool saveNeeded_;

    void toJson(std::ostream& os, const std::string& fileNameStem) const;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & tipoffPitchCalOffset;

        if (version >= 1) {
            ar & toaAnchoredZeroOffset;
            ar & toaAnchoredFactor;
        }

        if (version >= 2) {
            ar & hydOilTempEnabled;
        }

        if (version >= 3) {
            ar & audibleWeightEnabled;
        }

        if (version >= 4) {
            ar & loadCheckValveInstalled;
        }

        if (version >= 5) {
            ar & overloadWarningEnabled;
            ar & bucketPayloadTargetWeight;
            ar & lastSuggestedCalWeight;
            ar & weighRangeStart;
            ar & weighRangeSize;
        }

        if (version >= 6) {
            ar & flashEnabled;
        }

        if (version >= 7) {
            ar & reweighMaxPitch;
            ar & reweighMinPitch;
            ar & reweighMaxAbsRoll;
            ar & reweighMinLiftCylVel;
        }

        if (version >= 8) {
            ar & imuCompEnabled;
        }
    }
};

BOOST_CLASS_VERSION(LpsSaWeighAppCnfg, 8);

#endif
