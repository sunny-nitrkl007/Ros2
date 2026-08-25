/*
 * LpsSaWeighAppTestFixture.h
 *
 *  Created on: Dec 2, 2024
 *      Author: pf
 */

#ifndef LPSSAWEIGHAPPTESTFIXTURE_H_
#define LPSSAWEIGHAPPTESTFIXTURE_H_

#include <string>
#include <chrono>
#include <fstream>
#include <cmath>

#include <boost/filesystem.hpp>

#include <interfaces/LpsSaWeighTxChannel/LpsSaWeighTxChannel.h>

#include <imu/imu.hpp>

class LpsSaWeighAppTestFixture {
public:
    using TestStatus_t = LpsSaWeighTxChannel::TestStatus_t;

    LpsSaWeighAppTestFixture() :
        testStatus_(TestStatus_t::INACTIVE),
        active_(false),
        failed_(false),
        recording_(false),
        fs_(),
        startTime_(),
        duration_(),
        thisStep_(),
        nextStep_(),
        recordStep_() {}

    bool recordTestPlan(const boost::filesystem::path& filePath, const std::chrono::seconds& duration);

    bool runTestPlan(const boost::filesystem::path& filePath);

    void setupTestStep();

    TestStatus_t getTestStatus() const { return testStatus_; }
    bool isActive() const { return active_; }
    bool isFailed() const { return failed_; }
    bool isRecording() const { return recording_; }

    bool tpLiftHeadEndPressure(float& liftHeadEndPressure);
    bool tpLiftRodEndPressure(float& liftRodEndPressure);
    bool tpLiftPositionSensorDutyCycle(float& liftPositionSensorDutyCycle);
    bool tpTiltPositionSensorDutyCycle(float& tiltPositionSensorDutyCycle);
    bool tpHydaulicOilTemperature(float& hydaulicOilTemperature);
    bool tpIMUAcceleration(tes_common_ais::imu::IMU::Vector& imuLinAcc);
    bool tpIMUAngularVelocity(tes_common_ais::imu::IMU::Vector& imuAngVel);

    bool updateTestStep(float weight);

protected:
    void parseHeader(const std::string& line);
    void parseNextStep(const std::string& line);
    void splitLine(const std::string& line, std::vector<std::string>& fields);

private:
    TestStatus_t testStatus_;
    bool active_;
    bool failed_;
    bool recording_;
    std::fstream fs_;
    std::chrono::steady_clock::time_point startTime_;
    std::chrono::seconds duration_;

    struct Step {
        Step() :
            startTime(),
            liftHeadEndPressure(NAN),
            liftRodEndPressure(NAN),
            liftPositionSensorDutyCycle(NAN),
            tiltPositionSensorDutyCycle(NAN),
            hydaulicOilTemperature(NAN),
            expectedWeight(NAN),
            imuLinAcc(NAN, NAN, NAN),
            imuAngVel(NAN, NAN, NAN) {}

        std::chrono::steady_clock::time_point startTime;
        float liftHeadEndPressure;
        float liftRodEndPressure;
        float liftPositionSensorDutyCycle;
        float tiltPositionSensorDutyCycle;
        float hydaulicOilTemperature;
        float expectedWeight;
        tes_common_ais::imu::IMU::Vector imuLinAcc;
        tes_common_ais::imu::IMU::Vector imuAngVel;
    } thisStep_, nextStep_, recordStep_;
};


#endif /* LPSSAWEIGHAPPTESTFIXTURE_H_ */
