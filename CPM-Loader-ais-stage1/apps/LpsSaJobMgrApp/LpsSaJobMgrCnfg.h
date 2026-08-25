#ifndef LPSSAJOBMGRCNFG_H
#define LPSSAJOBMGRCNFG_H

#include <cstdint>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/filesystem.hpp>

#include <LpsPtPublic.h>

#define LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MIN (1)
#define LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_MAX (999)

#define LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_DEFAULT (50)

/*
 * LpsSaJobMgr Configuration Storage
 */
struct LpsSaJobMgrCnfg {
public:
    LpsSaJobMgrCnfg() :
        hornStoreEnable(false),
        tipOffTriggerType(TIP_OFF_TRIGGER_DISABLED),
        tipOffMode(TIP_OFF_MODE_PILE),
        autoStorePassCount(LPSSAJOBMGRCNFG_AUTO_STORE_PASS_COUNT_DEFAULT),
        autoTruckIdEnabled(false),
        autoMaterialIdEnabled(false),
        manualAddEnabled(false),
        multiTaskEnabled(false),
        truckListEnabled(true),
        materialListEnabled(true),
        tag1Enabled(false),
        tag2Enabled(false),
        tag3Enabled(false),
        tag4Enabled(false),
        splitModeEnabled(false) {}

    bool hornStoreEnable;
    uint8_t  tipOffTriggerType;
    uint8_t  tipOffMode;
    uint16_t autoStorePassCount;
    bool autoTruckIdEnabled;
    bool autoMaterialIdEnabled;
    bool manualAddEnabled;
    bool multiTaskEnabled;
    bool truckListEnabled;
    bool materialListEnabled;
    bool tag1Enabled;
    bool tag2Enabled;
    bool tag3Enabled;
    bool tag4Enabled;
    bool splitModeEnabled;

    inline void reset() {
        *this = LpsSaJobMgrCnfg();
    }

    bool load(const boost::filesystem::path& filePath);
    bool save(const boost::filesystem::path& filePath) const;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & hornStoreEnable;
        ar & tipOffTriggerType;
        ar & tipOffMode;

        if (version >= 1) {
            ar & autoStorePassCount;
        }

        if (version >= 2) {
            ar & autoTruckIdEnabled;
            ar & autoMaterialIdEnabled;
        }

        if (version >= 3) {
            ar & manualAddEnabled;
            ar & multiTaskEnabled;
        }

        if (version >= 4) {
            ar & tag1Enabled;
            ar & tag2Enabled;
            ar & tag3Enabled;
            ar & tag4Enabled;
        }

        if (version >= 5) {
            ar & truckListEnabled;
            ar & materialListEnabled;
        }

        if (version >= 6) {
            ar & splitModeEnabled;
        }
    }
};

BOOST_CLASS_VERSION(LpsSaJobMgrCnfg, 6);

#endif
