#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <deque>

// Rapid JSON
#include <ext/rapidjson/RapidJsonGuarded.h>

#include <ais/log/Logger.h>

#include <fileio/oflocker.hpp>
#include <fileio/sha1_fstream.hpp>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include <chrono/print.hpp>

#include "LpsSaWeighApp.h"

namespace fs = boost::filesystem;
namespace tes = tes_common_ais;

#define WEIGH_APP_LOG_ROOT (R"(/opt/appdata/CPM/debug)")

#define WEIGH_RANGE_WEIGH_LOG_PATH (R"(/opt/appdata/CPM/debug/wrw_log.csv)")
#define WEIGH_RANGE_WEIGH_LOG_MAX_SIZE (8400)
//#define WEIGH_RANGE_WEIGH_LOG_MAX_SIZE (300)

#define WEIGH_CAL_RESULTS_LOG_PATH (R"(/opt/appdata/CPM/debug/weigh_cal_results)")

#define WEIGH_PRODUCT_ID_LOG_PATH (R"(/opt/appdata/CPM/debug/product_id.txt)")

#define RECENT_WEIGH_RESULTS_MAX_ENTRIES (10)

struct WeighResultEntry {
    float weight; // tonnes, before zero and calibration adjust
    float weightStdErr; // tonnes, standard error
    float dispWeight; // tonnes, after zero and calibration adjust
    float liftCylVel; // mm/s, filtered and sampled at the end of the weigh range
    float hydOilTemp; // degrees C, sampled at the end of the weight range
    float pitch; // degrees, pitch from calibration pitch
    float roll; // degrees, roll from calibration roll
};

static std::deque<WeighResultEntry> recentWeighResults;

static void makeAppLogRoot()
{
    boost::system::error_code ec;

    // Make sure the directory exists
    fs::create_directories(WEIGH_APP_LOG_ROOT, ec);

    if (ec) {
        AIS_LOG_ERROR("Cannot create %s directory for logging.", WEIGH_APP_LOG_ROOT);
    }
}

/******************************************************************************
FUNCTION NAME:logWeighProductIdFile
DESCRIPTION: Create product_id.txt and store productId
PARAMETER DESCRIPTION: 
RETURN VALUE: void
*******************************************************************************/
void LpsSaWeighApp::logWeighProductIdFile(const std::string& newProductId) const
{
    makeAppLogRoot();

    /* Delete the same old filename and create a new one*/
    std::ofstream myFile(WEIGH_PRODUCT_ID_LOG_PATH, std::ios::out | std::ios::trunc);

    /* New product_id.txt is created. Fill it with the new ProductID */            
    if (myFile) {
        myFile << newProductId;
        myFile << std::endl;
        myFile.close();
    }
    else {
        AIS_LOG_INFO("Cannot create %s file for logging.", WEIGH_PRODUCT_ID_LOG_PATH);
    }
}

void LpsSaWeighApp::logWeighRangeWeighInit() const
{
    makeAppLogRoot();

    // Delete the file and get ready for a new one.
    std::ofstream ofs(WEIGH_RANGE_WEIGH_LOG_PATH, std::ios::out | std::ios::trunc);
    if (ofs) {
        ofs << "DateTime,Weight,WeightStdErr,Pitch,Roll,IMUAngle,IMUWtAdjust,IMUCompEnabled" << std::endl;
        ofs.close();
    }
    else {
        AIS_LOG_ERROR("Cannot create %s file for logging.", WEIGH_RANGE_WEIGH_LOG_PATH);
    }
}

void LpsSaWeighApp::logWeighRangeWeighUpdate() const
{
    WeighResultEntry entry;

    entry.weight = LpsWrk.WrwTbl.WkTbl.result.weightAve;
    entry.weightStdErr = LpsWrk.WrwTbl.WkTbl.result.weightStdErr;
    entry.dispWeight = LpsWrk.BestBktWt.Wt;
    entry.liftCylVel = LpsWrk.LiftPositionFilt.velocity;
    entry.hydOilTemp = weighUpdtTbl.HydOilTemp.Val;
    entry.pitch = LpsWrk.WrwTbl.WkTbl.result.pitchAngle;
    entry.roll = LpsWrk.WrwTbl.WkTbl.result.rollAngle;

    while (recentWeighResults.size() >= RECENT_WEIGH_RESULTS_MAX_ENTRIES) {
        recentWeighResults.pop_front();
    }
    recentWeighResults.push_back(entry);

    boost::system::error_code ec;

    boost::uintmax_t filesize = fs::file_size(WEIGH_RANGE_WEIGH_LOG_PATH, ec);
    if (ec || (filesize >= WEIGH_RANGE_WEIGH_LOG_MAX_SIZE)) {
        logWeighRangeWeighInit();
    }

    std::ofstream ofs(WEIGH_RANGE_WEIGH_LOG_PATH, std::ios::out | std::ios::app);
    if (ofs) {
        // Log the current time
        ofs << tes::putTime(std::chrono::system_clock::now(), tes::date_time_formats::ISO_DATE_TIME_LOCAL) << ",";

        { // Log the weigh range weight
            ofs << std::fixed << std::setprecision(3) << LpsWrk.WrwTbl.OpTbl.Wt << ",";
        }

        { // Log the standard error
            ofs << std::fixed << std::setprecision(4) << entry.weightStdErr << ",";
        }

        { // Log the chassis pitch
            ofs << std::fixed << std::setprecision(2) << entry.pitch << ",";
        }

        { // Log the chassis roll
            ofs << std::fixed << std::setprecision(2) << entry.roll << ",";
        }

        { // Log the chassis angle w.r.t. the calibration chassis angle.
            float value = LpsWrk.WrwTbl.WkTbl.result.imuAngle;
            ofs << std::fixed << std::setprecision(2) << value << ",";
        }

        { // Log the raw adjustment made by pitch and roll compensation
            ofs << std::fixed << std::setprecision(3) << LpsWrk.WrwTbl.WkTbl.result.weightAveRawIMUComp << ",";
        }

        { // Log the IMU compensation enable status
            ofs << bool(weighUpdtTbl.IMUCompEnabled) << std::endl;
        }

        ofs.close();
    }
}

bool LpsSaWeighApp::publishRecentWeighResults(const boost::filesystem::path& filePath) const
{
    bool success = false;

    { // Write to the file.
        tes_common_ais::OFlocker ofl(filePath);
        if (ofl) {
            std::ostream& os = ofl.ofstream();

            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

            writer.SetMaxDecimalPlaces(3);
            writer.StartObject();

            // File Name
            writer.Key("fileName");
            writer.String(filePath.stem().string());

            // Add measurements
            writer.Key("results");
            writer.StartArray();
            for (auto&& entry : recentWeighResults) {
                writer.StartObject();

                writer.Key("weight");
                writer.Double(entry.weight);

                writer.Key("weightStdErr");
                writer.Double(entry.weightStdErr);

                writer.Key("dispWeight");
                writer.Double(entry.dispWeight);

                writer.Key("liftCylVel");
                writer.Double(entry.liftCylVel);

                writer.Key("hydOilTemp");
                writer.Double(entry.hydOilTemp);

                writer.Key("pitch");
                writer.Double(entry.pitch);

                writer.Key("roll");
                writer.Double(entry.roll);

                writer.EndObject();
            }
            writer.EndArray();

            writer.EndObject();

            if (writer.IsComplete()) {
                os << sb.GetString();
            }

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

void LpsSaWeighApp::logWeighCalResults() const
{
    makeAppLogRoot();
    auto now = tes::putTime(std::chrono::system_clock::now(), tes::date_time_formats::YYYY_MM_DD_T_HH_MM_SS);

    std::string filename = std::string(WEIGH_CAL_RESULTS_LOG_PATH) + ".csv";
    std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::trunc);
    if (ofs) {
        // Log the calibration results.
        ofs << now << ",x,y";
        ofs << "\nProductId," << sealTracker_.getSeal().productId;
        ofs << "\nSoftwarePartNum," << sealTracker_.getSeal().softwarePartNumber;

        ofs << "\nCalStat," << payloadCalNvmTbl_.data.CalStatus;
        ofs << "\nCalVersion," << payloadCalNvmTbl_.data.CalVersion;
        ofs << "\nCalwt," << payloadCalNvmTbl_.data.CalWeight;

        ofs << "\nEmptyTemp," << payloadCalNvmTbl_.data.EmptyTemp;
        ofs << "\nEmptyBucketWeightEst," << payloadCalNvmTbl_.data.EmptyBucketWeightEst;
        ofs << "\nEmptyBktSlowRaiseSpd," << payloadCalNvmTbl_.data.EmptyBktSlowRaiseSpd;
        ofs << "\nEmptyBktFastRaiseSpd," << payloadCalNvmTbl_.data.EmptyBktFastRaiseSpd;
        ofs << "\nFastRaiseEmptyBktDeltaPres," << payloadCalNvmTbl_.data.FastRaiseEmptyBktDeltaPres;

        ofs << "\nSlowLowerEmptyBkt[0]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[0] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[0];
        ofs << "\nSlowLowerEmptyBkt[1]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[1] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[1];
        ofs << "\nSlowLowerEmptyBkt[2]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[2] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[2];
        ofs << "\nSlowLowerEmptyBkt[3]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[3] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[3];
        ofs << "\nSlowLowerEmptyBkt[4]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[4] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[4];
        ofs << "\nSlowLowerEmptyBkt[5]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[5] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[5];
        ofs << "\nSlowLowerEmptyBkt[6]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[6] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[6];
        ofs << "\nSlowLowerEmptyBkt[7]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[7] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[7];
        ofs << "\nSlowLowerEmptyBkt[8]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[8] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[8];
        ofs << "\nSlowLowerEmptyBkt[9]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[9] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[9];
        ofs << "\nSlowLowerEmptyBkt[10]," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftHt[10] << "," << payloadCalNvmTbl_.data.SlowLowerEmptyBktLiftPres[10];

        ofs << "\nSlowRaiseEmptyBkt[0]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[0] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[0];
        ofs << "\nSlowRaiseEmptyBkt[1]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[1] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[1];
        ofs << "\nSlowRaiseEmptyBkt[2]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[2] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[2];
        ofs << "\nSlowRaiseEmptyBkt[3]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[3] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[3];
        ofs << "\nSlowRaiseEmptyBkt[4]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[4] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[4];
        ofs << "\nSlowRaiseEmptyBkt[5]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[5] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[5];
        ofs << "\nSlowRaiseEmptyBkt[6]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[6] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[6];
        ofs << "\nSlowRaiseEmptyBkt[7]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[7] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[7];
        ofs << "\nSlowRaiseEmptyBkt[8]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[8] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[8];
        ofs << "\nSlowRaiseEmptyBkt[9]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[9] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[9];
        ofs << "\nSlowRaiseEmptyBkt[10]," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftHt[10] << "," << payloadCalNvmTbl_.data.SlowRaiseEmptyBktLiftPres[10];

        ofs << "\nFullTemp," << payloadCalNvmTbl_.data.FullTemp;
        ofs << "\nFullBktSlowRaiseSpd," << payloadCalNvmTbl_.data.FullBktSlowRaiseSpd;
        ofs << "\nFullBktFastRaiseSpd," << payloadCalNvmTbl_.data.FullBktFastRaiseSpd;
        ofs << "\nFastRaiseFullBktDeltaPres," << payloadCalNvmTbl_.data.FastRaiseFullBktDeltaPres;

        ofs << "\nSlowLowerFullBkt[0]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[0] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[0];
        ofs << "\nSlowLowerFullBkt[1]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[1] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[1];
        ofs << "\nSlowLowerFullBkt[2]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[2] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[2];
        ofs << "\nSlowLowerFullBkt[3]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[3] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[3];
        ofs << "\nSlowLowerFullBkt[4]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[4] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[4];
        ofs << "\nSlowLowerFullBkt[5]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[5] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[5];
        ofs << "\nSlowLowerFullBkt[6]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[6] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[6];
        ofs << "\nSlowLowerFullBkt[7]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[7] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[7];
        ofs << "\nSlowLowerFullBkt[8]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[8] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[8];
        ofs << "\nSlowLowerFullBkt[9]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[9] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[9];
        ofs << "\nSlowLowerFullBkt[10]," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftHt[10] << "," << payloadCalNvmTbl_.data.SlowLowerFullBktLiftPres[10];

        ofs << "\nSlowRaiseFullBkt[0]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[0] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[0];
        ofs << "\nSlowRaiseFullBkt[1]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[1] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[1];
        ofs << "\nSlowRaiseFullBkt[2]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[2] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[2];
        ofs << "\nSlowRaiseFullBkt[3]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[3] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[3];
        ofs << "\nSlowRaiseFullBkt[4]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[4] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[4];
        ofs << "\nSlowRaiseFullBkt[5]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[5] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[5];
        ofs << "\nSlowRaiseFullBkt[6]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[6] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[6];
        ofs << "\nSlowRaiseFullBkt[7]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[7] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[7];
        ofs << "\nSlowRaiseFullBkt[8]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[8] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[8];
        ofs << "\nSlowRaiseFullBkt[9]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[9] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[9];
        ofs << "\nSlowRaiseFullBkt[10]," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftHt[10] << "," << payloadCalNvmTbl_.data.SlowRaiseFullBktLiftPres[10];

        ofs << "\nSlowRaiseEmptyIMUAccel[0]," << payloadCalNvmTbl_.data.SlowRaiseEmptyIMUAccel[0];
        ofs << "\nSlowRaiseEmptyIMUAccel[1]," << payloadCalNvmTbl_.data.SlowRaiseEmptyIMUAccel[1];
        ofs << "\nSlowRaiseEmptyIMUAccel[2]," << payloadCalNvmTbl_.data.SlowRaiseEmptyIMUAccel[2];
        ofs << "\nSlowRaiseEmptyPitch," << payloadCalNvmTbl_.data.SlowRaiseEmptyPitch;
        ofs << "\nSlowRaiseEmptyRoll," << payloadCalNvmTbl_.data.SlowRaiseEmptyRoll;

        ofs << "\nSlowRaiseFullIMUAccel[0]," << payloadCalNvmTbl_.data.SlowRaiseFullIMUAccel[0];
        ofs << "\nSlowRaiseFullIMUAccel[1]," << payloadCalNvmTbl_.data.SlowRaiseFullIMUAccel[1];
        ofs << "\nSlowRaiseFullIMUAccel[2]," << payloadCalNvmTbl_.data.SlowRaiseFullIMUAccel[2];
        ofs << "\nSlowRaiseFullPitch," << payloadCalNvmTbl_.data.SlowRaiseFullPitch;
        ofs << "\nSlowRaiseFullRoll," << payloadCalNvmTbl_.data.SlowRaiseFullRoll;

        ofs << "\nAdvCalAdjust[0]," << payloadCalNvmTbl_.data.AdvCalAdjustX[0] << "," << payloadCalNvmTbl_.data.AdvCalAdjustY[0];
        ofs << "\nAdvCalAdjust[1]," << payloadCalNvmTbl_.data.AdvCalAdjustX[1] << "," << payloadCalNvmTbl_.data.AdvCalAdjustY[1];
        ofs << "\nAdvCalAdjust[2]," << payloadCalNvmTbl_.data.AdvCalAdjustX[2] << "," << payloadCalNvmTbl_.data.AdvCalAdjustY[2];
        ofs << "\nAdvCalAdjust[3]," << payloadCalNvmTbl_.data.AdvCalAdjustX[3] << "," << payloadCalNvmTbl_.data.AdvCalAdjustY[3];
        ofs << "\nAdvCalAdjust[4]," << payloadCalNvmTbl_.data.AdvCalAdjustX[4] << "," << payloadCalNvmTbl_.data.AdvCalAdjustY[4];

        ofs << "\nCalCounter," << payloadCalNvmTbl_.calCounter;

        ofs << std::endl;

        ofs.close();

        // create a date/time stamped file for system watch
        std::string filename_dest = std::string(WEIGH_CAL_RESULTS_LOG_PATH) + '_' + now + ".csv";
        boost::system::error_code ec;
        fs::copy(filename, filename_dest, ec); // copy file
    }

    // maintain a list of 10 newest time stamped files
    system("ls -t /opt/appdata/CPM/debug/weigh_cal_results_*.[cC][sS][vV]  | tail -n +11 | xargs rm -f");

}
