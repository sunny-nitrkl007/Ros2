/*
 * LpsSaWeighAppSeal.h
 */

#ifndef LPSSAWEIGHAPPSEAL_H_
#define LPSSAWEIGHAPPSEAL_H_

#include <string>
#include <cstdint>
#include <chrono>
#include <vector>
#include <utility>
#include <type_traits>
#include <cmath>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/adaptor/reversed.hpp>

// Rapid JSON
#include <ext/rapidjson/RapidJsonGuarded.h>

#include <ais/log/Logger.h>
#include <hub/filesystem/extensions/nvm.h>

#include <chrono/print.hpp>
#include <fileio/oflocker.hpp>

#include <catdllib_fid_def.h>
#include <LpsCommonWeight.h>

#include <release/legal_for_trade_identifiers.h>

#include <interfaces/LpsSaWeighTxChannel/LpsSaWeighTxChannel.h>

#include "LpsSaWeighAppNvm.h"

/* ACCEPTABLE SOLUTION
 * Event counter / event logger:
 * - An event counter registers each change of a device-specific parameter value.
 *   The current count can be displayed and can be compared with the initial value
 *   of the counter that was registered before putting the measuring instrument
 *   into use or at the last official verification respectively and is indelibly
 *   labelled on the instrument.
 * - Changes of device-specific parameters are registered in an event logger.
 *   It is an information record stored in a non-volatile memory. Each entry
 *   is generated automatically by the legally relevant software and contains:
 *   - the identifier of the parameter (e.g. the name)
 *   - the parameter value (the current or the value before)
 *   - the time stamp of the change
 * - The event logger cannot be deleted or be changed without destroying a seal.
 *   The content of the event logger is shown on the display or printed upon command.
 */


/*
 * LpsSaWeighAppSeal - Legal For Trade Seal Storage
 */
class LpsSaWeighAppSeal {

public:

/*----------------
 * We need to undefine True and False, because Xlib.h defines
 * these globally.
 */
#ifdef True
    #undef True
#endif

#ifdef False
    #undef False
#endif
/*---------------*/

    enum class Ternary_t : int_least8_t {
        Unknown = -1, False = 0, True = 1
    };


    LpsSaWeighAppSeal() : status() {}

    // Load data from storage file(s)
    bool load(const boost::filesystem::path& path, const std::string& fileName) {
        bool success = ais::filesystem::file::read(path, fileName, *this);

        if (success) {
            saveNeeded_ = false;
        }

        return success;
    }

    // Remove stored file(s)
    bool remove(const boost::filesystem::path& path, const std::string& fileName) {
        return ais::filesystem::sha1_fstream::remove_files(ais::filesystem::dir::makeFilePath(path, fileName));
    }

    // Save data to storage file(s)
    bool save(const boost::filesystem::path& path, const std::string& fileName, bool force = false) const {
        bool success = true;

        if (force || saveNeeded_) {
            success = ais::filesystem::file::store(path, fileName, *this);
            if (success) {
                saveNeeded_ = false;
            }
        }

        return success;
    };

    void setSaveNeeded() { saveNeeded_ = true; }

    bool setSealStatus(bool isSealed, const std::chrono::system_clock::time_point& time) {
        if (isSealed != status.sealed) {
            status.sealed = isSealed;
            status.sealTime = time;

            if (isSealed) {
                ++status.sealId;
            }

            saveNeeded_ = true;
            return true;
        }
        return false;
    }

    // Seal Status
    LpsSaLftSealStatus_t status;

    // Machine Configuration Code (Internal MSN)
    std::string machineConfigurationCode;

    // Product Identifier (PIN)
    std::string productId;

    // Software Part Number
    std::string softwarePartNumber;

    // Legal For Trade Software Enabled
    bool lftSoftwareEnabled = false;

    // Legal For Trade Software Identifier
    std::string lftSoftwareIdVersion;
    std::string lftSoftwareIdBuild;

    // ECM Serial Number
    std::string ecmSerialNumber;

    // Last Service Tool Serial Number
    std::string lastServiceToolSerialNumber;

    // Input Module Software Part Number
    std::string inputModuleSoftwarePartNumber;

    // Input Module Legal For Trade Software Identifier
    std::string inputModuleLftSoftwareIdVersion;
    std::string inputModuleLftSoftwareIdBuild;

    // Input Module ECM Serial Number
    std::string inputModuleEcmSerialNumber;

    // Input Module Last Service Tool Serial Number
    std::string inputModuleLastServiceToolSerialNumber;

    // Lift Position Sensor Identifier
    std::string liftPositionSensorId;

    // Tilt Position Sensor Identifier
    std::string tiltPositionSensorId;

    // Lift Head End Pressure Sensor Identifier
    std::string liftHeadEndPressureSensorId;

    // Lift Rod End Pressure Sensor Identifier
    std::string liftRodEndPressureSensorId;

    // Hydraulic Oil Temperature Sensor Identifier
    std::string hydraulicOilTemperatureSensorId;

    // Inertial Measurement Unit (IMU) Software Part Number
    std::string imuSoftwarePartNumber;

    // Inertial Measurement Unit (IMU) Serial Number
    std::string imuSerialNumber;

    // Lift Position Sensor Calibration Status
    bool liftPositionSensorIsCalibrated = false;
    uint_least32_t liftPositionSensorCalibrationCounter = 0;

    // Tilt Position Sensor Calibration Status
    bool tiltPositionSensorIsCalibrated = false;
    uint_least32_t tiltPositionSensorCalibrationCounter = 0;

    // Payload Calibration Status
    bool payloadIsCalibrated = false;
    uint_least32_t payloadCalibrationCounter = 0;

    // Full Calibration Weight
    float fullCalibrationWeight = 0.f;

    // Span Adjust
    float spanAdjustFactor = 0.f;

    // Weigh Range
    float weighRangeStart = 50.f;
    float weighRangeSize = 15.f;

    // Weight Measurement Interval and Units
    float weightInterval = 0.f;
    LpsCommonWeightUnits weightUnits = LpsCommonWeightUnits::TONNE;

    // Hydraulic Oil Temperature Compensation Enabled
    Ternary_t hydraulicOilTemperatureEnabled = Ternary_t::Unknown;

    // IMU Pitch/Roll Compensation Enabled
    Ternary_t imuCompensationEnabled = Ternary_t::Unknown;

    // Work Tool Identifier
    std::string workToolId;

    // Ticket Retention Period (days)
    uint16_t ticketRetentionPeriod = 0;

    // Equipment ID
    std::string equipmentId;

private:
    mutable bool saveNeeded_ = false;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & status.sealed;
        ar & status.sealTime;
        ar & status.sealId;

        ar & machineConfigurationCode;
        ar & softwarePartNumber;
        ar & lftSoftwareEnabled;
        ar & lftSoftwareIdVersion;
        ar & lftSoftwareIdBuild;
        ar & ecmSerialNumber;

        ar & inputModuleSoftwarePartNumber;
        ar & inputModuleLftSoftwareIdVersion;
        ar & inputModuleLftSoftwareIdBuild;
        ar & inputModuleEcmSerialNumber;

        ar & liftPositionSensorId;
        ar & tiltPositionSensorId;
        ar & liftHeadEndPressureSensorId;
        ar & liftRodEndPressureSensorId;
        ar & hydraulicOilTemperatureSensorId;

        ar & imuSoftwarePartNumber;
        ar & imuSerialNumber;

        ar & liftPositionSensorIsCalibrated;
        ar & liftPositionSensorCalibrationCounter;

        ar & tiltPositionSensorIsCalibrated;
        ar & tiltPositionSensorCalibrationCounter;

        ar & payloadIsCalibrated;
        ar & payloadCalibrationCounter;

        ar & fullCalibrationWeight;

        ar & spanAdjustFactor;

        ar & weighRangeStart;
        ar & weighRangeSize;

        ar & weightInterval;
        ar & weightUnits;

        if (version >= 2) {
            ar & lastServiceToolSerialNumber;
            ar & inputModuleLastServiceToolSerialNumber;
        }

        if (version >= 3) {
            ar & hydraulicOilTemperatureEnabled;
            ar & workToolId;
        }

        if (version >= 4) {
            ar & ticketRetentionPeriod;
        }

        if (version >= 5) {
            ar & productId;
        }

        if (version >= 6) {
            ar & imuCompensationEnabled;
        }

        if (version >= 7) {
            ar & equipmentId;
        }
    }
};

BOOST_CLASS_VERSION(LpsSaWeighAppSeal, 7);

class LpsSaWeighAppSealChangeLog {
public:

    struct ChangeLogEntry {
        ChangeLogEntry() :
            time(std::chrono::system_clock::time_point::min()),
            componentId(0),
            parameterId(0),
            value(),
            stateId(UNKNOWN2U) {}

        ChangeLogEntry(const std::chrono::system_clock::time_point& time_,
                uint16_t componentId_, uint32_t parameterId_,
                const std::string& value_, uint16_t stateId_) :
                    time(time_), componentId(componentId_),
                    parameterId(parameterId_), value(value_), stateId(stateId_) {}

        std::chrono::system_clock::time_point time;
        uint16_t componentId;
        uint32_t parameterId;
        std::string value;
        uint16_t stateId;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if (version < 1) {
                // we don't support this version
                return;
            }

            ar & time;
            ar & componentId;
            ar & parameterId;
            ar & value;
            ar & stateId;
        }
    };

    static constexpr std::vector<ChangeLogEntry>::size_type MAX_LOG_SIZE = 100;

    LpsSaWeighAppSealChangeLog() :
        entries_(),
        saveNeeded_{false} {}

    // Load data from storage file(s)
    bool load(const boost::filesystem::path& path, const std::string& fileName) {
        bool success = ais::filesystem::file::read(path, fileName, *this);

        if (success) {
            saveNeeded_ = false;
        }

        return success;
    }

    // Remove stored file(s)
    bool remove(const boost::filesystem::path& path, const std::string& fileName) {
        return ais::filesystem::sha1_fstream::remove_files(ais::filesystem::dir::makeFilePath(path, fileName));
    }

    // Save data to storage file(s)
    bool save(const boost::filesystem::path& path, const std::string& fileName, bool force = false) const {
        bool success = true;

        if (force || saveNeeded_) {
            success = ais::filesystem::file::store(path, fileName, *this);
            if (success) {
                saveNeeded_ = false;
            }
        }

        return success;
    };

    void setSaveNeeded() { saveNeeded_ = true; }

    const std::vector<ChangeLogEntry>& entries() const { return entries_; }

    void addEntry(const std::chrono::system_clock::time_point& time,
            uint16_t cid, uint32_t pid,
            const std::string& value, uint16_t stateId = UNKNOWN2U) {
        addEntry(ChangeLogEntry(time, cid, pid, value, stateId));
    }

    template<typename T>
    typename std::enable_if<std::is_convertible<typename std::remove_reference<T>::type, ChangeLogEntry>::value>::type
    addEntry(T&& entry) {
        if (entries_.size() >= MAX_LOG_SIZE) {
            entries_.erase(entries_.cbegin(), entries_.cend() - MAX_LOG_SIZE + 1);
        }
        entries_.push_back(std::forward<T>(entry));
        saveNeeded_ = true;
    }

    bool isFull(const std::chrono::system_clock::time_point& tp) {
        if (entries_.size() < MAX_LOG_SIZE) {
            return false;
        }
        else {
            const auto& it = entries_.cend() - MAX_LOG_SIZE;
            return it->time >= tp;
        }
    }

private:
    /*
     * These entries will be sorted oldest to newest so that new items are appended
     * to the end instead of inserted.
     * When the maximum retention time is exceeded, the front will be chopped off.
     */
    std::vector<ChangeLogEntry> entries_;
    mutable bool saveNeeded_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & entries_;
    }
};

BOOST_CLASS_VERSION(LpsSaWeighAppSealChangeLog, 0);

BOOST_CLASS_VERSION(LpsSaWeighAppSealChangeLog::ChangeLogEntry, 1);

//
class LpsSaWeighAppSealTracker {
public:
    LpsSaWeighAppSealTracker() :
        storageRoot_(),
        seal_(),
        changeLog_() {}

    void initialize(const boost::filesystem::path& storageRoot) {
        storageRoot_ = storageRoot;

        if (!seal_.load(storageRoot, sealFileName_)) {
            seal_ = LpsSaWeighAppSeal();
        }

        if (!changeLog_.load(storageRoot, changeLogFileName_)) {
            // Analysis Module - Data Logger Status
            breakSeal(5495, 0xD1061E, "", 0x0392 /* Empty */);
        }

        reportLegalForTradeSoftwareId(legal_for_trade_software_id_version, legal_for_trade_software_id_build);
    }

    void update() {
        /* uncomment for test
        // Every 10 times I'm going to toggle something in the seal
        if (isSealed()) {
            static int cnt = 0;
            if ((cnt++ % 100) == 0) {
                updateSeal(5495, 0xFFFF, std::to_string(cnt));
            }

            if (!isSealed()) {
                AIS_LOG_ALERT("Seal Broken!");
            }
        }
        */

        // Saves things if needed
        seal_.save(storageRoot_, sealFileName_);
        changeLog_.save(storageRoot_, changeLogFileName_);
    }

    bool isSealed() const { return seal_.status.sealed; }

    const LpsSaLftSealStatus_t& getSealStatus() const { return seal_.status; }

    const LpsSaWeighAppSeal& getSeal() const { return seal_; }

    void seal() {
        seal(std::chrono::system_clock::now());
    }

    void seal(const std::chrono::system_clock::time_point& time) {
        if (!seal_.lftSoftwareEnabled) {
            return; // do not allow sealing the system if LFT is not installed/enabled
        }

        if (seal_.setSealStatus(true, time)) {
            // Payload Monitoring System - Payload Legal For Trade Measurement Seal Status
            changeLog_.addEntry(time, 2183, 0xD11F99 /* Seal Status */, "", 0x0AA9 /* Sealed */);
        }
    }

    // unseal - user un-seals the system
    void unseal() {
        unseal(std::chrono::system_clock::now());
    }

    void unseal(const std::chrono::system_clock::time_point& time) {
        if (seal_.setSealStatus(false, time)) {
            // Payload Monitoring System - Payload Legal For Trade Measurement Seal Status
            changeLog_.addEntry(time, 2183, 0xD11F99 /* Seal Status */, "", 0x0AAA /* Not Sealed */);
        }
    }

    void seal(bool sealed) {
        if (sealed) {
            seal();
        }
        else {
            unseal();
        }
    }

    void updateSeal(uint16_t cid, uint32_t pid, const std::string& value, uint16_t stateId = UNKNOWN2U) {
        auto now = std::chrono::system_clock::now();
        if (seal_.status.sealed && changeLog_.isFull(seal_.status.sealTime)) {
            unseal(now);
        }
        changeLog_.addEntry(now, cid, pid, value, stateId);
    }

    void breakSeal(uint16_t cid, uint32_t pid, const std::string& value, uint16_t stateId = UNKNOWN2U) {
        auto now = std::chrono::system_clock::now();
        unseal(now);
        changeLog_.addEntry(now, cid, pid, value, stateId);
    }

    void reportMachineConfigurationCode(const std::string& code) {
        if (code != seal_.machineConfigurationCode) {
            seal_.machineConfigurationCode = code;
            seal_.setSaveNeeded();

            // Payload Monitoring System - Implement Linkage Configuration
            breakSeal(2183, 0xD10049, code);

            AIS_LOG_ALERT("LFT Seal Broken - Machine Configuration Code Changed.");
        }
    }

    void reportGenericConfigurationChange(const std::string& code) {
        //  ---------                                         ---------------
        // | Payload | Monitoring System - Implement Linkage | Configuration |
        //  ---------                                         ---------------
        breakSeal(2183, 0xD10049, code);

        AIS_LOG_ALERT("LFT Seal Broken - Generic Configuration Changed.");
    }

    // Returns true if the value changes from a known value to the new value.
    bool reportProductId(const std::string& id) {
        if (id != seal_.productId) {
            seal_.productId = id;
            seal_.setSaveNeeded();

            // Payload Monitoring System - Product Identification Number (0xF82D)

            /*
             * We are only tracking this change if it results in a machine
             * configuration code change, which is separately sealed.
             * We are storing it here so that it can be put on the ticket.
             */
            return true;
        }

        return false;
    }

    void reportLegalForTradeEnableStatus(bool enabled) {
        if (enabled != seal_.lftSoftwareEnabled) {
            seal_.lftSoftwareEnabled = enabled;
            seal_.setSaveNeeded();

            // Payload Monitoring System - Payload Legal For Trade Measurement Installation Status
            breakSeal(2183, 0xD11E8D, "", enabled ? 0x0010 : 0x0011);

            AIS_LOG_ALERT("LFT Seal Broken - LFT Enable Status Changed (%d).", enabled);
        }
    }

    /*
     * Analysis Module (This ECM)
     */
    void reportSoftwarePartNumber(const std::string& pn) {
        // Not seal breaking
        if (pn != seal_.softwarePartNumber) {
            seal_.softwarePartNumber = pn;
            seal_.setSaveNeeded();

            // Analysis Module - Software Group Part Number
            updateSeal(5495, 0xF849, pn);
        }
    }

    void reportLegalForTradeSoftwareId(const std::string& version, const std::string& build) {
        // Only seal breaking if major version changes or incompatible minor version change.
        if (version != seal_.lftSoftwareIdVersion) {
            bool compatible = false;

            // Check the compatibility of different minor versions.
            if (endsWith(seal_.lftSoftwareIdVersion, "4.0") || endsWith(seal_.lftSoftwareIdVersion, "4.4")) {
                if (endsWith(version, "4.0") || endsWith(version, "4.4")) {
                    // Standalone versions compatible with other standalone versions.
                    compatible = true;
                }
            }
            else if (endsWith(seal_.lftSoftwareIdVersion, "4.1") || endsWith(seal_.lftSoftwareIdVersion, "4.5")) {
                if (endsWith(version, "4.1") || endsWith(version, "4.5")) {
                    // Advanced versions compatible with other advanced versions.
                    compatible = true;
                }
            }

            seal_.lftSoftwareIdVersion = version;
            seal_.lftSoftwareIdBuild = build;
            seal_.setSaveNeeded();

            // Payload Monitoring System - Payload Legal For Trade Measurement Software Identifier
            if (compatible) {
                updateSeal(2183, 0xF9C0, version + "." + build);
            }
            else {
                breakSeal(2183, 0xF9C0, version + "." + build);
            }

            AIS_LOG_ALERT("LFT Seal Broken - LFT Software Id Changed (%s).", version.c_str());
        }
        else if (build != seal_.lftSoftwareIdBuild) {
            seal_.lftSoftwareIdBuild = build;
            seal_.setSaveNeeded();

            // Payload Monitoring System - Payload Legal For Trade Measurement Software Identifier
            updateSeal(2183, 0xF9C0, version + "." + build);
        }
        else {
            // No change
        }

        // TODO - Do we need to compare this against the value reported by implement?
    }

    void reportEcmSerialNumber(const std::string& sn) {
        // Seal breaking, althought I pretty difficult to get to happen.
        if (sn != seal_.ecmSerialNumber) {
            seal_.ecmSerialNumber = sn;
            seal_.setSaveNeeded();

            // Analysis Module - Electronic Control Module Serial Number
            breakSeal(5495, 0xF811, sn);

            AIS_LOG_ALERT("LFT Seal Broken - ECM Serial Number Changed (%s).", sn.c_str());
        }
    }

    void reportLastServiceToolSerialNumber(const std::string& sn) {
        // Not seal breaking
        if (sn != seal_.lastServiceToolSerialNumber) {
            seal_.lastServiceToolSerialNumber = sn;
            seal_.setSaveNeeded();

            // Analysis Module - Flash Programming Service Tool Serial Number
            updateSeal(5495, 0xF80F, sn);
        }
    }

    /*
     * Input Module (Implement ECM)
     */
    void reportInputModuleSoftwarePartNumber(const std::string& pn) {
        // Not seal breaking
        if (pn != seal_.inputModuleSoftwarePartNumber) {
            seal_.inputModuleSoftwarePartNumber = pn;
            seal_.setSaveNeeded();

            // Implement Control - Software Group Part Number
            updateSeal(596, 0xF849, pn);
        }
    }

    /*
     * Equipment ID
     */
    void reportEquipmentId(const std::string& id) {
        auto old_value = seal_.equipmentId;
        if (id != old_value) {
            seal_.equipmentId = id;
            seal_.setSaveNeeded();

            if (!old_value.empty()) {
                // Payload Monitoring System - Equipment ID - using the generic configuration code
                //breakSeal(2183, 0xF81A, id);
                breakSeal(2183, 0xD10049, "Equipment ID: " + id);

                AIS_LOG_ALERT("LFT Seal Broken - Equipment ID Changed.");
            }
        }
    }

    void reportInputModuleLegalForTradeSoftwareId(const std::string& version, const std::string& build) {
        if (version != seal_.inputModuleLftSoftwareIdVersion) {
            seal_.inputModuleLftSoftwareIdVersion = version;
            seal_.inputModuleLftSoftwareIdBuild = build;
            seal_.setSaveNeeded();

            // Implement Control - Payload Legal For Trade Measurement Software Identifier
            breakSeal(596, 0xF9C0, version + "." + build);

            AIS_LOG_ALERT("LFT Seal Broken - Input Module LFT Software Id Changed (%s).", version.c_str());
        }
        else if (build != seal_.inputModuleLftSoftwareIdBuild) {
            seal_.inputModuleLftSoftwareIdBuild = build;
            seal_.setSaveNeeded();

            // Implement Control - Payload Legal For Trade Measurement Software Identifier
            updateSeal(596, 0xF9C0, version + "." + build);
        }
        else {
            // no change
        }
        // TODO - Do we need to compare this against the version of this software?
    }

    // Returns true if the value changes from a known value to the new value.
    bool reportInputModuleEcmSerialNumber(const std::string& sn) {
        if (sn != seal_.inputModuleEcmSerialNumber) {
            bool wasEmpty = seal_.inputModuleEcmSerialNumber.empty();

            seal_.inputModuleEcmSerialNumber = sn;
            seal_.setSaveNeeded();

            // Implement Control - Electronic Control Module Serial Number
            breakSeal(596, 0xF811, sn);

            AIS_LOG_ALERT("LFT Seal Broken - Input Module ECM Serial Number Changed (%s).", sn.c_str());

            return wasEmpty ? false : true;
        }

        return false;
    }

    void reportInputModuleLastServiceToolSerialNumber(const std::string& sn) {
        // Not seal breaking
        if (sn != seal_.inputModuleLastServiceToolSerialNumber) {
            seal_.inputModuleLastServiceToolSerialNumber = sn;
            seal_.setSaveNeeded();

            // Implement Control - Flash Programming Service Tool Serial Number
            updateSeal(596, 0xF80F, sn);
        }
    }

    /*
     * Position Sensors
     */
    void reportLiftPositionSensorId(const std::string& id) {
        if (id != seal_.liftPositionSensorId) {
            seal_.liftPositionSensorId = id;
            seal_.setSaveNeeded();

            // Lift Linkage Position Sensor - Electronic Control Module Serial Number
            breakSeal(350, 0xF811 /* Seal Identifier */, id);

            AIS_LOG_ALERT("LFT Seal Broken - Lift Position Sensor Id Changed (%s).", id.c_str());
        }
    }

    void reportTiltPositionSensorId(const std::string& id) {
        if (id != seal_.tiltPositionSensorId) {
            seal_.tiltPositionSensorId = id;
            seal_.setSaveNeeded();

            // Tilt Linkage Position Sensor - Electronic Control Module Serial Number
            breakSeal(351, 0xF811 /* Seal Identifier */, id);

            AIS_LOG_ALERT("LFT Seal Broken - Tilt Position Sensor Id Changed (%s).", id.c_str());
        }
    }

    /*
     * Pressure Sensors
     */
    void reportLiftHeadEndPressureSensorId(const std::string& id) {
        if (id != seal_.liftHeadEndPressureSensorId) {
            seal_.liftHeadEndPressureSensorId = id;
            seal_.setSaveNeeded();

            // Lift Cylinder Head End Pressure Sensor - Electronic Control Module Serial Number
            breakSeal(364, 0xF811 /* Seal Identifier */, id);

            AIS_LOG_ALERT("LFT Seal Broken - Lift Head End Pressure Sensor Id Changed (%s).", id.c_str());
        }
    }

    void reportLiftRodEndPressureSensorId(const std::string& id) {
        if (id != seal_.liftRodEndPressureSensorId) {
            seal_.liftRodEndPressureSensorId = id;
            seal_.setSaveNeeded();

            // Lift Cylinder Rod End Pressure Sensor - Electronic Control Module Serial Number
            breakSeal(769, 0xF811 /* Seal Identifier */, id);

            AIS_LOG_ALERT("LFT Seal Broken - Lift Rod End Pressure Sensor Id Changed (%s).", id.c_str());
        }
    }

    /*
     * Temperature Sensor
     */
    void reportHydraulicOilTemperatureSensorId(const std::string& id) {
        if (id != seal_.hydraulicOilTemperatureSensorId) {
            seal_.hydraulicOilTemperatureSensorId = id;
            seal_.setSaveNeeded();

            // Hydraulic Oil Temperature Sensor - Electronic Control Module Serial Number
            breakSeal(600, 0xF811 /* Seal Identifier */, id);

            AIS_LOG_ALERT("LFT Seal Broken - Hydraulic Oil Temperature Sensor Id Changed (%s).", id.c_str());
        }
    }

    /*
     * Temperature Sensor Enable Status
     */
    void reportHydraulicOilTempEnableStatus(bool enabled) {
        auto newValue = enabled ? LpsSaWeighAppSeal::Ternary_t::True : LpsSaWeighAppSeal::Ternary_t::False;
        auto oldValue = seal_.hydraulicOilTemperatureEnabled;

        if (newValue != oldValue) {
            seal_.hydraulicOilTemperatureEnabled = newValue;
            seal_.setSaveNeeded();

            if (LpsSaWeighAppSeal::Ternary_t::Unknown != oldValue) {

                // Payload Monitoring System - Payload Hydraulic Oil Temperature Compensation Enabled Status
                breakSeal(2183, 0xD11B9A, "", enabled ? 0x000C : 0x000D);

                AIS_LOG_ALERT("LFT Seal Broken - Hydraulic Oil Temperature Enable Status Changed (%d).", enabled);
            }
        }
    }

    /*
     * IMU Pitch and Roll Compensation Enable Status
     */
    void reportIMUCompensationEnableStatus(bool enabled) {
        auto newValue = enabled ? LpsSaWeighAppSeal::Ternary_t::True : LpsSaWeighAppSeal::Ternary_t::False;
        auto oldValue = seal_.imuCompensationEnabled;

        if (newValue != oldValue) {
            seal_.imuCompensationEnabled = newValue;
            seal_.setSaveNeeded();

            if (LpsSaWeighAppSeal::Ternary_t::Unknown != oldValue) {
                //  ---------                                         ---------------
                // | Payload | Monitoring System - Implement Linkage | Configuration |
                //  ---------                                         ---------------
                if (enabled) {
                    breakSeal(2183, 0xD10049, "PitchRollComp: Enabled");
                }
                else {
                    breakSeal(2183, 0xD10049, "PitchRollComp: Disabled");
                }

                AIS_LOG_ALERT("LFT Seal Broken - IMU Compensation Enable Status Changed (%d).", enabled);
            }
        }
    }

    /*
     * Inertial Measurement Unit (IMU)
     */
    void reportIMUSoftwarePartNumber(const std::string& pn) {
        if (pn != seal_.imuSoftwarePartNumber) {
            seal_.imuSoftwarePartNumber = pn;
            seal_.setSaveNeeded();

            // Inertial Measurement Sensor - Software Group Part Number
            breakSeal(1893, 0xF849, pn);

            AIS_LOG_ALERT("LFT Seal Broken - IMU Software Part Number Changed (%s).", pn.c_str());
        }
    }

    // Returns true if the value changes from a known value to the new value.
    bool reportIMUSerialNumber(const std::string& sn) {
        if (sn != seal_.imuSerialNumber) {
            bool wasEmpty = seal_.imuSerialNumber.empty();

            seal_.imuSerialNumber = sn;
            seal_.setSaveNeeded();

            // Inertial Measurement Sensor - Electronic Control Module Serial Number
            breakSeal(1893, 0xF811, sn);

            AIS_LOG_ALERT("LFT Seal Broken - IMU Serial Number Changed (%s).", sn.c_str());

            return wasEmpty ? false : true;
        }

        return false;
    }

    /*
     * Work Tool
     */
    void reportWorkToolId(const std::string& id) {
        if (id != seal_.workToolId) {
            seal_.workToolId = id;
            seal_.setSaveNeeded();

            // Payload Monitoring System - Tool Program Part Number (used as Work Tool Identifier)
            breakSeal(2183, 0xF878, id);

            AIS_LOG_ALERT("LFT Seal Broken - Work Tool Id Changed (%s).", id.c_str());
        }
    }

    /*
     * Calibration
     * - Status
     * - Counter
     */
    void reportLiftPositionSensorCalStatus(bool isCalibrated, uint_least32_t counter) {
        if ((isCalibrated != seal_.liftPositionSensorIsCalibrated) ||
                (counter != seal_.liftPositionSensorCalibrationCounter)) {
            seal_.liftPositionSensorIsCalibrated = isCalibrated;
            seal_.liftPositionSensorCalibrationCounter = counter;
            seal_.setSaveNeeded();

            if (isCalibrated) {
                // Lift Linkage Position Sensor - Last Successful Calibration Operating Hours
                breakSeal(350, 0xF856, "", 0x04D5 /* Calibrated */);

                AIS_LOG_ALERT("LFT Seal Broken - Lift Sensor Calibrated (%d).", counter);
            }
            else {
                // Lift Linkage Position Sensor - Out of Calibration
                breakSeal(350, 0xF856, "", 0x0346 /* Not Calibrated */);

                AIS_LOG_ALERT("LFT Seal Broken - Lift Sensor Not Calibrated.");
            }
        }
    }

    void reportTiltPositionSensorCalStatus(bool isCalibrated, uint_least32_t counter) {
        if ((isCalibrated != seal_.tiltPositionSensorIsCalibrated) ||
                (counter != seal_.tiltPositionSensorCalibrationCounter)) {
            seal_.tiltPositionSensorIsCalibrated = isCalibrated;
            seal_.tiltPositionSensorCalibrationCounter = counter;
            seal_.setSaveNeeded();

            if (isCalibrated) {
                // Tilt Linkage Position Sensor - Last Successful Calibration Operating Hours
                breakSeal(351, 0xF856, "", 0x04D5 /* Calibrated */);

                AIS_LOG_ALERT("LFT Seal Broken - Tilt Sensor Calibrated (%d).", counter);
            }
            else {
                // Tilt Linkage Position Sensor - Out of Calibration
                breakSeal(351, 0xF856, "", 0x0346 /* Not Calibrated */);

                AIS_LOG_ALERT("LFT Seal Broken - Tilt Sensor Not Calibrated.");
            }
        }
    }

    void reportPayloadCalStatus(bool isCalibrated, uint_least32_t counter) {
        if ((isCalibrated != seal_.payloadIsCalibrated) ||
                (counter != seal_.payloadCalibrationCounter)) {
            seal_.payloadIsCalibrated = isCalibrated;
            seal_.payloadCalibrationCounter = counter;
            seal_.setSaveNeeded();

            if (isCalibrated) {
                // Payload Monitoring System - Last Successful Calibration Operating Hours
                breakSeal(2183, 0xF856, "", 0x04D5 /* Calibrated */);

                AIS_LOG_ALERT("LFT Seal Broken - Payload Calibrated (%d).", counter);
            }
            else {
                // Payload Monitoring System - Out of Calibration
                breakSeal(2183, 0xF856, "", 0x0346 /* Not Calibrated */);

                AIS_LOG_ALERT("LFT Seal Broken - Payload Not Calibrated.");
            }
        }
    }

    void reportCalibrationWeight(float weight) {
        if (weight != seal_.fullCalibrationWeight) {
            seal_.fullCalibrationWeight = weight;
            seal_.setSaveNeeded();

            float interval = seal_.weightInterval;
            int decimalPrecision = 0;
            if (interval > 0) {
                // The number of decimal points required by the interval + 1
                decimalPrecision = std::ceil(1 - std::log10(interval));
                if (decimalPrecision < 0) {
                    decimalPrecision = 0;
                }
            }

            float weightConverted = lps_common_weight_from_tonnes(weight, seal_.weightUnits); // Convert to current units

            // Fixed precision
            std::stringstream ss;
            ss << std::fixed << std::setprecision(decimalPrecision) << weightConverted;

            // Payload Monitoring System - Payload Control System Calibration Weight
            breakSeal(2183, 0xD009A7, ss.str(), LpsCommonWeightUnits_Base_t(seal_.weightUnits));

            AIS_LOG_ALERT("LFT Seal Broken - Calibration Weight Changed (%f).", weight);
        }
    }

    /*
     * Span Adjust
     */
    void reportSpanAdjustFactor(float factor) {
        // seal breaking
        if (factor != seal_.spanAdjustFactor) {
            seal_.spanAdjustFactor = factor;
            seal_.setSaveNeeded();

            // Fixed precision
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << factor;

            // Payload Monitoring System -  Payload Correction Factor
            breakSeal(2183, 0xD023EA, ss.str());

            AIS_LOG_ALERT("LFT Seal Broken - Span Adjust Changed (%f).", factor);
        }
    }

    /*
     * Weigh Range Setting
     */
    void reportWeighRange(float weighRangeStart, float weighRangeSize) {
        // seal breaking
        if (weighRangeStart != seal_.weighRangeStart) {
            seal_.weighRangeStart = weighRangeStart;
            seal_.setSaveNeeded();

            // Fixed precision
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << weighRangeStart << "%";

            // Payload Monitoring System - Payload Weigh Activate Lift Position
            breakSeal(2183, 0xD00C9F, ss.str());

            AIS_LOG_ALERT("LFT Seal Broken - Weigh Range Start Changed (%f).", weighRangeStart);
        }

        if (weighRangeSize != seal_.weighRangeSize) {
            seal_.weighRangeSize = weighRangeSize;
            seal_.setSaveNeeded();

            // Fixed precision
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << weighRangeSize << "%";

            // Payload Monitoring System - Payload Weigh Range Configuration
            breakSeal(2183, 0xD0102D, ss.str());

            AIS_LOG_ALERT("LFT Seal Broken - Weight Range Size Changed (%f).", weighRangeSize);
        }
    }

    /*
     * Display Weight Settings
     */
    void reportWeightInterval(float interval, int decimalPrecision, const LpsCommonWeightUnits& uom) {
        // seal breaking
        if (interval != seal_.weightInterval) {
            seal_.weightInterval = interval;
            seal_.setSaveNeeded();

            // Payload Monitoring System - Display Weight Precision Configuration
            breakSeal(2183, 0xD11B8D, lps_common_weight_to_string(interval, decimalPrecision, interval), LpsCommonWeightUnits_Base_t(uom));

            AIS_LOG_ALERT("LFT Seal Broken - Display Weight Precision Configuration Changed (%f).", interval);
        }

        if (uom != seal_.weightUnits) {
            seal_.weightUnits = uom;
            seal_.setSaveNeeded();

            // Payload Monitoring System - Display Weight Units Configuration
            breakSeal(2183, 0xD10938, "", LpsCommonWeightUnits_Base_t(uom));

            AIS_LOG_ALERT("LFT Seal Broken - Display Weight Units Configuration Changed (%d).", uom);
        }
    }

    /*
     * Report Ticket Number Write
     *  The ticket number will naturally change with each generated ticket.  We need to capture
     *  writes to the ticket number but we don't need to detect every change to the ticket
     *  number or keep track of the value here.
     */
    void reportTicketNumberWrite(uint32_t ticketNumber) {
        // Payload Monitoring System - Onboard Payload Printer Truck Ticket Number
        breakSeal(2183, 0xD01D2C, std::to_string(ticketNumber));

        AIS_LOG_ALERT("LFT Seal Broken - Ticket Number Written (%u).", ticketNumber);
    }

    /*
     * Ticket Retention Period
     */
    void reportTicketRetentionPeriod(uint16_t retentionPeriod) {
        // seal breaking (but not when it is initially reported)
        if (retentionPeriod != 0) {
            if (0 == seal_.ticketRetentionPeriod) {
                // Just initialize to the provided value
                seal_.ticketRetentionPeriod = retentionPeriod;
                seal_.setSaveNeeded();
            }
            else if (retentionPeriod != seal_.ticketRetentionPeriod) {
                // It is changing from non-zero to non-zero
                seal_.ticketRetentionPeriod = retentionPeriod;
                seal_.setSaveNeeded();

                // Payload Monitoring System - Payload Memory Ticket Retention Period
                breakSeal(2183, 0xD03093, std::to_string(retentionPeriod), 0x0658 /* Days */);

                AIS_LOG_ALERT("LFT Seal Broken - Ticket Retention Period (%u).", retentionPeriod);
            }
        }
    }

    /*
     * Convert to JSON
     */
    void toJson(std::ostream& os, const std::string& fileNameStem) const {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

        writer.SetMaxDecimalPlaces(3);
        writer.StartObject();

        // File Name
        writer.Key("fileName");
        writer.String(fileNameStem);

        // Totals
        writer.Key("history");
        writer.StartArray();
        for (const auto& entry : boost::adaptors::reverse(changeLog_.entries())) {
            if (entry.time > std::chrono::system_clock::time_point::min()) {
                writer.StartObject();
                writer.Key("cid");
                writer.Uint(entry.componentId);
                writer.Key("pid");
                writer.Uint(entry.parameterId);
                writer.Key("localDateTime");
                writer.String(tes_common_ais::putLocalTime(entry.time, tes_common_ais::date_time_formats::ISO_DATE_TIME_LOCAL));
                if (!entry.value.empty()) {
                    writer.Key("value");
                    writer.String(entry.value);
                }
                if (IS_KNOWN2U(entry.stateId)) {
                    writer.Key("stateId");
                    writer.Uint(entry.stateId);
                }
                writer.EndObject();
            }
        }
        writer.EndArray();

        writer.EndObject();

        if (writer.IsComplete()) {
            os << sb.GetString();
        }
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
    boost::filesystem::path storageRoot_;
    LpsSaWeighAppSeal seal_;
    LpsSaWeighAppSealChangeLog changeLog_;

    const std::string sealFileName_ = "seal.bin";
    const std::string changeLogFileName_ = "change_log.bin";

    static bool endsWith(const std::string& str, const std::string& ending) {
        if (str.length() >= ending.length()) {
            if (0 == str.compare(str.length() - ending.length(), ending.length(), ending)) {
                return true;
            }
        }
        return false;
    }

};


#endif /* LPSSAWEIGHAPPSEAL_H_ */
