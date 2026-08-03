/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaWeighApp.cpp
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <mutex>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstdio>
#include <iostream>

#include <hal_boot_proto.h>
#include <ecminfolib_public.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <hub/filesystem/extensions/file.h>
#include <fileio/directory.hpp>
#include <print.hpp>

#ifndef  _LPS_SA_WEIGHAPP_H_
#include "LpsSaWeighApp.h"
#include <ksw.h>
#endif

#ifndef __LPS_CAL_PRIVATE_H__
#include <LpsCalPrivate.h>
#endif

#include "src_nvm/app_nvm_file_access.h"

/*******************************************************************************/
/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
#define CONFIG_FILENAME_BIN (R"(WeighAppCnfg.bin)")
#define LIFT_CAL_FILENAME_BIN (R"(WeighAppLiftCalTbl.bin)")
#define TILT_CAL_FILENAME_BIN (R"(WeighAppTiltCalTbl.bin)")
#define PAYLOAD_CAL_FILENAME_BIN (R"(WeighAppPayloadCalTbl.bin)")

#define DEFAULT_STORAGE_ROOT (R"(/tmp/LpsSaWeighApp/storage)")
#define DEFAULT_TEMP_ROOT (R"(/tmp/LpsSaWeighApp/temp)")

//temporary until rpa.h is updated
#define APP_READY_CODE_PAYLOAL_LEGAL_FOR_TRADE_IS_SEALED 22

/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/
bool CalNVMReinitFlag = false;

using namespace task;

// This is the one and only one instance of this task.
LpsSaWeighApp thisTask("LpsSaWeighApp");

static float extractValFromString(const std::string& str);


/******************************************************************************
FUNCTION NAME:getTaskImplementation
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
AbstractTaskCore* task::getTaskImplementation(void)
{
    return &thisTask;
}

/******************************************************************************
FUNCTION NAME:LpsSaWeighApp
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
LpsSaWeighApp::LpsSaWeighApp( const std::string& taskName ):
    Task( taskName ),
    WeighPidTbl(),
    linkage_table_cnfg(),
    machineProperties(),
    CalibTblRuby(),
    storePressCount(0),
    sumOfAdjustedTruckWts(0.f),
    sumOfZeroedTruckWts(0.f),
    updatedSimpleCalFactor(0.f),
    demo_mode(false),
    machineMSN(),
    weighUpdtTbl(),
    DebugLpsSaXCPChannels(),
    LpsSaInitTbl(),
    LiftCylVelLpFilt(),
    TiltCylVelLpFilt(),
    LiftAngVelLpFilt(),
    TiltLinkageSensorDcLpFilt(),
    LiftLinkageSensorDcLpFilt(),
    request_(),
    LpsSaWeighInfoTbl(),
    inPwm_(),
    WeighRangeConfig(),
    cnfg_(),
    tiltCalNvmTbl_(),
    liftCalNvmTbl_(),
    payloadCalNvmTbl_(),
    chassisImu_(),
    sealTracker_(),
    testFixture_(),
    storageRoot_(DEFAULT_STORAGE_ROOT),
    tempRoot_(DEFAULT_TEMP_ROOT),
    serviceHourMeter_(0),
    tzInfo_{0, -1},
    demoInputs_(),
    LpsSaWeighScsReqstIn(nullptr),
    LpsSaWeighScsRespOut(nullptr),
    LpsSaWeighScsTxOut(nullptr),
    LpsSaJobMgrScsReqstOut(nullptr),
    ReadyToFlashStatusOutput(nullptr),
    PwmIn(nullptr),
    LpsSaJobMgrScsTxIn(nullptr),
    DemoAppTxIn(nullptr),
    AisJhm2TxInputScs(nullptr),
    AutonomyConditionDiagnosticsTxInputChannel(nullptr),
    MachineIn(nullptr),
    LpsSaWeighScsInitDebugOut(nullptr),
    LpsSaWeighScsDebugOut(nullptr),
    LpsCalCmdScsReqstIn(nullptr),
    LpsCalCmdScsRespOut(nullptr),
    LpsNvmDumpChanOut(nullptr),
    LpsNvmOnTheFlyDumpChanOut(nullptr),
    DataLinkDataInput_(nullptr),
    PartNumbersInput_(nullptr),
    SystemHardwareHealthInput_(nullptr),
    SystemHardwareHealthRequestOutput_(nullptr),
    displayStateInput_(nullptr),
    printerCnfgInput_(nullptr),
    shmClockInput_(nullptr),
    rosNode_(nullptr),
    executor_(),
    LinkageCalInProgress(false),
    audibleTriggered_(LpsSaWeighTxChannel::TONE_NONE),
    audibleTriggerTimepoint_(),
    flashUpdateTimepoint_(),
    prevWeighRangeIndicator_(LPS_IN_WEIGH_RANGE_NOT_WEIGHING),
    calLibMtx_(),
    transmitPeriodTime_(0.1f),
    transmitPeriodCount_(0)

{
    OelBootupFlag = FALSE;
}

/******************************************************************************
FUNCTION NAME:~LpsSaWeighApp
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
LpsSaWeighApp::~LpsSaWeighApp( )
{
}

/******************************************************************************
FUNCTION NAME:Initialize
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
bool LpsSaWeighApp::initialize( )
{
    if (ADVANCED == getApplicationVariant()) {
        AIS_LOG_INFO("LpsSaWeighApp::initialize for the ADVANCED application variant");
    }
    else if (STANDALONE == getApplicationVariant()) {
        AIS_LOG_INFO("LpsSaWeighApp::initialize for the STANDALONE application variant");
    }
    else {
        AIS_LOG_ERROR("LpsSaWeighApp::initialize, invalid application variant");
        return false;
    }

    { /* Weighing App execution rate in Hz */
        getTaskConfig().get("cycleRate_hz", LpsSaWeighInfoTbl.CycleRate_hz);
        AIS_LOG_INFO("cycleRate_hz %f", LpsSaWeighInfoTbl.CycleRate_hz);
    }

    /*
     * Initializing default configuration parameters
     *  This must be done before NVM is initialized because it
     *  configures the default NVM tables.
     */
    if (!LpsSaInitWeighTbl()) {
        return false;
    }

    /* Starting up OEL layer */
    commInitialize();

    /* Reading parameters from CSNS NVM */
    NvmInitialize();

    { // Set up storage root
        ConfigSection& configs = getTaskConfig();

        std::string configStr;

        if (configs.get("tempRoot", configStr)) {
            tempRoot_ = configStr;
        }
        else {
            tempRoot_ = DEFAULT_TEMP_ROOT;
        }

        tes_common_ais::directory::create(tempRoot_);

        if (configs.get("storageRoot", configStr)) {
            storageRoot_ = configStr;
        }
        else {
            storageRoot_ = DEFAULT_STORAGE_ROOT;
        }

        if (!tes_common_ais::directory::create(storageRoot_)) {
            // If we can't create the storage root, we can't run.
            return false;
        }

        cnfg_.setFilePath(storageRoot_ / CONFIG_FILENAME_BIN);
        liftCalNvmTbl_.setFilePath(storageRoot_, LIFT_CAL_FILENAME_BIN);
        tiltCalNvmTbl_.setFilePath(storageRoot_, TILT_CAL_FILENAME_BIN);
        payloadCalNvmTbl_.setFilePath(storageRoot_, PAYLOAD_CAL_FILENAME_BIN);

        // Get App Tx Period in Seconds
        {
            if (!configs.get("CPMExecTxPeriod", transmitPeriodTime_)) {
                transmitPeriodTime_ = 0.1; // 100 ms
                AIS_LOG_ERROR("Could not read 'CPMExecTxPeriod'");
            }
        }
    }

    // Initialize the hydOilTempEnabled default status
    cnfg_.hydOilTempEnabled = machineProperties.hydOilTempEnabledDefault;

    // Initialize the imuCompEnabled default status
    cnfg_.imuCompEnabled = false;

    // Initialize the loadCheckValveInstalled default status
    cnfg_.loadCheckValveInstalled = machineProperties.loadCheckValveInstalledDefault;

    // Initialize the default bucket payload target weight
    cnfg_.bucketPayloadTargetWeight = machineProperties.bucketPayloadTargetWeightDefault;

    // Initialize the default weigh range
    cnfg_.weighRangeStart = WeighRangeConfig.defaultStartOfWeighRange;
    cnfg_.weighRangeSize = (WeighRangeConfig.defaultEndOfWeighRange - WeighRangeConfig.defaultStartOfWeighRange);

    { // Load config from storage
        LpsSaWeighAppCnfg defaultCnfg(cnfg_);
        if (!cnfg_.load()) {
            // It was not successfully loaded, use default
            cnfg_ = defaultCnfg;
        }

        // Load old Machine Config
        if (NvmWeighingAppMachSpecificCfgFileExistsFlag == TRUE) {
            cnfg_.overloadWarningEnabled = (LpsSaWeighNvmMachSpecificCfg._PloadOvrloadWarnEn == 0x000C);
            cnfg_.bucketPayloadTargetWeight = LpsSaWeighNvmMachSpecificCfg._LoaderBktPloadTrgtWt;
            cnfg_.lastSuggestedCalWeight = LpsSaWeighNvmMachSpecificCfg._LastPloadWt;
            cnfg_.weighRangeStart = LpsSaWeighNvmMachSpecificCfg._WeighRangeBottom;
            cnfg_.weighRangeSize = LpsSaWeighNvmMachSpecificCfg._WeighRangeSize;
            cnfg_.setSaveNeeded();

            // Remove old file
            if (app_nvm_file_remove(LPS_WEIGH_MACH_SPECIFIC_CFG_NVID) == TRUE) {
                AIS_LOG_INFO("Successfully deleted old machine config NVM file");
            }
            else {
                AIS_LOG_WARN("Failed to delete old machine config NVM file");
            }
        }
    }

    { // Load lift sensor calibration from storage
        LpsSaLiftCalNvmTbl defaultCal(liftCalNvmTbl_);
        if (!liftCalNvmTbl_.load()) {
            liftCalNvmTbl_ = defaultCal;
        }

        // Convert old Lift Cal NVM file to new format and save
        if (NvmWeighingAppLiftCalFileExistsFlag == TRUE) {
            liftCalNvmTbl_.convertFromCsnsFormat(*const_cast<LpsSaLiftCalNvmTbl_t*>(&LpsSaWeighLiftNvmCal));
            liftCalNvmTbl_.setSaveNeeded();

            // Remove old file
            if (app_nvm_file_remove(LPS_WEIGH_NVM_LIFT_CAL_NVID) == TRUE) {
                AIS_LOG_INFO("Successfully deleted old Lift cal NVM file");
            }
            else {
                AIS_LOG_WARN("Failed to delete old Lift cal NVM file");
            }
        }
    }

    { // Load tilt sensor calibration from storage
        LpsSaTiltCalNvmTbl defaultCal(tiltCalNvmTbl_);
        if (!tiltCalNvmTbl_.load()) {
            tiltCalNvmTbl_ = defaultCal;
        }

        // Convert old Tilt Cal NVM file to new format and save
        if (NvmWeighingAppTiltCalFileExistsFlag == TRUE) {
            tiltCalNvmTbl_.convertFromCsnsFormat(*const_cast<LpsSaTiltCalNvmTbl_t*>(&LpsSaWeighTiltNvmCal), linkage_table_cnfg.tiltSensorType);
            tiltCalNvmTbl_.setSaveNeeded();

            // Remove old file
            if (app_nvm_file_remove(LPS_WEIGH_NVM_TILT_CAL_NVID) == TRUE) {
                AIS_LOG_INFO("Successfully deleted old Tilt cal NVM file");
            }
            else {
                AIS_LOG_WARN("Failed to delete old Tilt cal NVM file");
            }
        }

        if (tiltCalNvmTbl_.tilt_sensor_type != linkage_table_cnfg.tiltSensorType) {
            AIS_LOG_NOTICE("tiltSensorType in NVM %u does not match configured value %s, resetting", tiltCalNvmTbl_.tilt_sensor_type, linkage_table_cnfg.tiltSensorType);
            tiltCalNvmTbl_.tilt_cal_stat = 0;
        }
    }

    { // Load calibration from storage
        LpsSaPayloadCalNvmTbl defaultCal(payloadCalNvmTbl_);
        if (!payloadCalNvmTbl_.load()) {
            // It was not successfully loaded, use default
            payloadCalNvmTbl_ = defaultCal;
        }

        // Load old payload calibration
        if (NvmWeighingAppCalFileExistsFlag == TRUE) {
            if (payloadCalNvmTbl_.convertFromCsnsFormat(*const_cast<LpsSaCalNvmTbl_t*>(&LpsSaWeighNvmCal))) {
                payloadCalNvmTbl_.setSaveNeeded();
            }

            // Remove old file
            if (app_nvm_file_remove(LPS_WEIGH_NVM_CAL_NVID) == TRUE) {
                AIS_LOG_INFO("Successfully deleted old payload cal NVM file");
            }
            else {
                AIS_LOG_WARN("Failed to delete old payload cal NVM file");
            }
        }

        // Recover the calibration weight from the previous-previous location
        if (0 != (payloadCalNvmTbl_.data.CalStatus & CAL_BKT_WT_MASK)) {
            // Calibration weight is known, is it valid?
            if (payloadCalNvmTbl_.data.CalWeight <= 0.f) {
                // Calibration weight is invalid, recover from old location.
                if ((TRUE == NvmWeighingAppMachSpecificCfgFileExistsFlag) &&
                        (LpsSaWeighNvmMachSpecificCfg._CalWt > 0.f)) {
                    payloadCalNvmTbl_.data.CalWeight = LpsSaWeighNvmMachSpecificCfg._CalWt;
                    payloadCalNvmTbl_.setSaveNeeded();
                }
            }
        }
    }

    /*
     * If the hydraulic oil temperature sensor is not optional and it was
     * disabled, then we need to enable it and invalidate the calibration.
     */
    if ((!machineProperties.hydOilTempOptional) && (!cnfg_.hydOilTempEnabled)) {
        // Hydraulic oil temp is no longer disabled, current calibration is invalid.
        setPayloadNotCalibrated();

        cnfg_.hydOilTempEnabled = true;
        cnfg_.setSaveNeeded();
    }

    /*
     * If IMU compensation is not supported, then we can't have it.
     */
    // For this software, until IMU compensation is validated, we cannot allow IMU compensation to be enabled.
    //if ((!machineProperties.imuCompSupported) && (cnfg_.imuCompEnabled)) {
    if (cnfg_.imuCompEnabled) {
        cnfg_.imuCompEnabled = false;
        cnfg_.setSaveNeeded();
    }

    // Initialize the simple calibration adjustment factor variables
    storePressCount = 0;
    sumOfAdjustedTruckWts = 0.f;
    sumOfZeroedTruckWts = 0.f;
    updatedSimpleCalFactor = payloadCalNvmTbl_.data.CalAdjust; // From NVM

    /*
     * Initialize the Chassis IMU
     */
    if (!chassisImu_.init(getTaskParser())) {
        AIS_LOG_ERROR("Failed to init chassis imu.");
        return false;
    }

    /* Initializing dependent libraries */
    LpsSaInitErrorType_t initRet = LpsSaInit();
    if (LPS_SA_INIT_SUCCESS != initRet) {
        AIS_LOG_ERROR("LpsSaInit failed error code = %d", initRet);
        return false;
    }

    // Initialize cal
    if (!CalLibInit()) {
        AIS_LOG_ERROR("CalLibInit failed");
        return false;
    }

    { // Initialize the Legal For Trade Seal Tracker
        sealTracker_.initialize(storageRoot_);

        sealTracker_.reportMachineConfigurationCode(machineMSN);

        { // Report ECM serial number
            hal_ecm_serialno_t serialNum = hal_boot_get_ecm_serialno();
            std::string serialNumStr = std::string(reinterpret_cast<const char*>(serialNum.string), serialNum.length);
            sealTracker_.reportEcmSerialNumber(serialNumStr);
        }

        sealTracker_.reportLiftPositionSensorCalStatus(GetLiftCylCalStatus(), liftCalNvmTbl_.lift_cal_counter);
        sealTracker_.reportTiltPositionSensorCalStatus(GetTiltCylCalStatus(), tiltCalNvmTbl_.tilt_cal_counter);
        sealTracker_.reportPayloadCalStatus(GetPayloadCalStatus(), payloadCalNvmTbl_.calCounter);
        sealTracker_.reportCalibrationWeight(payloadCalNvmTbl_.data.CalWeight);
        sealTracker_.reportSpanAdjustFactor(payloadCalNvmTbl_.data.CalAdjust);
        sealTracker_.reportWeighRange(cnfg_.weighRangeStart, cnfg_.weighRangeSize);
        sealTracker_.reportHydraulicOilTempEnableStatus(cnfg_.hydOilTempEnabled);
        sealTracker_.reportIMUCompensationEnableStatus(cnfg_.imuCompEnabled);
    }

    { // Read the last service tool change level
        /*
         * Read XML file @ /opt/rpa-space/ipflash/FlashTargetInformation.xml
         *
         * --------------------------Records of interest------------------------
         *  <LastServiceToolToFlash>ETN41450</LastServiceToolToFlash>
         *  <ECMPartNumber>4946473-01</ECMPartNumber>
         * ---------------------------------------------------------------------
         */
        constexpr const char* filePath = "/opt/rpa-space/ipflash/FlashTargetInformation.xml";

        // Define input file
        std::ifstream myFile(filePath);

        if (myFile) {
            std::string lastServiceToolSerialNumber;

            try {
                // Create empty property tree object
                boost::property_tree::ptree tree;

                // Parse the XML into the property tree
                boost::property_tree::read_xml(myFile, tree);

                // Get last service tool serial number
                lastServiceToolSerialNumber = tree.get<std::string>("FlashTargetInformation.FlashTargetInformationMessage.LastServiceToolToFlash");
            }
            catch (const boost::property_tree::xml_parser_error& e) {
                AIS_LOG_ERROR("Unable to parse the xml file.");
            }
            catch (const boost::property_tree::ptree_error& e) {
                AIS_LOG_ERROR("Unable to get <LastServiceToolToFlash> from xml file.");
            }

            // If found Last Service Tool SN and Software PN, update the Flash Info log file
            if (!lastServiceToolSerialNumber.empty()) {
                sealTracker_.reportLastServiceToolSerialNumber(lastServiceToolSerialNumber);
            }
        }
        else {
            AIS_LOG_ERROR("Could not open XML file %s", filePath);
        }
    }

    // We just initialized, we don't need to re-initialize.
    CalNVMReinitFlag = false;

    if (machineMSN == "CAT99999") {
        demo_mode = true;
    }

    /* Initialize PWM input class */
    inPwm_.initializeRead();

    /*
     * Initialising SCS Channels
     */

    /* ROS2/DDS wrapper construction replaces InterfaceDb::bind()/
       SCSOutData::initPublishInterface(). One shared node for the whole
       app; every wrapper below just creates its own publisher/subscription
       on it. Topic names are the original SCS channel name in
       snake_case, minus the redundant Input/Output suffix -- and, for
       the channels shared directly with JobMgr (LpsSaJobMgrReqstChannel,
       LpsSaJobMgrTxChannel, AisJhm2TxChannel, AutonomyConditionDiagnostics
       TxChannel, ShmClockInput), the exact same topic name JobMgr itself
       uses, since these are the same live DDS topic on both sides. */
    rosNode_ = std::make_shared<rclcpp::Node>("weigh_app_node");
    executor_.add_node(rosNode_);

    SystemHardwareHealthRequestOutput_ = new ros_wrapper::RosOutputInterface<weigh_app_interfaces::msg::SystemHardwareHealthRequest>(rosNode_, "system_hardware_health_request");
    if (!SystemHardwareHealthRequestOutput_)
    {
        return false;
    }

    displayStateInput_ = new ros_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaUIDisplayStateInterface>(rosNode_, "display_state");
    if (!displayStateInput_) {
        AIS_LOG_ERROR("No DisplayStateInput input channel defined.");
        return false;
    }

    printerCnfgInput_ = new ros_wrapper::RosInputInterface<weigh_app_interfaces::msg::LpsSaTotalsPrinterCnfgInterfaceStorage>(rosNode_, "printer_cnfg");
    if (!printerCnfgInput_) {
        AIS_LOG_ERROR("No PrinterCnfgInput input channel defined.");
        return false;
    }

    SystemHardwareHealthInput_ = new ros_wrapper::RosInputInterface<weigh_app_interfaces::msg::SystemHardwareHealthStorage>(rosNode_, "system_hardware_health");
    if (!SystemHardwareHealthInput_) {
        AIS_LOG_ERROR("No SystemHardwareHealthInput channel defined.");
        return false;
    }

    PartNumbersInput_ = new ros_wrapper::RosInputInterface<weigh_app_interfaces::msg::PartNumbers>(rosNode_, "part_numbers");
    if (!PartNumbersInput_) {
        AIS_LOG_ERROR("No Part Numbers input channel defined.");
        return false;
    }

    DataLinkDataInput_ = new ros_wrapper::RosInputInterface<weigh_app_interfaces::msg::DataLinkData>(rosNode_, "weigh_app_data_link_data");
    if (!DataLinkDataInput_) {
        AIS_LOG_ERROR("DataLinkDataInput interface not defined");
        return false;
    }

    LpsSaJobMgrScsReqstOut = new ros_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel>(rosNode_, "lps_sa_job_mgr_reqst_channel");
    if (!LpsSaJobMgrScsReqstOut) {
        AIS_LOG_ERROR("LpsSaJobMgrReqstChannelOutput interface not defined");
        return false;
    }

    ReadyToFlashStatusOutput = new ros_wrapper::RosOutputInterface<weigh_app_interfaces::msg::ReadyToFlashStatus>(rosNode_, "ready_to_flash_status");
    if (!ReadyToFlashStatusOutput)
    {
        return false;
    }

    AutonomyConditionDiagnosticsTxInputChannel = new ros_wrapper::RosInputInterface<cpm_common_interfaces::msg::AutonomyConditionDiagnosticsTxChannel>(rosNode_, "autonomy_condition_diagnostics_tx_channel");
    if (!AutonomyConditionDiagnosticsTxInputChannel) {
        AIS_LOG_ERROR("AutonomyConditionDiagnosticsTxChannelInput interface not defined");
        return false;
    }

    LpsSaWeighScsTxOut = new ros_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>(rosNode_, "lps_sa_weigh_tx_channel");
    if (!LpsSaWeighScsTxOut) {
         AIS_LOG_ERROR("Interface LpsSaWeighTxChannelOutput not configured.");
         return false;
    }

    LpsSaWeighScsRespOut = new ros_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>(rosNode_, "lps_sa_weigh_resp_channel");
    if (!LpsSaWeighScsRespOut) {
        AIS_LOG_ERROR("Interface LpsSaWeighRespChannelOutput not configured.");
        return false;
    }

    LpsSaWeighScsInitDebugOut = new ros_wrapper::RosOutputInterface<weigh_app_interfaces::msg::LpsSaWeighInitDebugChannel>(rosNode_, "lps_sa_weigh_init_debug_channel");
    if (!LpsSaWeighScsInitDebugOut) {
        AIS_LOG_ERROR("Interface LpsSaWeighInitDebugChannelOutput not configured.");
        return false;
    }
    else {
        LpsSaWeighScsInitDebugTx();
    }

    LpsSaWeighScsDebugOut = new ros_wrapper::RosOutputInterface<weigh_app_interfaces::msg::LpsSaWeighDebugChannel>(rosNode_, "lps_sa_weigh_debug_channel");
    if (!LpsSaWeighScsDebugOut) {
        AIS_LOG_ERROR("Interface LpsSaWeighDebugChannelOutput not configured.");
        return false;
    }

    LpsSaWeighScsReqstIn = new ros_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>(rosNode_, "lps_sa_weigh_reqst_channel");
    if (!LpsSaWeighScsReqstIn) {
        //LpsSaWeighScsReqstIn->addNewDataSlot(boost::bind(&LpsSaWeighApp::LpsSaWeighReqstRead, this ) );
        AIS_LOG_ERROR("Interface LpsSaWeighReqstChannelInput not configured.");
        return false;
    }

    PwmIn = new ros_wrapper::RosInputInterface<weigh_app_interfaces::msg::PwmInputChannels>(rosNode_, "pwm_input_channels");
    if (!PwmIn) {
         AIS_LOG_ERROR("Interface PwmInputChannelsInput not configured.");
         return false;
    }

    MachineIn = new ros_wrapper::RosInputInterface<weigh_app_interfaces::msg::Machine>(rosNode_, "machine");
    if (!MachineIn) {
         AIS_LOG_ERROR("Interface MachineInput not configured.");
         return false;
    }

    LpsSaJobMgrScsTxIn = new ros_wrapper::RosInputInterface<job_mgr_interfaces::msg::LpsSaJobMgrTxChannel>(rosNode_, "lps_sa_job_mgr_tx_channel");
    if (!LpsSaJobMgrScsTxIn) {
         AIS_LOG_ERROR("Interface LpsSaJobMgrTxChannelInput not configured.");
         return false;
    }

    DemoAppTxIn = new ros_wrapper::RosInputInterface<weigh_app_interfaces::msg::DemoAppTxChannel>(rosNode_, "demo_app_tx_channel");
    if (!DemoAppTxIn) {
        AIS_LOG_ERROR("DemoAppTxChannelInput interface not configured");
        return false;
    }

    AisJhm2TxInputScs = new ros_wrapper::RosInputInterface<cpm_common_interfaces::msg::AisJhm2TxChannel>(rosNode_, "ais_jhm2_tx_channel");
    if (!AisJhm2TxInputScs) {
        AIS_LOG_ERROR("Interface AisJhm2TxChannelInput not configured.");
        return false;
    }

    LpsCalCmdScsReqstIn = new ros_wrapper::RosInputInterface<weigh_app_interfaces::msg::CalMgrCmdReqst>(rosNode_, "cal_mgr_cmd_reqst");
    if (!LpsCalCmdScsReqstIn) {
        AIS_LOG_ERROR("Interface CalMgrCmdReqstInput not configured.");
        return false;
    }
    else {
        LpsCalCmdScsReqstIn->addNewDataSlot(boost::bind(&LpsSaWeighApp::LpsSaWeighCalReqstCallback, this));
    }

    LpsCalCmdScsRespOut = new ros_wrapper::RosOutputInterface<weigh_app_interfaces::msg::CalMgrCmdResp>(rosNode_, "cal_mgr_cmd_resp");
    if (!LpsCalCmdScsRespOut) {
        AIS_LOG_ERROR("Interface CalMgrCmdRespOutput not configured.");
        return false;
    }

    shmClockInput_ = new ros_wrapper::RosInputInterface<cpm_common_interfaces::msg::ShmClockInput>(rosNode_, "shm_clock");
    if (!shmClockInput_) {
        AIS_LOG_ERROR("No ShmClockInput input channel defined.");
        return false;
    }

    LpsNvmDumpChanOut = new ros_wrapper::RosOutputInterface<weigh_app_interfaces::msg::LpsSaNvmCalDataChannel>(rosNode_, "lps_sa_nvm_cal_data_channel");
    if (!LpsNvmDumpChanOut) {
        AIS_LOG_ERROR("Interface LpsSaNvmCalDataChannelOutput not configured.");
    }

    LpsNvmOnTheFlyDumpChanOut = new ros_wrapper::RosOutputInterface<weigh_app_interfaces::msg::LpsSaNvmCalOnTheFlyDataChannel>(rosNode_, "lps_sa_nvm_cal_on_the_fly_data_channel");
    if (!LpsNvmOnTheFlyDumpChanOut) {
        AIS_LOG_ERROR("Interface LpsSaNvmCalOnTheFlyDataChannelOutput not configured.");
    }

    /* Initialize time information. */
    serviceHourMeter_ = 0;
    tzInfo_.offset = 0;
    tzInfo_.index = -1;

    /* Create appdata/CPM/debug/product_id.txt with the current product id */
    logWeighProductIdFile(sealTracker_.getSeal().productId);

    return true;
}

/******************************************************************************
FUNCTION NAME:executive
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/

bool LpsSaWeighApp::executive( )
{
    executor_.spin_some();

    static bool cal_data_published = false;
    if  (!cal_data_published) {
        /* Publish the key-on calibration data from NVM through SCS one-time for XCP app.
         * We do this in the executive to guarantee that XCP app is already running */
        PublishCalFromNvmPayload();

        cal_data_published = true;
    }

    testFixture_.setupTestStep();

    LpsSaWeighingScsRx();

    /* Did we receive new ProductID? 
        If we did, we need to create new 
          appdata/CPM/debug/product_id.txt file */
    if (nullptr != PartNumbersInput_) {
        weigh_app_interfaces::msg::PartNumbers partNumbers;

        while (PartNumbersInput_->get(partNumbers)) {
            /* We receive new ProductID */
            if (partNumbers.product_id_num_set) {
                std::string productId = partNumbers.product_id_num;
                if (sealTracker_.reportProductId(productId)) {
                    // If it changed, write the product id log file
                    logWeighProductIdFile(productId);
                }
            }

            if (partNumbers.sw_group_part_num_set) {
                std::string softwarePartNum = partNumbers.sw_group_part_num;
                sealTracker_.reportSoftwarePartNumber(softwarePartNum);
            }

            if (partNumbers.equipment_id_set) {
                std::string equipmentId = partNumbers.equipment_id;
                sealTracker_.reportEquipmentId(equipmentId);
            }
        }
    }

    /*
     * Update the chassis imu
     */
    chassisImu_.update(testFixture_);

    LpsSaUpdt();

    if(transmitScheduler()) {

        LpsSaWeighingScsTx();

        /* XCP data */
        LpsSaWeighScsInitDebugTx();
        LpsSaWeighScsDebugTx();
    }

    // Save config if needed
    cnfg_.save();

    // Save Calibration Tables NVM if needed
    liftCalNvmTbl_.save();
    tiltCalNvmTbl_.save();
    payloadCalNvmTbl_.save();

    // Update the seal tracker
    sealTracker_.update();

    // flash enabler update
    flashEnablerUpdate();

    // Check it against the currently running test
    testFixture_.updateTestStep(LpsSaWeighInfoTbl.BestBktWtInTonnes);

    return true;
}

void LpsSaWeighApp::flashEnablerUpdate() {

    weigh_app_interfaces::msg::ReadyToFlashStatus m_readyToFlashStatus;
    m_readyToFlashStatus.ready_code = weigh_app_interfaces::msg::ReadyToFlashStatus::APP_READY_CODE_OK_TO_FLASH;
    m_readyToFlashStatus.host_name = getHostName();
    m_readyToFlashStatus.task_name = getTaskName();

    // if lft is installed and sealed and flash is disabled, then disable flash
    if (payloadCalNvmTbl_.legalForTradeInstalled && sealTracker_.isSealed() &&
            !cnfg_.flashEnabled) {
        m_readyToFlashStatus.ready_code = weigh_app_interfaces::msg::ReadyToFlashStatus::APP_READY_CODE_PAYLOAL_LEGAL_FOR_TRADE_IS_SEALED;
    }
    else {
        // allow flash
    }

    // publish
    ReadyToFlashStatusOutput->publish(m_readyToFlashStatus);
}

bool LpsSaWeighApp::setHydOilTempEnableStatus(bool enabled) {
    if (machineProperties.hydOilTempOptional) {
        if (cnfg_.hydOilTempEnabled != enabled) {
            // A change is requested.
            cnfg_.hydOilTempEnabled = enabled;
            cnfg_.setSaveNeeded();

            // report it to seal tracker
            sealTracker_.reportHydraulicOilTempEnableStatus(enabled);

            /*
             * The calibration was performed under different conditions
             *  so a re-calibration is needed.
             */
            setPayloadNotCalibrated();
        }
        AIS_LOG_NOTICE("HydOilTempEnabled = %d", cnfg_.hydOilTempEnabled);
        return true;
    }
    else {
        // Cannot set this.
        AIS_LOG_WARN("HydOilTempEnabled not configurable");
        return false;
    }
}

bool LpsSaWeighApp::setIMUCompEnableStatus(bool enabled) {
    if (cnfg_.imuCompEnabled != enabled) {
        // A change is requested

        // For this software, until IMU compensation is validated, we cannot allow IMU compensation to be enabled.
        //if (enabled && (!machineProperties.imuCompSupported)) {
        if (enabled) {
            // Cannot enable this.
            AIS_LOG_WARN("IMUCompEnabled not supported");
            return false;
        }

        cnfg_.imuCompEnabled = enabled;
        cnfg_.setSaveNeeded();

        // report it to seal tracker
        sealTracker_.reportIMUCompensationEnableStatus(enabled);
    }
    AIS_LOG_NOTICE("IMUCompEnabled = %d", cnfg_.imuCompEnabled);
    return true;
}

bool LpsSaWeighApp::setLoadCheckValveInstallStatus(bool installed) {

    if (cnfg_.loadCheckValveInstalled != installed) {
        // A change is requested.
        cnfg_.loadCheckValveInstalled = installed;
        cnfg_.setSaveNeeded();

        /*
         * The calibration was performed under different conditions
         *  so a re-calibration is needed.
         */
        setPayloadNotCalibrated();
    }
    AIS_LOG_NOTICE("LoadCheckValveInstalled = %d", cnfg_.loadCheckValveInstalled);
    return true;
}

bool LpsSaWeighApp::setAudibleWeightEnableStatus(bool enabled) {
    if (cnfg_.audibleWeightEnabled != enabled) {
        // A change is requested.
        cnfg_.audibleWeightEnabled = enabled;
        cnfg_.setSaveNeeded();
    }
    AIS_LOG_NOTICE("AudibleWeightEnabled = %d", cnfg_.audibleWeightEnabled);
    return true;
}

bool LpsSaWeighApp::setFlashEnableStatus(bool enabled) {
    if (cnfg_.flashEnabled != enabled) {
        // A change is requested.
        cnfg_.flashEnabled = enabled;
        cnfg_.setSaveNeeded();
    }
    AIS_LOG_NOTICE("flashEnabled = %d", cnfg_.flashEnabled);
    return true;
}
/******************************************************************************
FUNCTION NAME:setIMUPitchCalOffsetNVM
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::setIMUPitchCalOffsetNVM(float imu_cal_offset)
{
    cnfg_.tipoffPitchCalOffset = imu_cal_offset;
    cnfg_.setSaveNeeded();
    AIS_LOG_NOTICE("tipoffPitchCalOffset = %f", cnfg_.tipoffPitchCalOffset);
}

void LpsSaWeighApp::setWeighRange(float weighRangeBottom, float weighRangeSize)
{
    // Limit the minimum weigh range start.
    if (weighRangeBottom < WeighRangeConfig.minimumWeighRangeStart) {
        weighRangeBottom = WeighRangeConfig.minimumWeighRangeStart;
    }

    // Limit the minimum weigh range size.
    if (weighRangeSize < WeighRangeConfig.minimumWeighRangeSize) {
        weighRangeSize = WeighRangeConfig.minimumWeighRangeSize;
    }

    // Calculate the new weigh range end.
    float weighRangeTop = weighRangeBottom + weighRangeSize;

    // Limit the maximum weigh range end.
    if (weighRangeTop > WeighRangeConfig.maximumWeighRangeEnd) {
        weighRangeTop = WeighRangeConfig.maximumWeighRangeEnd;

        /*
         * It is possible that the weigh range is now too small, so we need to adjust it back down
         */
        if ((weighRangeTop - weighRangeBottom) < WeighRangeConfig.minimumWeighRangeSize) {
            weighRangeBottom = weighRangeTop - WeighRangeConfig.minimumWeighRangeSize;
        }
    }

    AIS_LOG_NOTICE("Weigh Range = %f - %f", weighRangeBottom, weighRangeTop);

    cnfg_.weighRangeStart = weighRangeBottom;
    cnfg_.weighRangeSize = weighRangeTop - weighRangeBottom;
    sealTracker_.reportWeighRange(cnfg_.weighRangeStart, cnfg_.weighRangeSize);

    cnfg_.setSaveNeeded();

    CalNVMReinitFlag = TRUE;
}

/******************************************************************************
FUNCTION resetToaAnchorValues
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::resetToaAnchorValues()
{
    cnfg_.toaAnchoredFactor = 1.f;
    cnfg_.toaAnchoredZeroOffset = 0.f;
    cnfg_.setSaveNeeded();

    /* unlatch the tipoff weight which it might be holding with old anchor values */
    LpsSaWeighInfoTbl.TipoffInputs.unlatch_trigger = true;

    /* set anchor status to reset */
    LpsSaWeighInfoTbl.ToaAnchorStatus =
            LpsSaWeighTxChannelStorage::ToaAnchorStatus_t::RESET;

    AIS_LOG_NOTICE("ToaAnchor Reset");
}

/******************************************************************************
FUNCTION NAME:LpsSaWeighReqstRead
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::LpsSaWeighCalReqstCallback()
{
    weigh_app_interfaces::msg::CalMgrCmdReqst calMgrCmdReq;

    while (LpsCalCmdScsReqstIn->get(calMgrCmdReq)) {
        if (calMgrCmdReq.calibration_request.calibration_reqst_flag) {
            uint16_t calId = calMgrCmdReq.calibration_request.cal_id;
            CAL_MGR_MC_E calCmd = static_cast<CAL_MGR_MC_E>(calMgrCmdReq.calibration_request.calcmd);

            AIS_LOG_DEBUG("CalibrationRequest - id %d, cmd %d", calId, calCmd);

            weigh_app_interfaces::msg::CalMgrCmdResp calMgrCmdResp;
            CAL_MGR_MR_E calResp = CAL_MGR_MR_FAIL;

            weigh_app_interfaces::msg::LpsSaNvmCalOnTheFlyDataChannel calOtfData;

            { // This is protected with the calibration library mutex
                std::lock_guard<std::mutex> lck(calLibMtx_);

                // Update the cal_iterm
                memcpy(cal_iterm, calMgrCmdReq.calibration_request.cal_iterm.data(),
                        std::min(sizeof(cal_iterm), calMgrCmdReq.calibration_request.cal_iterm.size() * sizeof(uint32_t)));

                // Update calibration
                switch (calId) {
                case (SA_EMPTY_BUCKET_CAL_ID ):
                case (ADV_EMPTY_BUCKET_CAL_ID): {
                    calResp = LpsCalEmptyCalMain(calCmd, calId, &LpsSaWeighInfoTbl.stepNo, &LpsSaWeighInfoTbl.error);

                    if ((calCmd == CAL_MGR_MC_SUCCESS) && (calResp == CAL_MGR_MR_SUCCESS_SAVED)) {
                        /* Save off IMU Cal Results data in debug file */
                        saveIMUCalResultsToFile(LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData, calId);
                    }
                    break;
                }
                case (SA_FULL_BUCKET_CAL_ID):
                case (ADV_FULL_BUCKET_CAL_ID): {
                    calResp = LpsCalFullCalMain(calCmd, calId, &LpsSaWeighInfoTbl.stepNo, &LpsSaWeighInfoTbl.error);

                    if ((calCmd == CAL_MGR_MC_SUCCESS) && (calResp == CAL_MGR_MR_SUCCESS_SAVED)) {
                        /* get and store last payload weight (suggested cal weight) */
                        cnfg_.lastSuggestedCalWeight = LpsCalGetLastPayloadWeight();
                        cnfg_.setSaveNeeded();

                        /* reset TOA anchor values, since we just completed payload cals */
                        resetToaAnchorValues();

                        /* get and store the imu cal offset */
                        setIMUPitchCalOffsetNVM( LpsCalGetIMUCalOffset() );

                        /* Save off IMU Cal Results data in debug file */
                        saveIMUCalResultsToFile(LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData, calId);
                    }
                    break;
                }
                case (SA_LIFT_LINKAGE_CAL_ID): {
                    calResp = LpsSaLiftLinkageSensorCalibration(calCmd, calId, &LpsSaWeighInfoTbl.stepNo, &LpsSaWeighInfoTbl.error);
                    break;
                }
                case (SA_TILT_LINKAGE_CAL_ID): {
                    calResp = LpsSaTiltRotaryLinkageSensorCalibration(calCmd, calId, &LpsSaWeighInfoTbl.stepNo, &LpsSaWeighInfoTbl.error);
                    break;
                }
                case (SA_TILT_INLINE_LINKAGE_CAL_ID): {
                    calResp = LpsSaTiltInlineLinkageSensorCalibration(calCmd, calId, &LpsSaWeighInfoTbl.stepNo, &LpsSaWeighInfoTbl.error);
                    break;
                }
                default: {
                    calResp = CAL_MGR_MR_FAIL;
                    break;
                }
                }

                // Build the response
                calMgrCmdResp.calibration_resp.resp_code = static_cast<uint8_t>(LPS_SA_CAL_SUCCESS);
                calMgrCmdResp.calibration_resp.cal_resp = static_cast<uint8_t>(calResp);
                calMgrCmdResp.calibration_resp.error = LpsSaWeighInfoTbl.error;
                calMgrCmdResp.calibration_resp.step_no = LpsSaWeighInfoTbl.stepNo;
                calMgrCmdResp.calibration_resp.warning = LpsSaWeighInfoTbl.warning;
                calMgrCmdResp.calibration_resp.enable_qual_read = LpsCalGetQualReadStatus();

                // Build the cal OTF data
                LpsCalStatus_t calStatusFlags;
                LpsCalGetCalStatus(&calStatusFlags);
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.status_flags.cal_in_progress = calStatusFlags.CalInProgress;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.status_flags.empty_bkt_cal_done = calStatusFlags.EmptyBktCalDone;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.status_flags.full_bkt_cal_done = calStatusFlags.FullBktCalDone;

                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.lift_he_pres_stat = weighUpdtTbl.LiftCylHePres.Stat;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.lift_re_pres_stat = weighUpdtTbl.LiftCylRePres.Stat;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.lift_cyl_length_norm_stat = weighUpdtTbl.LiftCylLengthNorm.Stat;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.hyd_oil_temp_stat = weighUpdtTbl.HydOilTemp.Stat;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.lift_cyl_vel_stat = weighUpdtTbl.LiftCylVel.Stat;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.tilt_cyl_length_norm_stat = weighUpdtTbl.TiltCylLengthNorm.Stat;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.bkt_angle_stat = weighUpdtTbl.BktAngle.Stat;

                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.hyd_oil_temp_met = LpsCalWrkTbl.CurveFitInfo.HydOilTempMet;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.enforce_raise_detent = LpsCalWrkTbl.CurveFitInfo.EnforceRaiseDetent;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.enforce_lower_detent = LpsCalWrkTbl.CurveFitInfo.EnforceLowerDetent;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.raise_fast_vel_done = LpsCalWrkTbl.CurveFitInfo.RaiseFastVelDone;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.raise_slow_fit_done = LpsCalWrkTbl.CurveFitInfo.RaiseSlowFitDone;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.raise_fast_fit_done = LpsCalWrkTbl.CurveFitInfo.RaiseFastFitDone;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.lower_slow_fit_done = LpsCalWrkTbl.CurveFitInfo.LowerSlowFitDone;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.internal_step = LpsCalWrkTbl.CurveFitInfo.InternalStep;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.raise_num_points = LpsCalWrkTbl.CurveFitInfo.LpsCalRaiseCurveFitData.TotalNumPoints;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.lower_num_points = LpsCalWrkTbl.CurveFitInfo.LpsCalLowerCurveFitData.TotalNumPoints;

                { // LpsCalIMUResults_t -> weigh_app_interfaces::msg::LpsCalIMUResults, field-by-field (different types now)
                    const LpsCalIMUResults_t& src = LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData;
                    auto& dst = calOtfData.lps_sa_nvm_calibration_data_on_the_fly.curve_info.imu_cal_results;
                    dst.full_slow_imu_offset_temp1 = src.FullSlowImuOffsetTemp1;
                    dst.full_slow_imu_offset_temp2 = src.FullSlowImuOffsetTemp2;
                    dst.full_slow_imu_offset_temp3 = src.FullSlowImuOffsetTemp3;
                    dst.full_slow_lumped_weight_temp1 = src.FullSlowLumpedWeightTemp1;
                    dst.full_slow_lumped_weight_temp2 = src.FullSlowLumpedWeightTemp2;
                    dst.full_slow_lumped_weight_temp3 = src.FullSlowLumpedWeightTemp3;
                    dst.empty_slow_imu_offset_temp1 = src.EmptySlowImuOffsetTemp1;
                    dst.empty_slow_imu_offset_temp2 = src.EmptySlowImuOffsetTemp2;
                    dst.empty_slow_imu_offset_temp3 = src.EmptySlowImuOffsetTemp3;
                    dst.empty_slow_lumped_weight_temp1 = src.EmptySlowLumpedWeightTemp1;
                    dst.empty_slow_lumped_weight_temp2 = src.EmptySlowLumpedWeightTemp2;
                    dst.empty_slow_lumped_weight_temp3 = src.EmptySlowLumpedWeightTemp3;
                    dst.full_slow_imu_offset_final = src.FullSlowImuOffsetFinal;
                    dst.empty_slow_imu_offset_final = src.EmptySlowImuOffsetFinal;
                    dst.full_slow_lumped_weight_final = src.FullSlowLumpedWeightFinal;
                    dst.empty_slow_lumped_weight_final = src.EmptySlowLumpedWeightFinal;
                }

                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.lift_lever_info_lever_info_available = LpsCalWrkTbl.Update.LiftLeverInfo.LeverInfoAvailable;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_updates.lift_lever_info_faulted = LpsCalWrkTbl.Update.LiftLeverInfo.Faulted;

                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lps_cal_ovr_state = LpsCalWrkTbl.Ov.LpsCalOvrState;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lift_bottom_flag = LpsCalWrkTbl.Ov.LiftBottomFlag;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lift_top_flag = LpsCalWrkTbl.Ov.LiftTopFlag;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lps_cal_app_inf_cal_ovr_active = LpsCalWrkTbl.Ov.LpsCalAppInfCalOvrActive;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lps_cal_app_inf_lower_cmd_lmt = LpsCalWrkTbl.Ov.LpsCalAppInfLowerCmdLmt;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lps_cal_app_inf_raise_cmd_lmt = LpsCalWrkTbl.Ov.LpsCalAppInfRaiseCmdLmt;

                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lift_faulted = LpsSaWeighInfoTbl.LiftLeverInfo.Faulted;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lift_in_center = LpsSaWeighInfoTbl.LiftLeverInfo.InCenter;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lift_in_lower_detent = LpsSaWeighInfoTbl.LiftLeverInfo.InLowerDetent;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lift_in_raise_detent = LpsSaWeighInfoTbl.LiftLeverInfo.InRaiseDetent;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.lift_valve_cmd_percent = LpsSaWeighInfoTbl.LiftLeverInfo.ValveCmdPercent;

                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.cal_ovr_acknowledge = LpsCalWrkTbl.Update.CalOvrAcknowledge;

                /* tilt overrides */
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.cal_tilt_override_complete = LpsCalWrkTbl.Info.CalTiltOverrideComplete;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.cal_tilt_override_fail = LpsCalWrkTbl.Info.CalTiltOverrideFail;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.count_of_samples = LpsCalWrkTbl.Ov.LpsCalTiltOv.CountOfSamples;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.desired_tilt_extmm = LpsCalWrkTbl.Ov.LpsCalTiltOv.DesiredTiltExtmm;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.lps_cal_tilt_ovr_start_request = LpsCalWrkTbl.Ov.LpsCalTiltOv.LpsCalTiltOvrStartRequest;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.lps_cal_tilt_ovr_state = LpsCalWrkTbl.Ov.LpsCalTiltOv.LpsCalTiltOvrState;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.tilt_dump_cmd_ovr = LpsCalWrkTbl.Ov.LpsCalTiltOv.TiltDumpCmdOvr;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.tilt_noise_max = LpsCalWrkTbl.Ov.LpsCalTiltOv.TiltNoiseMax;
                calOtfData.lps_sa_nvm_calibration_data_on_the_fly.cal_overrides.tilt_overrides.tilt_noise_min = LpsCalWrkTbl.Ov.LpsCalTiltOv.TiltNoiseMin;

            }

            // Send Response
            if (nullptr != LpsCalCmdScsRespOut) {
                if (!LpsCalCmdScsRespOut->publish(calMgrCmdResp)) {
                    AIS_LOG_ERROR("Could not publish calibration response.");
                }
            }
            else {
                AIS_LOG_ERROR("LpsCalCmdScsRespOut is null.");
            }

            // Send the Cal OTF Data
            if (nullptr != LpsNvmOnTheFlyDumpChanOut) {
                LpsNvmOnTheFlyDumpChanOut->publish(calOtfData);
            }
        }
        else {
            // Do nothing
        }
    }
}

/******************************************************************************
FUNCTION saveIMUCalResultsToFile
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
****************************************************************************/
void LpsSaWeighApp::saveIMUCalResultsToFile(LpsCalIMUResults_t imu_cal_results, uint16_t cal_id)
{
    if (ADV_EMPTY_BUCKET_CAL_ID == cal_id) {
        // zero out the full call results
        imu_cal_results.FullSlowImuOffsetFinal = 0.f;
        imu_cal_results.FullSlowImuOffsetTemp1 = 0.f;
        imu_cal_results.FullSlowImuOffsetTemp2 = 0.f;
        imu_cal_results.FullSlowImuOffsetTemp3 = 0.f;
        imu_cal_results.FullSlowLumpedWeightFinal = 0.f;
        imu_cal_results.FullSlowLumpedWeightTemp1 = 0.f;
        imu_cal_results.FullSlowLumpedWeightTemp2 = 0.f;
        imu_cal_results.FullSlowLumpedWeightTemp3 = 0.f;
    }
    else if (ADV_FULL_BUCKET_CAL_ID == cal_id) {
        // read the empty cal and write back existing empty and new full
        LpsCalIMUResults_t existing_imu_cal_results = {};
        readIMUCalResultsFromFile(existing_imu_cal_results);

        imu_cal_results.EmptySlowImuOffsetFinal = existing_imu_cal_results.EmptySlowImuOffsetFinal;
        imu_cal_results.EmptySlowImuOffsetTemp1 = existing_imu_cal_results.EmptySlowImuOffsetTemp1;
        imu_cal_results.EmptySlowImuOffsetTemp2 = existing_imu_cal_results.EmptySlowImuOffsetTemp2;
        imu_cal_results.EmptySlowImuOffsetTemp3 = existing_imu_cal_results.EmptySlowImuOffsetTemp3;
        imu_cal_results.EmptySlowLumpedWeightFinal = existing_imu_cal_results.EmptySlowLumpedWeightFinal;
        imu_cal_results.EmptySlowLumpedWeightTemp1 = existing_imu_cal_results.EmptySlowLumpedWeightTemp1;
        imu_cal_results.EmptySlowLumpedWeightTemp2 = existing_imu_cal_results.EmptySlowLumpedWeightTemp2;
        imu_cal_results.EmptySlowLumpedWeightTemp3 = existing_imu_cal_results.EmptySlowLumpedWeightTemp3;
    }

    // Create an output filestream object
    boost::filesystem::create_directories("/opt/appdata/CPM/debug");
    std::ofstream myFile("/opt/appdata/CPM/debug/imu_cal_results.csv");

    // Send data to the stream
    myFile << tes_common_ais::putTime(std::chrono::system_clock::now(), tes_common_ais::date_time_formats::ISO_DATE_TIME_LOCAL) << "\n";
    myFile << "FullSlowImuOffsetFinal," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.FullSlowImuOffsetFinal << "\n";
    myFile << "EmptySlowImuOffsetFinal," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.EmptySlowImuOffsetFinal << "\n";
    myFile << "FullSlowLumpedWeightFinal," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.FullSlowLumpedWeightFinal << "\n";
    myFile << "EmptySlowLumpedWeightFinal," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.EmptySlowLumpedWeightFinal << "\n";
    myFile << "FullSlowImuOffsetTemp1," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.FullSlowImuOffsetTemp1 << "\n";
    myFile << "FullSlowImuOffsetTemp2," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.FullSlowImuOffsetTemp2 << "\n";
    myFile << "FullSlowImuOffsetTemp3," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.FullSlowImuOffsetTemp3 << "\n";
    myFile << "EmptySlowImuOffsetTemp1," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.EmptySlowImuOffsetTemp1 << "\n";
    myFile << "EmptySlowImuOffsetTemp2," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.EmptySlowImuOffsetTemp2 << "\n";
    myFile << "EmptySlowImuOffsetTemp3," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.EmptySlowImuOffsetTemp3 << "\n";
    myFile << "FullSlowLumpedWeightTemp1," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.FullSlowLumpedWeightTemp1 << "\n";
    myFile << "FullSlowLumpedWeightTemp2," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.FullSlowLumpedWeightTemp2 << "\n";
    myFile << "FullSlowLumpedWeightTemp3," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.FullSlowLumpedWeightTemp3 << "\n";
    myFile << "EmptySlowLumpedWeightTemp1," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.EmptySlowLumpedWeightTemp1 << "\n";
    myFile << "EmptySlowLumpedWeightTemp2," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.EmptySlowLumpedWeightTemp2 << "\n";
    myFile << "EmptySlowLumpedWeightTemp3," << LpsCalWrkTbl.CurveFitInfo.LpsCalIMUResultsData.EmptySlowLumpedWeightTemp3 << "\n";

    // Close the file
    myFile.close();
}

/******************************************************************************
FUNCTION readIMUCalResultsFromFile
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
****************************************************************************/
void LpsSaWeighApp::readIMUCalResultsFromFile(LpsCalIMUResults_t& imu_cal_results)
{
    // Create an output filestream object
    std::ifstream myFile("/opt/appdata/CPM/debug/imu_cal_results.csv");

    // Make sure the file is open

    if(!myFile.is_open()){
        AIS_LOG_INFO("Could not open imu cal data results file");
        return;
    }

    // read date/time and do nothing with it
    std::string temp;
    std::getline(myFile, temp);

    // read each line and extract the value from each line
    std::getline(myFile, temp);
    imu_cal_results.FullSlowImuOffsetFinal = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.FullSlowImuOffsetFinal = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.EmptySlowImuOffsetFinal = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.FullSlowLumpedWeightFinal = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.EmptySlowLumpedWeightFinal = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.FullSlowImuOffsetTemp1 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.FullSlowImuOffsetTemp2 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.FullSlowImuOffsetTemp3 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.EmptySlowImuOffsetTemp1 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.EmptySlowImuOffsetTemp2 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.EmptySlowImuOffsetTemp3 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.FullSlowLumpedWeightTemp1 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.FullSlowLumpedWeightTemp2 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.FullSlowLumpedWeightTemp3 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.EmptySlowLumpedWeightTemp1 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.EmptySlowLumpedWeightTemp2 = extractValFromString(temp);

    std::getline(myFile, temp);
    imu_cal_results.EmptySlowLumpedWeightTemp3 = extractValFromString(temp);

    // Close the file
    myFile.close();
}

static float extractValFromString(const std::string& str)
{
    // returns the number(float) after a ',' from the passed string
    std::string numberPart = str.substr(str.find(",")+1);
    float number = std::atof(numberPart.c_str());
    return number;
}

/******************************************************************************
FUNCTION NAME:LpsJobMgrTxRead
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::LpsJobMgrTxRead( )
{
    job_mgr_interfaces::msg::LpsSaJobMgrTxChannel jobMgrIn;
    while (LpsSaJobMgrScsTxIn->get(jobMgrIn)) {
        LpsSaWeighInfoTbl.passCount = jobMgrIn.pass_count;
        LpsSaWeighInfoTbl.TruckTargetWeight = jobMgrIn.truck_target_weight;
        LpsSaWeighInfoTbl.TruckStartWeight = jobMgrIn.truck_start_weight;
        LpsSaWeighInfoTbl.TipOffState = static_cast<LpsSaJobMgrTipOffState_t>(jobMgrIn.tip_off_state.value);
        LpsSaWeighInfoTbl.TipOffStateCfg = static_cast<LpsSaJobMgrTipOffState_t>(jobMgrIn.tip_off_state_cfg.value);

        /* Receive standby state  from job manager */
        LpsSaWeighInfoTbl.StandbyState = static_cast<LpsSaJobMgrStandbyState_t>(jobMgrIn.standby_state.value);

        /* Flag to indicate if tipoff is currently active */
        if ((jobMgrIn.tipoff_active) && (!LpsSaWeighInfoTbl.TipoffActive)) {
            // If we are entering tip-off, then we are done digging
            LpsWeighEndDig();
        }
        LpsSaWeighInfoTbl.TipoffActive = jobMgrIn.tipoff_active;

        if (storePressCount != jobMgrIn.store_count) {
            sumOfAdjustedTruckWts = 0.0f;
            sumOfZeroedTruckWts = 0.0f;
            AIS_LOG_NOTICE("Storebutton pressed. Adj = %f & Zeroed = %f", sumOfAdjustedTruckWts, sumOfZeroedTruckWts );
            storePressCount = jobMgrIn.store_count;
        }

        // only update the simple cal adjust factor when we are starting a new load which includes new split loads
        if ((0 == LpsSaWeighInfoTbl.passCount) &&
                (1 == jobMgrIn.subtotal_count ) &&
                (payloadCalNvmTbl_.data.CalAdjust != updatedSimpleCalFactor)) {
            // Store it in NVM
            payloadCalNvmTbl_.data.CalAdjust = updatedSimpleCalFactor;
            sealTracker_.reportSpanAdjustFactor(updatedSimpleCalFactor);

            payloadCalNvmTbl_.setSaveNeeded();

            /* Update the weigh lib to use new simple cal factor */
            LpsSetCalAdjustFactor(updatedSimpleCalFactor);

            /* Update the init table with new simple cal factor to keep it in sync. */
            LpsSaInitTbl.WeighInitTbl.MachSpecificCfg.CalibTbl.CalAdjust = updatedSimpleCalFactor;
        }
    }

    // demoInputs_ stays on its real, old type (see LpsSaWeighApp.h) -- drain
    // the wrapper into a new-typed local, then copy every field across.
    weigh_app_interfaces::msg::DemoAppTxChannel demoInputsNew;
    while (DemoAppTxIn->get(demoInputsNew)) {
        demoInputs_.liftposition = demoInputsNew.liftposition;
        demoInputs_.liftposition_rate = demoInputsNew.liftposition_rate;
        demoInputs_.tiltposition = demoInputsNew.tiltposition;
        demoInputs_.tiltposition_rate = demoInputsNew.tiltposition_rate;
        demoInputs_.payload = demoInputsNew.payload;
        demoInputs_.payload_rate = demoInputsNew.payload_rate;
        demoInputs_.dig = demoInputsNew.dig;
        demoInputs_.wrw = demoInputsNew.wrw;
        demoInputs_.llw = demoInputsNew.llw;
        demoInputs_.dump = demoInputsNew.dump;
        demoInputs_.pdump = demoInputsNew.pdump;
        demoInputs_.rack = demoInputsNew.rack;
        demoInputs_.carry = demoInputsNew.carry;
        demoInputs_.weight1 = demoInputsNew.weight1;
        demoInputs_.pres_lift_he_demo = demoInputsNew.pres_lift_he_demo;
        demoInputs_.pres_lift_re_demo = demoInputsNew.pres_lift_re_demo;
        demoInputs_.angle_lift_demo = demoInputsNew.angle_lift_demo;
        demoInputs_.angle_AFE_demo = demoInputsNew.angle_afe_demo;
        demoInputs_.angle_ABC_demo = demoInputsNew.angle_abc_demo;
    }
}

/******************************************************************************
FUNCTION LpsSaBmiRead
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void LpsSaWeighApp::LpsSaBmiRead( )
{
#define BMI_CDL_PID_HYD_OIL_TEMP     0x0045

    /* Gear PIDs supported for machine model variances */
#define BMI_CDL_PID_ACTUAL_GEAR                     0xF5D9
#define BMI_CDL_PID_DESIRED_GEAR                    0xF5D7
#define BMI_CDL_PID_DIRECTION_SWITCH_POSITION       0xF074
#define BMI_CDL_PID_TRANSMISSION_GEAR               0xF002
#define BMI_CDL_PID_TIP_ASSIST_ENABLE               0xD118CA
#define BMI_PGN_STEERING_ANGLE                      0xF00B
#define BMI_CDL_PID_GROUND_SPEED                    0xF4FD
#define BMI_CDL_PID_GROUND_SPEED_PID18              0x0018
#define BMI_CDL_PID_MACHINE_IDLE_STATUS             0xD10AB5
#define BMI_CDL_PID_LOAD_HOLD_CHECK_VALVE           0xD10F6E
#define BMI_CDL_PID_LIFT_POS_FULL_LOWER_DC          0xD010FB
#define BMI_CDL_PID_LIFT_POS_FULL_RAISE_DC          0xD010FA
#define BMI_CDL_PID_TILT_POS_FULL_DUMP_DC           0xD010FD
#define BMI_CDL_PID_TILT_POS_FULL_RACK_DC           0xD010FC
#define BMI_CDL_PID_LEGAL_FOR_TRADE_SW_ID           0xF9C0
#define BMI_CAT_EXT_FLASH_INFORMATION_MESSAGE       0xF003

#define ECM_SERIAL_NUMBER_MAX_SIZE  16

    weigh_app_interfaces::msg::DataLinkData dlData;
    while (DataLinkDataInput_->get(dlData)) {
        for (auto& dlParam : dlData.params) {
            // Skip this if no new data is received.
            if (!dlParam.pid_data_received) {
                continue;
            }

            AIS_LOG_INFO("sid %x paramId %x  dsi %x ParamValue raw %d eng %f scaling %f offset %f units %d",
                    dlParam.sid,
                    dlParam.param_id, dlParam.last_value_dsi,
                    dlParam.last_value_f32, dlParam.last_value_eng,
                    dlParam.scaling, dlParam.offset,
                    dlParam.units);

            // check Hyd-Oil--Temp DIAG only if we have selected a machine
            if (!LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::MACHINE_MODEL_NOT_SET]) {
                switch (dlParam.identifier_type) {
                // CDL PIDs
                case (weigh_app_interfaces::msg::DataLinkParam::IDENTIFIER_TYPE_PID): {
                    switch (dlParam.param_id) {
                    case BMI_CDL_PID_HYD_OIL_TEMP:
                    {
                        bool isBad;

                        if (0 == dlParam.last_value_dsi) {
                            WeighPidTbl.HydOilTemp = (float)dlParam.last_value_eng;
                            isBad = false;
                            AIS_LOG_DEBUG("Hydraulic Oil Temp received: %f", (float)dlParam.last_value_eng);
                        }
                        else {
                            WeighPidTbl.HydOilTemp = (float)HYDRAULIC_OIL_TEMP_MIN_VALID_DATA;  // which is -32736.0
                            isBad = true;
                            AIS_LOG_DEBUG("Hydraulic Oil Temp PID DSI reported: %d", dlParam.last_value_dsi);
                        }

                        // Set the diagnostic flag if supported.
                        if ((machineProperties.hydOilTempOptional) && (!cnfg_.hydOilTempEnabled)) {
                            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::HYDRAULIC_OIL_TEMP_BAD] = false;
                        }
                        else {
                            LpsSaWeighInfoTbl.DiagState[ACDDiagPopUp::HYDRAULIC_OIL_TEMP_BAD] = isBad;
                        }
                        break;
                    }

                    case BMI_CDL_PID_DESIRED_GEAR:
                    case BMI_CDL_PID_ACTUAL_GEAR:
                    {
                        if (0 == dlParam.last_value_dsi) {
                            weighUpdtTbl.RequestedGear.Stat = LPS_STATUS_OK;
                            weighUpdtTbl.RequestedGear.Val = (uint16_t)dlParam.last_value_eng;

                            AIS_LOG_DEBUG("Actual/Desired Gear PID received: %d", (uint16_t)dlParam.last_value_eng);
                        }
                        else {
                            weighUpdtTbl.RequestedGear.Stat = LPS_STATUS_BAD;
                            weighUpdtTbl.RequestedGear.Val = (uint16_t)dlParam.last_good_value_eng;

                            AIS_LOG_DEBUG("Actual/Desired Gear PID DSI reported: %d", dlParam.last_value_dsi);
                        }
                        break;
                    }

                    case BMI_CDL_PID_DIRECTION_SWITCH_POSITION:
                    {
                        if (0 == dlParam.last_value_dsi) {
                            weighUpdtTbl.RequestedGear.Stat = LPS_STATUS_OK;

                            if ((uint16_t)dlParam.last_good_value_eng == 0) {
                                weighUpdtTbl.RequestedGear.Val = 0x1000; /* Reverse, PID 0xF5D7 */
                            }
                            else if ((uint16_t)dlParam.last_good_value_eng == 1) {
                                weighUpdtTbl.RequestedGear.Val = 0x4000; /* Forward, PID 0xF5D7 */
                            }
                            else {
                                weighUpdtTbl.RequestedGear.Val = 0x0000; /* not forward, not reverse, PID 0xF5D7 */
                            }

                            AIS_LOG_DEBUG("Direction Switch Position PID received: %d", (uint16_t)dlParam.last_good_value_eng);
                        }
                        else {
                            weighUpdtTbl.RequestedGear.Stat = LPS_STATUS_BAD;

                            AIS_LOG_DEBUG("Direction Switch Position PID DSI reported: %d", dlParam.last_value_dsi);
                        }
                        break;
                    }

                    case BMI_CDL_PID_TRANSMISSION_GEAR:
                    {
                        if (0 == dlParam.last_value_dsi) {
                            weighUpdtTbl.RequestedGear.Stat = LPS_STATUS_OK;

                            if ((uint8_t)dlParam.last_good_value_eng & 0x80) {
                                weighUpdtTbl.RequestedGear.Val = 0x4000; /* Forward, PID 0xF5D7 */
                            }
                            else if ((uint8_t)dlParam.last_good_value_eng & 0x20) {
                                weighUpdtTbl.RequestedGear.Val = 0x1000; /* Reverse, PID 0xF5D7 */
                            }
                            else {
                                weighUpdtTbl.RequestedGear.Val = 0x0000; /* not forward, not reverse, PID 0xF5D7 */
                            }

                            AIS_LOG_DEBUG("Transmission Gear PID received: %d", (uint8_t)dlParam.last_good_value_eng);
                        }
                        else {
                            weighUpdtTbl.RequestedGear.Stat = LPS_STATUS_BAD;

                            AIS_LOG_DEBUG("Transmission Gear PID DSI reported: %d", dlParam.last_value_dsi);
                        }
                        break;
                    }
                    case BMI_CDL_PID_TIP_ASSIST_ENABLE:
                    {
                        if (0 == dlParam.last_value_dsi) {
                            if (0x000C == (uint16_t)dlParam.last_good_value_eng)
                                LpsSaWeighInfoTbl.TipOffAssistEnable = true;   /* enabled */
                            else
                                LpsSaWeighInfoTbl.TipOffAssistEnable = false;   /* disabled */

                            AIS_LOG_DEBUG("Tip-off Assist Enable val: %d", (uint8_t)dlParam.last_good_value_eng );
                        }
                        else {
                            LpsSaWeighInfoTbl.TipOffAssistEnable = 0;   /* disabled */
                            AIS_LOG_DEBUG("Tip-off Assist Enable PID DSI reported: %d", dlParam.last_value_dsi );
                        }
                        break;
                    }
                    case BMI_CDL_PID_LOAD_HOLD_CHECK_VALVE:
                    {
                        if (0 == dlParam.last_value_dsi) {
                            if (0x0010 == dlParam.last_value_i16)
                                setLoadCheckValveInstallStatus(true);   /* HBCV is installed */
                            else
                                setLoadCheckValveInstallStatus(false); /* HBCV is not installed */

                            AIS_LOG_DEBUG("Load hold valve check valve installation val: %d", dlParam.last_value_i16 );
                        }
                        else {
                            AIS_LOG_DEBUG("Load hold valve check valve installation PID DSI reported: %d", dlParam.last_value_dsi );
                        }
                        break;
                    }
                    case BMI_CDL_PID_GROUND_SPEED:
                    {
                        int16_t value = dlParam.last_value_i16;
                        uint16_t dsi = dlParam.last_value_dsi;

                        if (0 == dsi) {
                            // Store ground speed in mm/sec/bit (0.0036 km/hr/bit)
                            weighUpdtTbl.GroundSpeed = value;
                            AIS_LOG_DEBUG("GroundSpeed: %f", weighUpdtTbl.GroundSpeed );
                        }
                        else {
                            weighUpdtTbl.GroundSpeed = 0; // default
                        }

                        // Let the chassis imu know about this.
                        chassisImu_.rxPIDF4FD(value, dsi);
                        break;
                    }
                    case BMI_CDL_PID_GROUND_SPEED_PID18:
                    {
                        float value = dlParam.last_good_value_eng;
                        uint8_t dsi = dlParam.last_value_dsi;

                        if (0 == dsi) {
                            // Store ground speed in mm/sec/bit (1mph/bit = 447.04mms/bit)
                            weighUpdtTbl.GroundSpeed = value * 447.04;
                            AIS_LOG_DEBUG("GroundSpeed: %f", weighUpdtTbl.GroundSpeed );
                        }
                        else {
                            weighUpdtTbl.GroundSpeed = 0; // default
                        }

                        // Let the chassis imu know about this. (mph/bit to 0.0036kmhr/bit
                        chassisImu_.rxPIDF4FD(weighUpdtTbl.GroundSpeed, dsi);
                        break;
                    }
                    case BMI_CDL_PID_MACHINE_IDLE_STATUS: // Machine Idle Status
                    {
                        uint16_t value = dlParam.last_value_u16;
                        uint16_t dsi = dlParam.last_value_dsi;
                        chassisImu_.rxPIDD10AB5(value, dsi);
                        break;
                    }

                    case BMI_CDL_PID_LIFT_POS_FULL_LOWER_DC: // LIFT_POS_FULL_LOWER_DC
                    {
                        // Only used for AU2020 and Not legal for Trade
                        if (!payloadCalNvmTbl_.legalForTradeInstalled &&
                                ADVANCED == getApplicationVariant()) {
                            float lift_full_lower_dc = dlParam.last_good_value_eng;
                            uint16_t dsi = dlParam.last_value_dsi;

                            if (!dsi) {
                                /* set cal value if its different */
                                if (setLiftLinkageSensorFullLowerCalValue(lift_full_lower_dc)) {
                                    setPayloadNotCalibrated();
                                    AIS_LOG_ALERT("Payload Set to OOC. Lift Full Lower DC: %f", lift_full_lower_dc);
                                }
                            }
                            else {
                                /* if uncalibrated, then mark it uncalibrated */
                                if (dsi == (UNKNOWN2U+FMICAL)) {
                                    setLiftLinkageSensorNotCalibrated();
                                }
                                else {
                                    // any other dsi is ignored.  The payload system will still operate
                                    // with previous values which could be wrong.
                                    // We should set a fault flag here to lock the payload system.
                                }
                            }
                        }
                        break;
                    }

                    case BMI_CDL_PID_LIFT_POS_FULL_RAISE_DC: // LIFT_POS_FULL_RAISE_DC
                    {
                        // Only used for AU2020 and Not legal for Trade
                        if (!payloadCalNvmTbl_.legalForTradeInstalled &&
                                ADVANCED == getApplicationVariant()) {
                            float lift_full_raise_dc = dlParam.last_good_value_eng;
                            uint16_t dsi = dlParam.last_value_dsi;

                            if (!dsi) {
                                /* set cal value if its different */
                                if (setLiftLinkageSensorFullRaiseCalValue(lift_full_raise_dc)) {
                                    setPayloadNotCalibrated();
                                    AIS_LOG_ALERT("Payload set to OOC. Lift Full Raise DC: %f", lift_full_raise_dc);
                                }
                            }
                            else {
                                /* if uncalibrated, then mark it uncalibrated */
                                if (dsi == (UNKNOWN2U+FMICAL)) {
                                    setLiftLinkageSensorNotCalibrated();
                                }
                                else {
                                    // any other dsi is ignored.  The payload system will still operate
                                    // with previous values which could be wrong.
                                    // We should set a fault flag here to lock the payload system.
                                }
                            }
                        }
                        break;
                    }

                    case BMI_CDL_PID_TILT_POS_FULL_RACK_DC: // TILT_POS_FULL_RACK_DC
                    {
                        // Only used for AU2020 and Not legal for Trade
                        if (!payloadCalNvmTbl_.legalForTradeInstalled &&
                                ADVANCED == getApplicationVariant()) {
                            float tilt_full_rack_dc = dlParam.last_good_value_eng;
                            uint16_t dsi = dlParam.last_value_dsi;

                            if (!dsi) {
                                /* set cal value if its different */
                                if (setTiltLinkageSensorFullRackCalValue(tilt_full_rack_dc)) {
                                    setPayloadNotCalibrated();
                                    AIS_LOG_ALERT("Payload Set to OOC. Tilt Full Rack DC: %f", tilt_full_rack_dc);
                                }
                            }
                            else {
                                /* if uncalibrated, then mark it uncalibrated */
                                if (dsi == (UNKNOWN2U+FMICAL)) {
                                    setTiltLinkageSensorNotCalibrated();
                                }
                                else {
                                    // any other dsi is ignored.  The payload system will still operate
                                    // with previous values which could be wrong.
                                    // We should set a fault flag here to lock the payload system.
                                }
                            }
                        }
                        break;
                    }

                    case BMI_CDL_PID_TILT_POS_FULL_DUMP_DC: // TILT_POS_FULL_DUMP_DC
                    {
                        // Only used for AU2020 and Not legal for Trade
                        if (!payloadCalNvmTbl_.legalForTradeInstalled &&
                                ADVANCED == getApplicationVariant()) {
                            float tilt_full_dump_dc = dlParam.last_good_value_eng;
                            uint16_t dsi = dlParam.last_value_dsi;

                            if (!dsi) {
                                /* set cal value if its different */
                                if (setTiltLinkageSensorFullDumpCalValue(tilt_full_dump_dc)) {
                                    setPayloadNotCalibrated();
                                    AIS_LOG_ALERT("Payload Set to OOC. Tilt Full Dump DC: %f", tilt_full_dump_dc);
                                }
                            }
                            else {
                                /* if uncalibrated, then mark it uncalibrated */
                                if (dsi == (UNKNOWN2U+FMICAL)) {
                                    setTiltLinkageSensorNotCalibrated();
                                }
                                else {
                                    // any other dsi is ignored.  The payload system will still operate
                                    // with previous values which could be wrong.
                                    // We should set a fault flag here to lock the payload system.
                                }
                            }
                        }
                        break;
                    }

                    case BMI_CDL_PID_LEGAL_FOR_TRADE_SW_ID:
                    {
                        if ((0x2E == dlParam.sid) &&
                                (0 == dlParam.last_value_dsi) &&
                                (dlParam.last_value_vector.size() < 27) &&
                                (ADVANCED == getApplicationVariant())) {
                            const auto& lftSwId = dlParam.last_value_vector;
                            std::string LegalForTradeSwIdentifier(lftSwId.begin(), lftSwId.end());

                            auto pos = LegalForTradeSwIdentifier.rfind(".");
                            std::string version = LegalForTradeSwIdentifier.substr(0, pos);
                            std::string build;
                            if (pos != std::string::npos) {
                                build = LegalForTradeSwIdentifier.substr(pos+1);
                            }

                            /* "version" is passed as "vintage.variant" */
                            sealTracker_.reportInputModuleLegalForTradeSoftwareId(version, build);
                        }
                        break;
                    }

                    default: {
                        break;
                    }

                    }
                    break;
                }/* case (weigh_app_interfaces::msg::DataLinkParam::IDENTIFIER_TYPE_PID) */

                case (weigh_app_interfaces::msg::DataLinkParam::IDENTIFIER_TYPE_CAT_EXT): {
                    switch (dlParam.param_id) {
                    case BMI_CAT_EXT_FLASH_INFORMATION_MESSAGE: {
                        // Data Recieved
                        // Dta: F0 03
                        // 84 0B 01 34 39 31 36 33 30 39 2D 30 33
                        // 81 08 30 30 30 30 30 30 30 30
                        // 84 0B 02 35 33 33 37 35 32 37 2D 37 30
                        // 81 08 45 54 4E 33 36 39 34 38
                        // 90 2 0 2 2 1 0
                        // 84 0
                        uint8_t len = dlParam.var_param_block.size();

                        if ((0x2E == dlParam.sid) &&
                                (0 == dlParam.last_value_dsi) &&
                                (len > 2) &&
                                (ADVANCED == getApplicationVariant())) {
                            std::string lastServiceToolSerialNumber;
                            std::string softwarePartNumber;
                            std::string ecmSerialNumber;

                            // Locate the block of the data
                            const uint8_t* data = dlParam.var_param_block.data();

                            // Scan for Last Service Tool SN, SW Part Number and ECM Serial Number
                            // Skip the F003 Record
                            data += 2;
                            len  -= 2;

                            while (len > 0) {
                                if (len > 2) {
                                    uint8_t id = *data;
                                    uint8_t numBytesToFollow = *(data+1);

                                    // Skip the ID and LENGTH bytes
                                    data += 2;
                                    len -= 2;

                                    if (len < numBytesToFollow) {
                                        // Does not contain enough data to cover number of bytes to follow.
                                        break;
                                    }

                                    if ((0x81 == id) && (8 == numBytesToFollow)) {
                                        // Scan for a valid Last Service Tool SN - 81 Record with 8 bytes long
                                        // 81 08 45 54 4E 33 36 39 34 38
                                        lastServiceToolSerialNumber.assign((const char*)data, 8);
                                        data += 8;
                                        len -= 8;
                                    }
                                    else if ((0x84 == id) && ((SOFTWARE_GROUP_PN_LENGTH + 1) == numBytesToFollow)) {
                                        // Scan for Software Part Number on Software Type and Part Number - 84 Record with 11 bytes long
                                        // 84 0B 02 36 32 31 38 35 32 38 56 30 34 -- Software Type and Part Number

                                        // Skip the TYPE byte
                                        data += 1;
                                        len -= 1;

                                        softwarePartNumber.assign((const char*)data, SOFTWARE_GROUP_PN_LENGTH);
                                        data += SOFTWARE_GROUP_PN_LENGTH;
                                        len -= SOFTWARE_GROUP_PN_LENGTH;
                                    }
                                    else if ((0x80 == id) && ((ECM_SERIAL_NUMBER_LENGTH) == numBytesToFollow)) {
                                        //Scan for ECM Serial Number - 80 Record with *OLD* ECM (ECU with 1E2908 SN)
                                        //80 0A 3x 3x 3x 3x ...
                                        ecmSerialNumber.assign((const char*)data, ECM_SERIAL_NUMBER_LENGTH);
                                        data += ECM_SERIAL_NUMBER_LENGTH;
                                        len  -= ECM_SERIAL_NUMBER_LENGTH;
                                    }
                                    else if ((0x91 == id) && ((ECM_SERIAL_NUMBER_MAX_SIZE) == numBytesToFollow)) {
                                        //Scan for ECM Serial Number - 91 Record with *NEW* ECM (ECU with 1E2980 or 1E4978)
                                        //91 10 31 38 30 39 31 31 30 30 32 30 47 4C 30 30 30 48
                                        ecmSerialNumber.assign((const char*)data, ECM_SERIAL_NUMBER_MAX_SIZE);
                                        data += ECM_SERIAL_NUMBER_MAX_SIZE;
                                        len -= ECM_SERIAL_NUMBER_MAX_SIZE;
                                    }
                                    else {
                                        // Some other parameter that we aren't looking for.
                                        data += numBytesToFollow;
                                        len -= numBytesToFollow;
                                    }
                                }
                                else {
                                    // Nothing left
                                    data += len;
                                    len = 0;
                                }
                            }

                            if (!lastServiceToolSerialNumber.empty()) {
                                sealTracker_.reportInputModuleLastServiceToolSerialNumber(lastServiceToolSerialNumber);
                            }

                            if (!softwarePartNumber.empty()) {
                                sealTracker_.reportInputModuleSoftwarePartNumber(softwarePartNumber);
                            }

                            if (!ecmSerialNumber.empty()) {
                                if (sealTracker_.reportInputModuleEcmSerialNumber(ecmSerialNumber)) {
                                    // If the input module changes, recalibration is needed.
                                    setPayloadNotCalibrated();
                                }
                            }
                        }

                        break;
                    }

                    default: {
                        break;
                    }

                    }
                    break;
                }/* case (weigh_app_interfaces::msg::DataLinkParam::IDENTIFIER_TYPE_CAT_EXT) */

                case (weigh_app_interfaces::msg::DataLinkParam::IDENTIFIER_TYPE_PUBLIC_PGN): {
                    switch (dlParam.param_id) {
                    // Angular Rate Information (ARI)
                    case (61482): {
                        if (weigh_app_interfaces::msg::DataLinkParam::VAR_LENGTH_PARAM_TYPE_PGN == dlParam.var_length_param_type) {
                            uint16_t dsi = dlParam.last_value_dsi;
                            uint8_t len = dlParam.var_param_block.size();
                            const uint8_t* data = dlParam.var_param_block.data();

                            // Let the chassis imu know about this.
                            chassisImu_.rxPGN61482(data, len, dsi);
                        }
                        break;
                    }

                    // IMU - ECU Identification Information
                    case (64965): {
                        if (weigh_app_interfaces::msg::DataLinkParam::VAR_LENGTH_PARAM_TYPE_PGN == dlParam.var_length_param_type) {
                            uint16_t dsi = dlParam.last_value_dsi;
                            uint8_t len = dlParam.var_param_block.size();

                            // Parse the messages
                            if (len && (0 == dsi) && 0x82==dlParam.sid) {
                                const uint8_t* data = dlParam.var_param_block.data();
                                uint8_t dataIdx = 0;
                                std::string imu_id_info[6];
                                uint8_t temp;

                                for (int i=0; i<6; ++i) {
                                    std::ostringstream ss;
                                    OEL_UNPACK_LE_8(data, temp); ++dataIdx;
                                    while (temp != 0x2A) {
                                        ss << temp;
                                        if (dataIdx >= len) {
                                            break;
                                        }
                                        OEL_UNPACK_LE_8(data, temp); ++dataIdx;
                                    }
                                    imu_id_info[i] = ss.str();
                                    AIS_LOG_DEBUG("imu_id_info[%d]: %s", i, imu_id_info[i].c_str());
                                }

                                /*
                                 * std::string imu_partnum = imu_id_info[0];
                                 * std::string imu_ser_num = imu_id_info[1];
                                 * std::string imu_location = imu_id_info[2];
                                 * std::string imu_type = imu_id_info[3];
                                 * std::string imu_manufacturer_name = imu_id_info[4];
                                 * std::string imu_hw_id = imu_id_info[5];
                                 */

                                // Report the IMU Serial Number to the Seal Tracker
                                if (sealTracker_.reportIMUSerialNumber(imu_id_info[1])) {
                                    // If the IMU changes, recalibration is needed.
                                    setPayloadNotCalibrated();
                                }
                            }
                            else {
                                //AIS_LOG_ERROR("PGN64965 dsi: %d", dsi);
                            }
                        }
                        break;
                    }

                    // IMU - Software Identification
                    case (65242): {
                        if (weigh_app_interfaces::msg::DataLinkParam::VAR_LENGTH_PARAM_TYPE_PGN == dlParam.var_length_param_type) {
                            uint16_t dsi = dlParam.last_value_dsi;
                            uint8_t len = dlParam.var_param_block.size();

                            // Parse the messages
                            if (len && (0 == dsi) && 0x82==dlParam.sid) {
                                const uint8_t* data = dlParam.var_param_block.data();

                                // ignore first byte (number of fields)
                                std::string imu_sw_id_string = std::string((const char*)(data+1), len-1);

                                // Report the IMU Software Part Number to the Seal Tracker
                                sealTracker_.reportIMUSoftwarePartNumber(imu_sw_id_string);

                                AIS_LOG_DEBUG("imu_sw_id_string: %s", imu_sw_id_string.c_str());
                            }
                            else {
                                //AIS_LOG_ERROR("PGN65242 dsi: %d", dsi);
                            }
                        }
                        break;
                    }

                    // Acceleration Sensor (ACCS)
                    case (61485): {
                        if (weigh_app_interfaces::msg::DataLinkParam::VAR_LENGTH_PARAM_TYPE_PGN == dlParam.var_length_param_type) {
                            uint16_t dsi = dlParam.last_value_dsi;
                            uint8_t len = dlParam.var_param_block.size();

                            // Parse the messages for Tip-Off Assist
                            if ((0 == dsi) && (len >= 7)) {
                                const uint8_t* data = dlParam.var_param_block.data();
                                uint16_t temp;
                                OEL_UNPACK_LE_16(data, temp);
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelY = (float)temp*0.01-320;
                                OEL_UNPACK_LE_16(data, temp);
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelX = (float)temp*0.01-320;
                                OEL_UNPACK_LE_16(data, temp);
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelZ = (float)temp*0.01-320;
                                LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.imu_accel_bad = 0;
                            }
                            else {
                                LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.imu_accel_bad = 1;
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelX = 0.f;
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelY = 0.f;
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelZ = 0.f;

                                // Uncomment below for testing on bench without imu
#if 0
                                LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.imu_accel_bad = 0;
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelX = 9.81f;
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelY = 0.f;
                                LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelZ = 0.f;
#endif
                            }

                            // Let the chassis imu know about this.
                            chassisImu_.rxPGN61485(dlParam.var_param_block.data(), len, dsi);
                        }
                        else {
                            LpsSaWeighInfoTbl.TipoffInputs.input_status.flag.imu_accel_bad = 1;
                            LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelX = 0.f;
                            LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelY = 0.f;
                            LpsSaWeighInfoTbl.TipoffInputs.eef_imu_accelZ = 0.f;
                        }
                        break;
                    }
                    // PGN 61451  ELECTRONIC STEERING CONTROL (ESC1)
                    case (61451): {
                        if ((weigh_app_interfaces::msg::DataLinkParam::VAR_LENGTH_PARAM_TYPE_PGN == dlParam.var_length_param_type) &&
                                (0 == dlParam.var_length_param_dsi) &&
                                dlParam.var_param_block.size() >= 8) {
                            const uint8_t* data = dlParam.var_param_block.data();
                            uint16_t temp;
                            OEL_UNPACK_LE_16(data, temp);
                            LpsSaWeighInfoTbl.TipoffInputs.steering_angle = (float)temp/256.0-125;

                            /* check for valid range (-125deg to 125deg) */
                            if ((LpsSaWeighInfoTbl.TipoffInputs.steering_angle > 125) ||
                                    (LpsSaWeighInfoTbl.TipoffInputs.steering_angle < -125)) {
                                LpsSaWeighInfoTbl.TipoffInputs.steering_angle = 0.f;
                            }
                        }
                        else {
                            /* Assuming 0 when the steering angle message is not present
                             * (datalink issue or HMU steering machine without auto diff lock)*/
                            LpsSaWeighInfoTbl.TipoffInputs.steering_angle = 0.f;
                        }
                        break;
                    }
                    // EEC1
                    case (61444): {
                        if ((weigh_app_interfaces::msg::DataLinkParam::VAR_LENGTH_PARAM_TYPE_PGN == dlParam.var_length_param_type) &&
                                (0 == dlParam.var_length_param_dsi) &&
                                dlParam.var_param_block.size() >= 5) {
                            const uint8_t* data = dlParam.var_param_block.data();
                            uint16_t temp;
                            data += 3; // Advance to byte 4
                            OEL_UNPACK_LE_16_NO_INCR(data, temp);

                            // datalink scaling 0.125 rpm/bit
                            if (temp >= INT16_MAX) {
                                LpsSaWeighInfoTbl.EngineSpeedRPM = INT16_MAX * 0.125;
                            }
                            else {
                                LpsSaWeighInfoTbl.EngineSpeedRPM = (int16_t)temp * 0.125;
                            }
                        }
                        else {
                            LpsSaWeighInfoTbl.EngineSpeedRPM = UNKNOWN2S + FMICNM;

                            // Uncomment for testing only to fake out Engine speed
                            //LpsSaWeighInfoTbl.EngineSpeedRPM = INT16_MAX * 0.125;
                        }
                        break;
                    }

                    default: {
                        break;
                    }
                    } /* switch (dlParam.param_id)*/
                    break;
                } /* case (weigh_app_interfaces::msg::DataLinkParam::IDENTIFIER_TYPE_PUBLIC_PGN)*/

                default: {
                    break;
                }
                } /* switch (dlParam.identifier_type */
            } /* if (DIAG_INACTIVE == LpsSaWeighInfoTbl.DiagState.flag.MachineModelNotSet */
        } /* for each DataLinkParam */
    } /* while (DataLinkDataInput_->get(dlData))*/

    // measure cycle rate and print out rate every 100 cycles
    static int cn = 0;
    cn++;
    static TimeStamp ts;
    TimeStamp last = ts;
    ts= localhostNow();
    double cr = (ts - last).toDouble();

    if ( cn % 5 == 1)
    {
        AIS_LOG_INFO("cycle rate is %lf hertz", 1.0 / cr );
    }
}

/******************************************************************************
FUNCTION NAME:cleanup
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void  LpsSaWeighApp::cleanup( )
{
    AIS_LOG_INFO("LpsSaWeighApp::cleanup");

    // If oel hasn't booted up, then writing the nvm won't work.
    if (FALSE == OelBootupFlag) {
        return;
    }

    // I think that this is to wait for the robot file to be updated.
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::ifstream machineModelFp("/opt/appdata/CPM/etc/robot");
    std::string newMachineMSN;

    if (machineModelFp.good())
    {
        machineModelFp >> newMachineMSN;
        if ((machineMSN.compare(newMachineMSN) != 0) && !(newMachineMSN == "CAT99999") && !(machineMSN == "CAT99999"))
        {
            { /* Reset Lift Linkage Cals */
                liftCalNvmTbl_.remove();
                liftCalNvmTbl_.clearSaveNeeded();
            }

            { /* Reset Tilt Linkage Cals */
                tiltCalNvmTbl_.remove();
                tiltCalNvmTbl_.clearSaveNeeded();
            }

            { /* Reset Payload Cals */
                payloadCalNvmTbl_.remove();
                payloadCalNvmTbl_.clearSaveNeeded();
            }

            { /* Reset the config in nvm */
                cnfg_.reset();
            }
        }
    }

    AIS_LOG_NOTICE("cleanup nvm_block_write complete");
}

void LpsSaWeighApp::UpdateEvents( void )
{
    { // Overload Event
        LpsWeighOverloadLevel_t level = LpsWeighOverloadWarningLevel();
        bool eventActive = false;

        if (LPS_WEIGH_OVERLOAD_LEVEL_WARNING == level) {
            if (cnfg_.overloadWarningEnabled) {
                eventActive = true;
            }
        }
        else if (LPS_WEIGH_OVERLOAD_LEVEL_SEVERE == level) {
            eventActive = true;
        }

        if ((eventActive) && (!LpsSaWeighInfoTbl.EventState[ACDEventPopUp::OVERLOAD_LIMIT_EXCEEDED])) {
            if (cnfg_.overloadWarningEnabled) {
                setAutonomyCondition(PayLdOverLdLimitExceeded());
            }
            LpsSaWeighInfoTbl.EventState[ACDEventPopUp::OVERLOAD_LIMIT_EXCEEDED] = true;
            AIS_LOG_NOTICE("%s activated", PayLdOverLdLimitExceeded::getConditionTypeString().c_str());
        }
        else if ((!eventActive) && (LpsSaWeighInfoTbl.EventState[ACDEventPopUp::OVERLOAD_LIMIT_EXCEEDED])) {
            clearAutonomyCondition<PayLdOverLdLimitExceeded>();
            LpsSaWeighInfoTbl.EventState[ACDEventPopUp::OVERLOAD_LIMIT_EXCEEDED] = false;
            AIS_LOG_NOTICE("%s deactivated", PayLdOverLdLimitExceeded::getConditionTypeString().c_str());
        }
        else {
            // Do nothing
        }
    }

    // If payload legal for trade is installed, but not sealed, activate the event, otherwise deactivate the event.
    if ((payloadCalNvmTbl_.legalForTradeInstalled) && (!sealTracker_.isSealed())) {
        if (!LpsSaWeighInfoTbl.EventState[ACDEventPopUp::PAYLOAD_LFT_NOT_SEALED]) {
            setAutonomyCondition(PayloadLegalForTradeNotSealed());
            LpsSaWeighInfoTbl.EventState[ACDEventPopUp::PAYLOAD_LFT_NOT_SEALED] = true;
            AIS_LOG_NOTICE("%s activated", PayloadLegalForTradeNotSealed::getConditionTypeString().c_str());
        }
    }
    else {
        if (LpsSaWeighInfoTbl.EventState[ACDEventPopUp::PAYLOAD_LFT_NOT_SEALED]) {
            clearAutonomyCondition<PayloadLegalForTradeNotSealed>();
            LpsSaWeighInfoTbl.EventState[ACDEventPopUp::PAYLOAD_LFT_NOT_SEALED] = false;
            AIS_LOG_NOTICE("%s deactivated", PayloadLegalForTradeNotSealed::getConditionTypeString().c_str());
        }
    }
}

bool LpsSaWeighApp::GetPayloadMonSysCalStatus(void) {
    return GetLiftCylCalStatus() && GetTiltCylCalStatus() && GetPayloadCalStatus();
}

bool LpsSaWeighApp::GetPayloadCalStatus(void) {
    static constexpr unsigned_16 bitsToCheck = (CAL_EMPTY_BKT_CURVE_MASK | CAL_FULL_BKT_CURVE_MASK | CAL_BKT_WT_MASK);
    bool calibrated = (bitsToCheck == (bitsToCheck & payloadCalNvmTbl_.data.CalStatus));

    // If legal for trade (LFT) is installed, we require valid IMU calibration data to be fully calibrated.
    if ((calibrated) &&
            (payloadCalNvmTbl_.legalForTradeInstalled) &&
            (!payloadCalNvmTbl_.isIMUDataValid())) {
        calibrated = false;
    }

    return calibrated;
}

bool LpsSaWeighApp::GetCalWtRequired(void) {
    if (LpsCalCalInProgress()) {
        // We are in the middle of a calibration.
        return false;
    }
    else if (!GetPayloadCalStatus()) {
        // Payload is not fully calibrated
        if ((payloadCalNvmTbl_.data.CalStatus & CAL_BKT_WT_MASK) != CAL_BKT_WT_MASK) {
            // We have not entered the calibration weight.
            static constexpr unsigned_16 bitsToCheck = (CAL_EMPTY_BKT_CURVE_MASK | CAL_FULL_BKT_CURVE_MASK | CAL_VELCAL);
            if ((payloadCalNvmTbl_.data.CalStatus & bitsToCheck) == bitsToCheck) {
                // All other parts of the calibration are done, we need a calibration weight.
                return true;
            }
            else {
                // Wait until we finish the other parts of the calibration.
                return false;
            }
        }
    }

    return false;
}

LpsSaWeighTxChannel::AudibleAnnunciationPriority_t LpsSaWeighApp::getAudibleCommand(void) {
    LpsSaWeighTxChannel::AudibleAnnunciationPriority_t tone = audibleTriggered_;
    LpsWeighRangeIndicator_t indicator = LpsWeighGetWeighRangeIndicator();

    /* if audible feature is enabled and not in standby and cal is not in progress, check and set tones */
    if ((cnfg_.audibleWeightEnabled) &&
            (LPS_SA_JOB_MGR_STANDBY_DEACTIVATED == LpsSaWeighInfoTbl.StandbyState) &&
            (!LpsCalCalInProgress()) &&
            (!LinkageCalInProgress)) {

        if (LPS_IN_WEIGH_RANGE_WEIGHING == indicator) {
            /*
             * We are in the weigh range, reset.
             * This is to ensure we get a new confirmation tone again if we get a new GBW.
             */
            tone = LpsSaWeighTxChannel::TONE_NONE;
        }
        else if ((LPS_IN_WEIGH_RANGE_WEIGHING == prevWeighRangeIndicator_) &&
                (LPS_ABOVE_WEIGH_RANGE == indicator)) {
            /*
             * We got a new weigh range weight.
             *  - If green box (high accuracy), set confirmation tone.
             *  - If reweigh warnings, set attention tone.
             */
            if (LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_HIGH == GET_ACCURACY(LpsSaWeighInfoTbl.PayloadCalcMeth)) {
                tone = LpsSaWeighTxChannel::TONE_CONFIRMATION;
            }
            else {
                tone = LpsSaWeighTxChannel::TONE_ATTENTION;
            }

            audibleTriggerTimepoint_ = std::chrono::steady_clock::now();
        }
        else if (LpsSaWeighTxChannel::TONE_NONE != tone) {
            /*
             * Check if any tone has been triggered for more than 1 second.
             * We want to notify of any audible for only 1 second before it expires.
             */
            auto time_elapsed = std::chrono::steady_clock::now() - audibleTriggerTimepoint_;
            auto seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(time_elapsed).count();
            if (seconds_elapsed >= 1) {
                tone = LpsSaWeighTxChannel::TONE_NONE;
            }
        }
        else {
            // no change
        }
    }
    else {
        // no audible weights if in standby or cal in progress
        tone = LpsSaWeighTxChannel::TONE_NONE;
    }

    audibleTriggered_ = tone;
    prevWeighRangeIndicator_ = indicator;

    return tone;
}
