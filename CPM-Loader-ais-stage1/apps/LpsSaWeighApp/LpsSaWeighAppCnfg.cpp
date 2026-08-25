#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>

// Rapid JSON
#include <ext/rapidjson/RapidJsonGuarded.h>

#include <ais/log/Logger.h>

#include <fileio/oflocker.hpp>
#include <fileio/sha1_fstream.hpp>

#include "LpsSaWeighAppCnfg.h"

/*
 * Load configuration from storage
 */
bool LpsSaWeighAppCnfg::load() {
    bool success = false;
    tes_common_ais::sha1_ifstream ifs{ true }; // two_copy
    ifs.open(filePath_, std::ios_base::in | std::ios_base::binary);
    if (ifs) {
        // Read the file in
        try {
            boost::archive::binary_iarchive ia(ifs);
            ia >> *this;
            saveNeeded_ = false;
            AIS_LOG_INFO("Loaded weigh app configuration from storage.");
            success = true;
        }
        catch (const boost::archive::archive_exception& e) {
            AIS_LOG_ERROR("Could not deserialize weigh app configuration from storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (const std::exception& e) {
            AIS_LOG_ERROR("Could not deserialize weigh app configuration from storage.");
            AIS_LOG_ERROR(e.what());
        }
        catch (...) {
            AIS_LOG_ERROR("Could not deserialize weigh app configuration from storage, unexpected error.");
        }

        ifs.close();

        if (ifs.is_corrupt()) {
            if (ifs.fix_it()) {
                AIS_LOG_WARN("Weigh app configuration file was corrupt... fixed it.");
            }
            else {
                AIS_LOG_ERROR("Weigh app configuration file was corrupt... could not fix it.");
            }
        }
    }
    else {
        AIS_LOG_ERROR("Weigh app configuration file could not be opened from storage.");
    }

    return success;
}

/*
 * Reset configuration in storage
 */
bool LpsSaWeighAppCnfg::reset() {
    bool success = false;

    try {
        success = tes_common_ais::sha1_fstream::remove_files(filePath_);
    }
    catch (const boost::archive::archive_exception& e) {
        AIS_LOG_ERROR("Could not delete from storage.");
        AIS_LOG_ERROR(e.what());
    }

    return success;
}

/*
 * Save configuration to storage
 */
bool LpsSaWeighAppCnfg::save(bool force) const {
    bool success = false;

    if (force || saveNeeded_) {
        tes_common_ais::sha1_ofstream ofs{ true }; // two_copy
        ofs.open(filePath_, std::ios_base::out | std::ios_base::binary);
        if (ofs) {
            // Read the file in
            try {
                boost::archive::binary_oarchive oa(ofs);
                oa << *this;
                saveNeeded_ = false;
                AIS_LOG_INFO("Saved weigh app configuration to storage.");
                success = true;
            }
            catch (const boost::archive::archive_exception& e) {
                AIS_LOG_ERROR("Could not serialize weigh app configuration to storage.");
                AIS_LOG_ERROR(e.what());
            }

            ofs.close();
        }
        else {
            AIS_LOG_ERROR("Weigh app configuration could not be opened from storage.");
        }
    }
    else {
        success = true;
    }

    return success;
}

/*
 * Convert to JSON
 */
void LpsSaWeighAppCnfg::toJson(std::ostream& os, const std::string& fileNameStem) const {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.SetMaxDecimalPlaces(3);
    writer.StartObject();

    // File Name
    writer.Key("fileName");
    writer.String(fileNameStem);

    // Configuration
    writer.Key("parameters");
    writer.StartObject();

    writer.Key("hydOilTempEnabled");
    writer.tBool(hydOilTempEnabled);

    writer.Key("imuCompEnabled");
    writer.tBool(imuCompEnabled);

    writer.Key("loadCheckValveInstalled");
    writer.tBool(loadCheckValveInstalled);

    writer.Key("audibleWeightEnabled");
    writer.tBool(audibleWeightEnabled);

    writer.Key("overloadWarningEnabled");
    writer.tBool(overloadWarningEnabled);

    writer.Key("bucketPayloadTargetWeight");
    writer.Double(bucketPayloadTargetWeight);

    writer.Key("lastSuggestedCalWeight");
    writer.Double(lastSuggestedCalWeight);

    writer.Key("weighRangeStart");
    writer.Double(weighRangeStart);

    writer.Key("weighRangeSize");
    writer.Double(weighRangeSize);

    writer.Key("reweighMaxPitch");
    writer.Double(reweighMaxPitch);

    writer.Key("reweighMinPitch");
    writer.Double(reweighMinPitch);

    writer.Key("reweighMaxAbsRoll");
    writer.Double(reweighMaxAbsRoll);

    writer.Key("reweighMinLiftCylVel");
    writer.Double(reweighMinLiftCylVel);

    // We don't need these included here, they were already handled in PID data.
//    writer.Key("tipoffPitchCalOffset");
//    writer.Double(tipoffPitchCalOffset);
//
//    writer.Key("toaAnchoredZeroOffset");
//    writer.Double(toaAnchoredZeroOffset);
//
//    writer.Key("toaAnchoredFactor");
//    writer.Double(toaAnchoredFactor);

    writer.EndObject();

    writer.EndObject();

    if (writer.IsComplete()) {
        os << sb.GetString();
    }
}

bool LpsSaWeighAppCnfg::publish(const boost::filesystem::path& filePath) const {
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

