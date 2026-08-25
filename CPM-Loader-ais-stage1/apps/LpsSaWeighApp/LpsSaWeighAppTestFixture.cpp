/*
 * LpsSaWeighAppTestFixture.cpp
 *
 *  Created on: Dec 2, 2024
 *      Author: pf
 */

#include <cstdint>
#include <array>
#include <vector>
#include <iostream>
#include <iomanip>

#include <ais/log/Logger.h>

#include <fileio/directory.hpp>

#include "LpsSaWeighAppTestFixture.h"

enum COLUMN_IDX {
    TIME = 0,
    LIFT_HE_PRESS,
    LIFT_RE_PRESS,
    LIFT_POS_DC,
    TILT_POS_DC,
    HYD_OIL_TEMP,
    LIN_ACC_X,
    LIN_ACC_Y,
    LIN_ACC_Z,
    ANG_VEL_X,
    ANG_VEL_Y,
    ANG_VEL_Z,
    WEIGHT,
    MAX
};

const std::string COLUMN_NAMES[] = {
    "Time",
    "LiftHePress",
    "LiftRePress",
    "LiftPosDC",
    "TiltPosDC",
    "HydOilTemp",
    "LinAccX",
    "LinAccY",
    "LinAccZ",
    "AngVelX",
    "AngVelY",
    "AngVelZ",
    "Weight"
};
constexpr auto COLUMN_NAMES_SIZE = sizeof(COLUMN_NAMES) / sizeof(COLUMN_NAMES[0]);
static_assert(COLUMN_NAMES_SIZE == COLUMN_IDX::MAX, "Number of columns in COLUMN_NAMES mismatch.");

int COLUMN_IDX_MAP[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};
constexpr auto COLUMN_IDX_MAP_SIZE = sizeof(COLUMN_IDX_MAP) / sizeof(COLUMN_IDX_MAP[0]);
static_assert(COLUMN_IDX_MAP_SIZE == COLUMN_IDX::MAX, "Number of columns in COLUMN_IDX_MAP mismatch.");


bool LpsSaWeighAppTestFixture::recordTestPlan(const boost::filesystem::path& filePath, const std::chrono::seconds& duration) {
    // Close existing test plan
    testStatus_ = TestStatus_t::INACTIVE;
    active_ = false;
    failed_ = false;
    recording_ = false;
    startTime_ = std::chrono::steady_clock::now();
    duration_ = duration;
    if (fs_) {
        fs_.close();
    }

    // Initialize steps
    thisStep_ = Step();
    nextStep_ = Step();
    recordStep_ = Step();

    try {
        //boost::system::error_code ec;
        boost::filesystem::remove(filePath);
        tes_common_ais::directory::create(filePath.parent_path());
        fs_.open(filePath.string(), std::ios_base::out | std::ios_base::trunc);
        if (fs_) {
            fs_ << COLUMN_NAMES[0];
            for (int ii = 1; ii < COLUMN_IDX::MAX; ++ii) {
                fs_ << "," << COLUMN_NAMES[ii];
                COLUMN_IDX_MAP[ii] = ii;
            }
            fs_ << "\n";

            AIS_LOG_FATAL("Recording started...");
            testStatus_ = TestStatus_t::RECORDING;
            recording_ = true;
        }
        else {
            AIS_LOG_FATAL("Could not open %s", filePath.string().c_str());
        }
    }
    catch (const boost::filesystem::filesystem_error& e) {
        AIS_LOG_ERROR(e.what());
    }

    if (!recording_) {
        testStatus_ = TestStatus_t::FAILED;
        failed_ = true;
    }

    return recording_;
}

bool LpsSaWeighAppTestFixture::runTestPlan(const boost::filesystem::path& filePath) {
    // Close existing test plan
    testStatus_ = TestStatus_t::INACTIVE;
    active_ = false;
    failed_ = false;
    recording_ = false;
    startTime_ = std::chrono::steady_clock::now();
    duration_ = std::chrono::seconds(0);
    if (fs_) {
        fs_.close();
    }

    // Initialize steps
    thisStep_ = Step();
    nextStep_ = Step();
    recordStep_ = Step();

    try {
        tes_common_ais::directory::create(filePath.parent_path());
        if (boost::filesystem::exists(filePath)) {
            fs_.open(filePath.string(), std::ios_base::in);
            if (fs_) {
                // Read the first line.  Going to just assume column order for now.
                std::string line;
                if (std::getline(fs_, line)) {
                    // If this file is a Windows file, it could end in a \r carriage return
                    // this isn't stripped off at the moment due to us just matching
                    // the beginning of each string.
                    //if (*line.rbegin() == '\r') { line.pop_back(); }

                    AIS_LOG_FATAL("Test Plan Found: %s", line.c_str());

                    // Parse the columns.
                    parseHeader(line);

                    if (std::getline(fs_, line)) {
                        parseNextStep(line);
                        testStatus_ = TestStatus_t::TESTING;
                        active_ = true;
                    }
                }

                if (!active_) {
                    fs_.close();
                }
            }
        }
    }
    catch (const boost::filesystem::filesystem_error& e) {
        AIS_LOG_ERROR(e.what());
    }

    if (!active_) {
        testStatus_ = TestStatus_t::FAILED;
        failed_ = true;
    }

    return active_;
}

void LpsSaWeighAppTestFixture::setupTestStep() {
    if (active_) {
        // Test is active
        if (fs_) {
            const auto now = std::chrono::steady_clock::now();
            if (now >= nextStep_.startTime) {
                // Advance to next step
                thisStep_ = nextStep_;
                std::string line;
                if (std::getline(fs_, line)) {
                    parseNextStep(line);
                }
                else {
                    fs_.close();
                }
            }
        }
        else {
            if (failed_) {
                testStatus_ = TestStatus_t::FAILED;
            }
            else {
                testStatus_ = TestStatus_t::INACTIVE;
            }
            active_ = false;
        }
    }

    if (recording_) {
        // Recording is active, set up this steps time.
        recordStep_ = Step();
        recordStep_.startTime = std::chrono::steady_clock::now();
    }
}

bool LpsSaWeighAppTestFixture::updateTestStep(float weight) {
    bool pass = true;

    if (active_) {
        if (std::isfinite(thisStep_.expectedWeight)) {
            if (std::fabs(weight - thisStep_.expectedWeight) > 0.01) {
                AIS_LOG_FATAL("Weight of %f doesn't match expected weight of %f.", weight, thisStep_.expectedWeight);
                thisStep_.expectedWeight = NAN; // Don't warn again!
                pass = false;
            }
        }

        if (!pass) {
            failed_ = true;
        }
    }

    if (recording_) {
        if (!fs_) {
            testStatus_ = TestStatus_t::INACTIVE;
            recording_ = false;
        }
        else if (recordStep_.startTime >= startTime_) {
            auto duration = recordStep_.startTime - startTime_;
            if (duration > duration_) {
                fs_.close();
                testStatus_ = TestStatus_t::INACTIVE;
                recording_ = false;
            }
            else {
                fs_ << std::fixed;

                { // Time since test start
                    float time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.f;
                    fs_ << std::setprecision(3) << time;
                }

                fs_ << "," << std::setprecision(0);

                if (std::isfinite(recordStep_.liftHeadEndPressure)) {
                    fs_ << recordStep_.liftHeadEndPressure;
                }

                fs_ << ",";

                if (std::isfinite(recordStep_.liftRodEndPressure)) {
                    fs_ << recordStep_.liftRodEndPressure;
                }

                fs_ << "," << std::setprecision(3);

                if (std::isfinite(recordStep_.liftPositionSensorDutyCycle)) {
                    fs_ << recordStep_.liftPositionSensorDutyCycle;
                }

                fs_ << ",";

                if (std::isfinite(recordStep_.tiltPositionSensorDutyCycle)) {
                    fs_ << recordStep_.tiltPositionSensorDutyCycle;
                }

                fs_ << "," << std::setprecision(1);

                if (std::isfinite(recordStep_.hydaulicOilTemperature)) {
                    fs_ << recordStep_.hydaulicOilTemperature;
                }

                fs_ << "," << std::setprecision(3);

                if (std::isfinite(recordStep_.imuLinAcc.x())) {
                    fs_ << recordStep_.imuLinAcc.x();
                }

                fs_ << ",";

                if (std::isfinite(recordStep_.imuLinAcc.y())) {
                    fs_ << recordStep_.imuLinAcc.y();
                }

                fs_ << ",";

                if (std::isfinite(recordStep_.imuLinAcc.z())) {
                    fs_ << recordStep_.imuLinAcc.z();
                }

                fs_ << ",";

                if (std::isfinite(recordStep_.imuAngVel.x())) {
                    fs_ << recordStep_.imuAngVel.x();
                }

                fs_ << ",";

                if (std::isfinite(recordStep_.imuAngVel.y())) {
                    fs_ << recordStep_.imuAngVel.y();
                }

                fs_ << ",";

                if (std::isfinite(recordStep_.imuAngVel.z())) {
                    fs_ << recordStep_.imuAngVel.z();
                }

                fs_ << ",";

                fs_ << weight;

                fs_ << "\n";
            }
        }
    }

    return pass;
}

bool LpsSaWeighAppTestFixture::tpLiftHeadEndPressure(float& liftHeadEndPressure) {
    bool ovr = false;

    if (active_) {
        if (std::isfinite(thisStep_.liftHeadEndPressure)) {
            liftHeadEndPressure = thisStep_.liftHeadEndPressure;
            ovr = true;
        }
    }

    if (recording_) {
        recordStep_.liftHeadEndPressure = liftHeadEndPressure;
    }

    return ovr;
}

bool LpsSaWeighAppTestFixture::tpLiftRodEndPressure(float& liftRodEndPressure) {
    bool ovr = false;

    if (active_) {
        if (std::isfinite(thisStep_.liftRodEndPressure)) {
            liftRodEndPressure = thisStep_.liftRodEndPressure;
            ovr = true;
        }
    }

    if (recording_) {
        recordStep_.liftRodEndPressure = liftRodEndPressure;
    }

    return ovr;
}

bool LpsSaWeighAppTestFixture::tpLiftPositionSensorDutyCycle(float& liftPositionSensorDutyCycle) {
    bool ovr = false;

    if (active_) {
        if (std::isfinite(thisStep_.liftPositionSensorDutyCycle)) {
            liftPositionSensorDutyCycle = thisStep_.liftPositionSensorDutyCycle;
            ovr = true;
        }
    }

    if (recording_) {
        recordStep_.liftPositionSensorDutyCycle = liftPositionSensorDutyCycle;
    }

    return ovr;
}

bool LpsSaWeighAppTestFixture::tpTiltPositionSensorDutyCycle(float& tiltPositionSensorDutyCycle) {
    bool ovr = false;

    if (active_) {
        if (std::isfinite(thisStep_.tiltPositionSensorDutyCycle)) {
            tiltPositionSensorDutyCycle = thisStep_.tiltPositionSensorDutyCycle;
            ovr = true;
        }
    }

    if (recording_) {
        recordStep_.tiltPositionSensorDutyCycle = tiltPositionSensorDutyCycle;
    }

    return ovr;
}

bool LpsSaWeighAppTestFixture::tpHydaulicOilTemperature(float& hydaulicOilTemperature) {
    bool ovr = false;

    if (active_) {
        if (std::isfinite(thisStep_.hydaulicOilTemperature)) {
            hydaulicOilTemperature = thisStep_.hydaulicOilTemperature;
            ovr = true;
        }
    }

    if (recording_) {
        recordStep_.hydaulicOilTemperature = hydaulicOilTemperature;
    }

    return ovr;
}

bool LpsSaWeighAppTestFixture::tpIMUAcceleration(tes_common_ais::imu::IMU::Vector& imuLinAcc) {
    bool ovr = false;

    if (active_) {
        if (std::isfinite(thisStep_.imuLinAcc.x())) {
            // Assuming all are provided if one is.
            imuLinAcc.x() = thisStep_.imuLinAcc.x();
            imuLinAcc.y() = thisStep_.imuLinAcc.y();
            imuLinAcc.z() = thisStep_.imuLinAcc.z();
            ovr = true;
        }
    }

    if (recording_) {
        recordStep_.imuLinAcc.x() = imuLinAcc.x();
        recordStep_.imuLinAcc.y() = imuLinAcc.y();
        recordStep_.imuLinAcc.z() = imuLinAcc.z();
    }

    return ovr;
}

bool LpsSaWeighAppTestFixture::tpIMUAngularVelocity(tes_common_ais::imu::IMU::Vector& imuAngVel) {
    bool ovr = false;

    if (active_) {
        if (std::isfinite(thisStep_.imuAngVel.x())) {
            // Assuming all are provided if one is.
            imuAngVel.x() = thisStep_.imuAngVel.x();
            imuAngVel.y() = thisStep_.imuAngVel.y();
            imuAngVel.z() = thisStep_.imuAngVel.z();
            ovr = true;
        }
    }

    if (recording_) {
        recordStep_.imuAngVel.x() = imuAngVel.x();
        recordStep_.imuAngVel.y() = imuAngVel.y();
        recordStep_.imuAngVel.z() = imuAngVel.z();
    }

    return ovr;
}

void LpsSaWeighAppTestFixture::splitLine(const std::string& line, std::vector<std::string>& fields) {
    fields.clear();

    // Split the line into the fields
    std::size_t startPos = 0;
    std::size_t endPos = line.find(",");
    while (endPos != std::string::npos) {
        fields.push_back(line.substr(startPos, endPos-startPos));

        /*
         *  If endPos is the last position of the string, startPos
         *  will be beyond the end of the string.  In this case,
         *  "find" will return "npos" and we will safely break
         *  out of the loop.
         */
        startPos = endPos + 1;
        endPos = line.find(",", startPos);
    }

    if (!line.empty()) {
        /*
         * startPos will be one position beyond the previous comma.
         *
         * - If there weren't any commas, startPos is 0
         *
         * - If the string ended in a comma, startPos equals string length,
         *   in which case the last string is an empty string.
         *
         * - If the string has commas, but doens't end in one,
         *   startPos is at the first character of the last field.
         *
         * In any case, startPos will always be <= string length, so
         * line.substr should not fail.
         */
        fields.push_back(line.substr(startPos, line.size()-startPos));
    }
}

void LpsSaWeighAppTestFixture::parseHeader(const std::string& line) {
    std::vector<std::string> fields;

    splitLine(line, fields);

    int numFields = fields.size();

    AIS_LOG_FATAL("Number of header fields: %d", numFields);

    int fieldIdx = 0;
    for (int colIdx = 0; colIdx < COLUMN_IDX::MAX; ++colIdx) {
        const std::string& colName = COLUMN_NAMES[colIdx];

        // If the files field name starts with the column name we are looking for, then it's a match
        if ((fieldIdx < numFields) && (0 == fields[fieldIdx].compare(0, colName.size(), colName))) {
            COLUMN_IDX_MAP[colIdx] = fieldIdx;
            ++fieldIdx;
            AIS_LOG_FATAL("%s column found.", colName.c_str());
        }
        else {
            COLUMN_IDX_MAP[colIdx] = -1;
            AIS_LOG_FATAL("%s column not found.", colName.c_str());
        }
    }
}

void LpsSaWeighAppTestFixture::parseNextStep(const std::string& line) {
    std::vector<std::string> fields;

    splitLine(line, fields);

    int numFields = fields.size();

    // Fill in new info
    try {
        // Time
        int fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::TIME];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            // Start immediately on the next update.
            nextStep_.startTime = std::chrono::steady_clock::time_point::min();
        }
        else {
            nextStep_.startTime = startTime_ + std::chrono::milliseconds(
                    std::chrono::milliseconds::rep(std::stof(fields[fieldIdx])*1000.f));
        }

        // Lift Head End Pressure
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::LIFT_HE_PRESS];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.liftHeadEndPressure = thisStep_.liftHeadEndPressure;
        }
        else {
            nextStep_.liftHeadEndPressure = std::stof(fields[fieldIdx]);
        }

        // Lift Rod End Pressure
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::LIFT_RE_PRESS];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.liftRodEndPressure = thisStep_.liftRodEndPressure;
        }
        else {
            nextStep_.liftRodEndPressure = std::stof(fields[fieldIdx]);
        }

        // Lift Position Sensor Duty Cycle
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::LIFT_POS_DC];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.liftPositionSensorDutyCycle = thisStep_.liftPositionSensorDutyCycle;
        }
        else {
            nextStep_.liftPositionSensorDutyCycle = std::stof(fields[fieldIdx]);
        }

        // Tilt Position Sensor Duty Cycle
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::TILT_POS_DC];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.tiltPositionSensorDutyCycle = thisStep_.tiltPositionSensorDutyCycle;
        }
        else {
            nextStep_.tiltPositionSensorDutyCycle = std::stof(fields[fieldIdx]);
        }

        // Hydraulic Oil Temperature
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::HYD_OIL_TEMP];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.hydaulicOilTemperature = thisStep_.hydaulicOilTemperature;
        }
        else {
            nextStep_.hydaulicOilTemperature = std::stof(fields[fieldIdx]);
        }

        // IMU Linear Acceleration X
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::LIN_ACC_X];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.imuLinAcc.x() = thisStep_.imuLinAcc.x();
        }
        else {
            nextStep_.imuLinAcc.x() = std::stof(fields[fieldIdx]);
        }

        // IMU Linear Acceleration Y
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::LIN_ACC_Y];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.imuLinAcc.y() = thisStep_.imuLinAcc.y();
        }
        else {
            nextStep_.imuLinAcc.y() = std::stof(fields[fieldIdx]);
        }

        // IMU Linear Acceleration Z
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::LIN_ACC_Z];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.imuLinAcc.z() = thisStep_.imuLinAcc.z();
        }
        else {
            nextStep_.imuLinAcc.z() = std::stof(fields[fieldIdx]);
        }

        // IMU Angular Velocity X
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::ANG_VEL_X];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.imuAngVel.x() = thisStep_.imuAngVel.x();
        }
        else {
            nextStep_.imuAngVel.x() = std::stof(fields[fieldIdx]);
        }

        // IMU Angular Velocity Y
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::ANG_VEL_Y];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.imuAngVel.y() = thisStep_.imuAngVel.y();
        }
        else {
            nextStep_.imuAngVel.y() = std::stof(fields[fieldIdx]);
        }

        // IMU Angular Velocity Z
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::ANG_VEL_Z];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.imuAngVel.z() = thisStep_.imuAngVel.z();
        }
        else {
            nextStep_.imuAngVel.z() = std::stof(fields[fieldIdx]);
        }

        // Expected Weight
        fieldIdx = COLUMN_IDX_MAP[COLUMN_IDX::WEIGHT];
        if ((fieldIdx >= numFields) || (fieldIdx < 0) || (fields[fieldIdx].empty())) {
            nextStep_.expectedWeight = NAN;
        }
        else {
            nextStep_.expectedWeight = std::stof(fields[fieldIdx]);
        }
    }
    catch (const std::invalid_argument& e) {
        AIS_LOG_FATAL(e.what());
    }
}



