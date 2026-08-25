#ifndef LPSSAWEIGHAPPNVM_H
#define LPSSAWEIGHAPPNVM_H

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>
#include <algorithm>
#include <cmath>
#include <iterator>

#include <boost/serialization/version.hpp>
#include <boost/serialization/array.hpp>
#include <boost/filesystem.hpp>

#include <hub/filesystem/extensions/nvm.h>

#include <fileio/oflocker.hpp>

#include "LpsSaIncludes.h"

#include "src_nvm/app_nvm_file_access.h"

/*
 * Base class for NVM Storage
 */
template <class T>
class NvmStorageWrapper : public T {
public:
    NvmStorageWrapper() : saveNeeded_{false} {}
    virtual ~NvmStorageWrapper() {}

    void setFilePath(const boost::filesystem::path& path, const std::string& fileName) {
        path_ = path;
        fileName_ = fileName;
    }

    std::string getFilePath() {
        return path_.string() + fileName_;
    }

    bool load() {
        bool success = ais::filesystem::file::read(path_, fileName_, *this);

        if (success) {
            saveNeeded_ = false;
        }

        return success;
    }

    // Remove stored files
    bool remove() {
        return ais::filesystem::sha1_fstream::remove_files(ais::filesystem::dir::makeFilePath(path_, fileName_));
    }

    bool save(bool force = false) const {
        bool success = true;

        if (force || saveNeeded_) {
            success = ais::filesystem::file::store(path_, fileName_, *this);
            if (success) {
                saveNeeded_ = false;
            }
        }

        return success;
    };

    void setSaveNeeded() { saveNeeded_ = true; }

    void clearSaveNeeded() { saveNeeded_ = false; }

    void toJson(std::ostream& os, const std::string& fileNameStem) const override {
        T::toJson(os, fileNameStem);
    }

    bool publish(const boost::filesystem::path& filePath) const {
        bool success = false;

        { // Write to the file.
            tes_common_ais::OFlocker ofl(filePath);
            if (ofl) {
                toJson(ofl.ofstream(), filePath.stem().string());
                ofl.close();
                success = true;
                AIS_LOG_INFO("%s: published JSON file.", filePath.filename().c_str());
            }
            else {
                AIS_LOG_ERROR("%s: file could not be opened and locked for publishing.", filePath.filename().c_str());
            }
        }

        if (!success) {
            try {
                if (boost::filesystem::is_regular_file(filePath)) {
                    boost::filesystem::remove(filePath);
                    AIS_LOG_WARN("%s: file removed.", filePath.filename().c_str());
                }
            }
            catch (const boost::filesystem::filesystem_error& e) {
                AIS_LOG_WARN("%s: file could not be removed.", filePath.filename().c_str());
            }
        }

        return success;
    }

private:
    boost::filesystem::path path_;
    std::string fileName_;
    mutable bool saveNeeded_;
};


/*
 * LpsSaWeighApp Tilt Sensor Calibration NVM Storage
 */
class LpsSaTiltCalNvmTblStorage {

public:
    LpsSaTiltCalNvmTblStorage() :
        tilt_sensor_type{TILT_SENSOR_TYPE_NA},
        tilt_full_rack_dc{0.f},
        tilt_full_dump_dc{0.f},
        tilt_full_dump_dc_recd{false},
        tilt_full_rack_dc_recd{false},
        tilt_full_rack_stop_angle{0.f},
        tilt_full_dump_stop_angle{0.f},
        tilt_cal_stat{0}, // Not calibrated
        tilt_cal_counter{0}
    {}

    LpsSaTiltCalNvmTblStorage(const LpsSaTiltCalNvmTblStorage&) = default;

    // Convert from CSNS NVM file format to LpsSaTiltCalNvmTbl. Should only be used for backwards compatibility the first
    // time the software is flashed, after which the old file should be removed
    void convertFromCsnsFormat(const LpsSaTiltCalNvmTbl_t& other, LpsSaTiltSensorType_t tiltSensorType) {
        tilt_sensor_type = tiltSensorType;
        tilt_full_rack_dc = other._tilt_full_rack_dc;
        tilt_full_dump_dc = other._tilt_full_dump_dc;

        // Some very old calibration files were messed up and didn't set the proper calibration flag.
        if (0 != other._tilt_cal_stat) {
            tilt_cal_stat = CAL_TILT_LINKAGE_MASK;
        }
        else {
            tilt_cal_stat = 0;
        }

        // Reset calibration status if using Inline Cylinder sensors
        if (tilt_sensor_type != TILT_SENSOR_TYPE_ROTARY) {
            tilt_cal_stat = 0;
        }

        tilt_cal_counter = 0;
    }

    LpsSaTiltSensorType_t tilt_sensor_type;
    float tilt_full_rack_dc;
    float tilt_full_dump_dc;

    /* add for receiving cal values over datalink */
    bool tilt_full_dump_dc_recd;
    bool tilt_full_rack_dc_recd;

    float tilt_full_rack_stop_angle;
    float tilt_full_dump_stop_angle;
    uint16_t tilt_cal_stat; // steps_complete renamed to CalStat.
    uint32_t tilt_cal_counter;

    virtual void toJson(std::ostream& os, const std::string& fileNameStem) const;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & tilt_sensor_type;
        ar & tilt_full_rack_dc;
        ar & tilt_full_dump_dc;
        ar & tilt_full_dump_stop_angle;
        ar & tilt_full_rack_stop_angle;
        ar & tilt_cal_stat;

        if (version >= 1) {
            ar & tilt_cal_counter;
        }
    }
};

typedef NvmStorageWrapper<LpsSaTiltCalNvmTblStorage> LpsSaTiltCalNvmTbl;

BOOST_CLASS_VERSION(LpsSaTiltCalNvmTbl, 1);

/*
 * LpsSaWeighApp Lift Sensor Calibration NVM Storage
 */
class LpsSaLiftCalNvmTblStorage {

public:
    LpsSaLiftCalNvmTblStorage() :
        lift_full_raise_dc{0.f},
        lift_full_lower_dc{0.f},
        lift_full_raise_dc_recd{false},
        lift_full_lower_dc_recd{false},
        lift_cal_stat{0}, // Not calibrated
        lift_cal_counter{0}
    {}

    LpsSaLiftCalNvmTblStorage(const LpsSaLiftCalNvmTblStorage&) = default;

    // Convert from CSNS NVM file format to LpsSaLiftCalNvmTbl. Should only be used for backwards compatibility the first
    // time the software is flashed, after which the old file should be removed
    void convertFromCsnsFormat(const LpsSaLiftCalNvmTbl_t& other) {
        lift_full_raise_dc = other._lift_full_raise_dc;
        lift_full_lower_dc = other._lift_full_lower_dc;

        // Some very old calibration files were messed up and didn't set the proper calibration flag.
        if (0 != other._lift_cal_stat) {
            lift_cal_stat = CAL_LIFT_LINKAGE_MASK;
        }
        else {
            lift_cal_stat = 0;
        }

        lift_cal_counter = 0;
    }

    float lift_full_raise_dc;
    float lift_full_lower_dc;

    /* add for receiving cal values over datalink */
    bool lift_full_raise_dc_recd;
    bool lift_full_lower_dc_recd;

    uint16_t lift_cal_stat; // steps_complete renamed to CalStat.
    uint32_t lift_cal_counter;

    virtual void toJson(std::ostream& os, const std::string& fileNameStem) const;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & lift_full_raise_dc;
        ar & lift_full_lower_dc;
        ar & lift_cal_stat;

        if (version >= 1) {
            ar & lift_cal_counter;
        }
    }
};

typedef NvmStorageWrapper<LpsSaLiftCalNvmTblStorage> LpsSaLiftCalNvmTbl;

BOOST_CLASS_VERSION(LpsSaLiftCalNvmTbl, 1);

/*
 * LpsSaWeighApp Payload Calibration NVM Storage
 */
class LpsSaPayloadCalNvmTblStorage {

public:
    LpsSaPayloadCalNvmTblStorage() :
        data {0 /* Not Calibrated */},
        calCounter{0},
        legalForTradeInstalled{false} {
            clearAdvCalAdjust();
        }

        LpsSaPayloadCalNvmTblStorage(const LpsSaPayloadCalNvmTblStorage&) = default;

    // Convert from CSNS NVM file format to LpsSaCalNvmTbl_t. Should only be used for backwards compatibility the first
    // time the software is flashed, after which the old file should be removed
    bool convertFromCsnsFormat(const LpsSaCalNvmTbl_t& other) {

        if (sizeof(data.SlowRaiseEmptyBktLiftHt) != sizeof(other._SlowRaiseEmptyBktLiftHt)) {
            return false;
        }

        data.CalStatus = other._CalStat;

        memcpy(data.SlowRaiseEmptyBktLiftHt, other._SlowRaiseEmptyBktLiftHt, sizeof(data.SlowRaiseEmptyBktLiftHt));
        memcpy(data.SlowRaiseEmptyBktLiftPres, other._SlowRaiseEmptyBktLiftPres, sizeof(data.SlowRaiseEmptyBktLiftPres));
        memcpy(data.SlowRaiseFullBktLiftHt, other._SlowRaiseFullBktLiftHt, sizeof(data.SlowRaiseFullBktLiftHt));
        memcpy(data.SlowRaiseFullBktLiftPres, other._SlowRaiseFullBktLiftPres, sizeof(data.SlowRaiseFullBktLiftPres));

        memcpy(data.SlowLowerEmptyBktLiftHt, other._SlowLowerEmptyBktLiftHt, sizeof(data.SlowLowerEmptyBktLiftHt));
        memcpy(data.SlowLowerEmptyBktLiftPres, other._SlowLowerEmptyBktLiftPres, sizeof(data.SlowLowerEmptyBktLiftPres));
        memcpy(data.SlowLowerFullBktLiftHt, other._SlowLowerFullBktLiftHt, sizeof(data.SlowLowerFullBktLiftHt));
        memcpy(data.SlowLowerFullBktLiftPres, other._SlowLowerFullBktLiftPres, sizeof(data.SlowLowerFullBktLiftPres));

        data.EmptyBktSlowRaiseSpd = other._EmptyBktSlowRaiseSpd;
        data.EmptyBktFastRaiseSpd = other._EmptyBktFastRaiseSpd;
        data.FullBktSlowRaiseSpd = other._FullBktSlowRaiseSpd;
        data.FullBktFastRaiseSpd = other._FullBktFastRaiseSpd;

        data.CalWeight = other._Calwt;

        data.ShmEmptyMain = other._ShmEmptyMain;
        data.ShmFullMain = other._ShmFullMain;

        data.EmptyTemp = other._EmptyTemp;
        data.FullTemp = other._FullTemp;

        data.EmptyBucketWeightEst = other._EmptyBucketWeightEst;

        data.CalAdjust = other._CalAdjust;
        data.ZeroWeight = other._ZeroWeight;

        data.SlowRaiseEmptyIMUAccel[0] = data.SlowRaiseEmptyIMUAccel[1] = data.SlowRaiseEmptyIMUAccel[2] = 0.f;
        data.SlowRaiseFullIMUAccel[0] = data.SlowRaiseFullIMUAccel[1] = data.SlowRaiseFullIMUAccel[2] = 0.f;

        calCounter = 0;

        data.SlowRaiseEmptyPitch = 0.f;
        data.SlowRaiseEmptyRoll = 0.f;
        data.SlowRaiseFullPitch = 0.f;
        data.SlowRaiseFullRoll = 0.f;

        setAdvCalAdjust({{0.f, 0.f}, {other._Calwt, other._Calwt}});

        legalForTradeInstalled = false;

        return true;
    }

    bool isIMUDataValid() const {
        bool valid = true;
        if ((0.f == data.SlowRaiseEmptyIMUAccel[0]) &&
                (0.f == data.SlowRaiseEmptyIMUAccel[1]) &&
                (0.f == data.SlowRaiseEmptyIMUAccel[2])) {
            valid = false;
        }
        else if ((0.f == data.SlowRaiseFullIMUAccel[0]) &&
                (0.f == data.SlowRaiseFullIMUAccel[1]) &&
                (0.f == data.SlowRaiseFullIMUAccel[2])) {
            valid = false;
        }
        return valid;
    }

    void clearAdvCalAdjust() {
        for (auto ii = 0; ii < LPS_CAL_ADV_CAL_ADJUST_NUM_POINTS; ++ii) {
            data.AdvCalAdjustX[ii] = NAN;
            data.AdvCalAdjustY[ii] = NAN;
        }
    }

    void setAdvCalAdjust(const std::vector<std::pair<float, float>>& cmap) {
        using point = std::pair<float, float>;
        std::vector<point> map;

        // Only keep the finite numbers
        std::copy_if(cmap.cbegin(), cmap.cend(),
                std::back_inserter(map), [](const point& p) {
            return std::isfinite(p.first) && std::isfinite(p.second);
        });

        // Sort the entries
        std::sort(map.begin(), map.end(), [](const point& p1, const point& p2) {
            bool p1First = p1.first < p2.first;
            if (p1.first == p2.first) {
                p1First = p1.second > p2.second;
            }
            return p1First;
        });

        // Only keep unique x values
        auto last = std::unique(map.begin(), map.end(), [](const point& p1, const point& p2) {
            return p1.first == p2.first;
        });

        map.erase(last, map.end());

        // Fill in the final map
        for (unsigned int ii = 0; ii < LPS_CAL_ADV_CAL_ADJUST_NUM_POINTS; ++ii) {
            if (ii < map.size()) {
                const point& p = map[ii];
                data.AdvCalAdjustX[ii] = p.first;
                data.AdvCalAdjustY[ii] = p.second;
            }
            else {
                data.AdvCalAdjustX[ii] = NAN;
                data.AdvCalAdjustY[ii] = NAN;
            }
        }
    }

    LpsCalNvmTbl_t data;
    uint32_t calCounter;
    bool legalForTradeInstalled;

    virtual void toJson(std::ostream& os, const std::string& fileNameStem) const;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & data.CalStatus;

        ar & boost::serialization::make_array<float>(data.SlowRaiseEmptyBktLiftHt, LPS_CAL_CURVE_FIT_NUM_POINTS);
        ar & boost::serialization::make_array<float>(data.SlowRaiseEmptyBktLiftPres, LPS_CAL_CURVE_FIT_NUM_POINTS);
        ar & boost::serialization::make_array<float>(data.SlowRaiseFullBktLiftHt, LPS_CAL_CURVE_FIT_NUM_POINTS);
        ar & boost::serialization::make_array<float>(data.SlowRaiseFullBktLiftPres, LPS_CAL_CURVE_FIT_NUM_POINTS);

        ar & boost::serialization::make_array<float>(data.SlowLowerEmptyBktLiftHt, LPS_CAL_CURVE_FIT_NUM_POINTS);
        ar & boost::serialization::make_array<float>(data.SlowLowerEmptyBktLiftPres, LPS_CAL_CURVE_FIT_NUM_POINTS);
        ar & boost::serialization::make_array<float>(data.SlowLowerFullBktLiftHt, LPS_CAL_CURVE_FIT_NUM_POINTS);
        ar & boost::serialization::make_array<float>(data.SlowLowerFullBktLiftPres, LPS_CAL_CURVE_FIT_NUM_POINTS);

        ar & data.FastRaiseEmptyBktDeltaPres;
        ar & data.FastRaiseFullBktDeltaPres;

        ar & data.EmptyBktSlowRaiseSpd;
        ar & data.EmptyBktFastRaiseSpd;
        ar & data.FullBktSlowRaiseSpd;
        ar & data.FullBktFastRaiseSpd;

        ar & data.CalWeight;

        ar & data.ShmEmptyMain;
        ar & data.ShmFullMain;

        ar & data.EmptyTemp;
        ar & data.FullTemp;

        ar & data.EmptyBucketWeightEst;

        ar & data.CalAdjust;
        ar & data.ZeroWeight;

        if (version >= 2) {
            ar & boost::serialization::make_array<float>(data.SlowRaiseEmptyIMUAccel, 3);
            ar & boost::serialization::make_array<float>(data.SlowRaiseFullIMUAccel, 3);
        }

        if (version >= 1) {
            ar & calCounter;
        }

        if (version >= 3) {
            ar & legalForTradeInstalled;
        }

        if (version >= 4) {
            ar & data.CalVersion;
        }

        if (version >= 5) {
            ar & data.SlowRaiseEmptyPitch;
            ar & data.SlowRaiseEmptyRoll;
            ar & data.SlowRaiseFullPitch;
            ar & data.SlowRaiseFullRoll;
        }

        if (version >= 6) {
            ar & boost::serialization::make_array<float>(data.AdvCalAdjustX, LPS_CAL_ADV_CAL_ADJUST_NUM_POINTS);
            ar & boost::serialization::make_array<float>(data.AdvCalAdjustY, LPS_CAL_ADV_CAL_ADJUST_NUM_POINTS);
        }
    }
};

typedef NvmStorageWrapper<LpsSaPayloadCalNvmTblStorage> LpsSaPayloadCalNvmTbl;

BOOST_CLASS_VERSION(LpsSaPayloadCalNvmTbl, 6);

static_assert(LPS_CAL_CURVE_FIT_NUM_POINTS == 11, "Cannot change the table size.");
static_assert(LPS_CAL_ADV_CAL_ADJUST_NUM_POINTS == 5, "Cannot change the table size.");

#endif
