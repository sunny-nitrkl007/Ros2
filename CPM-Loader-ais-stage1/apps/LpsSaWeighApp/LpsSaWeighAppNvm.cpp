#include <cmath>

// Rapid JSON
#include <ext/rapidjson/RapidJsonGuarded.h>

#include "LpsSaWeighAppNvm.h"


void LpsSaTiltCalNvmTblStorage::toJson(std::ostream& os, const std::string& fileNameStem) const {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.SetMaxDecimalPlaces(3);
    writer.StartObject();

    // File Name
    writer.Key("fileName");
    writer.String(fileNameStem);

    // Calibration
    writer.Key("parameters");
    writer.StartObject();

    writer.Key("sensorType");
    writer.Uint(tilt_sensor_type);

    writer.Key("calStatus");
    writer.Uint(tilt_cal_stat);

    writer.Key("calCounter");
    writer.Uint(tilt_cal_counter);

    writer.Key("fullRackDutyCycle");
    writer.Double(tilt_full_rack_dc);

    writer.Key("fullDumpDutyCycle");
    writer.Double(tilt_full_dump_dc);

    writer.Key("fullRackStopAngle");
    writer.Double(tilt_full_rack_stop_angle);

    writer.Key("fullDumpStopAngle");
    writer.Double(tilt_full_dump_stop_angle);

    writer.EndObject();

    writer.EndObject();

    if (writer.IsComplete()) {
        os << sb.GetString();
    }
}

void LpsSaLiftCalNvmTblStorage::toJson(std::ostream& os, const std::string& fileNameStem) const {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.SetMaxDecimalPlaces(3);
    writer.StartObject();

    // File Name
    writer.Key("fileName");
    writer.String(fileNameStem);

    // Calibration
    writer.Key("parameters");
    writer.StartObject();

    writer.Key("calStatus");
    writer.Uint(lift_cal_stat);

    writer.Key("calCounter");
    writer.Uint(lift_cal_counter);

    writer.Key("fullRaiseDutyCycle");
    writer.Double(lift_full_raise_dc);

    writer.Key("fullLowerDutyCycle");
    writer.Double(lift_full_lower_dc);

    writer.EndObject();

    writer.EndObject();

    if (writer.IsComplete()) {
        os << sb.GetString();
    }
}

void LpsSaPayloadCalNvmTblStorage::toJson(std::ostream& os, const std::string& fileNameStem) const {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.SetMaxDecimalPlaces(3);
    writer.StartObject();

    // File Name
    writer.Key("fileName");
    writer.String(fileNameStem);

    // Calibration
    writer.Key("parameters");
    writer.StartObject();

    writer.Key("calVersion");
    writer.Uint(data.CalVersion);

    writer.Key("calStatus");
    writer.Uint(data.CalStatus);

    writer.Key("calCounter");
    writer.Uint(calCounter);

    writer.Key("legalForTradeInstalled");
    writer.tBool(legalForTradeInstalled);

    writer.Key("slowRaiseEmptyBktLiftHt");
    writer.StartArray();
    for (const auto& val : data.SlowRaiseEmptyBktLiftHt) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowRaiseEmptyBktLiftPres");
    writer.StartArray();
    for (const auto& val : data.SlowRaiseEmptyBktLiftPres) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowRaiseFullBktLiftHt");
    writer.StartArray();
    for (const auto& val : data.SlowRaiseFullBktLiftHt) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowRaiseFullBktLiftPres");
    writer.StartArray();
    for (const auto& val : data.SlowRaiseFullBktLiftPres) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowLowerEmptyBktLiftHt");
    writer.StartArray();
    for (const auto& val : data.SlowLowerEmptyBktLiftHt) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowLowerEmptyBktLiftPres");
    writer.StartArray();
    for (const auto& val : data.SlowLowerEmptyBktLiftPres) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowLowerFullBktLiftHt");
    writer.StartArray();
    for (const auto& val : data.SlowLowerFullBktLiftHt) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowLowerFullBktLiftPres");
    writer.StartArray();
    for (const auto& val : data.SlowLowerFullBktLiftPres) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("fastRaiseEmptyBktDeltaPres");
    writer.Double(data.FastRaiseEmptyBktDeltaPres);

    writer.Key("fastRaiseFullBktDeltaPres");
    writer.Double(data.FastRaiseFullBktDeltaPres);

    writer.Key("emptyBktSlowRaiseSpd");
    writer.Double(data.EmptyBktSlowRaiseSpd);

    writer.Key("emptyBktFastRaiseSpd");
    writer.Double(data.EmptyBktFastRaiseSpd);

    writer.Key("fullBktSlowRaiseSpd");
    writer.Double(data.FullBktSlowRaiseSpd);

    writer.Key("fullBktFastRaiseSpd");
    writer.Double(data.FullBktFastRaiseSpd);

    writer.Key("calWeight");
    writer.Double(data.CalWeight);

    writer.Key("shmEmptyMain");
    writer.Uint(data.ShmEmptyMain);

    writer.Key("shmFullMain");
    writer.Uint(data.ShmFullMain);

    writer.Key("emptyTemp");
    writer.Double(data.EmptyTemp);

    writer.Key("fullTemp");
    writer.Double(data.FullTemp);

    writer.Key("emptyBucketWeightEst");
    writer.Double(data.EmptyBucketWeightEst);

    writer.Key("calAdjust");
    writer.Double(data.CalAdjust);

    writer.Key("zeroWeight");
    writer.Double(data.ZeroWeight);

    writer.Key("slowRaiseEmptyIMUAccel");
    writer.StartArray();
    for (const auto& val : data.SlowRaiseEmptyIMUAccel) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowRaiseFullIMUAccel");
    writer.StartArray();
    for (const auto& val : data.SlowRaiseFullIMUAccel) {
        writer.Double(val);
    }
    writer.EndArray();

    writer.Key("slowRaiseEmptyPitch");
    writer.Double(data.SlowRaiseEmptyPitch);

    writer.Key("slowRaiseEmptyRoll");
    writer.Double(data.SlowRaiseEmptyRoll);

    writer.Key("slowRaiseFullPitch");
    writer.Double(data.SlowRaiseFullPitch);

    writer.Key("slowRaiseFullRoll");
    writer.Double(data.SlowRaiseFullRoll);

    writer.Key("advCalAdjust");
    writer.StartArray();
    for (auto ii = 0; ii < LPS_CAL_ADV_CAL_ADJUST_NUM_POINTS; ++ii) {
        const auto& x = data.AdvCalAdjustX[ii];
        const auto& y = data.AdvCalAdjustY[ii];
        if (std::isfinite(x) && std::isfinite(y)) {
            writer.StartArray();
            writer.Double(x);
            writer.Double(y);
            writer.EndArray();
        }
    }
    writer.EndArray();

    writer.EndObject();

    writer.EndObject();

    if (writer.IsComplete()) {
        os << sb.GetString();
    }
}

