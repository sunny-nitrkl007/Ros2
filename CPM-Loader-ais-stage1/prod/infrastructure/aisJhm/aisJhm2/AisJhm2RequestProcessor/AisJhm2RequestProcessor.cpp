///////////////////////////////////////////////////////////////////////////////
/// @file      AisJhm2RequestProcessor.cpp
/// @author    strubjc
/// @date      Aug 15, 2013
/// @brief     
///
/// @attention COPYRIGHT (C) 2012 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <utility>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iterator>
#include <cmath>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>

#include <ais/config/ConfigSection.h>
#include <ais/log/Logger.h>
#include <ais/time/ConvertTime.h>
#include <ais/util/stringUtils.h>
#include <jhm2/Jhm2Param/Jhm2SimpleParam.h>
#include <jhm2/Jhm2Param/XmlTypeConverters.h>
#include <nvm.h>
#include <scl_prmsw.h>
#include <scl_dcli_svc_types.h>

// Rapid JSON
#include <ext/rapidjson/RapidJsonGuarded.h>

#include <chrono/print.hpp>
#include <chrono/tz.hpp>
#include <fileio/iflocker.hpp>
#include <fileio/oflocker.hpp>

#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighReqstChannel.h>
#include <interfaces/LpsSaJobMgrReqstChannel/LpsSaJobMgrReqstChannel.h>
#include <interfaces/LpsSaListMgr/TxInterface.hpp>
#include <interfaces/LpsSaListMgr/PostInterface.hpp>
#include <interfaces/LpsSaTotals/RequestInterface.hpp>
#include <interfaces/LpsSaTotals/ResponseInterface.hpp>
#include <interfaces/LpsSaUI/DisplayStateInterface.hpp>
#include <interfaces/LpsSaUI/DisplaySettingsInterface.hpp>
#include <interfaces/LpsSaUI/BEMSimStateInterface.hpp>
#include <interfaces/ShmClock/ShmClock.h>
#include <interfaces/LpsSaWeighReqstChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighRespChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighTxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrRespChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighTxChannel/LpsSaWeighTxChannel.h>
#include <interfaces/DataLinkData/SecuritySystemCurrentSecurityIDParam.h>
#include <interfaces/DataLinkData/DataLinkParam.h>
#include <interfaces/SerialPrinter/RequestInterface.hpp>
#include <interfaces/LpsSaJobMgrTxChannel/LpsSaJobMgrTxChannel.h>
#include <interfaces/LpsSaTotals/PrinterCnfg.hpp>
#include <interfaces/AutonomyConditionDiagnostics/RequestInterface.hpp>
#include <interfaces/AutonomyConditionDiagnostics/ResponseInterface.hpp>
#include <interfaces/LpsSaLoadRecordChannel/InterfaceCommon.h>

#include <LoaderdiagnosticEventConfig/Loader_autonomy_diagnostics_config.h>
#include <LoaderdiagnosticEventConfig/Loader_autonomy_event_config.h>

#include <lps_sea_defs.h>

#include "AisJhm2RequestProcessor.h"
#include "AisJhm2MultiParam.h"
#include "AisJhm2FloatParam.h"
#include "AisJhm2BoolParam.h"
#include "AisJhm2UintParam.h"
#include "AisJhm2FileMsg.h"
#include "machineModelList.h"
#include "BEMSim/BEMSimCnfgMgr.h"

#define LAST(k,n) ((k) & ((1<<(n))-1))
#define MID(k,m,n) LAST((k)>>(m),((n)-(m)))

#define TRUCK_LIST_FILENAME_JSON (R"(TruckList.json)")
#define MATERIAL_LIST_FILENAME_JSON (R"(MaterialList.json)")
#define CUSTOM_LIST1_FILENAME_JSON (R"(CustomerList1.json)")
#define CUSTOM_LIST2_FILENAME_JSON (R"(CustomerList2.json)")
#define CUSTOM_LIST3_FILENAME_JSON (R"(CustomerList3.json)")
#define CUSTOM_LIST4_FILENAME_JSON (R"(CustomerList4.json)")
#define DISPLAY_SETTINGS_FILENAME_BIN (R"(DisplaySettings.bin)")
#define TARGET_WEIGHTS_FILENAME_BIN (R"(TargetWeights.bin)")
#define TRUCK_SHORTLIST_FILENAME_BIN (R"(TruckShortList.bin)")
#define MATERIAL_SHORTLIST_FILENAME_BIN (R"(MaterialShortList.bin)")
#define TAG1_SHORTLIST_FILENAME_BIN (R"(Tag1ShortList.bin)")
#define TAG2_SHORTLIST_FILENAME_BIN (R"(Tag2ShortList.bin)")
#define TAG3_SHORTLIST_FILENAME_BIN (R"(Tag3ShortList.bin)")
#define TAG4_SHORTLIST_FILENAME_BIN (R"(Tag4ShortList.bin)")

#define DEFAULT_STORAGE_ROOT (R"(/tmp/AisJhm2DataServer/storage)")
#define DEFAULT_TEMP_ROOT (R"(/tmp/AisJhm2DataServer/temp)")

static const std::string BEMSimFileNameAr = R"(BEMSimCnfg.bin)";

#define DETAILED_DIAGNOSTICS_FILEPATH_JSON \
    (R"(/tmp/appdata/CPM/AutonomyConditionDiagnostics/DetailedDiagnostics.json)")

#define ECM_SUMMARY_FILEPATH_JSON \
    (R"(/tmp/appdata/CPM/AutonomyConditionDiagnostics/AnalysisModuleECMSummary.json)")

#define DISPLAY_SERVICEDASHBOARD_FILEPATH_XML \
    (R"(/tmp/appdata/CPM/AutonomyConditionDiagnostics/AnalysisDisplayServiceDashboard.xml)")

#define TELEMATICS_SERVICEDASHBOARD_FILEPATH_XML \
    (R"(/tmp/appdata/CPM/AutonomyConditionDiagnostics/ProductLinkServiceDashboard.xml)")

namespace rj = rapidjson;
namespace fs = boost::filesystem;

std::string MM_Selected_Make_Rd = "";
std::string MM_Selected_Machine_Rd = "";
std::string MM_Selected_Linkage_Rd = "";
std::string machineMSN = "";

static uint_least8_t getBucketWeightDisplayAccuracy(unsigned short int payloadCalcMethod, bool displayWeightOk);

 //=============================================================================
AisJhm2RequestProcessor::AisJhm2RequestProcessor() :
        bemSimDerateSetEvent(false),
        bemSimDerateClrEvent(false),
        bemSimWarningSetEvent(false),
        bemSimWarningClrEvent(false),
        updateRate_(10.0f),
        storageRoot_(DEFAULT_STORAGE_ROOT),
        tempRoot_(DEFAULT_TEMP_ROOT),
        publishDisplayState_(true),
        displaySettingsChanged_(false),
        showHideReady_(false),
        displayBrightnessSupported_(false),
        payloadRecipesSupported_(false),
        displayStatePublishCounter_(0),
        displayState_(),
        targetWeights_(),
        materialFreqList_(),
        truckFreqList_(),
        custom1FreqList_(),
        custom2FreqList_(),
        custom3FreqList_(),
        custom4FreqList_(),
        prevStoreCount_(0),
        prevSubtotalCount_(1),
        displayStateOutputChannel_(nullptr),
        displaySettingsInputChannel_(nullptr),
        shmClockInput_(nullptr),
        tzInfo_{0, -1},
        dataLinkDataInput_(nullptr),
        dataLinkDataOutput_(nullptr),
        dlData_(),
        m_scsInputs(),
        m_scsOutputs(),
        calUIClientInf_(),
        seaStatusLevel1_{0, LPS_SEA_REASON_CODE_149, SCL_PRMSW_FEAT_STATUS_NONE},
        seaStatusLevel2_{0, LPS_SEA_REASON_CODE_245, SCL_PRMSW_FEAT_STATUS_NONE},
        seaStatusLegalForTrade_{0, LPS_SEA_LFT_REASON_CODE_312, SCL_PRMSW_FEAT_STATUS_NONE},
        legalForTradeInstalled_(false),
        legalForTradeInstalledStatusReceived_(false),
        partNumbers_(),
        partInputChannel_(nullptr),
        totalsReceivedTime_(TimePoint::min()),
        totals_(),
        totalsInputChannel_(nullptr),
        lpsSaLoadRecordChannelInput_(nullptr),
        totalsRequestHelper_(),
        printerCnfgInput_(nullptr),
        writePrinterCnfgOutput_(nullptr),
        printerRequestChannelOutput_(nullptr),
        aisJhm2TxOutput_(nullptr),
        DemoAppRequestOutput_(nullptr),
        systemRestartRequestOutput_(nullptr),
        printerCnfgReceivedTime_(TimePoint::min()),
        printerCnfg_(),
        dispatchSettingsInput_(nullptr),
        USBSaveProgressInput_(nullptr),
        detailedDiagnosticsFilePath_(DETAILED_DIAGNOSTICS_FILEPATH_JSON),
        ecmSummaryFilePath_(ECM_SUMMARY_FILEPATH_JSON),
        displayServiceDashboardFilePath_(DISPLAY_SERVICEDASHBOARD_FILEPATH_XML),
        telematicsServiceDashboardFilePath_(TELEMATICS_SERVICEDASHBOARD_FILEPATH_XML),
        paramMsgRequestHandlerMap_(),
        cmdMsgRequestHandlerMap_(),
        fileMsgRequestHandlerMap_(),
        operatorIdCsv_(true),
        operatorIdListMgr_(),
        jobMgrTxData_(),
        jobMgrHelper_(),
        autonomyConditionDiagnosticsTxData_(),
        autonomyConditionDiagnosticsHelper_(),
        weighAppTxData_(),
        weighAppInf_(),
        listMgrTxReceived_(false),
        listMgrTxData_(),
        listMgrRequestHelper_(),
        can11bitProcess_(),
        keypad_(),
        machineConfig_(),
        totalFuel_(),
        bemSimMgr(),
        demo_mode_exit_time(std::chrono::steady_clock::time_point::min()),
        showHideConfig_(),
        infoState_(),
        diagState_(),
        eventState_(),
        bemSimCnfgChanged_(false),
        bemSimStateOutputChannel_(nullptr)
{
}

//=============================================================================
AisJhm2RequestProcessor::~AisJhm2RequestProcessor()
{
  // TODO Auto-generated destructor stub
}

/******************************************************************************
FUNCTION:                   configure
DESCRIPTION:                Initialize to configure the SCS channels
PARAMETER DESCRIPTION:
RETURN VALUE:               boolean  
*******************************************************************************/
bool AisJhm2RequestProcessor::configure( TaskParser& tp )
{
    bool everythingOk = true;

    paramMsgRequestHandlerMap_.clear();
    paramMsgRequestHandlerMap_["Heartbeat"] = &AisJhm2RequestProcessor::processParamRequestHeartbeat;
    paramMsgRequestHandlerMap_["PayloadInfo"] = &AisJhm2RequestProcessor::processParamRequestPayloadInfo;
    paramMsgRequestHandlerMap_["TargetWeight"] = &AisJhm2RequestProcessor::processParamRequestRemainingWt;
    paramMsgRequestHandlerMap_["TargetWeights"] = &AisJhm2RequestProcessor::processParamRequestTargetWeights;
    paramMsgRequestHandlerMap_["MaterialInfo"] = &AisJhm2RequestProcessor::processParamRequestMaterialInfo;
    paramMsgRequestHandlerMap_["TruckInfo"] = &AisJhm2RequestProcessor::processParamRequestTruckInfo;
    paramMsgRequestHandlerMap_["PrinterCnfg"] = &AisJhm2RequestProcessor::processParamRequestPrinterCnfg;
    paramMsgRequestHandlerMap_["Units_Rd"] = &AisJhm2RequestProcessor::processParamRequestDisplaySettings;
    paramMsgRequestHandlerMap_["Language_Rd"] = &AisJhm2RequestProcessor::processParamRequestDisplaySettings;
    paramMsgRequestHandlerMap_["DisplayBrightness_Rd"] = &AisJhm2RequestProcessor::processParamRequestDisplaySettings;
    paramMsgRequestHandlerMap_["DisplaySettings"] = &AisJhm2RequestProcessor::processParamRequestDisplaySettings;
    paramMsgRequestHandlerMap_["ServiceParameters"] = &AisJhm2RequestProcessor::processParamRequestServiceParameters;
    paramMsgRequestHandlerMap_["KeyboardLayoutSetting"] = &AisJhm2RequestProcessor::processParamRequestKeyboardLayoutSetting;
    paramMsgRequestHandlerMap_["PayloadSettings"] = &AisJhm2RequestProcessor::processParamRequestPayloadSettings;
    paramMsgRequestHandlerMap_["ServiceMode_Rd"] = &AisJhm2RequestProcessor::processParamRequestServiceMode;
    paramMsgRequestHandlerMap_["ServicePassword_Rd"] = &AisJhm2RequestProcessor::processParamRequestServiceMode;
    paramMsgRequestHandlerMap_["LocalTime"] = &AisJhm2RequestProcessor::processParamRequestLocalTime;
    paramMsgRequestHandlerMap_["CalibrationData"] = &AisJhm2RequestProcessor::processParamRequestCalibrationData;
    paramMsgRequestHandlerMap_["OperatorIdCnfg_Rd"] = &AisJhm2RequestProcessor::processParamRequestOperatorId;
    paramMsgRequestHandlerMap_["MachImuPitchOffset"] = &AisJhm2RequestProcessor::processParamRequestMachImuPitchOffset;
    paramMsgRequestHandlerMap_["CustomListSettings"] = &AisJhm2RequestProcessor::processParamRequestCustomListSettings;
    paramMsgRequestHandlerMap_["TipoffWeightAdjustData"] = &AisJhm2RequestProcessor::processParamRequestTipoffWeightAdjustData;
    paramMsgRequestHandlerMap_["CustomInfo"] = &AisJhm2RequestProcessor::processParamRequestCustomInfo;
    paramMsgRequestHandlerMap_["Demo_App_Inputs_Rd"] = &AisJhm2RequestProcessor::processParamRequestDemoAppInputs;
    paramMsgRequestHandlerMap_["UDP_Transfer_Details"] = &AisJhm2RequestProcessor::processParamRequestUDPTransferDetails;
    paramMsgRequestHandlerMap_["PayloadLevel2TempInstallCnfg"] = &AisJhm2RequestProcessor::processParamRequestPayloadLevel2TempInstallCnfg;
    paramMsgRequestHandlerMap_["BEMSimCnfg"] = &AisJhm2RequestProcessor::processParamRequestBEMSimCnfg;
    paramMsgRequestHandlerMap_["BEMSimInfo"] = &AisJhm2RequestProcessor::processParamRequestBEMSimInfo;
    paramMsgRequestHandlerMap_["USBSaveProgress"] = &AisJhm2RequestProcessor::processParamRequestUSBSaveProgress;
    paramMsgRequestHandlerMap_["IMUData"] = &AisJhm2RequestProcessor::processParamRequestIMUData;
    paramMsgRequestHandlerMap_["TestStatus"] = &AisJhm2RequestProcessor::processParamRequestTestStatus;

    cmdMsgRequestHandlerMap_.clear();
    cmdMsgRequestHandlerMap_["TargetWeightEntry"] = &AisJhm2RequestProcessor::processCmdRequestTargetWtWr;
    cmdMsgRequestHandlerMap_["MaterialInfo_Wr"] = &AisJhm2RequestProcessor::processCmdRequestMaterialInfoWr;
    cmdMsgRequestHandlerMap_["TruckInfo_Wr"] = &AisJhm2RequestProcessor::processCmdRequestTruckInfoWr;
    cmdMsgRequestHandlerMap_["MaterialList"] = &AisJhm2RequestProcessor::processCmdRequestListWr;
    cmdMsgRequestHandlerMap_["TruckList"] = &AisJhm2RequestProcessor::processCmdRequestListWr;
    cmdMsgRequestHandlerMap_["CustomList1"] = &AisJhm2RequestProcessor::processCmdRequestListWr;
    cmdMsgRequestHandlerMap_["CustomList2"] = &AisJhm2RequestProcessor::processCmdRequestListWr;
    cmdMsgRequestHandlerMap_["CustomList3"] = &AisJhm2RequestProcessor::processCmdRequestListWr;
    cmdMsgRequestHandlerMap_["CustomList4"] = &AisJhm2RequestProcessor::processCmdRequestListWr;
    cmdMsgRequestHandlerMap_["PayloadNextSubtotal"] = &AisJhm2RequestProcessor::processCmdRequestPayloadNextSubtotal;
    cmdMsgRequestHandlerMap_["PrinterCnfg_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPrinterCnfgWr;
    cmdMsgRequestHandlerMap_["Units_Wr"] = &AisJhm2RequestProcessor::processCmdRequestDisplaySettingsWr;
    cmdMsgRequestHandlerMap_["Language_Wr"] = &AisJhm2RequestProcessor::processCmdRequestDisplaySettingsWr;
    cmdMsgRequestHandlerMap_["DisplayBrightness_Wr"] = &AisJhm2RequestProcessor::processCmdRequestDisplaySettingsWr;
    cmdMsgRequestHandlerMap_["DisplaySettings_Wr"] = &AisJhm2RequestProcessor::processCmdRequestDisplaySettingsWr;
    cmdMsgRequestHandlerMap_["KeyboardLayoutSetting_Wr"] = &AisJhm2RequestProcessor::processParamRequestKeyboardLayoutSettingWr;
    cmdMsgRequestHandlerMap_["PayloadSettings_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPayloadSettingsWr;
    cmdMsgRequestHandlerMap_["ServiceMode_Wr"] = &AisJhm2RequestProcessor::processCmdRequestServiceModeWr;
    cmdMsgRequestHandlerMap_["ServicePassword_Wr"] = &AisJhm2RequestProcessor::processCmdRequestServiceModeWr;
    cmdMsgRequestHandlerMap_["OperatorIdCnfg_Wr"] = &AisJhm2RequestProcessor::processCmdRequestOperatorIdWr;
    cmdMsgRequestHandlerMap_["MachImuPitchOffset"] = &AisJhm2RequestProcessor::processCmdRequestMachImuPitchOffset;
    cmdMsgRequestHandlerMap_["SelectTask"] = &AisJhm2RequestProcessor::processCmdRequestSelectTask;
    cmdMsgRequestHandlerMap_["TipoffWeightAdjustData_Wr"] = &AisJhm2RequestProcessor::processCmdRequestTipoffWeightAdjustData;
    cmdMsgRequestHandlerMap_["TipoffWeightAdjustData_Reset"] = &AisJhm2RequestProcessor::processCmdRequestTipoffWeightAdjustReset;
    cmdMsgRequestHandlerMap_["CustomListSettings_Wr"] = &AisJhm2RequestProcessor::processCmdRequestCustomListSettingsWr;
    cmdMsgRequestHandlerMap_["CustomInfo_Wr"] = &AisJhm2RequestProcessor::processCmdRequestCustomInfoWr;
    cmdMsgRequestHandlerMap_["Demo_App_Inputs_Wr"] = &AisJhm2RequestProcessor::processCmdRequestDemoAppInputs;
    cmdMsgRequestHandlerMap_["UDP_Transfer_Details_Wr"] = &AisJhm2RequestProcessor::processCmdRequestUDPTransferDetails;
    cmdMsgRequestHandlerMap_["PayloadLevel2TempInstallCnfg_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPayloadLevel2TempInstallCnfgWr;
    cmdMsgRequestHandlerMap_["BEMSimCmd_Wr"] = &AisJhm2RequestProcessor::processCmdRequestBEMSimCmdWr;
    cmdMsgRequestHandlerMap_["BEMSimCnfg_Wr"] = &AisJhm2RequestProcessor::processCmdRequestBEMSimCnfgWr;
    cmdMsgRequestHandlerMap_["USBSave"] = &AisJhm2RequestProcessor::processCmdRequestUSBSaveWr;
    cmdMsgRequestHandlerMap_["WeighConfiguration_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPayloadServiceDashboard;
    cmdMsgRequestHandlerMap_["WeighCalibration_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPayloadServiceDashboard;
    cmdMsgRequestHandlerMap_["TestRun"] = &AisJhm2RequestProcessor::processCmdRequestTest;
    cmdMsgRequestHandlerMap_["TestRecord"] = &AisJhm2RequestProcessor::processCmdRequestTest;
    
    fileMsgRequestHandlerMap_.clear();
    fileMsgRequestHandlerMap_["TruckList"] = &AisJhm2RequestProcessor::processFileRequestList;
    fileMsgRequestHandlerMap_["MaterialList"] = &AisJhm2RequestProcessor::processFileRequestList;
    fileMsgRequestHandlerMap_["CustomList1"] = &AisJhm2RequestProcessor::processFileRequestList;
    fileMsgRequestHandlerMap_["CustomList2"] = &AisJhm2RequestProcessor::processFileRequestList;
    fileMsgRequestHandlerMap_["CustomList3"] = &AisJhm2RequestProcessor::processFileRequestList;
    fileMsgRequestHandlerMap_["CustomList4"] = &AisJhm2RequestProcessor::processFileRequestList;
    fileMsgRequestHandlerMap_["DetailedDiagnostics"] = &AisJhm2RequestProcessor::processFileRequestDetailedDiagnostics;
    fileMsgRequestHandlerMap_["ECMSummary"] = &AisJhm2RequestProcessor::processFileRequestECMSummary;
    fileMsgRequestHandlerMap_["DisplayServiceDashboard"] = &AisJhm2RequestProcessor::processFileRequestDisplayServiceDashboard;
    fileMsgRequestHandlerMap_["TelematicsServiceDashboard"] = &AisJhm2RequestProcessor::processFileRequestTelematicsServiceDashboard;
    fileMsgRequestHandlerMap_["UI_SHOW_FEATURE_CONFIG"] = &AisJhm2RequestProcessor::processFileRequestUIShowFeatureConfig;
    fileMsgRequestHandlerMap_["ServiceHistory"] = &AisJhm2RequestProcessor::processFileRequestServiceHistory;
    fileMsgRequestHandlerMap_["LiftSensorCalibration"] = &AisJhm2RequestProcessor::processFileRequestPayloadServiceDashboard;
    fileMsgRequestHandlerMap_["TiltSensorCalibration"] = &AisJhm2RequestProcessor::processFileRequestPayloadServiceDashboard;
    fileMsgRequestHandlerMap_["WeighCalibration"] = &AisJhm2RequestProcessor::processFileRequestPayloadServiceDashboard;
    fileMsgRequestHandlerMap_["WeighConfiguration"] = &AisJhm2RequestProcessor::processFileRequestPayloadServiceDashboard;
    fileMsgRequestHandlerMap_["RecentWeighResults"] = &AisJhm2RequestProcessor::processFileRequestPayloadServiceDashboard;

    // Install other handlers
    installTotalsHandlers();
    installTicketHandlers();
    installLFTHandlers();
    installWorkToolHandlers();
    installRecipeHandlers();

    { // Get task configs
        ConfigSection cs, machineType;
        if (tp.getTaskParameters(cs)) {
            std::string storageRoot;
            if (cs.get("storageRoot", storageRoot)) {
                storageRoot_ = storageRoot;
            }
            else {
                storageRoot_ = DEFAULT_STORAGE_ROOT;
            }

            std::string tempRoot;
            if (cs.get("tempRoot", tempRoot)) {
                tempRoot_ = tempRoot;
            }
            else {
                tempRoot_ = DEFAULT_TEMP_ROOT;
            }

            std::string msnLinkageMappingAU2020Path;
            if (cs.get("msnLinkageMappingAU2020Path", msnLinkageMappingAU2020Path))
            {
                msnLinkageMappingAU2020Path_ = msnLinkageMappingAU2020Path;
            }
            else
            {
                AIS_LOG_WARN("msnLinkageMappingAU2020Path NOT FOUND");
            }

            double updateRate;
            if (cs.get("cycleRate_hz", updateRate)) {
                updateRate_ = static_cast<float>(updateRate);
            }
            else {
                updateRate_ = 10.0f;
            }

            bemSimMgr.setUpdateInfoPeriod(updateRate_);

        }
        else {
            storageRoot_ = DEFAULT_STORAGE_ROOT;
            tempRoot_ = DEFAULT_TEMP_ROOT;
        }


        AIS_LOG_INFO("Storage Root: %s", storageRoot_.c_str());
        AIS_LOG_INFO("Temp Root: %s", tempRoot_.c_str());
        AIS_LOG_INFO("msn Linkage Mapping Path %s", msnLinkageMappingAU2020Path_.c_str());
    }

    { // Configure SCS inputs
        ConfigSection cs;
        if (!tp.getSection("ScsRxTimeouts", cs)) {
            AIS_LOG_FATAL("Couldn't find ScsRxTimeouts Config Section in rb file");
            everythingOk = false;
        }

        if (!m_scsInputs.initReadInterface(cs)) {
            AIS_LOG_FATAL("Couldn't initialize SCS inputs - check config");
            everythingOk = false;
        }

        if (!tp.getSection("ScsTxRates", cs)) {
            AIS_LOG_FATAL("Couldn't find ScsTxRates Config Section in rb file");
            everythingOk = false;
        }

        if(!m_scsOutputs.initPublishInterface(cs)) {
            AIS_LOG_FATAL("Couldn't initialize SCS outputs - check config");
            everythingOk = false;
        }
    }

    // Initialize input and output channels
    lpsSaLoadRecordChannelInput_ = dynamic_cast<LpsSaLoadRecordChannelInputChannel*>(task::InterfaceDb::fetch("LoadRecordInput"));
    if (nullptr == lpsSaLoadRecordChannelInput_) {
        AIS_LOG_ERROR("No load record input channel defined.");
        everythingOk = false;
    }

    // Initialize Ais Jhm2 Tx Output
    if (!task::InterfaceDb::bind("AisJhm2TxChannelOutput", aisJhm2TxOutput_)) {
        AIS_LOG_ERROR("No Ais Jhm2 Tx output channel defined.");
        everythingOk = false;
    }

    // Initialize Demo App Output
    if (!task::InterfaceDb::bind("DemoAppReqstChannelOutput", DemoAppRequestOutput_)) {
        AIS_LOG_ERROR("No Demo App request output channel defined");
        everythingOk = false;
    }

    // Initialize the Calibration UI interface
    if (!calUIClientInf_.start("CalibrationRequestUIOutput", "CalibrationResponseUIInput")) {
        AIS_LOG_ERROR("No Calibration UI interface defined");
        everythingOk = false;
    }

    // Initialize the part numbers input channel
    partNumbers_ = PartNumbers();
    if (!task::InterfaceDb::bind("PartNumbersInput", partInputChannel_)) {
        AIS_LOG_ERROR("No Part Numbers input channel defined.");
        everythingOk = false;
    }

    // Initialize the totals input channel.
    totalsReceivedTime_ = TimePoint::min();
    totalsInputChannel_ = dynamic_cast<LpsSaTotalsInterfaceInputChannel*>(task::InterfaceDb::fetch("TotalsInput"));
    if (nullptr == totalsInputChannel_) {
        AIS_LOG_ERROR("No totals input channel defined.");
        everythingOk = false;
    }

    // Start the totals app request helper
    if (!totalsRequestHelper_.start("AisJhm2", "TotalsRequestOutput", "TotalsResponseInput")) {
        AIS_LOG_ERROR("No totals request helper defined.");
        everythingOk = false;
    }

    // Start the ListMgr app request helper
    listMgrTxReceived_ = false;
    if (!listMgrRequestHelper_.start("AisJhm2", "ListPostOutput", "ListNotifyInput", "ListTxInput")) {
        AIS_LOG_ERROR("No ListMgr request helper defined.");
        everythingOk = false;
    }

    // Start the JobMgr request helper
    if (!jobMgrHelper_.start("AisJhm2", "LpsSaJobMgrReqstChannelOutput", "LpsSaJobMgrRespChannelInput", "LpsSaJobMgrTxChannelInput")) {
        AIS_LOG_ERROR("No JobMgr request helper defined.");
        everythingOk = false;
    }

    // Start the ACD request helper
    if (!autonomyConditionDiagnosticsHelper_.start("AisJhm2", "AutonomyConditionDiagnosticsRequestChannelOutput", "AutonomyConditionDiagnosticsResponseChannelInput", "AutonomyConditionDiagnosticsTxChannelInput")) {
        AIS_LOG_ERROR("No ACD request helper defined.");
        everythingOk = false;
    }

    // Start the WeighApp request helper
    if (!weighAppInf_.start("AisJhm2", "LpsSaWeighReqstChannelOutput", "LpsSaWeighRespChannelInput", "LpsSaWeighTxChannelInput")) {
        AIS_LOG_ERROR("No WeighApp request helper defined.");
        everythingOk = false;
    }

    // Initialize the printer cnfg stuff.
    printerCnfgReceivedTime_ = TimePoint::min();

    printerCnfgInput_ = dynamic_cast<LpsSaTotalsPrinterCnfgInterfaceInputChannel*>(task::InterfaceDb::fetch("PrinterCnfgInput"));
    if (nullptr == printerCnfgInput_) {
        AIS_LOG_ERROR("No printer config input channel defined.");
        everythingOk = false;
    }

    writePrinterCnfgOutput_ = dynamic_cast<LpsSaTotalsPrinterCnfgInterfaceOutputChannel*>(task::InterfaceDb::fetch("WritePrinterCnfgOutput"));
    if (nullptr == writePrinterCnfgOutput_) {
        AIS_LOG_ERROR("No printer config write output channel defined.");
        everythingOk = false;
    }

    if (!task::InterfaceDb::bind("PrinterRequestOutput", printerRequestChannelOutput_)) {
        AIS_LOG_ERROR("No printer request output channel defined.");
        everythingOk = false;
    }

    // Initialize the can11-bit processor object
    can11bitProcess_.init(updateRate_);

    // Initialize the display state and setting interfaces.
    displayStateOutputChannel_ = dynamic_cast<LpsSaUIDisplayStateInterfaceOutputChannel*>(task::InterfaceDb::fetch("DisplayStateOutput"));
    if (nullptr == displayStateOutputChannel_) {
        AIS_LOG_ERROR("No display state output channel defined.");
        everythingOk = false;
    }

    // Initialize the BEM simulator state and setting interfaces.
    bemSimStateOutputChannel_ = dynamic_cast<LpsSaUIBEMSimStateInterfaceOutputChannel*>(task::InterfaceDb::fetch("BEMSimStateOutput"));
    if (nullptr == bemSimStateOutputChannel_) {
        AIS_LOG_ERROR("No display state output channel defined.");
        everythingOk = false;
    }

    displaySettingsInputChannel_ = dynamic_cast<LpsSaUIDisplaySettingsInterfaceInputChannel*>(task::InterfaceDb::fetch("DisplaySettingsInput"));
    if (nullptr == displaySettingsInputChannel_) {
        AIS_LOG_ERROR("No display settings input channel defined.");
        everythingOk = false;
    }

    shmClockInput_ = dynamic_cast<ShmClockInput*>(task::InterfaceDb::fetch("ShmClockInput"));
    if (nullptr == shmClockInput_) {
        AIS_LOG_ERROR("No SHM Clock input channel defined.");
        everythingOk = false;
    }

    // Bind all scs channels, return false if any failures occur
    if (!task::InterfaceDb::bind("DataLinkDataInput", dataLinkDataInput_)) {
        AIS_LOG_FATAL("No DataLinkDataInput channel defined.");
        everythingOk = false;
    }

    if(!task::InterfaceDb::bind("DataLinkDataOutput", dataLinkDataOutput_)) {
        AIS_LOG_FATAL("No DataLinkDataOutput channel defined.");
        everythingOk = false;
    }
    else {
        // Initialize DataLinkData parameters
        DataLinkParam dlParam;
        dlParam.SetDataLinkType(DataLinkParamInfo::DATA_LINK_PARAM_INT);
        dlParam.SetParamId(SecuritySystemCurrentSecurityIDParam::CatExtId);
        dlParam.SetParamIdentifierType(DataLinkParamInfo::DATA_LINK_PARAM_IDENTIFIER_CAT_EXT);
        dlParam.SetParameterType(CDL2_VAR_NO_DSI);
        dlParam.SetSid(0x00);
        dlParam.SetScaling(0);
        dlParam.SetOffset(0);
        if (!dlData_.AddDataLinkParam(dlParam)) {
            AIS_LOG_ERROR("Couldn't add DataLinkParam - check config");
            everythingOk = false;
        }
    }

    systemRestartRequestOutput_ = dynamic_cast<GenericRequestOutput*>( task::InterfaceDb::fetch("SystemRestartRequestOutput"));
    if( NULL == systemRestartRequestOutput_ )
    {
        AIS_LOG_FATAL("No System Restart Request Output Channel defined");
        everythingOk = false;
    }

    if (!task::InterfaceDb::bind("DispatchSettingsInput", dispatchSettingsInput_)) {
        AIS_LOG_ERROR("No DispatchSettingsInput channel defined.");
        everythingOk = false;
    }

    if (!task::InterfaceDb::bind("LpsSaTotalsUSBSaveProgressInput", USBSaveProgressInput_)) {
        AIS_LOG_ERROR("No LpsSaTotalsUSBSaveProgressInput channel defined.");
        everythingOk = false;
    }

    // Create storage and temp directory and load stuff from storage.
    try {
        fs::create_directories(storageRoot_);
        fs::create_directories(tempRoot_);

        // Load display settings from storage
        if (!displayState_.loadSettings(makeStoragePath(DISPLAY_SETTINGS_FILENAME_BIN))) {
            displayState_.reset();
        }

        // Load frequent lists from storage
        if (!targetWeights_.loadTargetWeights(makeStoragePath(TARGET_WEIGHTS_FILENAME_BIN))) {
            targetWeights_.reset();
        }
        if (!truckFreqList_.loadList(makeStoragePath(TRUCK_SHORTLIST_FILENAME_BIN))) {
            truckFreqList_.reset();
        }
        if (!materialFreqList_.loadList(makeStoragePath(MATERIAL_SHORTLIST_FILENAME_BIN))) {
            materialFreqList_.reset();
        }
        if (!custom1FreqList_.loadList(makeStoragePath(TAG1_SHORTLIST_FILENAME_BIN))) {
            custom1FreqList_.reset();
        }
        if (!custom2FreqList_.loadList(makeStoragePath(TAG2_SHORTLIST_FILENAME_BIN))) {
            custom2FreqList_.reset();
        }
        if (!custom3FreqList_.loadList(makeStoragePath(TAG3_SHORTLIST_FILENAME_BIN))) {
            custom3FreqList_.reset();
        }
        if (!custom4FreqList_.loadList(makeStoragePath(TAG4_SHORTLIST_FILENAME_BIN))) {
            custom4FreqList_.reset();
        }

        bemSimMgr.cnfg.load(makeStoragePath(BEMSimFileNameAr));
    }
    catch (const fs::filesystem_error& e) {
        AIS_LOG_ERROR(e.what());
    }

    { // Read machine specific configuration
        ConfigSection cs;
        if (tp.getSection("MachineSpecificConfig", cs)) {
            // Load the UI show/hide configuration.
            loadShowHideConfig(cs);

            // Set the scale weight capacity based on machine configuration.
            if (!displayState_.setWeightCapacity(cs)) {
                AIS_LOG_ERROR("Failed to set weight capacity");
                everythingOk = false;
            }

            // Configure BEMSim init config MachineSpecificConfig
            double bat_cap = 0.0f;
            if(!cs.get("BEMSimBatteryCapacity",bat_cap)) {
                AIS_LOG_INFO("Couldn't find BEMSim Config Battery Cap Section in rb file");
            }

            double k_sfc = 0.0f;
            if(!cs.get("BEMSimKsfc",k_sfc)) {
                AIS_LOG_INFO("Couldn't find BEMSim Config K_sfc Section in rb file");
            }

            std::string battery_model_str ("linear");
            if(!cs.get("BEMSimBatteryModel", battery_model_str)) {
                AIS_LOG_INFO("Couldn't find BEMSim Config BEMSimBatteryModel Section in rb file");
            }

            bemSimMgr.setConstInit(bat_cap, k_sfc, battery_model_str);
        }
        else {
            AIS_LOG_ERROR("MachineSpecificConfig section not found");
        }
    }

    // Initialize the LFT Descriptive Markings
    showHideConfig_.set("PayloadLFT.description", "******");

    // Initialize Member Variables
    publishDisplayState_ = true;
    displaySettingsChanged_ = false;
    bemSimCnfgChanged_ = false;
    displayStatePublishCounter_ = 0;
    tzInfo_.offset = 0;
    tzInfo_.index = -1;

    // Initialize machine config
    machineConfig_ = MachineConfig();

    // Initialize total fuel
    totalFuel_ = TotalFuel();

    // Initialize BEM Simulator config
    bemSimMgr.initialize();

    return everythingOk;
}

/******************************************************************************
FUNCTION:                   loadShowHideConfig
DESCRIPTION:                Load the UI Show Hide config
PARAMETER DESCRIPTION:
RETURN VALUE:               bool
*******************************************************************************/
bool AisJhm2RequestProcessor::loadShowHideConfig(ConfigSection& cs) {
    bool success = true;
    std::string uiConfigJsonFilePath;

    if (cs.get("UI_SHOW_FEATURE_CONFIG_JSON_FILE_PATH", uiConfigJsonFilePath)) {
        success = showHideConfig_.load(uiConfigJsonFilePath);

        if (!success) {
            AIS_LOG_ERROR("Failed to load UI Show/Hide config");
        }

        { // Check if legal for trade is supported
            bool legalForTradeSupported;
            if (cs.get("LegalForTradeSupported", legalForTradeSupported)) {
                showHideConfig_.set("PayloadLFT.supported", legalForTradeSupported);
            }
            else {
                showHideConfig_.set("PayloadLFT.supported", false);
            }
        }

        // Check if flags are set
        operatorIdCsv_ = showHideConfig_.get<bool>("OperatorIdCsv.supported");
        operatorIdListMgr_.setEnabled(operatorIdCsv_);

        // Check if display brightness show is supported
        displayBrightnessSupported_ = showHideConfig_.get<bool>("DisplayBrightness.supported");

        // Check if recipe show is supported
        payloadRecipesSupported_ = showHideConfig_.get<bool>("PayloadRecipes.supported");

        { // Check if hydraulic oil temp is optional
            bool hydOilTempOptional;
            if (cs.get("HydOilTempOptional", hydOilTempOptional)) {
                showHideConfig_.set("HydOilTempEnableConfiguration.supported", hydOilTempOptional);
            }
            else {
                showHideConfig_.set("HydOilTempEnableConfiguration.supported", false);
            }
        }
    }
    else {
        AIS_LOG_ERROR("UI_SHOW_FEATURE_CONFIG_JSON_FILE_PATH not found in MachineSpecificConfig");
        success = false;
    }

    return success;
}

/******************************************************************************
FUNCTION:                   update
DESCRIPTION:                call the SCS INPUTS update function
PARAMETER DESCRIPTION:
RETURN VALUE:               Boolean  
*******************************************************************************/
bool AisJhm2RequestProcessor::update() {

    // Update inputs
    m_scsInputs.update();

    /* Receive local time offset and override the local time offset in chrono/print.hpp */
    if (nullptr != shmClockInput_) {
        ShmClock shmClock;
        while (shmClockInput_->get(shmClock)) {
            int32_t offset = shmClock.get_UTC_offset();
            tzone_tx_comm_struct tzone;
            if (tes_common_ais::get_tz_struct(tzone, shmClock)) {
                if ((tzInfo_.offset != offset) || (tzInfo_.index != tzone.tzone_id)) {
                    std::string tzStr = tes_common_ais::makeTZString(tzone);
                    if (tes_common_ais::setTZString(tzStr)) {
                        AIS_LOG_NOTICE("Set TZ environment variable to '%s'.", tzStr.c_str());
                        tes_common_ais::clearLocalTimeOffsetOverride();
                    }
                    else {
                        AIS_LOG_ERROR("Could not set TZ environment variable to '%s'.", tzStr.c_str());
                        tes_common_ais::setLocalTimeOffsetOverride(std::chrono::minutes(offset));
                    }
                    tzInfo_.offset = offset;
                    tzInfo_.index = tzone.tzone_id;
                }
            }
            else {
                AIS_LOG_ERROR("Could not get tzone_tx_comm_struct");
            }
        }
    }

    // Update Dispatch (Work Order Assist) installation status in the UI show/hide config.
    if (nullptr != dispatchSettingsInput_) {
        WorkOrderAssistSettingsInterface settings;
        while (dispatchSettingsInput_->get(settings)) {
            showHideConfig_.set("WorkOrderAssist.installed", settings.installed && settings.enabled);
        }
    }

    // Get latest ACD app data
    autonomyConditionDiagnosticsHelper_.waitForTxData(autonomyConditionDiagnosticsTxData_);

    { // Read SEA status
        bool statusChanged = false;

        for (const auto& element : autonomyConditionDiagnosticsTxData_.seaList) {
            if (element.reason_code == LPS_SEA_REASON_CODE_149) {
                // update the sea level 1 status object
                if (element.status != seaStatusLevel1_.status) {
                    statusChanged = true;
                }
                seaStatusLevel1_ = element;
            }
            else if (element.reason_code == LPS_SEA_REASON_CODE_245) {
                // update the sea level 2 status object
                if (element.status != seaStatusLevel2_.status) {
                    statusChanged = true;
                }
                seaStatusLevel2_ = element;
            }
            else if (element.reason_code == LPS_SEA_LFT_REASON_CODE_312) {
                if (element.status != seaStatusLegalForTrade_.status) {
                    statusChanged = true;
                }
                seaStatusLegalForTrade_ = element;

                // we are ready to serve up the Show/Hide. The SEAs are published in this predictable order
                //  lvl1, lvl2, lft...
                showHideReady_ = true;

                legalForTradeInstalledStatusReceived_ = true;
            }
        }

        bool level1Installed = autonomyConditionDiagnosticsTxData_.checkSEAEnableStatus(seaStatusLevel1_.status);

        bool level2Installed = level1Installed && autonomyConditionDiagnosticsTxData_.checkSEAEnableStatus(seaStatusLevel2_.status);

        legalForTradeInstalled_ = level2Installed && autonomyConditionDiagnosticsTxData_.checkSEAEnableStatus(seaStatusLegalForTrade_.status);

        if (statusChanged) {
            // Display Settings
            if (showHideConfig_.get<bool>("DisplaySettings.supported")) {
                showHideConfig_.set("DisplaySettingsUnitsOnly.supported", false);
            }
            else if (legalForTradeInstalled_) {
                showHideConfig_.set("DisplaySettingsUnitsOnly.supported", true);
            }
            else {
                showHideConfig_.set("DisplaySettingsUnitsOnly.supported", false);
            }

            // Level 2 Temp installation show/hide. Hide if permanant installed or Level 1 not enabled
            if (showHideConfig_.get<bool>("PayloadLevel2TempInstall.supported")) {
                if (level1Installed &&
                        (SCL_PRMSW_FEAT_STATUS_INSTALLED_ENABLED != seaStatusLevel2_.status) &&
                        (SCL_PRMSW_FEAT_STATUS_INSTALLED_DISABLED != seaStatusLevel2_.status)) {
                    showHideConfig_.set("PayloadLevel2TempInstall.installed", true);
                }
                else {
                    showHideConfig_.set("PayloadLevel2TempInstall.installed", false);
                }
            }

            // Set target type to default (0-Single Target) if level2 is not installed
            if (!level2Installed && showHideReady_) {
            	LpsSaJobMgrReqstChannel req;
            	req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TARGET_TYPE;
            	req.data.targetType = 0; //default (single target)
            	jobMgrHelper_.sendRequest(req);
            }

            // Level 2 Features

            if (showHideConfig_.get<bool>("PayloadRecipes.supported")) {
            	showHideConfig_.set("PayloadRecipes.installed", level2Installed);
            }

            if (showHideConfig_.get<bool>("Lists.supported")) {
                showHideConfig_.set("Lists.installed", level2Installed);
            }

            if (showHideConfig_.get<bool>("HornOnStore.supported")) {
                showHideConfig_.set("HornOnStore.installed", level2Installed);
            }

            if (showHideConfig_.get<bool>("PayloadMultiTask.supported")) {
                showHideConfig_.set("PayloadMultiTask.installed", level2Installed);
            }

            if (showHideConfig_.get<bool>("PayloadManualAdd.supported")) {
                showHideConfig_.set("PayloadManualAdd.installed", level2Installed);
            }

            if (showHideConfig_.get<bool>("PayloadAudibleWeight.supported")) {
                showHideConfig_.set("PayloadAudibleWeight.installed", level2Installed);
            }

            if (showHideConfig_.get<bool>("PayloadSplitMode.supported")) {
                showHideConfig_.set("PayloadSplitMode.installed", level2Installed);
            }

            if (showHideConfig_.get<bool>("SaveToUSB.supported")) {
                showHideConfig_.set("SaveToUSB.installed", level2Installed);
            }

            // Linkage Sensor Calibrations
            if (showHideConfig_.get<bool>("LinkageSensorCalibrations.supported")) {
                if (level1Installed) {
                    if (legalForTradeInstalled_) {
                        showHideConfig_.set("LinkageSensorCalibrations.installed", true);
                    }
                    else if (ADVANCED == getApplicationVariant()) {
                        showHideConfig_.set("LinkageSensorCalibrations.installed", false);
                    }
                    else /* STANDALONE */ {
                        showHideConfig_.set("LinkageSensorCalibrations.installed", true);
                    }
                }
                else {
                    showHideConfig_.set("LinkageSensorCalibrations.installed", false);
                }
            }

            // Payload Calibrations
            if (showHideConfig_.get<bool>("AutoPayloadCalibrations.supported")) {
                if (level2Installed) {
                    showHideConfig_.set("AutoPayloadCalibrations.installed", true);

                    if (showHideConfig_.get<bool>("ManualPayloadCalibrations.supported")) {
                        showHideConfig_.set("ManualPayloadCalibrations.installed", false);
                    }
                }
                else {
                    showHideConfig_.set("AutoPayloadCalibrations.installed", false);

                    if (showHideConfig_.get<bool>("ManualPayloadCalibrations.supported")) {
                        showHideConfig_.set("ManualPayloadCalibrations.installed", level1Installed);
                    }
                }
            }
            else if (showHideConfig_.get<bool>("ManualPayloadCalibrations.supported")) {
                showHideConfig_.set("ManualPayloadCalibrations.installed", level1Installed);
            }

            // Legal For Trade
            if (showHideConfig_.get<bool>("PayloadLFT.supported")) {
                showHideConfig_.set("PayloadLFT.installed", legalForTradeInstalled_);
            }
        }
    }

    // Read Part Numbers
    if (nullptr != partInputChannel_) {
        while (partInputChannel_->get(partNumbers_)) {}
    }

    // Read the totals
    if (nullptr != totalsInputChannel_) {
        while (totalsInputChannel_->get(totals_)) {
            totalsReceivedTime_ = Clock::now();
        }
    }

    // Read the printer config
    if (nullptr != printerCnfgInput_) {
        while (printerCnfgInput_->get(printerCnfg_)) {
            showHideConfig_.set("Printer.installed", printerCnfg_.config.installed);
            printerCnfgReceivedTime_ = Clock::now();
        }
    }

    if (showHideConfig_.get<bool>("BEMSim.supported")) {
        showHideConfig_.set("BEMSim.enabled", bemSimMgr.cnfg.bemSimEnabled);
    }
    else if (bemSimMgr.cnfg.bemSimEnabled) {
        bemSimMgr.cnfg.bemSimEnabled = false;
        bemSimCnfgChanged_ = true;
    }

    // Read the display settings input channel and update if needed
    if (nullptr != displaySettingsInputChannel_) {
        bool newSettings = false;
        LpsSaUIDisplaySettingsInterface input;
        while (displaySettingsInputChannel_->get(input)) {
            newSettings = true;
        }

        if (newSettings) {
            displayState_.setSettings(input.settings);
            displaySettingsChanged_ = true;
        }
    }

    processDataLinkInputs();

    // Process the keypad commands received on datalink
    processKeypadInputs();

    // Enforce things in the display state that depend on LFT installation status
    if (legalForTradeInstalledStatusReceived_) {
        if (legalForTradeInstalled_) {
            // Do not allow anything other that "STANDARD" weight precision when Legal For Trade is installed.
            if (LpsCommonWeightPrecision::STANDARD != displayState_.getSettings().weightPrecision) {
                displayState_.setWeightPrecision(LpsCommonWeightPrecision::STANDARD);
                displaySettingsChanged_ = true;
            }
        }
        else /* LFT Not Installed */ {
            // Verification mode isn't available unless LFT is installed
            if (displayState_.isInVerificationMode()) {
                displayState_.setVerificationMode(false);
            }
        }
    }

    // Update the machine configuration if it changed
    //machineConfigCheck();

    // If we are currently in demo mode...
    if ("CAT99999" == machineMSN) {
        if (std::chrono::steady_clock::now() > demo_mode_exit_time) {
            // We should exit demo mode at next power up because of demo mode timeout
            std::ifstream machineMSNFp("/opt/appdata/CPM/AisJhm2DataServer/nvm/PreviousMachineMSN");
            std::string previousMSNdemo = "";
            if(machineMSNFp.good()) {
                machineMSNFp >> previousMSNdemo;
                // We can publish machine sn every loop here, it will not have any effect if it is the same.
                m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_status = 0;
                m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_value = previousMSNdemo;
                m_scsOutputs.m_jhm_MachineSN.setNewDataFlag();
                m_scsOutputs.send();
            }
            else {
                AIS_LOG_ERROR("previous machine MSN is not good");
                // Update the machine configuration if it changed
                machineConfigCheck();
            }
        }
        else {
            // We should stay in demo mode for next power up because demo mode has not timed out
            // We can publish machine sn every loop here, it will not have any effect if it is the same.
            m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_status = 0;
            m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_value = "CAT99999";
            m_scsOutputs.m_jhm_MachineSN.setNewDataFlag();
            m_scsOutputs.send();
        }
    }
    else {
        // Update the machine configuration if it changed
        machineConfigCheck();
    }

    // Get latest job manager data
    jobMgrHelper_.waitForTxData(jobMgrTxData_);

    { // Process load record (one per executive loop) - to add frequently used items
        LpsSaLoadRecordChannel loadRecord;
        if (lpsSaLoadRecordChannelInput_->get(loadRecord)) {
            auto subtotalCount = loadRecord.subtotalCount();
            auto targetType = loadRecord.targetType();

            // add target weights to frequently used list
            switch (targetType) {
            case LpsSaLoadRecordTargetType::SINGLE:
            case LpsSaLoadRecordTargetType::SPLIT:
            case LpsSaLoadRecordTargetType::BLEND_WEIGHT:
            {
            	for (uint32_t x=1; x<=subtotalCount; x++) {
            		const auto& subtotal = loadRecord.getSubtotalByIndex(x);
            		float subtotalTargetWt = subtotal.truckTargetWeightTonnes;
            		std::string display_wt_string = weightUnitConvertToDisplay(subtotalTargetWt);
            		targetWeights_.addNewTargetWt(display_wt_string, subtotalTargetWt);
            	}
            	break;
            }
            case LpsSaLoadRecordTargetType::BATCH:
            {
            	float totalTargetWt = loadRecord.getTotalTargetWeight();
            	std::string display_wt_string = weightUnitConvertToDisplay(totalTargetWt);
            	targetWeights_.addNewTargetWt(display_wt_string, totalTargetWt);
            	break;
            }

            case LpsSaLoadRecordTargetType::MIX:
            case LpsSaLoadRecordTargetType::BLEND_LOADS:
            default:
                // ignore target weight for mix, blend(loads)
                break;
            }

            // add material, to frequently used list
            switch (targetType) {
            case LpsSaLoadRecordTargetType::SINGLE:
            case LpsSaLoadRecordTargetType::BATCH:
            case LpsSaLoadRecordTargetType::MIX:
            case LpsSaLoadRecordTargetType::BLEND_LOADS:
            case LpsSaLoadRecordTargetType::BLEND_WEIGHT:
            {
                for (uint32_t x=1; x<=subtotalCount; x++) {
                    const auto& subtotal = loadRecord.getSubtotalByIndex(x);
                    materialFreqList_.addNewListItem(subtotal.materialName, subtotal.materialId, subtotal.materialDensity);
                }
                break;
            }

            case LpsSaLoadRecordTargetType::SPLIT:
            {
                const auto& subtotal = loadRecord.getSubtotalByIndex(subtotalCount);
                materialFreqList_.addNewListItem(subtotal.materialName, subtotal.materialId, subtotal.materialDensity);
                break;
            }

            default:
                // nothing to do here
                break;
            }

            // add truck and custom tags to frequently used list
            const auto& subtotal = loadRecord.getSubtotalByIndex(subtotalCount);
            truckFreqList_.addNewListItem(subtotal.truckName, subtotal.truckId, subtotal.truckTargetWeightTonnes);
            custom1FreqList_.addNewListItem(subtotal.tag1);
            custom2FreqList_.addNewListItem(subtotal.tag2);
            custom3FreqList_.addNewListItem(subtotal.tag3);
            custom4FreqList_.addNewListItem(subtotal.tag4);
        }
    }

    // capture prev store, subtotal count
    prevStoreCount_ = jobMgrTxData_.storeCount;
    prevSubtotalCount_ = jobMgrTxData_.subtotalCount;

    // Get latest weigh app data
    weighAppInf_.waitForTxData(weighAppTxData_);

    if (listMgrRequestHelper_.waitForTxData(listMgrTxData_)) {
        listMgrTxReceived_ = true;
    }

    // UPDATE BEM Simulator Information states
   if(bemSimMgr.updateInfoScheduler()) {
       bemSimMgr.setTotalFuelUsedIsOK(totalFuel_.isTotalFuelAvailable());
       double fuel_used = totalFuel_.getTotalFuelUsedLiters();
       bemSimMgr.setTotalFuelUsed(fuel_used);
       bemSimMgr.updateInfo();
       bemSimMgr.cnfg.saveCnfgScheduler(makeStoragePath(BEMSimFileNameAr));
       bemSimMgr.updateEvents(bemSimWarningSetEvent, bemSimWarningClrEvent,
                              bemSimDerateSetEvent, bemSimDerateClrEvent);
   }

    // Update Pop Ups
    aisJhmPopUpRead();

    { // Update can11bitProcess
        AisJhm2CANProcess::PayloadInfo_t payloadInfo;
        payloadInfo.truckWeight = weightUnitConvertToDisplay(jobMgrTxData_.truckWeight);
        payloadInfo.operationMode = jobMgrTxData_.OperationMode;
        payloadInfo.accuracy = getBucketWeightDisplayAccuracy(weighAppTxData_.PayloadCalcMeth, jobMgrTxData_.DispBestBktWt.isOk);
        payloadInfo.bucketWeight = weightUnitConvertToDisplay(weighAppTxData_.BestBktWtInTonnes);
        payloadInfo.remainingWeight = weightUnitConvertToDisplay(jobMgrTxData_.remainingWeight);
        payloadInfo.showExclamationPoint = weighAppTxData_.ShowExclamationPoint;
        payloadInfo.decimalPrecision = displayState_.getWeightDecimalPrecision();
        payloadInfo.weighUnits = displayState_.getSettings().weightUnits;


        // If Legal For Trade is installed, don't show green box on front dash.
        if ((legalForTradeInstalled_) && (payloadInfo.accuracy > 2)) {
            payloadInfo.accuracy = 2;
        }

        can11bitProcess_.update(payloadInfo);
    }

    return true;
}

/*
 * Processes all incoming datalink inputs
 * Not thread safe with other threads that access displayState_
 */
void AisJhm2RequestProcessor::processDataLinkInputs() {
    if (nullptr == dataLinkDataInput_) {
        AIS_LOG_INFO("dataLinkDataInput_ is NULL");
        return;
    }

    AIS_LOG_INFO("Inside processDataLinkInputs");

    bool settingsChanged = false;
    DataLinkData data;
    while (dataLinkDataInput_->get(data)) {
        // loop through the parameters looking for the ones we care about.
        for (const auto& param : data.GetParams()) {
            // Skip this if no new data is received.
            if (!param.IsPIDDataReceived()) {
                continue;
            }

            switch (param.GetParamIdentifierType()) {
            case (DataLinkParamInfo::DATA_LINK_PARAM_IDENTIFIER_PID): {
                switch (param.GetParamId()) {

                // Selected Language
                case (0xD0022C): {
                    if (0 == param.GetLastValueDsi()) {
                        // If the value is different than my current setting
                        // Update my current setting.
                        auto value = param.GetLastValue<LpsSaUIDisplaySettingsLanguage>();
                        if (value != displayState_.getSettings().language) {
                            displayState_.setLanguage(value);
                            settingsChanged = true;
                        }
                    }
                    break;
                }

                // Units
                case (0xF25B): {
                    // If Legal For Trade is installed, do not synchronize to datalink units.
                    if (legalForTradeInstalled_ || !legalForTradeInstalledStatusReceived_) {
                        break;
                    }

                    // We only care about receiving this from display.
                    if (0x28 != param.GetSid()) {
                        break;
                    }

                    if (0 == param.GetLastValueDsi()) {
                        // If the value is different than my current setting
                        // Update my current setting.
                        auto value = param.GetLastValue<LpsSaUIDisplaySettingsUnits>();
                        if (value != displayState_.getSettings().units) {
                            displayState_.setUnits(value);
                            settingsChanged = true;
                        }
                    }
                    break;
                }

                // Weigh Units
                case (0xD10938): {
                    // If Legal For Trade is installed, do not synchronize to datalink weight units.
                    if (legalForTradeInstalled_ || !legalForTradeInstalledStatusReceived_) {
                        break;
                    }

                    // We only care about receiving this from display.
                    if (0x28 != param.GetSid()) {
                        break;
                    }

                    if (0 == param.GetLastValueDsi()) {
                        // If the value is different than my current setting
                        // Update my current setting.
                        auto value = param.GetLastValue<LpsCommonWeightUnits>();

                        if (value != displayState_.getSettings().weightUnits) {
                            displayState_.setWeightUnits(value);
                            settingsChanged = true;
                        }
                    }                    
                    break;
                }

                // Weigh Precision
                case (0xD11B8D): {
                    // If Legal For Trade is installed, do not synchronize to datalink weight precision.
                    if (legalForTradeInstalled_ || !legalForTradeInstalledStatusReceived_) {
                        break;
                    }

                    // We only care about receiving this from display.
                    if (0x28 != param.GetSid()) {
                        break;
                    }

                    if (0 == param.GetLastValueDsi()) {
                        // If the value is different than my current setting
                        // Update my current setting.
                        auto value = param.GetLastValue<LpsCommonWeightPrecision>();

                        if (value != displayState_.getSettings().weightPrecision) {
                            displayState_.setWeightPrecision(value);
                            settingsChanged = true;
                        }
                    }
                    break;
                }

                // Backlight Percent
                case (0xD0022B): {
                    if (0 == param.GetLastValueDsi()) {
                        // Backlight percent has been successfully received. If show/hide
                        // for display brightness is supported then hide
                        if (displayBrightnessSupported_) {
                            showHideConfig_.set("DisplayBrightness.supported", false);
                            displayBrightnessSupported_ = false;
                        }

                        // If the value is different than my current setting
                        // Update my current setting.
                        uint16_t value = param.GetLastValue<uint16_t>();
                        if (value > 100) {
                            value = 100;
                        }

                        if (value != displayState_.getSettings().brightness) {
                            displayState_.setBrightness(static_cast<uint8_t>(value));
                            settingsChanged = true;
                        }
                    }
                    else {
                        // Backlight percent has a dsi. Show display brightness
                        // if it is currently not supported (on change)
                        if (!displayBrightnessSupported_) {
                            showHideConfig_.set("DisplayBrightness.supported", true);
                            displayBrightnessSupported_ = true;
                        }

                    }
                    break;
                }

                // Service Mode
                case (0xD1000E): {
                    if (0 == param.GetLastValueDsi()) {
                        // If the value is different than my current setting
                        // Update my current setting.
                        uint16_t value = param.GetLastValue<uint16_t>();
                        if (0x000C /* Enabled */ == value) {
                            displayState_.enterServiceMode(true); // Force enter service mode
                        }
                        else {
                            displayState_.exitServiceMode();
                        }
                    }
                    break;
                }

                // Tool config
                case (0xD10321): {
                    /* only update if no dsi */
                    if (0 == param.GetLastValueDsi()) {
                        machineConfig_.tool.isOk = true;
                        machineConfig_.tool.val = param.GetLastValue<uint16_t>();
                    }
                    else {
                        machineConfig_.tool.isOk = false;
                    }

                    AIS_LOG_INFO("Rxed Data for PID 0x%X with val = %d, isOk = %d ",
                            param.GetParamId(), machineConfig_.tool.val, machineConfig_.tool.isOk);
                    break;
                }

                // Linkage config
                case (0xD10049): {
                    /* only update if no dsi */
                    if(0 == param.GetLastValueDsi()) {
                        machineConfig_.linkage.isOk = true;
                        machineConfig_.linkage.val = param.GetLastValue<uint16_t>();
                    }
                    else {
                        machineConfig_.linkage.isOk = false;
                    }

                    AIS_LOG_INFO("Rxed Data for PID 0x%X with val = %d, isOk = %d ",
                            param.GetParamId(), machineConfig_.linkage.val, machineConfig_.linkage.isOk);
                    break;
                }

                // Total Fuel Low Res
                case (0xC8): {
                    /* only update if no dsi */
                    if(0 == param.GetLastValueDsi()) {
                        totalFuel_.lowResFuel.isOk = true;
                        totalFuel_.lowResFuel.val = param.GetLastValueEng();
                    }
                    else {
                        totalFuel_.lowResFuel.isOk = false;
                    }

                    AIS_LOG_INFO("Rxed Data for PID 0x%X with val = %d, isOk = %d ",
                            param.GetParamId(), totalFuel_.lowResFuel.val, totalFuel_.lowResFuel.isOk);
                    break;
                }

                // Total Fuel (high res)
                case (0xFE5D): {
                    /* only update if no dsi */
                    if(0 == param.GetLastValueDsi()) {
                        totalFuel_.highResFuel.isOk = true;
                        totalFuel_.highResFuel.val = param.GetLastValueEng();
                    }
                    else {
                        totalFuel_.highResFuel.isOk = false;
                    }

                    AIS_LOG_INFO("Rxed Data for PID 0x%X with val = %d, isOk = %d ",
                            param.GetParamId(), totalFuel_.highResFuel.val, totalFuel_.highResFuel.isOk);
                    break;
                }

                case BUTTON_1_MODE_PID: {
                    if (0 == param.GetLastValueDsi()) {
                        uint16_t dlValue = param.GetLastGoodValue<uint16_t>();
                        keypad_.button1.mode = (KeypadButtonMode_t)dlValue;
                        AIS_LOG_INFO("BUTTON_1_MODE_PID Recd: %d", dlValue);
                    }
                    else {
                        //DSI
                        keypad_.button1.mode = KeypadButtonMode_t::PAYLOAD_NONE;
                    }
                    break;
                }
                case BUTTON_2_MODE_PID: {
                    if (0 == param.GetLastValueDsi()) {
                        uint16_t dlValue = param.GetLastGoodValue<uint16_t>();
                        keypad_.button2.mode = (KeypadButtonMode_t)dlValue;
                        AIS_LOG_INFO("BUTTON_2_MODE_PID Recd: %d", dlValue);
                    }
                    else {
                        //DSI
                        keypad_.button2.mode = KeypadButtonMode_t::PAYLOAD_NONE;
                    }
                    break;
                }
                case BUTTON_3_MODE_PID: {
                    if (0 == param.GetLastValueDsi()) {
                        uint16_t dlValue = param.GetLastGoodValue<uint16_t>();
                        keypad_.button3.mode = (KeypadButtonMode_t)dlValue;
                        AIS_LOG_INFO("BUTTON_3_MODE_PID Recd: %d", dlValue);
                    }
                    else {
                        //DSI
                        keypad_.button3.mode = KeypadButtonMode_t::PAYLOAD_NONE;
                    }
                    break;
                }

                case BUTTON_1_PRESS_PID: {
                    updateNumberOfReleases(param, keypad_.button1);
                    break;
                }

                case BUTTON_2_PRESS_PID: {
                    updateNumberOfReleases(param, keypad_.button2);
                    break;
                }

                case BUTTON_3_PRESS_PID: {
                    updateNumberOfReleases(param, keypad_.button3);
                    break;
                }
                }
                break;
            }
            default: {
                break;
            }
            }
        }
    }

    if (settingsChanged) {
        displaySettingsChanged_ = true;
    }
}

void AisJhm2RequestProcessor::aisJhmPopUpRead() {
    // Only updated the diagnostics if we received new data.
    if (m_scsInputs.m_jhm_eddt.getNewDataFlag()) {
        m_scsInputs.m_jhm_eddt.clearNewDataFlag();

        // Reset all the flags and build them back up again.
        infoState_.reset();
        diagState_.reset();
        eventState_.reset();

        // Handles LFT overload pop-up if overload EID disabled
        eventState_[AisJhmEventPopUp::OVERLOAD_LIMIT_EXCEEDED] = weighAppTxData_.EventState[ACDEventPopUp::OVERLOAD_LIMIT_EXCEEDED];

        const auto* pDiags = m_scsInputs.m_jhm_eddt.getListOfEventDiagnostics();

        if (nullptr != pDiags) {
            for (const auto& diag : *pDiags) {
                if (SCL_DCLI_FAULT_ACTIVE == diag.status || SCL_DCLI_FAULT_ACTIVE_AND_LOGGED == diag.status) {
                    /*
                     * Tip-Off Assist CDL Event Ids
                     *  63038   Tip-Off Assist Disarmed Due to Time Out
                     *  63039   Tip-Off Assist Disarmed Due to Payload Not Achievable
                     *  63040   Tip-Off Assist Disarmed Due to Tilt Lever Racked
                     *  63041   Tip-Off Assist Disarmed Due to Not in Pile Tip-Off Mode
                     *  63042   Tip-Off Assist Disarmed Due to Truck Target Not Set
                     *  63043   Tip-Off Assist Disarmed Due to Tilt Lever Released
                     *  63044   Tip-Off Assist Disarmed Due to End of Travel
                     *  63045   Tip-Off Assist Armed
                     *  63046   Tip-Off Assist Complete
                     *  63078   Tip-Off Assist Disarmed Due to Excessive Machine Speed
                     *  63084   Tip-Off Assist Disarmed Due to Lift Command
                     */
                    switch (diag.getCID()) {
                    case (63038): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_DUE_TO_TIMEOUT] = true;
                        break;
                    }
                    case (63039): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_PAYLOAD_NOT_ACHIEVABLE] = true;
                        break;
                    }
                    case (63040): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_TILT_LEVER_RACKED] = true;
                        break;
                    }
                    case (63041): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_NOT_IN_PILE_TIP_OFF_MODE] = true;
                        break;
                    }
                    case (63042): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_TRUCK_TARGET_NOT_SET] = true;
                        break;
                    }
                    case (63043): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_TILT_LEVER_RELEASED] = true;
                        break;
                    }
                    case (63044): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_END_OF_TRAVEL] = true;
                        break;
                    }
                    case (63045): {
                        infoState_[AisJhmInfoPopUp::TOA_ARMED] = true;
                        break;
                    }
                    case (63046): {
                        infoState_[AisJhmInfoPopUp::TOA_ASSIST_COMPLETE] = true;
                        break;
                    }
                    case (63078): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_EXCESSIVE_MACHINE_SPEED] = true;
                        break;
                    }
                    case (63084): {
                        infoState_[AisJhmInfoPopUp::TOA_DISARMED_LIFT_LEVER_COMMAND] = true;
                        break;
                    }

                    default: {
                        break;
                    }
                    }

                    /*
                     *  Weigh App (MID 0x41) CDL Event Ids
                     *  E????    - Payload Legal For Trade Measurement Not Sealed
                     *  E2083    - Payload Memory Low
                     *  E2138    - Payload Memory Full
                     *  E2126    - Payload Overload Limit Exceeded
                     *  351      - Tilt Linkage Position Sensor
                     *  350      - Lift Linkage Position Sensor
                     *  364      - Lift Cylinder Head End Pressure Sensor
                     *  769      - Lift Cylinder Rod End Pressure Sensor
                     *  2183     - Payload Monitoring System
                     *  967      - Machine Application
                     *  248      - CAT Data Link
                     *  3900     - Ethernet Data Link #1
                     *  247      - SAE J1939 Data Link
                     *  E63028    - BEM Simulator Events
                     *  E1378     - High ECM Power Input Voltage
                     *  E1379     - Low ECM Power Input Voltage
                     */
                    if (0x41 == diag.getID()) {
                        if (2 == diag.category) {
                            // Events
                            switch (diag.getCID()) {
                            case (HIGH_ECU_VOLTAGE_EID): {
                                eventState_[AisJhmEventPopUp::HIGH_ECU_VOLTAGE] = true;
                                break;
                            }                            
                            case (LOW_ECU_VOLTAGE_EID): {
                                eventState_[AisJhmEventPopUp::LOW_ECU_VOLTAGE] = true;
                                break;
                            }
                            case (PAYLOAD_LFT_NOT_SEALED_EID): {
                                eventState_[AisJhmEventPopUp::PAYLOAD_LFT_NOT_SEALED] = true;
                                break;
                            }
                            case (PAYLOAD_MEMORY_LOW_EID): {
                                eventState_[AisJhmEventPopUp::PAYLOAD_MEMORY_LOW] = true;
                                break;
                            }
                            case (PAYLOAD_MEMORY_FULL_EID): {
                                eventState_[AisJhmEventPopUp::PAYLOAD_MEMORY_FULL] = true;
                                break;
                            }
                            case (PAYLOAD_OVERLOAD_LIMIT_EXCEEDED_EID): {
                                eventState_[AisJhmEventPopUp::OVERLOAD_LIMIT_EXCEEDED] = true;
                                break;
                            }
                            case (BEMSIM_EID): {
                                if(2 == diag.getWCI()) {
                                    eventState_[AisJhmEventPopUp::BEMSIM_LEVEL_BELOW_DERATE_THRESHOLD] = true;
                                }
                                if(1 == diag.getWCI()) {
                                    eventState_[AisJhmEventPopUp::BEMSIM_LEVEL_NEED_TO_CHARGE_WARNING] = true;
                                }
                                break;
                            }
                            default: {
                                break;
                            }
                            }
                        }
                        else {
                            // Diagnostics
                            switch (diag.getCID()) {
                            case (351): {
                                if (3 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::TILT_LINK_VOLTAGE_ABOVE_NORMAL] = true;
                                }
                                else if (4 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::TILT_LINK_VOLTAGE_BELOW_NORMAL] = true;
                                }
                                else if (8 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::TILT_LINK_FREQ_ABNORMAL] = true;
                                }
                                else if (13 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::TILT_LINK_OUT_OF_CAL] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }
                            case (350): {
                                if (3 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_LINK_VOLTAGE_ABOVE_NORMAL] = true;
                                }
                                else if (4 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_LINK_VOLTAGE_BELOW_NORMAL] = true;
                                }
                                else if (8 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_LINK_FREQ_ABNORMAL] = true;
                                }
                                else if (13 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_LINK_OUT_OF_CAL] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }
                            case (364): {
                                if (3 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_HE_VOLTAGE_ABOVE_NORMAL] = true;
                                }
                                else if (4 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_HE_VOLTAGE_BELOW_NORMAL] = true;
                                }

                                else if (8 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_HE_FREQ_ABNORMAL] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }
                            case (769): {
                                if (3 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_RE_VOLTAGE_ABOVE_NORMAL] = true;
                                }
                                else if (4 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_RE_VOLTAGE_BELOW_NORMAL] = true;
                                }

                                else if (8 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::LIFT_RE_FREQ_ABNORMAL] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }

                            case (2183): {
                                if (13 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::PAYLOAD_SYSTEM_OUT_OF_CAL] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }
                            case (967): {
                                if (14 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::MACHINE_MODEL_NOT_SET] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }
                            case (248): {
                                if (9 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::CAT_DATALINK_ABNORMAL] = true;
                                }
                                else if (14 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::CAT_DATALINK_SPECIAL_INSTRUCTION] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }

                            case (3900): {
                                if (9 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::ETHERNET_DATALINK_1_ABNORMAL] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }

                            case (247): {
                                if (9 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::J1939_DATALINK_1_ABNORMAL] = true;
                                }
                                else if (14 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::J1939_DATALINK_1_SPECIAL_INSTRUCTION] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }

                            case (2348): {
                                if (9 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::J1939_DATALINK_2_ABNORMAL] = true;
                                }
                                else if (14 == diag.getFMI()) {
                                    diagState_[AisJhmDiagPopUp::J1939_DATALINK_2_SPECIAL_INSTRUCTION] = true;
                                }
                                else {
                                    AIS_LOG_INFO("Diag %d-%d not supported by UI", diag.getCID(), diag.getFMI());
                                }
                                break;
                            }

                            default: {
                                break;
                            }
                            }
                        }
                    }
                }
            }
        }
    }

    // Work Order Assist (Dispatch for Loaders)
    infoState_[AisJhmInfoPopUp::DISPATCH_COMMUNICATION_LOSS] = m_scsInputs.m_jhm_dispatch_tx.commsWarningActive;

    // Set Memory Full Popup
    //  Reusing the event popup because we don't have a dedicated popup for a different kind of memory full.
    //  This will get cleared the next time we get new EDDT data and the memory full event is not active.
    if (jobMgrTxData_.passCount >= jobMgrTxData_.AutoStorePassCount) {
        // Only set the memory full event when autostore is disabled.
        // Autostore is disabled when LFT is installed.
        if (legalForTradeInstalled_ && legalForTradeInstalledStatusReceived_) {
            eventState_[AisJhmEventPopUp::PAYLOAD_MEMORY_FULL] = true;
        }
    }

    /*
     * Copying over the info pop-ups we get from Weighing App and Job Manager
     */

    // Only show raise/lower stall info pop-ups if in truck tip-off
    if (LPS_SA_JOB_MGR_TIP_OFF_TRUCK_ENABLE == jobMgrTxData_.TipOffState) {
        infoState_[AisJhmInfoPopUp::PAYLOAD_LOWER_STALL] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_LOWER_STALL];
        infoState_[AisJhmInfoPopUp::PAYLOAD_RAISE_STALL] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_RAISE_STALL];
    }

    infoState_[AisJhmInfoPopUp::PAYLOAD_STORE_REJECTED_NO_GBW] = jobMgrTxData_.storeRejected;

    infoState_[AisJhmInfoPopUp::PAYLOAD_REWEIGH_EXCESSIVE_PITCH] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_EXCESSIVE_PITCH];
    infoState_[AisJhmInfoPopUp::PAYLOAD_REWEIGH_PRESSURE_CHANGING] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_PRESSURE_CHANGING];
    infoState_[AisJhmInfoPopUp::PAYLOAD_REWEIGH_INCONSISTENT] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_INCONSISTENT];
    infoState_[AisJhmInfoPopUp::PAYLOAD_REWEIGH_SPEED_CHANGING] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_SPEED_CHANGING];
    infoState_[AisJhmInfoPopUp::PAYLOAD_REWEIGH_NOT_RACKED] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_NOT_RACKED];
    infoState_[AisJhmInfoPopUp::PAYLOAD_REWEIGH_STOPPED_IN_RANGE] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_STOPPED_IN_RANGE];
    infoState_[AisJhmInfoPopUp::PAYLOAD_REWEIGH_LIFT_TOO_SLOW] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_LIFT_TOO_SLOW];
    infoState_[AisJhmInfoPopUp::PAYLOAD_ZERO_ACCEPTED] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_ZERO_ACCEPTED];
    infoState_[AisJhmInfoPopUp::PAYLOAD_CAL_ACCEPTED] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_CAL_ACCEPTED];
    infoState_[AisJhmInfoPopUp::PAYLOAD_NOT_ZEROED] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_NOT_ZEROED];
    infoState_[AisJhmInfoPopUp::PAYLOAD_TOO_HEAVY_TO_ZERO] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_TOO_HEAVY_TO_ZERO];
    infoState_[AisJhmInfoPopUp::PAYLOAD_CAL_WT_ENTRY_REQUIRED] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_CAL_WT_ENTRY_REQUIRED];
    infoState_[AisJhmInfoPopUp::PAYLOAD_REWEIGH_WARMUP_LIFT] = weighAppTxData_.InfoState[ACDInfoPopUp::PAYLOAD_REWEIGH_WARMUP_LIFT];

    /* Set not installed popup from weigh app */
    diagState_[AisJhmDiagPopUp::PAYLOAD_SYSTEM_NOT_INSTALLED] = weighAppTxData_.DiagState[ACDDiagPopUp::PAYLOAD_SYSTEM_NOT_INSTALLED];
}


/*
 * postUpdate
 *  This takes care of updating things that need to be updated after
 *  requests have been processed.
 */
void AisJhm2RequestProcessor::postUpdate() {
    bool publishDisplayState = publishDisplayState_;

    { // Update descriptive markings
        static bool previousLFTDisabled = false;
        uint8_t lftDisabled = jobMgrTxData_.lftDisabled;

        if (previousLFTDisabled != lftDisabled) {
            if (lftDisabled) {
                showHideConfig_.set("PayloadLFT.description", "******");
            }
            else {
                showHideConfig_.set("PayloadLFT.description", displayState_.getLegalForTradeDescriptiveMarking());
            }
            previousLFTDisabled = lftDisabled;
        }
        else if (lftDisabled) {
            // Just leave it *'s
        }
        else if (displayState_.legalForTradeDescriptionMarkingChanged()) {
            // Markings changed, update them.
            showHideConfig_.set("PayloadLFT.description", displayState_.getLegalForTradeDescriptiveMarking());
        }
    }

    // Save display settings if needed.
    if (displaySettingsChanged_) {
        displayState_.saveSettings(makeStoragePath(DISPLAY_SETTINGS_FILENAME_BIN));
        publishDisplayState = true;
        displaySettingsChanged_ = false;
        AIS_LOG_INFO("Saved display settings.");
    }

    // Save short list if needed.
    targetWeights_.saveTargetWeights(makeStoragePath(TARGET_WEIGHTS_FILENAME_BIN));
    truckFreqList_.saveList(makeStoragePath(TRUCK_SHORTLIST_FILENAME_BIN));
    materialFreqList_.saveList(makeStoragePath(MATERIAL_SHORTLIST_FILENAME_BIN));
    custom1FreqList_.saveList(makeStoragePath(TAG1_SHORTLIST_FILENAME_BIN));
    custom2FreqList_.saveList(makeStoragePath(TAG2_SHORTLIST_FILENAME_BIN));
    custom3FreqList_.saveList(makeStoragePath(TAG3_SHORTLIST_FILENAME_BIN));
    custom4FreqList_.saveList(makeStoragePath(TAG4_SHORTLIST_FILENAME_BIN));

    // Save BEM Sim Config if needed.
    if (bemSimCnfgChanged_) {
        bemSimMgr.cnfg.save(makeStoragePath(BEMSimFileNameAr));
        bemSimCnfgChanged_ = false;
        AIS_LOG_INFO("Saved BEM Sim Config... AisJhm2");
    }

    if (publishDisplayState) {
        // If we are already publishing display state, reset the counter.
        displayStatePublishCounter_ = 0;
    }
    else {
        // See if we have to publish display state.
        float maxPublishCount = DISPLAY_STATE_PUBLISH_PERIOD_SEC * updateRate_;
        ++displayStatePublishCounter_;
        if (displayStatePublishCounter_ >= maxPublishCount) {
            displayStatePublishCounter_ = 0;
            publishDisplayState = true;
        }
    }

    // Publish the display state if needed.
    if (publishDisplayState && (nullptr != displayStateOutputChannel_)) {
        LpsSaUIDisplayStateInterface displayState;
        displayState.state = displayState_;
        displayStateOutputChannel_->publish(displayState);
        AIS_LOG_INFO("Published display state.");
    }

    // Publish the BEM Simulator state if needed.
    if (nullptr != bemSimStateOutputChannel_) {
        LpsSaUIBEMSimStateInterface bemSimStateIntf;
        bemSimStateIntf.cnfg = bemSimMgr.getCnfg();
        bemSimStateIntf.chargeLevel = bemSimMgr.getChargeLevel();
        bemSimStateIntf.dischargeRate = bemSimMgr.getDischargeRate();
        bemSimStateIntf.totalFuelUsed = static_cast<float>(bemSimMgr.getTotalFuelUsed());
        bemSimStateIntf.derateEngineCmd = bemSimMgr.getDerateEngineCmd();
        bemSimStateIntf.derateEngineRPM = bemSimMgr.getDerateEngineRPM();
        bemSimStateIntf.chargeRate = bemSimMgr.calc.getChargeRate();
        bemSimStateIntf.timeToCharged = bemSimMgr.calc.getTimeToCharged();
        bemSimStateIntf.timeToDischarged = bemSimMgr.calc.getTimeToDischarged();
        bemSimStateOutputChannel_->publish(bemSimStateIntf);
        AIS_LOG_INFO("Published BEMSim state.");
    }

    publishDisplayState_ = false;
}

void AisJhm2RequestProcessor::cleanup() {
    calUIClientInf_.stop();
    totalsRequestHelper_.stop();
    jobMgrHelper_.stop();
    weighAppInf_.stop();
    listMgrRequestHelper_.stop();
    autonomyConditionDiagnosticsHelper_.stop();
}

/******************************************************************************
FUNCTION:                   getResponse
DESCRIPTION:                get the respose from the Jhm2ParamMsgRequest
PARAMETER DESCRIPTION:
RETURN VALUE:               address  
*******************************************************************************/
Jhm2Msg* AisJhm2RequestProcessor::getResponse(const Jhm2ParamMsgRequest& req) {

    AIS_LOG_DEBUG("Request received.");

    { // This request needs parameters requested
        if (req.hasReqData(m_PARAMSREQUESTED_KEY)) {
            Jhm2ParamMsg* newResp = new Jhm2ParamMsg();
            if (nullptr != newResp) {
                if (processParamRequest(req, *newResp)) {
                    return newResp;
                }
                delete newResp;
            }
            return nullptr;
        }
    }

    { // This request is to execute a command
        if (req.hasReqData(m_COMMANDREQUESTED_KEY)) {
            Jhm2CmdResponseMsg* newResp = new Jhm2CmdResponseMsg();
            if (nullptr != newResp) {
                if (processCmdRequest(req, *newResp)) {
                    return newResp;
                }
                delete newResp;
            }
            return nullptr;
        }
    }

    { // This request is for a file
        if (req.hasReqData(m_FILEREQUESTED_KEY)) {
            AisJhm2FileMsg* newResp = new AisJhm2FileMsg();
            if (nullptr != newResp) {
                if (processFileRequest(req, *newResp)) {
                    return newResp;
                }
                delete newResp;
            }
            return nullptr;
        }
    }

    // Can't process this request
    return nullptr;
}

/******************************************************************************
FUNCTION:                   processParamRequest
DESCRIPTION:                extract the HTTP request Query String and pass handler
PARAMETER DESCRIPTION:
RETURN VALUE:               boolean
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequest(const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AIS_LOG_INFO("Parameters requested.");
    const Jhm2ParamMsgRequest::ReqDataT& paramsRequested = req.getReqData(m_PARAMSREQUESTED_KEY);
    for (Jhm2ParamMsgRequest::ReqDataT_Cit it = paramsRequested.begin(); it != paramsRequested.end(); ++it) {
        const std::string& str = *it;

        AIS_LOG_INFO(" parameter - %s", str.c_str());

        // If there is a handler installed, use it.
        if (paramMsgRequestHandlerMap_.count(str) > 0) {
            ParamMsgRequestHandler handler = paramMsgRequestHandlerMap_[str];
            if (nullptr != handler) {
                AIS_LOG_DEBUG(" parameter handler found.");
                (this->*handler)(str, req, resp);
            }
        }
        else {
            // Call the default handler.
            AIS_LOG_DEBUG(" default handler used.");
            processParamRequestDefault(str, req, resp);
        }
    }

    return true;
}

/******************************************************************************
FUNCTION:                   weightUnitConvertToDisplay
DESCRIPTION:                used to convert the weight to either TonnesToTon(US) or TonnesToPounds
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
template<typename T>
std::string AisJhm2RequestProcessor::weightUnitConvertToDisplay(T metricValue) {
    T value = lps_common_weight_from_tonnes(metricValue, displayState_.getSettings().weightUnits);
    return lps_common_weight_to_string(value, displayState_.getWeightDecimalPrecision(), displayState_.getWeightInterval());
}

/******************************************************************************
FUNCTION:                   weightUnitConvertFromDisplay
DESCRIPTION:                used to convert the weight to whether TonToPounds or PoundsToTonnes
PARAMETER DESCRIPTION:
RETURN VALUE:                
*******************************************************************************/
template<typename T>
T AisJhm2RequestProcessor::weightUnitConvertFromDisplay(T displayValue) {
    return lps_common_weight_to_tonnes(displayValue, displayState_.getSettings().weightUnits);
}

// Instantiate this template explicitly for use outside of this file.
template float AisJhm2RequestProcessor::weightUnitConvertFromDisplay<float>(float displayValue);

/******************************************************************************
FUNCTION:                   weightUnitsString
DESCRIPTION:                Get the units string
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
std::string AisJhm2RequestProcessor::weightUnitsString() {
    switch (displayState_.getSettings().weightUnits) {
    case (LpsCommonWeightUnits::TONNE): {
        return "tonnes";
    }
    case (LpsCommonWeightUnits::TON): {
        return "Tons";
    }
    case (LpsCommonWeightUnits::KLB): {
        return "lbsx1000";
    }
    case (LpsCommonWeightUnits::LB): {
        return "lbs";
    }
    case (LpsCommonWeightUnits::KG): {
        return "kg";
    }
    }
    return "tonnes";
}

/******************************************************************************
FUNCTION:                   processParamRequestDefault
DESCRIPTION:                Read the http request Query string and send the Respose in JSON format to UI
PARAMETER DESCRIPTION:
RETURN VALUE:               boolean 
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestDefault(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    if (paramName == "AutoStore_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, jobMgrTxData_.AutoStorePassCount));
    }
    else if (paramName == "WeighRangeBottom") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.WeighRange.WeighRangeBottom));
    }
    else if (paramName == "WeighRangeSize") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.WeighRange.WeighRangeSize));
    }
    else if (paramName == "LiftCylinderPos_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.LiftPosition.percentCylinderLength));
    }
    else if (paramName == "LiftCylinderVelMilliPerSec_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.LiftCylVel.Val));
    }
    else if (paramName == "LiftCylinderPosNormalized_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylPos));
    }
    else if (paramName == "LiftCylinderHEPres_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylHEPres));
    }
    else if (paramName == "LiftCylinderREPres_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylREPres));
    }
    else if (paramName == "HydOilTempC_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.PidData.ProdMeasureSensorStatus.HydOilTemp));
    }
    else if (paramName == "LiftLinkageDC_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftLinkDC));
    }
    else if (paramName == "TiltLinkageDC_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weighAppTxData_.PidData.ProdMeasureSensorStatus.TiltLinkDC));
    }
    else if (paramName == "MM_Selected_Rd") {
        if (MM_Selected_Make_Rd.empty()) {
            resp.insertParam(new Jhm2SimpleParam(paramName, -1));
        }
        else {
            resp.insertParam(new Jhm2SimpleParam("MM_Selected_Make_Rd", MM_Selected_Make_Rd));
            resp.insertParam(new Jhm2SimpleParam("MM_Selected_Machine_Rd", MM_Selected_Machine_Rd));
            resp.insertParam(new Jhm2SimpleParam("MM_Selected_Linkage_Rd", MM_Selected_Linkage_Rd));
        }
        AIS_LOG_DEBUG("\n MM_Selected_Make_Rd  %s \n", MM_Selected_Make_Rd.c_str());
        AIS_LOG_DEBUG("\n MM_Selected_Machine_Rd  %s \n", MM_Selected_Machine_Rd.c_str());
        AIS_LOG_DEBUG("\n MM_Selected_Linkage_Rd  %s \n", MM_Selected_Linkage_Rd.c_str());
    }
    else if (paramName == "LifeTimeTruckLoadCount_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, totals_.loadCount));
    }
    else if (paramName == "LifeTimeTotalPayLoad_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, weightUnitConvertToDisplay(totals_.weightTonnes)));
    }
    else if (paramName == "LifeTimeCycleCount_Rd") {
        resp.insertParam(new Jhm2SimpleParam(paramName, totals_.passCount));
    }
    else if (paramName == "SuggestedCalWeightEntry_Rd") {
        float weightTonnes = weighAppTxData_.PidData.LastPloadWt;

        if (weightTonnes <= 0) {
            weightTonnes = weighAppTxData_.CalWt;
        }

        float weightDisplay = lps_common_weight_from_tonnes(weightTonnes, displayState_.getSettings().weightUnits);

        resp.insertParam(new Jhm2SimpleParam(paramName, lps_common_weight_to_string(weightDisplay, 3, 0.001)));
    }
    else if (paramName == "SimpleCalList_Rd") { // SimpleCalList response to UI if UI requests SimpleCalList_Rd and the data fetched from the LpsSaJobMgrTxChannel SCS object
        resp.insertParam(new Jhm2SimpleParam(paramName, fetchSimpleCalList()));
    }
    else if (paramName == "ActiveEvent_Rd") { // event state response to UI if UI requests ActiveEvent_Rd and response data fetched from the LpsSaWeighTxChannel scs object
        resp.insertParam(new Jhm2SimpleParam(paramName, eventState_.to_ulong()));
    }
    else if (paramName == "ActiveDiag_Rd") { // diagonostic state response to UI if UI requests ActiveDiag_Rd and response data fetched from the LpsSaWeighTxChannel scs object
        resp.insertParam(new Jhm2SimpleParam(paramName, diagState_.to_ulong()));
    }
    else if (paramName == "ActiveInfo_Rd") { // Info state response to UI if UI requests ActiveInfo_Rd and the response data fetched from the LpsSaWeighTxChannel scs object
        resp.insertParam(new Jhm2SimpleParam(paramName, infoState_.to_ulong()));
    }
    else if (paramName == "FaultIdentifier_Rd") {
        // FaultIdentifier response to UI if UI requests the FaultIdentifier_Rd and response data fetched from the LpsSaWeighTxChannel scs object
        if (weighAppTxData_.ShowExclamationPoint) {
            resp.insertParam(new Jhm2SimpleParam(paramName, "1"));
        }
        else {
            resp.insertParam(new Jhm2SimpleParam(paramName, "0"));
        }
    }
    else if (paramName == "HornStoreState") {
        resp.insertParam(new Jhm2SimpleParam(paramName, jobMgrTxData_.ReadHornStoreEnable()));
    }

    return true;
}

/******************************************************************************
FUNCTION:                   fetchSimpleCalList
DESCRIPTION:                convert the parameters as JSON format.
PARAMETER DESCRIPTION:
RETURN VALUE:               character array 
*******************************************************************************/
std::string AisJhm2RequestProcessor::fetchSimpleCalList() {
    std::stringstream ss;

    // Open the top level object and list.
    ss << "{\"CalEntries\":[";

    // Put in all of the simple cal data in the list.
    for (const SimpleCalData_t& calData : jobMgrTxData_.simpleCalData) {
        ss << "{\"Timestamp\":\"" << calData.timeStamp << "\",";
        ss << "\"Weight\":\"" << weightUnitConvertToDisplay(calData.truckWt) << "\",";
        ss << "\"ZeroedTruckWeight\":\"" << weightUnitConvertToDisplay(calData.zeroedTruckWt) << "\"},";
    }

    // Back up one to remove the tailing comma
    if (!jobMgrTxData_.simpleCalData.empty()) {
        ss.seekp(-1, ss.cur);
    }

    // Close the list and top level object.
    ss << "]}";

    // Return the JSON string.
    return ss.str();
}

/*
 *  FUNCTION: processFileRequest
 *  DESCRIPTION:
 *      Maps a file request to a file request handler.
 *  PARAMETER DESCRIPTION:
 *      req - Request
 *      resp - Response
 *  RETURN VALUE:
 *      true if successful, else false
 */
bool AisJhm2RequestProcessor::processFileRequest(const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    AIS_LOG_INFO("File requested.");

    const Jhm2ParamMsgRequest::ReqDataT& files = req.getReqData(m_FILEREQUESTED_KEY);
    if (files.empty()) {
        AIS_LOG_WARN("No 'fileRequested' argument provided.");
        return false;
    }

    const std::string& str = *(files.begin());

    AIS_LOG_INFO(" file - %s", str.c_str());

    // If there is a handler installed, use it.
    if (fileMsgRequestHandlerMap_.count(str) > 0) {
        FileMsgRequestHandler handler = fileMsgRequestHandlerMap_[str];
        if (nullptr != handler) {
            return (this->*handler)(str, req, resp);
        }
    }

    // Call the default handler.
    AIS_LOG_WARN(" handler not found for %s.", str.c_str());
    return false;
}


/******************************************************************************
FUNCTION: processFileRequestList
DESCRIPTION:
    Populates the file request response message with the response.
PARAMETER DESCRIPTION:
    req - Requested File Name
    resp - File Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processFileRequestList(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    if (listMgrTxReceived_) {
        tes_common_ais::IFlocker ifl;

        // Load the correct list
        if ("TruckList" == file) {
            ifl.open(listMgrTxData_.getTruckListJsonPath());
        }
        else if ("MaterialList" == file) {
            ifl.open(listMgrTxData_.getMaterialListJsonPath());
        }
        else if ("CustomList1" == file) {
            ifl.open(listMgrTxData_.getCustomListJsonPath(LpsSaListMgrInterfaceListType::CUSTOM1));
        }
        else if ("CustomList2" == file) {
            ifl.open(listMgrTxData_.getCustomListJsonPath(LpsSaListMgrInterfaceListType::CUSTOM2));
        }
        else if ("CustomList3" == file) {
            ifl.open(listMgrTxData_.getCustomListJsonPath(LpsSaListMgrInterfaceListType::CUSTOM3));
        }
        else if ("CustomList4" == file) {
            ifl.open(listMgrTxData_.getCustomListJsonPath(LpsSaListMgrInterfaceListType::CUSTOM4));
        }

        if (ifl) {
            // Read the file and print it.
            std::istreambuf_iterator<char> begin(ifl.ifstream()), end;
            std::string contents{ begin, end };
            ifl.close();
            resp.contents(contents);
            AIS_LOG_INFO("Responding with %s JSON.", file.c_str());
            return true;
        }
        else {
            AIS_LOG_ERROR("%s JSON file could not be opened and locked for reading.", file.c_str());
        }
    }
    else {
        AIS_LOG_WARN("%s location not yet known.", file.c_str());
    }
    return false;
}

/******************************************************************************
FUNCTION: processFileRequestDetailedDiagnostics
DESCRIPTION:
    Populates the file request response message with the response.
PARAMETER DESCRIPTION:
    req - Requested File Name
    resp - File Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processFileRequestDetailedDiagnostics(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    tes_common_ais::IFlocker ifl(detailedDiagnosticsFilePath_);
    if (ifl) {
        // Read the file and send it.
        std::istreambuf_iterator<char> begin(ifl.ifstream()), end;
        std::string contents{ begin, end };
        ifl.close();
        resp.contents(contents);
        AIS_LOG_INFO("Responding with detailed diagnostics JSON.");
        return true;
    }
    else {
        resp.contents(R"({"status":"error"})");
        AIS_LOG_ERROR("Detailed Diagnostics JSON file could not be opened and locked for reading.");
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processFileRequestECMSummary
DESCRIPTION:
    Populates the file request response message with the response.
PARAMETER DESCRIPTION:
    req - Requested File Name
    resp - File Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processFileRequestECMSummary(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    tes_common_ais::IFlocker ifl(ecmSummaryFilePath_);
    if (ifl) {
        // Read the file and send it.
        std::istreambuf_iterator<char> begin(ifl.ifstream()), end;
        std::string contents{ begin, end };
        ifl.close();
        resp.contents(contents);
        AIS_LOG_INFO("Responding with ECM Summary JSON.");
        return true;
    }
    else {
        resp.contents(R"({"status":"error"})");
        AIS_LOG_ERROR("ECM Summary JSON file could not be opened or locked for reading.");
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processFileRequestDisplayServiceDashboard
DESCRIPTION:
    Populates the file request response message with the response.
PARAMETER DESCRIPTION:
    req - Requested File Name
    resp - File Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processFileRequestDisplayServiceDashboard(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    tes_common_ais::IFlocker ifl(displayServiceDashboardFilePath_);
    if (ifl) {
        // Read the file and send it.
        std::istreambuf_iterator<char> begin(ifl.ifstream()), end;
        std::string contents{ begin, end };
        ifl.close();
        resp.contents(contents);
        AIS_LOG_INFO("Responding with Display Service Dashboard XML.");
        return true;
    }
    else {
        resp.contents(R"(<Status>Error</Status>)");
        AIS_LOG_ERROR("Display Service Dashboard XML file not available");
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processFileRequestTelematicsServiceDashboard
DESCRIPTION:
    Populates the file request response message with the response.
PARAMETER DESCRIPTION:
    req - Requested File Name
    resp - File Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processFileRequestTelematicsServiceDashboard(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    tes_common_ais::IFlocker ifl(telematicsServiceDashboardFilePath_);
    if (ifl) {
        // Read the file and send it.
        std::istreambuf_iterator<char> begin(ifl.ifstream()), end;
        std::string contents{ begin, end };
        ifl.close();
        resp.contents(contents);
        AIS_LOG_INFO("Responding with Telematics Service Dashboard XML.");
        return true;
    }
    else {
        resp.contents(R"(<Status>Error</Status>)");
        AIS_LOG_ERROR("Telematics Service Dashboard XML file not available");
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processFileRequestUIShowFeatureConfig
DESCRIPTION:
    Populates the file request response message with the response.
PARAMETER DESCRIPTION:
    req - Requested File Name
    resp - File Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processFileRequestUIShowFeatureConfig(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    showHideReady_ = true;   //TODO: temp
    if (showHideReady_) {
        resp.contents(showHideConfig_.getSerializedContent());
        AIS_LOG_INFO("Responding with UI Configurable Features JSON.");
    }
    else {
        // not ready
        resp.contents(R"(<Status>NotReady</Status>)");
    }
    return true;
}

/******************************************************************************
FUNCTION: processParamRequestHeartbeat
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestHeartbeat(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        { // Get Software Group Part Number
            if (partNumbers_.IsSwGroupPartNumSet()) { // If Software Group Part Number Set
                m->addParameter(Jhm2SimpleParam("SoftwareVersion", partNumbers_.GetSwGroupPartNum()));
            }
            else {
                // We don't have a valid value yet, put a non-zero dsi
                // Using DSI 16 - Parameter Not Available
                m->addParameter(Jhm2SimpleParam("SoftwareVersion", 16, ""));
            }
        }

        // Get UI Show Feature config Sha1
        m->addParameter(Jhm2SimpleParam("UIShowFeatureConfigSHA1", showHideConfig_.getSHA1()));

        resp.insertParam(m);

        // Report the display heartbeat
        displayState_.reportHeartbeat();

        { // Get additional parameters
            const std::set<std::string>& qp = req.getReqData("did");
            if (qp.empty()) {
                displayState_.setDocumentId(0);
            }
            else {
                try {
                    int did = stoi(*qp.cbegin());
                    if (did >= 0 && did <= 65503) {
                        displayState_.setDocumentId(static_cast<uint16_t>(did));
                    }
                    else {
                        displayState_.setDocumentId(0);
                    }
                }
                catch (...) {
                    displayState_.setDocumentId(0);
                }
            }
        }

        publishDisplayState_ = true;

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestPayloadInfo
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestPayloadInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        const LpsSaJobMgrTxChannelStorage& jobMgr = jobMgrTxData_;
        const LpsSaWeighTxChannelStorage& weighApp = weighAppTxData_;

        { // Payload Accuracy
            uint_least8_t bucketWeightDisplayAccuracy =
                            getBucketWeightDisplayAccuracy(weighAppTxData_.PayloadCalcMeth, jobMgrTxData_.DispBestBktWt.isOk);
            std::string str = std::to_string(bucketWeightDisplayAccuracy);
            m->addParameter(Jhm2SimpleParam("bucketWeightAccuracy", str));
            //m->addParameter(AisJhm2UintParam("bucketWeightAccuracy", bucketWeightDisplayAccuracy));
        }

        { // Bucket Weight (in tonnes)
            // Don't allow green box if weight is not a number!
            m->addParameter(AisJhm2FloatParam("bucketWeight", weighApp.BestBktWtInTonnes));
        }

        // Warmup Lifts Required, if non-zero, otherwise omit
        if (weighApp.WarmupLiftsRequired)
        {
            m->addParameter(AisJhm2FloatParam("warmupLiftsRequired", weighApp.WarmupLiftsRequired));
        }

        { // Manual Tip Off State
            const std::string& str = std::to_string(jobMgr.ManualTipOffState);
            m->addParameter(Jhm2SimpleParam("manualTipOffState", str));
            //m->addParameter(AisJhm2UintParam("manualTipOffState", jobMgr.ManualTipOffState));
        }

        { // Minus One/Clear State
            const std::string& str = std::to_string(jobMgr.ClearMinusOneEnableStat);
            m->addParameter(Jhm2SimpleParam("minusOneClearState", str));
            //m->addParameter(AisJhm2UintParam("minusOneClearState", jobMgr.ClearMinusOneEnableStat));
        }

        { // Pass Count
            const std::string& str = std::to_string(jobMgr.passCount);
            m->addParameter(Jhm2SimpleParam("passCount", str));
            //m->addParameter(AisJhm2UintParam("passCount", jobMgr.passCount));
        }

        { // Standby State
            if (LPS_SA_JOB_MGR_STANDBY_ACTIVATED == jobMgr.StandbyState) {
                m->addParameter(Jhm2SimpleParam("standbyState", "0"));
            }
            else {
                m->addParameter(Jhm2SimpleParam("standbyState", "1"));
            }
            //m->addParameter(AisJhm2BoolParam("standbyState", LPS_SA_JOB_MGR_STANDBY_ACTIVATED != jobMgr.StandbyState));
        }

        { // Remaining Weight
            m->addParameter(AisJhm2FloatParam("remainingWeight", jobMgr.remainingWeight));
        }

        // show the split weight in UI if we have split totals ie more than 1 subtotal
        if (jobMgr.subtotalCount > 1)
        { // Total Weight (split mode)
            m->addParameter(AisJhm2FloatParam("totalWeight", jobMgr.totalWeight));

            if (legalForTradeInstalled_) {
                std::string str = std::to_string(jobMgr.totalWeightAccuracy);
                m->addParameter(Jhm2SimpleParam("totalWeightAccuracy", str));
            }
        }

        { // Load Record ticketId number
            if ((0xFFFF > printerCnfg_.config.truckTicket.ticketNumber) && legalForTradeInstalled_) {
                const std::string& str = "#" + std::to_string(printerCnfg_.config.truckTicket.ticketNumber);
                m->addParameter(Jhm2SimpleParam("ticketId", str));
            }
            else {
                m->addParameter(Jhm2SimpleParam("ticketId", ""));
            }
        }

        { // Tip Off State
            const std::string& str = std::to_string(jobMgr.TipOffState);
            m->addParameter(Jhm2SimpleParam("tipOffState", str));
            //m->addParameter(AisJhm2UintParam("tipOffState", jobMgr.TipOffState));
        }

        { // Truck Weight
            m->addParameter(AisJhm2FloatParam("truckWeight", jobMgr.truckWeight));

            if (legalForTradeInstalled_) {
                std::string str = std::to_string(jobMgr.truckWeightAccuracy);
                m->addParameter(Jhm2SimpleParam("truckWeightAccuracy", str));
                //m->addParameter(AisJhm2UintParam("truckWeightAccuracy", jobMgr.truckWeightAccuracy));
            }
        }

        { // Bucket Icon
            // Bucket not Racked Icon
            if (!weighApp.BucketFullyRacked) {
                m->addParameter(AisJhm2UintParam("bucketIcon", BucketIcon::NOT_RACKED));
            }
            // Excessive Pitch Icon
            else if (weighApp.ExcessivePitch){
                m->addParameter(AisJhm2UintParam("bucketIcon", BucketIcon::EXCESSIVE_PITCH));
            }
            // In Weigh Range Icon
            else if ((weighApp.LiftPosition.percentCylinderLength >= weighApp.WeighRange.WeighRangeBottom) &&
                        (weighApp.LiftPosition.percentCylinderLength <= (weighApp.WeighRange.WeighRangeBottom + weighApp.WeighRange.WeighRangeSize))) {
                m->addParameter(AisJhm2UintParam("bucketIcon", BucketIcon::IN_WEIGH_RANGE));
            }
            // Otherwise set no Icon
            else {
                m->addParameter(AisJhm2UintParam("bucketIcon", BucketIcon::NONE));
            }
        }

        { // Load Count
            // This is just a number that increments when a load is stored (but not auto-stored)
            // We are just filling in lifetime total load count here to service this purpose.
            const std::string& str = std::to_string(totals_.loadCount);
            m->addParameter(Jhm2SimpleParam("loadCount", str));
            //m->addParameter(AisJhm2UintParam("loadCount", totals_.loadCount));
        }

        { // Task Number
            // This is the current task selected.  For use in multi task mode.
            const std::string& str = std::to_string(static_cast<int>(jobMgr.taskNumber));
            m->addParameter(Jhm2SimpleParam("task", str));
            //m->addParameter(AisJhm2UintParam("task", jobMgr.taskNumber));
        }

        if (jobMgr.truckListEnabled){
            m->addParameter(Jhm2SimpleParam("truckName", jobMgr.truckName));
        }

        if (jobMgr.materialListEnabled) {
            m->addParameter(Jhm2SimpleParam("materialName", jobMgr.materialName));
        }

        { // Custom Tags
            if (jobMgr.tag1Enabled) {
                m->addParameter(Jhm2SimpleParam("tag1", jobMgr.tag1));
            }
            if (jobMgr.tag2Enabled) {
                m->addParameter(Jhm2SimpleParam("tag2", jobMgr.tag2));
            }
            if (jobMgr.tag3Enabled) {
                m->addParameter(Jhm2SimpleParam("tag3", jobMgr.tag3));
            }
            if (jobMgr.tag4Enabled) {
                m->addParameter(Jhm2SimpleParam("tag4", jobMgr.tag4));
            }
        }

        { // Manual Add (+1 button) Available
            const std::string& str = std::to_string(jobMgr.manualAddAvailable);
            m->addParameter(Jhm2SimpleParam("manualAddAvailable", str));
            //m->addParameter(AisJhm2BoolParam("manualAddAvailable", jobMgr.manualAddAvailable));
        }

        { // Zero (->0<- button) Available
            m->addParameter(AisJhm2BoolParam("zeroAvailable", weighApp.zeroAvailable));
        }

        { // LFT Enabled state (only show if LFT is installed and multitask is enabled)

            if (jobMgr.multiTaskEnabled && legalForTradeInstalled_) {
                m->addParameter(AisJhm2BoolParam("lftEnabled", !jobMgr.lftDisabled));
            }
        }

        { // Current load target type
            const std::string& str = std::to_string(static_cast<int>(jobMgr.targetType));
            m->addParameter(Jhm2SimpleParam("targetType", str));
        }

        { // Target passes if blend(loads) target
            if ((LpsSaLoadRecordTargetType)jobMgr.targetType == LpsSaLoadRecordTargetType::BLEND_LOADS) {
                const std::string& str = std::to_string(static_cast<int>(jobMgr.targetPasses));
                m->addParameter(Jhm2SimpleParam("targetPasses", str));
            }
        }

        { // Selected step number
            const std::string& str = std::to_string(static_cast<int>(jobMgr.stepNumber));
            m->addParameter(Jhm2SimpleParam("stepNumber", str));
        }

        if (jobMgr.iconType) { // Current IconType, add only if not 0 (default)
            const std::string& str = std::to_string(static_cast<int>(jobMgr.iconType));
            m->addParameter(Jhm2SimpleParam("iconType", str));
        }

        resp.insertParam(m);

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestRemainingWt
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestRemainingWt(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    std::string remainingWeight = weightUnitConvertToDisplay(jobMgrTxData_.remainingWeight);
    resp.insertParam(new Jhm2SimpleParam(paramName, remainingWeight));
    return true;
}

/******************************************************************************
FUNCTION: processParamRequestTargetWeights
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestTargetWeights(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        //float nan_val = std::numeric_limits<double>::quiet_NaN();
        //m->addParameter(AisJhm2FloatArrayParam("frequent", {5.1f, 10.01f, 15.0, nan_val, 20.0 }));
        std::vector<float> vec = targetWeights_.getTargetWeightsVector();
        m->addParameter(AisJhm2FloatArrayParam("frequent", vec));
        resp.insertParam(m);
        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestMaterialInfo
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestMaterialInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {

        /*
         * Ideally, we would serialize numbers as numbers, but the underlying Jhm2 crap converts everything
         * to strings.  I would rather have control over precision than let Jhm2 pick a fixed implementation
         * for every number, so I am converting things to strings here.
         */

        m->addParameter(Jhm2SimpleParam("enabled", std::to_string(jobMgrTxData_.materialListEnabled)));// Material List Enabled status

        { // Material Id
            const std::string& str = std::to_string(jobMgrTxData_.materialId);
            m->addParameter(Jhm2SimpleParam("id", str));
        }

        { // Material Name
            const std::string& str = jobMgrTxData_.materialName;
            m->addParameter(Jhm2SimpleParam("name", str));
        }

        { // Material Density
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << jobMgrTxData_.materialDensity;
            const std::string& str = ss.str();
            m->addParameter(Jhm2SimpleParam("density", str));
        }

        resp.insertParam(m);

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequest PWM inputs
DESCRIPTION:Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:paramName - Requested Parameter Name
                      resp - Parameter Request Response Message
RETURN VALUE:true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestDemoAppInputs(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        { // Lift Position
            const std::string& str = std::to_string(m_scsInputs.m_jhm_demo.liftposition);
            m->addParameter(Jhm2SimpleParam("Lift_Position", str));
        }
        { // Lift Position Rate
            const std::string& str = std::to_string(m_scsInputs.m_jhm_demo.liftposition_rate);
            m->addParameter(Jhm2SimpleParam("Lift_Position_Rate", str));
        }
        { // Tilt Position
            const std::string& str = std::to_string(m_scsInputs.m_jhm_demo.tiltposition);
            m->addParameter(Jhm2SimpleParam("Tilt_Position", str));
        }
        { // Tilt Position rate
            const std::string& str = std::to_string(m_scsInputs.m_jhm_demo.tiltposition_rate);
            m->addParameter(Jhm2SimpleParam("Tilt_Position_Rate", str));
        }
        { // Payload
            const std::string& str = std::to_string(m_scsInputs.m_jhm_demo.payload);
            m->addParameter(Jhm2SimpleParam("Payload", str));
        }
        { // Payload rate
            const std::string& str = std::to_string(m_scsInputs.m_jhm_demo.payload_rate);
            m->addParameter(Jhm2SimpleParam("Payload_rate", str));
        }
        {
            const std::string& str = std::to_string(weighAppTxData_.LiftPosition.angle);
            m->addParameter(Jhm2SimpleParam("Lift_Angle", str));
        }
        {
            const std::string & str = std::to_string(weighAppTxData_.TiltPosition.bucketAngle);
            m->addParameter(Jhm2SimpleParam("Tilt_Angle", str));
        }
        demo_mode_exit_time = std::chrono::steady_clock::now() + std::chrono::seconds(30);
        resp.insertParam(m);
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processParamRequestTruckInfo
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestTruckInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {

        /*
         * Ideally, we would serialize numbers as numbers, but the underlying Jhm2 crap converts everything
         * to strings.  I would rather have control over precision than let Jhm2 pick a fixed implementation
         * for every number, so I am converting things to strings here.
         */

        m->addParameter(Jhm2SimpleParam("enabled", std::to_string(jobMgrTxData_.truckListEnabled))); // Truck List Enabled status

        { // Truck Id
            const std::string& str = std::to_string(jobMgrTxData_.truckId);
            m->addParameter(Jhm2SimpleParam("id", str));
            AIS_LOG_DEBUG("Truck Id: '%s'", str.c_str());
        }

        { // Truck Name
            const std::string& str = jobMgrTxData_.truckName;
            m->addParameter(Jhm2SimpleParam("name", str));
            AIS_LOG_DEBUG("Truck Name: '%s'", str.c_str());
        }

        { // Truck Target Weight
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << jobMgrTxData_.truckTargetWeight;
            const std::string& str = ss.str();
            m->addParameter(Jhm2SimpleParam("targetWeight", str));
            AIS_LOG_DEBUG("Truck Target Weight: '%s'", str.c_str());
        }

        resp.insertParam(m);

        return true;
    }

    return false;
}


/******************************************************************************
FUNCTION: processParamRequestPrinterCnfg
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestPrinterCnfg(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        if (printerCnfgReceivedTime_ > TimePoint::min()) {
            if (!printerCnfg_.config.installed) {
                m->addParameter(Jhm2SimpleParam("installed", "false"));
                m->addParameter(Jhm2SimpleParam("numCopies", 20 /* DSI */, "0"));
                AIS_LOG_DEBUG("Printer Not Installed");
            }
            else {
                // Installed
                m->addParameter(Jhm2SimpleParam("installed", "true"));

                { // Number of Copies
                    const std::string& str = std::to_string(printerCnfg_.config.truckTicket.numCopies);
                    m->addParameter(Jhm2SimpleParam("numCopies", str));
                    AIS_LOG_DEBUG("Printer Config Number of Copies: '%s'", str.c_str());
                }

                { // Leading Blank Lines
                    const std::string& str = std::to_string(printerCnfg_.config.leadingBlankLines);
                    m->addParameter(Jhm2SimpleParam("leadingBlankLines", str));
                }

                // Header Lines
                m->addParameter(Jhm2SimpleParam("headerLine1", printerCnfg_.config.truckTicket.headerLine1));
                m->addParameter(Jhm2SimpleParam("headerLine2", printerCnfg_.config.truckTicket.headerLine2));
                m->addParameter(Jhm2SimpleParam("headerLine3", printerCnfg_.config.truckTicket.headerLine3));

                // Footer Lines
                m->addParameter(Jhm2SimpleParam("footerLine1", printerCnfg_.config.truckTicket.footerLine1));

                { // Trailing Blank Lines
                    const std::string& str = std::to_string(printerCnfg_.config.trailingBlankLines);
                    m->addParameter(Jhm2SimpleParam("trailingBlankLines", str));
                }

                { // Ticket Number
                    const std::string& str = std::to_string(printerCnfg_.config.truckTicket.ticketNumber);
                    m->addParameter(Jhm2SimpleParam("ticketNumber", str));
                }

                { // Date Format
                    const std::string& str = std::to_string(static_cast<uint16_t>(printerCnfg_.config.dateFormat));
                    m->addParameter(Jhm2SimpleParam("dateFormat", str));
                }

                { // Date Separator
                    const std::string& str = std::to_string(static_cast<uint16_t>(printerCnfg_.config.dateSeparator));
                    m->addParameter(Jhm2SimpleParam("dateSeperator", str)); // Spelling is wrong, but UI depends on it.
                }
            }
        }
        else {
            { // Number of Copies
                m->addParameter(Jhm2SimpleParam("installed", 19 /* DSI */, "false"));
                m->addParameter(Jhm2SimpleParam("numCopies", 19 /* DSI */, "0"));
                AIS_LOG_WARN("Printer Config Unknown");
            }
        }

        resp.insertParam(m);
        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestUDPTransferDetails
DESCRIPTION:Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:paramName - Requested Parameter Name, resp - Parameter Request Response Message
RETURN VALUE:true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestUDPTransferDetails(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        m->addParameter(Jhm2SimpleParam("udpTransferIpAddress", printerCnfg_.config.udpTransferIp));
        m->addParameter(Jhm2SimpleParam("udpTransferPort", printerCnfg_.config.udpTransferPort));
        m->addParameter(Jhm2SimpleParam("udpTransferEnabled", std::to_string(printerCnfg_.config.udpTransferEnabled)));
        resp.insertParam(m);
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processParamRequestPayloadLevel2TempInstallCnfg
DESCRIPTION:Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:paramName - Requested Parameter Name, resp - Parameter Request Response Message
RETURN VALUE:true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestPayloadLevel2TempInstallCnfg(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        if (seaStatusLevel2_.status == SCL_PRMSW_FEAT_STATUS_FREE_USE_INSTALLED) {
            m->addParameter(Jhm2SimpleParam("installed", true));
            m->addParameter(Jhm2SimpleParam("remainingMinutes",seaStatusLevel2_.free_count ));
        }
        else {
            m->addParameter(Jhm2SimpleParam("installed", false));
            m->addParameter(Jhm2SimpleParam("remainingMinutes",seaStatusLevel2_.free_count ));
        }

        resp.insertParam(m);
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processParamRequestDisplaySettings
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestDisplaySettings(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    if ("DisplaySettings" == paramName) {
        AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
        if (nullptr != m) {
            m->addParameter(Jhm2SimpleParam("units", std::to_string(LpsSaUIDisplaySettingsUnits_Base_t(displayState_.getSettings().units))));
            m->addParameter(Jhm2SimpleParam("language", std::to_string(LpsSaUIDisplaySettingsLanguage_Base_t(displayState_.getSettings().language))));
            m->addParameter(Jhm2SimpleParam("brightness", std::to_string(displayState_.getSettings().brightness)));
            m->addParameter(Jhm2SimpleParam("timeFormat", std::to_string(LpsSaUIDisplaySettingsTimeFormat_Base_t(displayState_.getSettings().timeFormat))));
            m->addParameter(Jhm2SimpleParam("dateFormat", std::to_string(LpsSaUIDisplaySettingsDateFormat_Base_t(displayState_.getSettings().dateFormat))));
            m->addParameter(Jhm2SimpleParam("weightUnits", std::to_string(LpsCommonWeightUnits_Base_t(displayState_.getSettings().weightUnits))));
            m->addParameter(Jhm2SimpleParam("weightPrecision", std::to_string(LpsCommonWeightPrecision_Base_t(displayState_.getSettings().weightPrecision))));
            m->addParameter(Jhm2SimpleParam("weightDecimalPrecision", std::to_string(displayState_.getWeightDecimalPrecision())));
            { // weightInterval
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << displayState_.getWeightInterval();
                const std::string& str = ss.str();
                m->addParameter(Jhm2SimpleParam("weightInterval", str));
            }
            resp.insertParam(m);
            return true;
        }
    }
    else if ("DisplayBrightness_Rd" == paramName) {
        Jhm2SimpleParam* p = new Jhm2SimpleParam(paramName, std::to_string(displayState_.getSettings().brightness));
        if (nullptr != p) {
            resp.insertParam(p);
            return true;
        }
    }
    else if ("Units_Rd" == paramName) {
        Jhm2SimpleParam* p = new Jhm2SimpleParam(paramName, std::to_string(LpsSaUIDisplaySettingsUnits_Base_t(displayState_.getSettings().units)));
        if (nullptr != p) {
            resp.insertParam(p);
            return true;
        }
    }
    else if ("Language_Rd" == paramName) {
        Jhm2SimpleParam* p = new Jhm2SimpleParam(paramName, std::to_string(LpsSaUIDisplaySettingsLanguage_Base_t(displayState_.getSettings().language)));
        if (nullptr != p) {
            resp.insertParam(p);
            return true;
        }
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestServiceParameters
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestServiceParameters(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    if ("ServiceParameters" == paramName) {
        AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
        if (nullptr != m) {
            { // hydOilTemp
                if (!weighAppTxData_.PidData.HydOilTempEnabled) {
                    // omit, since it is disabled
                }
                else if((int_16)weighAppTxData_.PidData.ProdMeasureSensorStatus.HydOilTemp == -32736)
                {
                    /* faulted, set fmi */
                    m->addParameter(AisJhm2FloatParam("hydOilTemp", FMICNM, 0.f));
                }
                else
                {
                    m->addParameter(AisJhm2FloatParam("hydOilTemp", weighAppTxData_.PidData.ProdMeasureSensorStatus.HydOilTemp));
                }
            }

            { // liftCylPosPct, liftCylExt, liftLinkPosSensorDC
                if((uint16_t)weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylPos == UNKNOWN2U + FMICNM)
                {
                    /* faulted, set fmi */
                    m->addParameter(AisJhm2FloatParam("liftCylPosPct", FMICNM, 0.f));
                    m->addParameter(AisJhm2FloatParam("liftCylExt", FMICNM, 0.f));
                    m->addParameter(AisJhm2FloatParam("liftLinkPosSensorDC", FMICNM, 0.f));
                }
                else
                {
                    auto liftCylExt = weighAppTxData_.LiftPosition.cylinderExtension < 0.0f ? 0:weighAppTxData_.LiftPosition.cylinderExtension;
                    m->addParameter(AisJhm2FloatParam("liftCylPosPct", weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylPos));
                    m->addParameter(AisJhm2FloatParam("liftCylExt", liftCylExt));
                    m->addParameter(AisJhm2FloatParam("liftLinkPosSensorDC", weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftLinkDC));
                }
            }

            { // tiltCylExt, bucketAngle, tiltLinkPosSensorDC
                if(weighAppTxData_.TiltPosition.status == LPS_STATUS_BAD)
                {
                    /* faulted, set fmi */
                    m->addParameter(AisJhm2FloatParam("tiltCylExt", FMICNM, 0.f));
                    m->addParameter(AisJhm2FloatParam("bucketAngle", FMICNM, 0.f));
                    m->addParameter(AisJhm2FloatParam("tiltLinkPosSensorDC", FMICNM, 0.f));
                }
                else
                {
                    auto tiltCylExt = weighAppTxData_.TiltPosition.cylinderExtension < 0 ? 0:weighAppTxData_.TiltPosition.cylinderExtension;
                    m->addParameter(AisJhm2FloatParam("tiltCylExt", tiltCylExt));
                    m->addParameter(AisJhm2FloatParam("bucketAngle", weighAppTxData_.TiltPosition.bucketAngle));
                    m->addParameter(AisJhm2FloatParam("tiltLinkPosSensorDC", weighAppTxData_.PidData.ProdMeasureSensorStatus.TiltLinkDC));
                }
            }

            { // liftCylHEPress
                if((uint16_t)weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylHEPres == UNKNOWN2U + FMICNM)
                {
                    /* faulted, set fmi */
                    m->addParameter(AisJhm2FloatParam("liftCylHEPress", FMICNM, 0.f));
                }
                else
                {
                    m->addParameter(AisJhm2FloatParam("liftCylHEPress", weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylHEPres));
                }
            }

            { // liftCylREPress
                if((uint16_t)weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylREPres == UNKNOWN2U + FMICNM)
                {
                    /* faulted, set fmi */
                    m->addParameter(AisJhm2FloatParam("liftCylREPress", FMICNM, 0.f));
                }
                else
                {
                    m->addParameter(AisJhm2FloatParam("liftCylREPress", weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylREPres));
                }
            }

            { // tiltCylHEPress
                if((uint16_t)weighAppTxData_.PidData.ProdMeasureSensorStatus.TiltCylHEPres == UNKNOWN2U + FMICNM)
                {
                    /* faulted, set fmi */
                    m->addParameter(AisJhm2FloatParam("tiltCylHEPress", FMICNM, 0.f));
                }
                else
                {
                    m->addParameter(AisJhm2FloatParam("tiltCylHEPress", weighAppTxData_.PidData.ProdMeasureSensorStatus.TiltCylHEPres));
                }
            }

            { // tiltCylREPress (only available on AU2020)
                if (!showHideConfig_.get<bool>("AutoPayloadCalibrations.supported")) {
                    // we are not next-gen, so tiltCylREPress is not supported
                    // omit if not supported
                }
                else if((uint16_t)weighAppTxData_.PidData.ProdMeasureSensorStatus.TiltCylREPres == UNKNOWN2U + FMICNM)
                {
                    /* faulted, set fmi */
                    m->addParameter(AisJhm2FloatParam("tiltCylREPress", FMICNM, 0.f));
                }
                else
                {
                    m->addParameter(AisJhm2FloatParam("tiltCylREPress", weighAppTxData_.PidData.ProdMeasureSensorStatus.TiltCylREPres));
                }
            }

            { //  (only available on AU2020 and LFT)
                if (!showHideConfig_.get<bool>("AutoPayloadCalibrations.supported") && (!legalForTradeInstalled_)) {
                    // we are not next-gen and also not LFT, so imu is not supported
                    // omit if not supported
                }
                else if((int16_t)weighAppTxData_.PidData.MachinePitch == UNKNOWN2S + FMICNM)
                {
                    /* faulted, set fmi */
                    m->addParameter(AisJhm2FloatParam("machinePitch", FMICNM, 0.f));
                    m->addParameter(AisJhm2FloatParam("machineRoll", FMICNM, 0.f));
                }
                else
                {
                    m->addParameter(AisJhm2FloatParam("machinePitch", (float)weighAppTxData_.PidData.MachinePitch / 10.f));
                    m->addParameter(AisJhm2FloatParam("machineRoll", (float)weighAppTxData_.PidData.MachineRoll / 100.f));
                }
            }

            /* Test software for reading internal ECM temperature
            { // Putting ECM temperature in here for test purposes
                FILE* fh = fopen("/sys/class/tty/ttyS2/adc/adc_temp_sns", "r");
                if (nullptr != fh) {
                    int rawValue;
                    if (0 == fscanf(fh, "%d", &rawValue)) {
                        AIS_LOG_ERROR("Failed to read /sys/class/tty/ttyS2/adc/adc_temp_sns");
                        m->addParameter(AisJhm2FloatParam("ecmTemp", FMICNM, 0.f));
                    }
                    else {
                        // volt is the temperature in Kelvin * 10 ,
                        float ecmTemp = ((float)rawValue - 2731.5f) / 10.f;
                        m->addParameter(AisJhm2FloatParam("ecmTemp", ecmTemp));
                    }
                    fclose(fh);
                }
            }
            */

            resp.insertParam(m);
            return true;
        }
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestKeyboardLayoutSetting
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestKeyboardLayoutSetting(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    if ("KeyboardLayoutSetting" == paramName) {
        Jhm2SimpleParam* p = new Jhm2SimpleParam(paramName, std::to_string(LpsSaUIKeyboardLayoutSetting_Base_t(displayState_.getSettings().keyboardLayoutSetting)));
        if (nullptr != p) {
            resp.insertParam(p);
            return true;
        }
    }

    return false;
}


/******************************************************************************
FUNCTION: processParamRequestDisplaySettings
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestPayloadSettings(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {

    if ("PayloadSettings" == paramName) {
        if ((LpsSaLoadRecordTargetType)jobMgrTxData_.targetType != LpsSaLoadRecordTargetType::SINGLE) {
        	jobMgrTxData_.AutoTruckIdEnabled = false;
        	jobMgrTxData_.AutoMaterialIdEnabled = false;
    	}

    	AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
        if (nullptr != m) {
            m->addParameter(Jhm2SimpleParam("tipOffMode", std::to_string(jobMgrTxData_.TipOffStateCfg)));
            m->addParameter(Jhm2SimpleParam("tipOffTriggerType", std::to_string(jobMgrTxData_.TipOffTriggerType)));
            m->addParameter(Jhm2SimpleParam("hornStoreEnable", std::to_string(jobMgrTxData_.ReadHornStoreEnable())));
            m->addParameter(Jhm2SimpleParam("autoStorePassCount", std::to_string(jobMgrTxData_.AutoStorePassCount)));
            m->addParameter(Jhm2SimpleParam("autoTruckIdEnabled", std::to_string(jobMgrTxData_.AutoTruckIdEnabled)));
            m->addParameter(Jhm2SimpleParam("autoMaterialIdEnabled", std::to_string(jobMgrTxData_.AutoMaterialIdEnabled)));
            m->addParameter(Jhm2SimpleParam("manualAddEnabled", std::to_string(jobMgrTxData_.manualAddEnabled)));
            m->addParameter(Jhm2SimpleParam("multiTaskEnabled", std::to_string(jobMgrTxData_.multiTaskEnabled)));
            m->addParameter(AisJhm2UintParam("multiTaskCount", jobMgrTxData_.multiTaskCount));
            m->addParameter(Jhm2SimpleParam("splitModeEnabled", std::to_string(jobMgrTxData_.splitModeEnabled)));
            m->addParameter(Jhm2SimpleParam("hydOilTempEnabled", std::to_string(weighAppTxData_.PidData.HydOilTempEnabled)));
            m->addParameter(Jhm2SimpleParam("audibleWeightEnabled", std::to_string(weighAppTxData_.PidData.AudibleWeightEnabled)));
            m->addParameter(AisJhm2FloatParam("weighRangeStart", weighAppTxData_.WeighRange.WeighRangeBottom));
            m->addParameter(AisJhm2FloatParam("weighRangeEnd", weighAppTxData_.WeighRange.WeighRangeBottom + weighAppTxData_.WeighRange.WeighRangeSize));
            resp.insertParam(m);
            return true;
        }
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestServiceMode
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestServiceMode(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    if ("ServiceMode_Rd" == paramName) {
        Jhm2SimpleParam* p = new Jhm2SimpleParam(paramName, std::to_string(displayState_.isInServiceMode()));
        if (nullptr != p) {
            resp.insertParam(p);
            return true;
        }
    }
    else if ("ServicePassword_Rd" == paramName) {
        Jhm2SimpleParam* p = new Jhm2SimpleParam(paramName, std::to_string(displayState_.isServiceModeEnableCodeEntered()));
        if (nullptr != p) {
            resp.insertParam(p);
            return true;
        }
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestLocalTime
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestLocalTime(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    auto now = std::chrono::system_clock::now();
    std::string str = tes_common_ais::putLocalTime(now, tes_common_ais::date_time_formats::ISO_DATE_TIME_LOCAL);
    AIS_LOG_INFO("LocalTime: %s", str.c_str());
    Jhm2SimpleParam* p = new Jhm2SimpleParam("LocalTime", str);
    if (nullptr != p) {
        resp.insertParam(p);
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processParamRequestCalibrationData
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestCalibrationData(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    if ("CalibrationData" == paramName) {
        AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
        if (nullptr != m) {
            CalibrationResponseUI calResponse;
            calUIClientInf_.getLastResponse(calResponse);

            if (0 != calResponse.stepno) {
                // Send No Command to keep inter-lock happy if we are doing calibration
                CalibrationRequestUI calRequest;
                calRequest.CalibParam.CalId = calResponse.CalId;
                calRequest.CalibParam.ControlBits = 0x80; // Calibration Enabled
                calRequest.CalibParam.InputAction = 0x00; // No Command
                calRequest.CalibParam.InputActionCommand = 0x00;
                calRequest.CalibParam.CurrStep = calResponse.stepno;
                calUIClientInf_.sendRequest(calRequest);
            }

            m->addParameter(Jhm2SimpleParam("CALID_Rd", calResponse.CalId));
            m->addParameter(Jhm2SimpleParam("CALSTATUS_Rd", calResponse.statusParams.Status));
            m->addParameter(Jhm2SimpleParam("CALSTEP_Rd", calResponse.stepno));
            { // Get list of errors/warnings
                std::stringstream result;
                result << "[";
                if (calResponse.activeWarningErrorVector.size() > 0) {
                    std::copy(calResponse.activeWarningErrorVector.begin(),
                            calResponse.activeWarningErrorVector.end(),
                            std::ostream_iterator<int>(result, ","));
                    result.seekp(-1, std::ios::cur); // Back up one to get rid of the trailing ","
                }
                result << "]";
                m->addParameter(Jhm2SimpleParam("CALERROR_Rd", result.str()));
            }
            m->addParameter(Jhm2SimpleParam("CAL_QR_Min_HydOilTemp_celsius_Rd", weighAppTxData_.PidData.QR_HydOilTempMin_C));
            m->addParameter(Jhm2SimpleParam("CAL_QR_Min_LiftCylVelocity_mm_sec_Rd", weighAppTxData_.PidData.QR_LiftCylVelMin_mm_sec));
            m->addParameter(Jhm2SimpleParam("CAL_QR_Max_LiftCylVelocity_mm_sec_Rd", weighAppTxData_.PidData.QR_LiftCylVelMax_mm_sec));
            m->addParameter(Jhm2SimpleParam("LiftCylinderVelMilliPerSec_Rd", weighAppTxData_.LiftCylVel.Val));
            m->addParameter(Jhm2SimpleParam("LiftCylinderPosNormalized_Rd", weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylPos));
            m->addParameter(Jhm2SimpleParam("LiftCylinderHEPres_Rd", weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylHEPres));
            m->addParameter(Jhm2SimpleParam("LiftCylinderREPres_Rd", weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftCylREPres));

            /* hydraulic oil temp */
            if((int_16)weighAppTxData_.PidData.ProdMeasureSensorStatus.HydOilTemp == -32736)
            {
                /* faulted, set fmi */
                m->addParameter(Jhm2SimpleParam("HydOilTempC_Rd", FMICNM, weighAppTxData_.PidData.ProdMeasureSensorStatus.HydOilTemp));
            }
            else
            {
                m->addParameter(Jhm2SimpleParam("HydOilTempC_Rd", weighAppTxData_.PidData.ProdMeasureSensorStatus.HydOilTemp));
            }

            m->addParameter(Jhm2SimpleParam("LiftLinkageDC_Rd", weighAppTxData_.PidData.ProdMeasureSensorStatus.LiftLinkDC));
            m->addParameter(Jhm2SimpleParam("TiltLinkageDC_Rd", weighAppTxData_.PidData.ProdMeasureSensorStatus.TiltLinkDC));

            /* Engine Speed */
            if(IS_KNOWN2S(weighAppTxData_.EngineSpeedRPM))
            {
                m->addParameter(Jhm2SimpleParam("EngineSpeedRPM_Rd", weighAppTxData_.EngineSpeedRPM));
            }
            else
            {
                /* faulted, set fmi */
                uint8_t fmi = weighAppTxData_.EngineSpeedRPM - UNKNOWN2S;
                m->addParameter(Jhm2SimpleParam("EngineSpeedRPM_Rd", fmi, weighAppTxData_.EngineSpeedRPM));
            }

            // added for TTL calibration of In-Line Tilt Sensor, CalId 350
            m->addParameter(Jhm2SimpleParam("LiftCylExtAbsolute_Rd", weighAppTxData_.LiftPosition.cylinderExtension));
            m->addParameter(Jhm2SimpleParam("TiltPosAngle_Rd", weighAppTxData_.TiltPosition.angle));

            resp.insertParam(m);
            return true;
        }
    }
    return false;
}

/******************************************************************************
FUNCTION:                   processRequest for Operator ID
DESCRIPTION:                Extract the Response String from HTTP response and call the processCmdRequest with argument of Response String
PARAMETER DESCRIPTION:
RETURN VALUE:               bool
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestOperatorId(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp){
    // Read Operator ID from storage
    if (operatorIdCsv_ && listMgrTxReceived_) {
        operatorIdListMgr_.setStoragePath(listMgrTxData_.basePath);
    }
    auto operatorIdStatus = operatorIdListMgr_.getOperatorIdStatus();

    // Create JSON response
    auto operatorIdParam = new AisJhm2MultiParam(paramName);
    operatorIdParam->addParameter(Jhm2SimpleParam("name", operatorIdStatus.operatorName));
    operatorIdParam->addParameter(Jhm2SimpleParam("state", operatorIdStatus.getStateString()));
    operatorIdParam->addParameter(Jhm2SimpleParam("numOfRecords", operatorIdStatus.numRecords));
    resp.insertParam(operatorIdParam);

    // Publish Operator ID over DataLinkData
    if (operatorIdStatus.state == OperatorIdState::Valid) {
        DataLinkParam dlParam;
        dlParam.SetParamId(SecuritySystemCurrentSecurityIDParam::CatExtId);
        dlParam.SetParamIdentifierType(DataLinkParamInfo::DATA_LINK_PARAM_IDENTIFIER_CAT_EXT);
        dlParam.SetSid(0x00);
        dlParam.SetVarLengthParamValue(nullptr, 0, 0); // Set DSI without changing LastValue
        dlParam.SetVarLengthParamType(VarLengthDataLinkParamPool::CAT_EXT);

        { // Fill in the data
            SecuritySystemCurrentSecurityIDParam idParam(dlParam);
            idParam.setID(operatorIdStatus.currentPasscode);
            idParam.setIDDescription(operatorIdStatus.operatorName);
        }

        dlData_.UpdateDataLinkParam(dlParam);
        dataLinkDataOutput_->publish(dlData_);
        dlData_.HandleDataLinkDataMap();
    }
    return true;
}


/******************************************************************************
FUNCTION: processParamRequestCustomListSettings
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestCustomListSettings(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        m->addParameter(Jhm2SimpleParam("truckListEnabled", std::to_string(jobMgrTxData_.truckListEnabled)));
        m->addParameter(Jhm2SimpleParam("materialListEnabled", std::to_string(jobMgrTxData_.materialListEnabled)));
        m->addParameter(Jhm2SimpleParam("list1Enabled", std::to_string(jobMgrTxData_.tag1Enabled)));
        m->addParameter(Jhm2SimpleParam("list1Name", listMgrTxData_.customList1Name));
        m->addParameter(Jhm2SimpleParam("list2Enabled", std::to_string(jobMgrTxData_.tag2Enabled)));
        m->addParameter(Jhm2SimpleParam("list2Name", listMgrTxData_.customList2Name));
        m->addParameter(Jhm2SimpleParam("list3Enabled", std::to_string(jobMgrTxData_.tag3Enabled)));
        m->addParameter(Jhm2SimpleParam("list3Name", listMgrTxData_.customList3Name));
        m->addParameter(Jhm2SimpleParam("list4Enabled", std::to_string(jobMgrTxData_.tag4Enabled)));
        m->addParameter(Jhm2SimpleParam("list4Name", listMgrTxData_.customList4Name));
        resp.insertParam(m);
        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestCustomInfo
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestCustomInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        if (jobMgrTxData_.tag1Enabled) {
            m->addParameter(Jhm2SimpleParam("tag1", jobMgrTxData_.tag1));
        }
        if (jobMgrTxData_.tag2Enabled) {
            m->addParameter(Jhm2SimpleParam("tag2", jobMgrTxData_.tag2));
        }
        if (jobMgrTxData_.tag3Enabled) {
            m->addParameter(Jhm2SimpleParam("tag3", jobMgrTxData_.tag3));
        }
        if (jobMgrTxData_.tag4Enabled) {
            m->addParameter(Jhm2SimpleParam("tag4", jobMgrTxData_.tag4));
        }
        resp.insertParam(m);
        return true;
    }

    return false;
}

/*
 * Function: processParamRequestTestStatus
 * Purpose: Provide update of test status.
 */
bool AisJhm2RequestProcessor::processParamRequestTestStatus(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp)
{
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);

    if (nullptr != m) {
        switch (weighAppTxData_.TestStatus) {
        case (LpsSaWeighTxChannel::TestStatus_t::INACTIVE): {
            m->addParameter(Jhm2SimpleParam("state", "Inactive"));
            break;
        }
        case (LpsSaWeighTxChannel::TestStatus_t::TESTING): {
            m->addParameter(Jhm2SimpleParam("state", "Testing"));
            break;
        }
        case (LpsSaWeighTxChannel::TestStatus_t::RECORDING): {
            m->addParameter(Jhm2SimpleParam("state", "Recording"));
            break;
        }
        case (LpsSaWeighTxChannel::TestStatus_t::FAILED): {
            m->addParameter(Jhm2SimpleParam("state", "Failed"));
            break;
        }
        default: {
            m->addParameter(Jhm2SimpleParam("state", "Unknown"));
            break;
        }
        }
        resp.insertParam(m);
        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION:                   processRequest
DESCRIPTION:                Extract the Response String from HTTP response and call the processCmdRequest with argument of Response String
PARAMETER DESCRIPTION:
RETURN VALUE:               bool  
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequest(const Jhm2ParamMsgRequest& req, Jhm2CmdResponseMsg& resp) {
    const Jhm2ParamMsgRequest::ReqDataT& cmdsRequested = req.getReqData(m_COMMANDREQUESTED_KEY);
    if (cmdsRequested.empty()) {
        resp.insertParam(new Jhm2SimpleParam("CommandStatus", "CannotExecute"));
        resp.insertParam(new Jhm2SimpleParam("CommandFailDetails", "No Command Given"));
        return true; // Return true to send the error response.
    }

    return processCmdRequest(*(cmdsRequested.begin()), resp);
}

/******************************************************************************
FUNCTION:                   processCmdRequest
DESCRIPTION:                according to the UI response publish the data via SCS object or store in NVM  
PARAMETER DESCRIPTION:
RETURN VALUE:                     
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequest(const std::string& cmd, Jhm2CmdResponseMsg& resp) {
    bool commandHandled = false;

    m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_status = 1;

    std::vector< std::pair<std::string, float> > calib;
    std::vector<std::pair<std::string,std::string>> calibstr;

    boost::property_tree::ptree pt;
    std::stringstream ss(cmd.c_str());
    try {
        boost::property_tree::read_json(ss, pt);
    }
    catch (const boost::property_tree::json_parser::json_parser_error& e) {
        AIS_LOG_ERROR("JSON Cmd Parsing Error %s", e.what());
        resp.insertParam( new Jhm2SimpleParam( "CommandStatus", "CannotExecute" ) );
        resp.insertParam( new Jhm2SimpleParam( "CommandFailDetails", "Invalid JSON body" ) );
        return true; // Return true to send the error response.
    }

    if (!pt.empty() && pt.data().empty()) {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt) {
            if (!v.first.empty()) {
                const std::string& command = v.first;

                // See if we have a specific handler installed for it.
                if (cmdMsgRequestHandlerMap_.count(command) > 0) {
                    CmdMsgRequestHandler handler = cmdMsgRequestHandlerMap_[command];
                    if (nullptr != handler) {
                        AIS_LOG_NOTICE("command: %s", command.c_str());
                        (this->*handler)(command, v.second, resp);
                        commandHandled = true;
                    }
                }
                else {
                    // Do the old thing...
                    std::string convstr = v.second.data();

                    if ((command == "MM_Selected_MSN_Wr") || (command == "SimpleCalAdjWeight_Wr") ||
                            (command == "SimpleCalTimestamp_Wr" ) || (command == "SimpleCalZeroWeight_Wr" ) ||
                            (command == "CALID_Wr") || (command == "CALCONTROL_Wr") || (command == "CALINPUT_Wr") ||
                            (command == "CALCOMMAND_Wr") || (command == "CALSTEP_Wr")) {
                        AIS_LOG_NOTICE("command: %s %s", command.c_str(), convstr.c_str());
                        calibstr.push_back(std::make_pair(command, convstr));
                    }
                    else {
                        float calibVal = atof(convstr.c_str());
                        AIS_LOG_NOTICE("command: %s %f", command.c_str(), calibVal);
                        calib.push_back(std::make_pair(command, calibVal));
                    }
                }
            }
        }
    }

    // The code below this expects at least something to be at calibstr[0]
    if (calibstr.empty()) {
        calibstr={{"",""}};
    }

    // The code below this expects at least something to be at calib[0]
    if (calib.empty()) {
        calib = {{"",0.0}};
    }

   
    if ((calibstr[0].first == "SimpleCalAdjWeight_Wr") || (calibstr[0].first == "SimpleCalTimestamp_Wr") ||
            (calibstr[0].first == "SimpleCalZeroWeight_Wr")) {
        if (!(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            AisJhm2TxChannel reqAisJhm2TxChannel;

            // simplecal timestamp publish through aisJhm2Txchannel
            reqAisJhm2TxChannel.simplecal_data.timeStamp = calibstr[0].second;

            // simplecal zeroed weight publish through aisJhm2Txchannel
            reqAisJhm2TxChannel.simplecal_data.zeroedTruckWt =
                    weightUnitConvertFromDisplay(atof((calibstr[1].second).c_str()));


            // simplecal adjweight publish through aisJhm2txchannel
            reqAisJhm2TxChannel.simplecal_data.adjtruckweight =
                    weightUnitConvertFromDisplay(atof((calibstr[2].second).c_str()));

            reqAisJhm2TxChannel.simplecal_data.newDataFlag = true;

            /* Publish */
            if (aisJhm2TxOutput_) {
                aisJhm2TxOutput_->publish(reqAisJhm2TxChannel);
            }

            AIS_LOG_NOTICE("the value returned from the aistxchannel is %s", (m_scsInputs.m_jhm_payLoad.simplecal_data.timeStamp).c_str());
            commandHandled = true;
        }
    }
    else if ((calibstr[0].first == "CALID_Wr") ||
            (calibstr[0].first == "CALCONTROL_Wr") ||
            (calibstr[0].first == "CALINPUT_Wr") ||
            (calibstr[0].first == "CALCOMMAND_Wr") ||
            (calibstr[0].first == "CALSTEP_Wr")) {
        if (displayState_.isInServiceMode() && !(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            CalibrationRequestUI calRequest;
            calRequest.CalibParam.CalId = (uint16_t)atoi((calibstr[0].second).c_str());
            calRequest.CalibParam.ControlBits =(uint8_t)atoi((calibstr[1].second).c_str());
            calRequest.CalibParam.InputAction = (uint16_t)atoi((calibstr[2].second).c_str());
            calRequest.CalibParam.InputActionCommand = (uint8_t)atoi((calibstr[3].second).c_str());
            calRequest.CalibParam.CurrStep = (uint8_t)atoi((calibstr[4].second).c_str());

            calUIClientInf_.sendRequest(calRequest);

            AIS_LOG_ERROR("POST CAL: ID:%02x  Cntl:%02x  Action:%02x  Cmd:%02x  Step:%02x", calRequest.CalibParam.CalId, calRequest.CalibParam.ControlBits, calRequest.CalibParam.InputAction, calRequest.CalibParam.InputActionCommand, calRequest.CalibParam.CurrStep);

            if ((1 /* Step 1 */ == calRequest.CalibParam.CurrStep) &&
                    (0xF000 /* CAL_ACT_INIT */ == calRequest.CalibParam.InputAction) &&
                    (0x80 /* CAL_CMD_CTRL_ENABLED */ == calRequest.CalibParam.ControlBits)) {
                // Try to wait for ACD to start the calibration and handle this.
                calUIClientInf_.waitForResponse();
            }

            commandHandled = true;
        }
    }
    else if (calibstr[0].first == "MM_Selected_MSN_Wr") {
        if (displayState_.isInServiceMode() && !(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_status = 0;
            m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_value = calibstr[0].second;
            std::string newMachineMSN;
            newMachineMSN = m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_value;
            if (machineMSN.compare(newMachineMSN) != 0) {
                m_scsOutputs.m_jhm_MachineSN.setNewDataFlag();
                m_scsOutputs.send();

                sleep(1); // sleep 1 second to let other apps get the new MSN.

                { // Send a request to restart
                    GenericRequest systemRestartRequest;
                    systemRestartRequest.setRequest(true);
                    systemRestartRequest.setRequester("AisJhm2DataServer");
                    systemRestartRequestOutput_->publish(systemRestartRequest);
                }
            }

            commandHandled = true;
        }
    }
    else if (calib[0].first == "ReWeighEnable") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::REWEIGH;
        jobMgrHelper_.sendRequest(req);
        resp.insertParam( new Jhm2SimpleParam( "ReWeighEnable", "1" ) );
        commandHandled = true;
    }
    else if (calib[0].first == "StoreReqstEnable") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::STORE;
        jobMgrHelper_.sendRequest(req);
        commandHandled = true;
    }
    else if (calib[0].first == "ZeroEnable") {
        /*
         * Send command to weigh app to perform the zero.
         * Weigh app will then notify job manager to remove the last pass.
         */
        LpsSaWeighReqstChannel req;
        req.command = LpsSaWeighReqstChannel::Command::ZERO;
        weighAppInf_.sendRequest(req);
        AIS_LOG_NOTICE("ZeroEnable Button Press Executed");
        commandHandled = true;
    }
    else if (calib[0].first == "MinusOneEnable") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::MINUS_ONE;
        jobMgrHelper_.sendRequest(req);
        AIS_LOG_NOTICE("MinusOneEnable Button Press Executed");
        commandHandled = true;
    }
    else if (calib[0].first == "ClearEnable") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::CLEAR;
        jobMgrHelper_.sendRequest(req);
        AIS_LOG_NOTICE("ClearEnable Button Press Executed");
        commandHandled = true;
    }
    else if (calib[0].first == "ManualAdd") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::MANUAL_ADD;
        jobMgrHelper_.sendRequest(req);
        AIS_LOG_NOTICE("ManualAdd Button Press Executed");
        commandHandled = true;
    }
    else if (calib[0].first == "StandByEnable") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::STANDBY_ACTIVATE;
        jobMgrHelper_.sendRequest(req);
        commandHandled = true;
    }
    else if (calib[0].first == "StandByDisable") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::STANDBY_DEACTIVATE;
        jobMgrHelper_.sendRequest(req);
        commandHandled = true;
    }
    else if (calib[0].first == "ManualTipOffActReqst") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::MANUAL_TIPOFF_ACTIVATE;
        jobMgrHelper_.sendRequest(req);
        commandHandled = true;
    }
    else if (calib[0].first == "ManualTipOffDeactReqst") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::MANUAL_TIPOFF_DEACTIVATE;
        jobMgrHelper_.sendRequest(req);
        commandHandled = true;
    }
    else if ((calib[0].first == "TruckTipOffEnable") || (calib[0].first == "PileTipOffEnable")) {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::TIPOFF_MODE_TOGGLE;
        jobMgrHelper_.sendRequest(req);
        AIS_LOG_NOTICE("ManualTipOffDeactReqst Button Press Executed");
        commandHandled = true;
    }
    else if (calib[0].first == "SetWeigh_LowLimit_Wr") {
        if (displayState_.isInServiceMode() && !(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            float weighRangeBottom = calib[0].second;
            float weighRangeSize = calib[1].second;
            LpsSaWeighReqstChannel req;
            req.command = LpsSaWeighReqstChannel::Command::WRITE_WEIGH_RANGE;
            req.arg.f1 = weighRangeBottom;
            req.arg.f2 = weighRangeSize;
            weighAppInf_.sendRequestWaitForTxData(req, weighAppTxData_);
            AIS_LOG_NOTICE("SetWeigh_LowLimit_Wr Executed");
            commandHandled = true;
        }
    } 
    else if (calib[0].first == "TipOffTriggerMode_Wr") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TIPOFF_TRIGGER_TYPE;
        req.data.tipoffTriggerType = LpsSaTipOffTriggerType_t(calib[0].second);
        jobMgrHelper_.sendRequestWaitForTxData(req, jobMgrTxData_);
        commandHandled = true;
    }
    else if (calib[0].first == "TipOffMode_Wr") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TIPOFF_MODE;
        req.data.tipoffMode = LpsSaJobMgrTipOffState_t(calib[0].second);
        jobMgrHelper_.sendRequestWaitForTxData(req, jobMgrTxData_);
        commandHandled = true;
    }
    else if (calib[0].first == "CalibrationWeightEntry_Wr") {
        if (displayState_.isInServiceMode() && !(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            LpsSaWeighReqstChannel req;
            req.command = LpsSaWeighReqstChannel::Command::WRITE_CALIBRATION_WEIGHT;
            req.arg.f1 = weightUnitConvertFromDisplay(calib[0].second);
            weighAppInf_.sendRequestWaitForTxData(req, weighAppTxData_);
            AIS_LOG_NOTICE("CalibrationWeightEntry_Wr Executed");
            commandHandled = true;
        }
    }
    else if (calib[0].first == "HornStoreState") {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_HORN_ON_STORE_ENABLED;
        if (calib[0].second) {
            req.data.enabled = true;
        }
        else {
            req.data.enabled = false;
        }
        jobMgrHelper_.sendRequestWaitForTxData(req, jobMgrTxData_);
        commandHandled = true;
    }
    else if (calib[0].first == "AutoStore_Wr" ) {
        if (!legalForTradeInstalled_) {
            LpsSaJobMgrReqstChannel req;
            req.command = LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_STORE_PASS_COUNT;
            req.data.autoStorePassCount = calib[0].second;
            jobMgrHelper_.sendRequestWaitForTxData(req, jobMgrTxData_);
            commandHandled = true;
        }
    }

    // Send will check to see if anything has changed prior to publishing each individual output
    m_scsOutputs.send();

    if (!commandHandled) {
        AIS_LOG_ERROR("JSON Cmd Not Handled %s, %s", calibstr[0].first.c_str(), calib[0].first.c_str());
        resp.insertParam( new Jhm2SimpleParam( "CommandStatus", "CannotExecute" ) );
        resp.insertParam( new Jhm2SimpleParam( "CommandFailDetails", "Unknown Command" ) );
        return true; // Return true to send the error response.
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestMaterialInfoWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestMaterialInfoWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    LpsSaJobMgrReqstChannel msg;
    bool newData = false;

    // Set id if provided
    boost::optional<uint32_t> id = pt.get_optional<uint32_t>("id");
    if (id) {
        LpsSaJobMgrReqst req;
        req.command = LpsSaJobMgrReqstCommand::WRITE_MATERIAL_ID;
        req.materialId(*id);
        msg.requests.push_back(std::move(req));
        newData = true;
    }

    // Set name if provided
    boost::optional<std::string> name = pt.get_optional<std::string>("name");
    if (name) {
        LpsSaJobMgrReqst req;
        req.command = LpsSaJobMgrReqstCommand::WRITE_MATERIAL_NAME;
        req.materialName(*name);
        msg.requests.push_back(std::move(req));
        newData = true;
    }

    // Set density if provided
    boost::optional<float> density = pt.get_optional<float>("density");
    if (density) {
        LpsSaJobMgrReqst req;
        req.command = LpsSaJobMgrReqstCommand::WRITE_MATERIAL_DENSITY;
        req.materialDensity(*density);
        msg.requests.push_back(std::move(req));
        newData = true;
    }

    if (newData) {
        jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestPWM
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestDemoAppInputs(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    DemoAppReqstChannel msg;
    bool newData = false;
    std::string PreviousMachineMSN = "/opt/appdata/CPM/AisJhm2DataServer/nvm/PreviousMachineMSN";
    boost::optional<bool> demo_mode_enabled = pt.get_optional<bool>("demomodeEnabled");
    if (demo_mode_enabled) {
        bool demo_mode_status = *demo_mode_enabled;
        if (demo_mode_status) {
            if (machineMSN == "CAT99999") {
                //If you are alredy in demo mode do nothing
            }
            else {
                std::ofstream out(PreviousMachineMSN);
                out << machineMSN.c_str();
                out.close();
                m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_status = 0;
                m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_value = "CAT99999";
            }
        }
        std::string newMachineMSN;
        newMachineMSN = m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_value;
        if (machineMSN.compare(newMachineMSN) != 0) {
            m_scsOutputs.m_jhm_MachineSN.setNewDataFlag();
            m_scsOutputs.send();
            sleep(1);
            {
                GenericRequest systemRestartRequest;
                systemRestartRequest.setRequest(true);
                systemRestartRequest.setRequester("AisJhm2DataServer");
                systemRestartRequestOutput_->publish(systemRestartRequest);
            }
        }
    }

    boost::optional<float> liftposition = pt.get_optional<float>("liftposition");
    if (liftposition) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::WRITE_LIFT_POSITION;
        req.liftposition(*liftposition);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.liftposition = *liftposition;
        newData = true;
    }

    boost::optional<float> liftposition_rate = pt.get_optional<float>("liftposition_rate");
    if (liftposition_rate) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::WRITE_LIFT_POSITION_RATE;
        req.liftposition_rate(*liftposition_rate);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.liftposition_rate = *liftposition_rate;
        newData = true;
    }

    boost::optional<float> tiltposition = pt.get_optional<float>("tiltposition");
    if (tiltposition) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::WRITE_TILT_POSITION;
        req.tiltposition(*tiltposition);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.tiltposition = *tiltposition;
        newData = true;
    }

    boost::optional<float> tiltposition_rate = pt.get_optional<float>("tiltposition_rate");
    if (tiltposition_rate) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::WRITE_TILT_POSITION_RATE;
        req.tiltposition_rate(*tiltposition_rate);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.tiltposition_rate = *tiltposition_rate;
        newData = true;
    }

    boost::optional<float> payload = pt.get_optional<float>("payload");
    if (payload) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::WRITE_PAYLOAD;
        req.payload(*payload);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.payload = *payload;
        newData = true;
    }

    boost::optional<float> payload_rate = pt.get_optional<float>("payload_rate");
    if (payload_rate) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::WRITE_PAYLOAD_RATE;
        req.payload_rate(*payload_rate);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.payload_rate = *payload_rate;
        newData = true;
    }

    boost::optional<bool> Dig = pt.get_optional<bool>("dig");
    boost::optional<float> weight1 = pt.get_optional<float>("weight1");
    if (Dig) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::DIG;
        req.dig(*Dig);
        req.weight1(*weight1);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.dig = *Dig;
        m_scsInputs.m_jhm_demo.weight1 = *weight1;
        newData = true;
    }

    boost::optional<bool> Wrw = pt.get_optional<bool>("wrw");
    if (Wrw) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::WRW;
        req.wrw(*Wrw);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.wrw = *Wrw;
        newData = true;
    }

    boost::optional<bool> Llw = pt.get_optional<bool>("llw");
    if (Llw) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::LLW;
        req.llw(*Llw);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.llw = *Llw;
        newData = true;
    }

    boost::optional<bool> Pdump = pt.get_optional<bool>("pdump");
    boost::optional<float> weight2 = pt.get_optional<float>("weight1");
    if (Pdump) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::PDUMP;
        req.pdump(*Pdump);
        req.weight1(*weight2);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.pdump = *Pdump;
        m_scsInputs.m_jhm_demo.weight1 = *weight2;
        newData = true;
    }

    boost::optional<bool> Dump = pt.get_optional<bool>("dump");
    if (Dump) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::DUMP;
        req.dump(*Dump);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.dump = *Dump;
        newData = true;
    }

    boost::optional<bool> Carry = pt.get_optional<bool>("carry");
    if (Carry) {
        DemoAppReqst req;
        req.command = DemoAppReqstCommand::CARRY;
        req.rack(*Carry);
        msg.requests.push_back(std::move(req));
        m_scsInputs.m_jhm_demo.carry = *Carry;
        newData = true;
    }

    if (newData) {
        DemoAppRequestOutput_->publish(msg);
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestTargetWtWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestTargetWtWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    LpsSaJobMgrReqstChannel msg;
    bool newData = false;

    auto oTargetWeight = pt.get_value_optional<float>();
    if (oTargetWeight) {
        float targetWeight = *oTargetWeight;
        targetWeight = weightUnitConvertFromDisplay(targetWeight);
        LpsSaJobMgrReqst req;
        req.command = LpsSaJobMgrReqstCommand::WRITE_TRUCK_TARGET_WEIGHT;
        req.truckTargetWeight(targetWeight);
        msg.requests.push_back(std::move(req));
        newData = true;
    }

    if (newData) {
        jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestTruckInfoWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestTruckInfoWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    LpsSaJobMgrReqstChannel msg;
    bool newData = false;

    // Set id if provided
    boost::optional<uint32_t> id = pt.get_optional<uint32_t>("id");
    if (id) {
        LpsSaJobMgrReqst req;
        req.command = LpsSaJobMgrReqstCommand::WRITE_TRUCK_ID;
        req.truckId(*id);
        msg.requests.push_back(std::move(req));
        newData = true;
    }

    // Set name if provided
    boost::optional<std::string> name = pt.get_optional<std::string>("name");
    if (name) {
        LpsSaJobMgrReqst req;
        req.command = LpsSaJobMgrReqstCommand::WRITE_TRUCK_NAME;
        req.truckName(*name);
        msg.requests.push_back(std::move(req));
        newData = true;
    }

    /* Set target weight if provided. This API is used to write the truck info by selecting
     * from the truck list. In this case we do NOT use this target wt for the frequently
     * used weights pick list.
     */
    boost::optional<float> targetWeight = pt.get_optional<float>("targetWeight");
    if (targetWeight) {
        LpsSaJobMgrReqst req;
        req.command = LpsSaJobMgrReqstCommand::WRITE_TRUCK_TARGET_WEIGHT;
        req.truckTargetWeight(*targetWeight);
        msg.requests.push_back(std::move(req));
        newData = true;
    }

    if (newData) {
        jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestTruckInfoWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestListWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    bool postIt = false;
    LpsSaListMgrPostInterface post;

    rj::StringBuffer sb;
    rj::Writer<rj::StringBuffer> writer(sb);

    if ("MaterialList" == cmd) {
        writer.SetMaxDecimalPlaces(2);
        writer.StartObject();
        writer.Key("MaterialList");
        writer.StartArray();

        // pt should be an array of materials
        for (const auto& o : pt) {
            // Set id if provided
            boost::optional<uint32_t> id = o.second.get_optional<uint32_t>("id");
            boost::optional<std::string> name = o.second.get_optional<std::string>("name");
            boost::optional<float> density = o.second.get_optional<float>("density");
            if (id || name || density) {
                writer.StartObject();
                if (id) {
                    writer.Key("id");
                    writer.Uint(*id);
                }
                if (name) {
                    writer.Key("name");
                    writer.String(*name);
                }
                if (density) {
                    writer.Key("density");
                    writer.Double(*density);
                }
                writer.EndObject();
            }
        }
        writer.EndArray();
        writer.EndObject();

        if (writer.IsComplete()) {
            post.type = LpsSaListMgrInterfaceListType::MATERIAL;
            post.format = LpsSaListMgrInterfaceListFormat::JSON;
            post.filePath = makeTempPath(MATERIAL_LIST_FILENAME_JSON);
            postIt = true;
        }
    }
    else if ("TruckList" == cmd) {
        writer.SetMaxDecimalPlaces(3);
        writer.StartObject();
        writer.Key("TruckList");
        writer.StartArray();

        // pt should be an array of trucks
        for (const auto& o : pt) {
            // Set id if provided
            boost::optional<uint32_t> id = o.second.get_optional<uint32_t>("id");
            boost::optional<std::string> name = o.second.get_optional<std::string>("name");
            boost::optional<float> targetWeight = o.second.get_optional<float>("targetWeight");
            if (id || name || targetWeight) {
                writer.StartObject();
                if (id) {
                    writer.Key("id");
                    writer.Uint(*id);
                }
                if (name) {
                    writer.Key("name");
                    writer.String(*name);
                }
                if (targetWeight) {
                    writer.Key("targetWeight");
                    writer.Double(*targetWeight);
                }
                writer.EndObject();
            }
        }
        writer.EndArray();
        writer.EndObject();

        if (writer.IsComplete()) {
            post.type = LpsSaListMgrInterfaceListType::TRUCK;
            post.format = LpsSaListMgrInterfaceListFormat::JSON;
            post.filePath = makeTempPath(TRUCK_LIST_FILENAME_JSON);
            postIt = true;
        }
    }
    else if (cmd.rfind("CustomList", 0) == 0) {
        // Starts with "CustomList"
        if (listMgrTxReceived_) {
            bool validList = true;
            writer.StartObject();

            // Look at the last character to see which custom list it is.
            const auto& lastChar = cmd.back();
            switch (lastChar) {
            case ('1'): {
                post.type = LpsSaListMgrInterfaceListType::CUSTOM1;
                post.format = LpsSaListMgrInterfaceListFormat::JSON;
                post.filePath = makeTempPath(CUSTOM_LIST1_FILENAME_JSON);
                writer.Key(listMgrTxData_.customList1Name.c_str());
                break;
            }
            case ('2'): {
                post.type = LpsSaListMgrInterfaceListType::CUSTOM2;
                post.format = LpsSaListMgrInterfaceListFormat::JSON;
                post.filePath = makeTempPath(CUSTOM_LIST2_FILENAME_JSON);
                writer.Key(listMgrTxData_.customList2Name.c_str());
                break;
            }
            case ('3'): {
                post.type = LpsSaListMgrInterfaceListType::CUSTOM3;
                post.format = LpsSaListMgrInterfaceListFormat::JSON;
                post.filePath = makeTempPath(CUSTOM_LIST3_FILENAME_JSON);
                writer.Key(listMgrTxData_.customList3Name.c_str());
                break;
            }
            case ('4'): {
                post.type = LpsSaListMgrInterfaceListType::CUSTOM4;
                post.format = LpsSaListMgrInterfaceListFormat::JSON;
                post.filePath = makeTempPath(CUSTOM_LIST4_FILENAME_JSON);
                writer.Key(listMgrTxData_.customList4Name.c_str());
                break;
            }
            default: {
                validList = false;
                break;
            }
            }

            if (validList) {
                writer.StartArray();

                // pt should be an array of names
                for (const auto& o : pt) {
                    boost::optional<std::string> name = o.second.get_optional<std::string>("name");
                    if (name) {
                        writer.StartObject();
                        writer.Key("name");
                        writer.String(*name);
                        writer.EndObject();
                    }
                }
                writer.EndArray();
            }

            writer.EndObject();

            if (validList && writer.IsComplete()) {
                postIt = true;
            }
        }
    }

    if (postIt) {
        tes_common_ais::OFlocker ofl(post.filePath);
        if (ofl) {
            ofl.ofstream() << sb.GetString();
            ofl.close();
            listMgrRequestHelper_.sendRequestWaitForTxData(post, listMgrTxData_);
            AIS_LOG_INFO("Posted new file.");
        }
        else {
            AIS_LOG_ERROR("File could not be opened and locked for writing.");
        }
    }

    return true;
}


/******************************************************************************
FUNCTION: processCmdRequestPayloadNextSubtotal
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestPayloadNextSubtotal(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    LpsSaJobMgrReqstChannel req;
    req.command = LpsSaJobMgrReqstChannel::Command::WRITE_PAYLOAD_NEXT_SUBTOTAL;
    jobMgrHelper_.sendRequestWaitForTxData(req, jobMgrTxData_);

    AIS_LOG_INFO("*** PayloadNextSubtotal for Split Mode Pressed ***");

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestPrinterCnfgWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestPrinterCnfgWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    // Only allow writing new printer config if the current config has been received.
    if ((printerCnfgReceivedTime_ > TimePoint::min()) && (nullptr != writePrinterCnfgOutput_)) {
        bool newData = false;

        // Set numCopies if provided, this can be changes even when not in service mode
        boost::optional<uint32_t> numCopies = pt.get_optional<uint32_t>("numCopies");
        if (numCopies) {
            uint32_t val = *numCopies;
            if ((val >= 0) && (val <= 4)) {
                printerCnfg_.config.truckTicket.numCopies = *numCopies;
                newData = true;
            }
        }

        // Don't allow changing these configurations unless we are in service mode
        if (displayState_.isInServiceMode()) {
            // Set installed if provided
            auto installed = pt.get_optional<bool>("installed");
            if (installed) {
                // The "installed" argument is given
                printerCnfg_.config.installed = *installed;
                showHideConfig_.set("Printer.installed", printerCnfg_.config.installed);
                newData = true;
            }

            // Set leading blank lines if provided
            boost::optional<uint32_t> leadingBlankLines = pt.get_optional<uint32_t>("leadingBlankLines");
            if (leadingBlankLines) {
                uint32_t val = *leadingBlankLines;
                if ((val >= 0) && (val <= 3)) {
                    printerCnfg_.config.leadingBlankLines = *leadingBlankLines;
                    newData = true;
                }
            }

            // Set trailing blank lines if provided
            boost::optional<uint32_t> trailingBlankLines = pt.get_optional<uint32_t>("trailingBlankLines");
            if (trailingBlankLines) {
                uint32_t val = *trailingBlankLines;
                if ((val >= 0) && (val <= 3)) {
                    printerCnfg_.config.trailingBlankLines = *trailingBlankLines;
                    newData = true;
                }
            }

            // Set header lines if provided
            boost::optional<std::string> headerLine1 = pt.get_optional<std::string>("headerLine1");
            if (headerLine1) {
                printerCnfg_.config.truckTicket.headerLine1 = *headerLine1;
                newData = true;
            }

            boost::optional<std::string> headerLine2 = pt.get_optional<std::string>("headerLine2");
            if (headerLine2) {
                printerCnfg_.config.truckTicket.headerLine2 = *headerLine2;
                newData = true;
            }

            boost::optional<std::string> headerLine3 = pt.get_optional<std::string>("headerLine3");
            if (headerLine3) {
                printerCnfg_.config.truckTicket.headerLine3 = *headerLine3;
                newData = true;
            }

            // Set footer lines if provided
            boost::optional<std::string> footerLine1 = pt.get_optional<std::string>("footerLine1");
            if (footerLine1) {
                printerCnfg_.config.truckTicket.footerLine1 = *footerLine1;
                newData = true;
            }

            // Don't allow changing ticket number LFT is installed and sealed
            if (!(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
                // Set ticket number if provided
                boost::optional<uint32_t> ticketNumber = pt.get_optional<uint32_t>("ticketNumber");
                if (ticketNumber) {
                    uint32_t val = *ticketNumber;
                    if ((val >= 0) && (val <= 65503)) {
                        printerCnfg_.config.truckTicket.ticketNumber = *ticketNumber;
                        newData = true;
                    }
                }
            }

            // Set date format if provided
            boost::optional<uint16_t> dateFormat = pt.get_optional<uint16_t>("dateFormat");
            if (dateFormat) {
                printerCnfg_.config.dateFormat = static_cast<LpsSaTotalsPrinterCnfgDateFormat>(*dateFormat);
                newData = true;
            }

            // Set date Separator if provided
            boost::optional<uint16_t> dateSeparator = pt.get_optional<uint16_t>("dateSeperator"); // Spelling is wrong, but UI depends on it.
            if (dateSeparator) {
                printerCnfg_.config.dateSeparator = static_cast<LpsSaTotalsPrinterCnfgDateSeparator>(*dateSeparator);
                newData = true;
            }
        }

        if (newData) {
            writePrinterCnfgOutput_->publish(printerCnfg_);
            AIS_LOG_ALERT("Printer configuration change published.");
        }

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestUDPTransferDetails
DESCRIPTION: Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name, pt - Property tree containing data, resp - Command Request Response Message
RETURN VALUE: true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestUDPTransferDetails(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if ((printerCnfgReceivedTime_ > TimePoint::min()) && (nullptr != writePrinterCnfgOutput_)) {
        bool newData = false;

        // udpTransferIpAddress
        boost::optional<std::string> ip = pt.get_optional<std::string>("udpTransferIpAddress");
        if (ip) {
            printerCnfg_.config.udpTransferIp = *ip;
            newData = true;
        }

        // udpTransferPort
        boost::optional<uint32_t> port = pt.get_optional<uint32_t>("udpTransferPort");
        if (port) {
            printerCnfg_.config.udpTransferPort = *port;
            newData = true;
        }

        // udpTransferEnabled
        boost::optional<bool> enabled = pt.get_optional<bool>("udpTransferEnabled");
        if (enabled) {
            printerCnfg_.config.udpTransferEnabled = *enabled;
            newData = true;
        }

        if (newData) {
            writePrinterCnfgOutput_->publish(printerCnfg_);
            AIS_LOG_ALERT("Printer configuration change published.");
        }

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestPayloadLevel2TempInstallCnfgWr
DESCRIPTION: Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name, pt - Property tree containing data, resp - Command Request Response Message
RETURN VALUE: true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestPayloadLevel2TempInstallCnfgWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    // install status
    boost::optional<bool> installed = pt.get_optional<bool>("installed");

    if (installed) {
        AutonomyConditionDiagnosticsRequestInterface req;
        bool newData = false;

        if (*installed && (SCL_PRMSW_FEAT_STATUS_FREE_USE_INSTALLED != seaStatusLevel2_.status)) {
            req.data.installed = true;
            newData = true;
        }
        else if (!*installed && (SCL_PRMSW_FEAT_STATUS_FREE_USE_INSTALLED == seaStatusLevel2_.status)) {
            req.data.installed = false;
            newData = true;
        }
        else {
            //no change
        }

        if (newData) {
            req.command = AutonomyConditionDiagnosticsRequestInterfaceCommand::SEA_LEVEL2_TEMP_INSTALL;
            autonomyConditionDiagnosticsHelper_.sendRequestWaitForTxData(req, autonomyConditionDiagnosticsTxData_);
        }
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestDisplaySettingsWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestDisplaySettingsWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if ("DisplaySettings_Wr" == cmd) {

        // Don't allow changing units if LFT is installed and sealed
        if (!(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            { // units
                auto units = pt.get_optional<uint8_t>("units");
                if (units) {
                    displayState_.setUnits(static_cast<LpsSaUIDisplaySettingsUnits>(*units));
                    displaySettingsChanged_ = true;
                }
            }

            { // weightUnits
                auto weightUnits = pt.get_optional<uint16_t>("weightUnits");
                if (weightUnits) {
                    displayState_.setWeightUnits(static_cast<LpsCommonWeightUnits>(*weightUnits));
                    displaySettingsChanged_ = true;
                }
            }
        }

        // Don't allow changing precision if LFT is installed
        if (!legalForTradeInstalled_) {
            { // weightPrecision
                auto weightPrecision = pt.get_optional<int8_t>("weightPrecision");
                if (weightPrecision) {
                    displayState_.setWeightPrecision(static_cast<LpsCommonWeightPrecision>(*weightPrecision));
                    displaySettingsChanged_ = true;
                }
            }
        }

        { // language
            auto language = pt.get_optional<uint16_t>("language");
            if (language) {
                displayState_.setLanguage(static_cast<LpsSaUIDisplaySettingsLanguage>(*language));
                displaySettingsChanged_ = true;
            }
        }

        { // brightness
            auto brightness = pt.get_optional<uint8_t>("brightness");
            if (brightness) {
                displayState_.setBrightness(*brightness);
                displaySettingsChanged_ = true;
            }
        }

        { // timeFormat
            auto timeFormat = pt.get_optional<uint8_t>("timeFormat");
            if (timeFormat) {
                displayState_.setTimeFormat(static_cast<LpsSaUIDisplaySettingsTimeFormat>(*timeFormat));
                displaySettingsChanged_ = true;
            }
        }

        { // dateFormat
            auto dateFormat = pt.get_optional<uint16_t>("dateFormat");
            if (dateFormat) {
                displayState_.setDateFormat(static_cast<LpsSaUIDisplaySettingsDateFormat>(*dateFormat));
                displaySettingsChanged_ = true;
            }
        }

        return true;
    }

    else if ("DisplayBrightness_Wr" == cmd) {
        auto brightness = pt.get_value_optional<uint8_t>();
        if (brightness) {
            displayState_.setBrightness(*brightness);
            displaySettingsChanged_ = true;
            return true;
        }
    }
    else if ("Units_Wr" == cmd) {
        // Don't allow changing units if LFT is installed and sealed
        if (!(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            auto units = pt.get_value_optional<uint32_t>();
            if (units) {
                displayState_.setUnits(static_cast<LpsSaUIDisplaySettingsUnits>(*units));
                displaySettingsChanged_ = true;
                return true;
            }
        }
    }
    else if ("Language_Wr" == cmd) {
        auto language = pt.get_value_optional<uint32_t>();
        if (language) {
            displayState_.setLanguage(static_cast<LpsSaUIDisplaySettingsLanguage>(*language));
            displaySettingsChanged_ = true;
            return true;
        }
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestKeyboardLayoutSettingWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestKeyboardLayoutSettingWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if ("KeyboardLayoutSetting_Wr" == cmd) {
            auto keyboardLayoutSetting = pt.get_value_optional<uint8_t>();
            if (keyboardLayoutSetting) {
                displayState_.setKeyboardLayoutSetting(static_cast<LpsSaUIKeyboardLayoutSetting>(*keyboardLayoutSetting));
                displaySettingsChanged_ = true;
                return true;
            }
        }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestPayloadSettingsWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestPayloadSettingsWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if ("PayloadSettings_Wr" == cmd) {
        bool jobMgrReqMade = false;
        bool weighAppReqMade = false;

        { // tipOffMode
            auto tipOffMode = pt.get_optional<uint8_t>("tipOffMode");
            if (tipOffMode) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TIPOFF_MODE;
                req.data.tipoffMode = LpsSaJobMgrTipOffState_t(*tipOffMode);
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // tipOffTriggerType
            auto tipOffTriggerType = pt.get_optional<uint8_t>("tipOffTriggerType");
            if (tipOffTriggerType) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TIPOFF_TRIGGER_TYPE;
                req.data.tipoffTriggerType = LpsSaTipOffTriggerType_t(*tipOffTriggerType);
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // hornStoreEnable
            auto hornStoreEnable = pt.get_optional<uint8_t>("hornStoreEnable");
            if (hornStoreEnable) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_HORN_ON_STORE_ENABLED;
                if (0 != *hornStoreEnable) {
                    req.data.enabled = true;
                }
                else {
                    req.data.enabled = false;
                }
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        // autoStorePassCount
        if (!legalForTradeInstalled_) {
            auto autoStorePassCount = pt.get_optional<uint16_t>("autoStorePassCount");
            if (autoStorePassCount) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_STORE_PASS_COUNT;
                req.data.autoStorePassCount = *autoStorePassCount;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        // autoTruckIdEnabled
        if (LpsSaLoadRecordTargetType::SINGLE == (LpsSaLoadRecordTargetType)jobMgrTxData_.targetType) {
            auto autoTruckIdEnabled = pt.get_optional<bool>("autoTruckIdEnabled");
            if (autoTruckIdEnabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_TRUCK_ID_ENABLED;
                req.data.enabled = *autoTruckIdEnabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        // autoMaterialIdEnabled
        if (LpsSaLoadRecordTargetType::SINGLE == (LpsSaLoadRecordTargetType)jobMgrTxData_.targetType) {
            auto autoMaterialIdEnabled = pt.get_optional<bool>("autoMaterialIdEnabled");
            if (autoMaterialIdEnabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_MATERIAL_ID_ENABLED;
                req.data.enabled = *autoMaterialIdEnabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // manualAddEnabled
            auto manualAddEnabled = pt.get_optional<bool>("manualAddEnabled");
            if (manualAddEnabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_MANUAL_ADD_ENABLED;
                req.data.enabled = *manualAddEnabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // multiTaskEnabled
            auto multiTaskEnabled = pt.get_optional<bool>("multiTaskEnabled");

            if (!multiTaskEnabled) {
                // Initial UI implementation forgot the 'd' in the API call.
                multiTaskEnabled = pt.get_optional<bool>("multiTaskEnable");
            }

            if (multiTaskEnabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_MULTI_TASK_ENABLED;
                req.data.enabled = *multiTaskEnabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // multiTaskCount
            auto multiTaskCount = pt.get_optional<uint8_t>("multiTaskCount");

            if (multiTaskCount) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_MULTI_TASK_COUNT;
                req.data.taskNumber = *multiTaskCount;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // splitMode
            auto splitModeEnabled = pt.get_optional<bool>("splitModeEnabled");

            if (splitModeEnabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_SPLIT_MODE_ENABLED;
                req.data.enabled = *splitModeEnabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        // hydOilTempEnabled
        if (displayState_.isInServiceMode() && !(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            auto hydOilTempEnabled = pt.get_optional<bool>("hydOilTempEnabled");
            if (hydOilTempEnabled) {
                LpsSaWeighReqstChannel req;
                req.command = LpsSaWeighReqstChannel::Command::WRITE_HYD_OIL_TEMP_ENABLE;
                req.arg.b = *hydOilTempEnabled;
                weighAppInf_.sendRequest(req);
                weighAppReqMade = true;
            }
        }

        { // audibleWeightEnabled
            auto audibleWeightEnabled = pt.get_optional<bool>("audibleWeightEnabled");
            if (audibleWeightEnabled) {
                LpsSaWeighReqstChannel req;
                req.command = LpsSaWeighReqstChannel::Command::WRITE_AUDIBLE_WEIGHT_ENABLE;
                req.arg.b = *audibleWeightEnabled;
                weighAppInf_.sendRequest(req);
                weighAppReqMade = true;
            }
        }

        if (jobMgrReqMade) {
            jobMgrHelper_.waitForTxData(jobMgrTxData_);
        }

        if (weighAppReqMade) {
            weighAppInf_.waitForTxData(weighAppTxData_);
        }

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestServiceModeWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestServiceModeWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if ("ServiceMode_Wr" == cmd) {
        // Set service mode
        boost::optional<bool> mode = pt.get_value_optional<bool>();
        if (mode) {
            if (*mode) {
                displayState_.enterServiceMode();
            }
            else {
                displayState_.exitServiceMode();
            }
            publishDisplayState_ = true;
            return true;
        }
    }
    else if ("ServicePassword_Wr" == cmd) {
        // Enter service mode code
        boost::optional<uint32_t> code = pt.get_value_optional<uint32_t>();
        if (code) {
            displayState_.enterServiceModeEnableCode(*code);
            publishDisplayState_ = true;
            return true;
        }
    }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestOperatorIdWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestOperatorIdWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if("OperatorIdCnfg_Wr" == cmd) {
        auto passcode = pt.get_optional<int16_t>("passcode");
        if(passcode) {
            operatorIdListMgr_.setOperatorId(*passcode);
            AIS_LOG_INFO("*** Operator ID passcode is set to %u ***", *passcode);
        }
        else {
            AIS_LOG_ERROR("*** Failed to set the Operator Id passcode ***");
        }
    }

    return false;
}


/******************************************************************************
FUNCTION: processCmdRequestSelectTask
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestSelectTask(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    auto oTaskNumber = pt.get_value_optional<uint8_t>();
    if (oTaskNumber) {
        LpsSaJobMgrReqstChannel req;
        uint8_t taskNumber = *oTaskNumber;

        if (0 == taskNumber) {
            req.command = LpsSaJobMgrReqstChannel::Command::SELECT_PREVIOUS_TASK;
        }
        else if (255 == taskNumber) {
            req.command = LpsSaJobMgrReqstChannel::Command::SELECT_NEXT_TASK;
        }
        else {
            req.command = LpsSaJobMgrReqstChannel::Command::SELECT_TASK;
            req.data.taskNumber = taskNumber;
        }

        jobMgrHelper_.sendRequestWaitForTxData(req, jobMgrTxData_);
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestCustomListSettingsWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestCustomListSettingsWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if ("CustomListSettings_Wr" == cmd) {
        bool jobMgrReqMade = false;

        { // Truck List Enabled
            auto enabled = pt.get_optional<bool>("truckListEnabled");
            if (enabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TRUCK_LIST_ENABLED;
                req.data.enabled = *enabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // Material List Enabled
            auto enabled = pt.get_optional<bool>("materialListEnabled");
            if(enabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_MATERIAL_LIST_ENABLED;
                req.data.enabled = *enabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // list1Enabled
            auto enabled = pt.get_optional<bool>("list1Enabled");
            if (enabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TAG1_ENABLED;
                req.data.enabled = *enabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // list2Enabled
            auto enabled = pt.get_optional<bool>("list2Enabled");
            if (enabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TAG2_ENABLED;
                req.data.enabled = *enabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // list3Enabled
            auto enabled = pt.get_optional<bool>("list3Enabled");
            if (enabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TAG3_ENABLED;
                req.data.enabled = *enabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // list4Enabled
            auto enabled = pt.get_optional<bool>("list4Enabled");
            if (enabled) {
                LpsSaJobMgrReqstChannel req;
                req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TAG4_ENABLED;
                req.data.enabled = *enabled;
                jobMgrHelper_.sendRequest(req);
                jobMgrReqMade = true;
            }
        }

        { // list1Name
            auto name = pt.get_optional<std::string>("list1Name");
            if (name) {
                LpsSaListMgrPostInterface req;
                req.type = LpsSaListMgrInterfaceListType::CUSTOM1;
                req.name = *name;
                listMgrRequestHelper_.sendRequestWaitForTxData(req, listMgrTxData_);
            }
        }

        { // list2Name
            auto name = pt.get_optional<std::string>("list2Name");
            if (name) {
                LpsSaListMgrPostInterface req;
                req.type = LpsSaListMgrInterfaceListType::CUSTOM2;
                req.name = *name;
                listMgrRequestHelper_.sendRequestWaitForTxData(req, listMgrTxData_);
            }
        }

        { // list3Name
            auto name = pt.get_optional<std::string>("list3Name");
            if (name) {
                LpsSaListMgrPostInterface req;
                req.type = LpsSaListMgrInterfaceListType::CUSTOM3;
                req.name = *name;
                listMgrRequestHelper_.sendRequestWaitForTxData(req, listMgrTxData_);
            }
        }

        { // list4Name
            auto name = pt.get_optional<std::string>("list4Name");
            if (name) {
                LpsSaListMgrPostInterface req;
                req.type = LpsSaListMgrInterfaceListType::CUSTOM4;
                req.name = *name;
                listMgrRequestHelper_.sendRequestWaitForTxData(req, listMgrTxData_);
            }
        }

        if (jobMgrReqMade) {
            jobMgrHelper_.waitForTxData(jobMgrTxData_);
        }

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestCustomInfoWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestCustomInfoWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    LpsSaJobMgrReqstChannel request;

    { // tag1
        auto tag = pt.get_optional<std::string>("tag1");
        if (tag) {
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TAG1;
            req.tag(*tag, listMgrTxData_.customList1Name);
            request.requests.push_back(std::move(req));
        }
    }

    { // tag2
        auto tag = pt.get_optional<std::string>("tag2");
        if (tag) {
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TAG2;
            req.tag(*tag, listMgrTxData_.customList2Name);
            request.requests.push_back(std::move(req));
        }
    }

    { // tag3
        auto tag = pt.get_optional<std::string>("tag3");
        if (tag) {
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TAG3;
            req.tag(*tag, listMgrTxData_.customList3Name);
            request.requests.push_back(std::move(req));
        }
    }

    { // tag4
        auto tag = pt.get_optional<std::string>("tag4");
        if (tag) {
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TAG4;
            req.tag(*tag, listMgrTxData_.customList4Name);
            request.requests.push_back(std::move(req));
        }
    }

    if (!request.requests.empty()) {
        jobMgrHelper_.sendRequestWaitForTxData(request, jobMgrTxData_);
    }

    return true;
}

/*
 * Process command requests for running and recording tests
 */
bool AisJhm2RequestProcessor::processCmdRequestTest(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    if ("TestRun" == cmd) {
        auto oArg = pt.get_value_optional<std::string>();
        if (oArg) {
            LpsSaWeighReqstChannel req;
            req.command = LpsSaWeighReqstChannel::Command::RUN_TEST;
            req.arg.s = *oArg;
            weighAppInf_.sendRequest(req);
        }
    }
    else if ("TestRecord" == cmd) {
        auto oArg = pt.get_value_optional<uint32_t>();
        if (oArg) {
            LpsSaWeighReqstChannel req;
            req.command = LpsSaWeighReqstChannel::Command::RECORD_TEST;
            req.arg.u = *oArg;
            weighAppInf_.sendRequest(req);
            AIS_LOG_FATAL("TestRecord %u", req.arg.u);
        }
    }

    return true;
}

void AisJhm2RequestProcessor::machineConfigCheck()
{
    std::string productId = machineConfig_.prevProductId;
    bool productIdChanged = false;
    bool linkageChanged = false;
    bool toolChanged = false;

    // Check for changes in product id
    if (partNumbers_.IsProductIdNumSet()) {
        productId = partNumbers_.GetProductIdNum();
        if (machineConfig_.prevProductId != productId) {
            AIS_LOG_INFO("ProductId has changed");
            productIdChanged = true;
        }
    }

    // Check for changes in linkage config
    if (machineConfig_.linkage.isOk) {
        if (machineConfig_.prevLinkage.isOk) {
            if (machineConfig_.linkage.val != machineConfig_.prevLinkage.val) {
                linkageChanged = true;
            }
        }
        else {
            linkageChanged = true;
        }
    }

    // Check for changes in tool config
    if (machineConfig_.tool.isOk) {
        if (machineConfig_.prevTool.isOk) {
            if (machineConfig_.tool.val != machineConfig_.prevTool.val) {
                toolChanged = true;
            }
        }
        else {
            toolChanged = true;
        }
    }

    if (productIdChanged || linkageChanged || toolChanged) {
        // Something changed... do we have all of our inputs?
        if ((productId.length() >= 8) && machineConfig_.linkage.isOk && machineConfig_.tool.isOk) {
            // Handle the change... find a match
            std::string newInternalMsn("");

            try {
                boost::property_tree::ptree root;
                boost::property_tree::read_json(const_cast<char*>(msnLinkageMappingAU2020Path_.c_str()), root);

                const std::string snPrefix(productId, 0, 3);
                const std::string snSuffix(productId, 3, 5);

                const boost::property_tree::ptree& msnMap = root.get_child("msn");
                const boost::property_tree::ptree& msnRangeList = msnMap.get_child(snPrefix);

                for (const auto& it : msnRangeList) {
                    const boost::property_tree::ptree& msnRangeEntry = it.second;
                    const uint32_t firstSn = msnRangeEntry.get<uint32_t>("firstSn");
                    const uint32_t lastSn = msnRangeEntry.get<uint32_t>("lastSn");

                    const uint32_t sn = std::stoi(snSuffix); // This will throw if it fails

                    if (sn >= firstSn && sn <= lastSn) {
                        const std::string& config = msnRangeEntry.get<std::string>("config");
                        const std::string& factory = msnRangeEntry.get<std::string>("factory");

                        AIS_LOG_INFO("firstSn %ld, lastSn %ld, config  %s factory %s", firstSn, lastSn, config.c_str(), factory.c_str());

                        // It is within the serial number range, look for a config match
                        const boost::property_tree::ptree& configMap = root.get_child("config");
                        const boost::property_tree::ptree& configList = configMap.get_child(config);

                        for (const auto &it1 : configList) {
                            const boost::property_tree::ptree& configEntry = it1.second;

                            // Get linkage config, defaulting to the one we are looking for
                            const uint16_t linkage = configEntry.get<uint16_t>("linkage", machineConfig_.linkage.val);

                            // Get tool config, defaulting to the one we are looking for
                            const uint16_t tool = configEntry.get<uint16_t>("tool", machineConfig_.tool.val);

                            if ((linkage == machineConfig_.linkage.val) && (tool == machineConfig_.tool.val)) {
                                // We found a match, now get the internalMsn
                                newInternalMsn = configEntry.get<std::string>("internalMsn"); // This will throw if it fails
                                break; // break if we found a match
                            }
                        }
                    }

                    if (!newInternalMsn.empty()) {
                        break; // break if we found a match
                    }
                }
            }
            catch(std::exception const &e) {
                AIS_LOG_ERROR("Cannot find new config - %s", e.what());
            }

            if (newInternalMsn.empty()) {
                // No match found.
                newInternalMsn = "NOT00000";
            }
            else {
                // Match found.
                newInternalMsn.insert(0, "CAT");
            }

            if (newInternalMsn != machineMSN) {
                // It changed.
                m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_status = 0;
                m_scsOutputs.m_jhm_MachineSN.m_machineSN.m_value = newInternalMsn;
                m_scsOutputs.m_jhm_MachineSN.setNewDataFlag();
                m_scsOutputs.send();

                AIS_LOG_ERROR("Internal MSN has changed, restarting system");
                AIS_LOG_ERROR("New Internal MSN = %s", newInternalMsn.c_str());
                AIS_LOG_ERROR("Old Internal MSN = %s", machineMSN.c_str());
                AIS_LOG_ERROR("ProductId = %s", productId.c_str());

                // Need to wait at least 1 second for other apps to learn of the new config
                // We are waiting 5 seconds so that we have time to receive all changes before resetting.
                // For example, if product id changes, followed by linkage type and tool config, we will reset 3 times.
                // We only want to reset once when the changes are all received.
                machineConfig_.timeToReset = std::chrono::steady_clock::now() + std::chrono::seconds(5);
            }

            // Update the previous values to detect the next change.
            machineConfig_.prevProductId = productId;
            machineConfig_.prevLinkage = machineConfig_.linkage;
            machineConfig_.prevTool = machineConfig_.tool;
        }
    }
    else {
        // Nothing changed, is it time to reset?
        if (machineConfig_.timeToReset < std::chrono::steady_clock::time_point::max()) {
            if (std::chrono::steady_clock::now() > machineConfig_.timeToReset) {
                // Send a request to restart
                GenericRequest systemRestartRequest;
                systemRestartRequest.setRequest(true);
                systemRestartRequest.setRequester("AisJhm2DataServer");
                if (systemRestartRequestOutput_->publish(systemRestartRequest)) {
                    machineConfig_.timeToReset = std::chrono::steady_clock::time_point::max();
                }
            }
        }
    }
}


static uint_least8_t getBucketWeightDisplayAccuracy(unsigned short int payloadCalcMethod, bool displayLiveWeightOk) {
    /*
     * In order to speed up how fast the green box shows up, we are going to show the weight it is
     * level 2 accuracy or higher, knowing that the truck information will be updated when job manager
     * sees the same thing.  We don't need to wait for job manager to tell us that the weight is good
     * enough to show unless it is level 1 accuracy (live weight).
     */
    uint_least8_t bucketWeightDisplayAccuracy;
    constexpr unsigned short int accuracyStartbit = 8;
    constexpr unsigned short int accuracyEndbit = 12;
    unsigned short int accuracy = MID(payloadCalcMethod, accuracyStartbit, accuracyEndbit);

    if (accuracy >= 3) {
        bucketWeightDisplayAccuracy = 3; // Green box weight
    }
    else if (accuracy >= 2) {
        bucketWeightDisplayAccuracy = 1; // Latched weight
    }
    else if (accuracy >= 1) {
        if (displayLiveWeightOk) {
            bucketWeightDisplayAccuracy = 1; // Live weight
        }
        else {
            bucketWeightDisplayAccuracy = 0; // *'s
        }
    }
    else {
        bucketWeightDisplayAccuracy = 0; // *'s
    }

    return bucketWeightDisplayAccuracy;
}



