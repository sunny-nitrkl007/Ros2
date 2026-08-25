///////////////////////////////////////////////////////////////////////////////
/// @file      AisJhm2RequestProcessor.h
/// @author    strubjc
/// @date      Aug 15, 2014
/// @brief     
///
///
/// @attention COPYRIGHT (C) 2012 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////

#ifndef AISJHM2REQUESTPROCESSOR_H_
#define AISJHM2REQUESTPROCESSOR_H_

#include <string>
#include <unordered_map>
#include <deque>
#include <chrono>
#include <cstdint>
#include <bitset>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>

//Config Types
#include <ais/config/ConfigParser.h>
#include <ais/config/TaskParser.h>
#include <ais/config/ConfigSection.h>
#include <jhm2/Jhm2RequestProcessor/Jhm2RequestProcessor.h>
#include <jhm2/Jhm2Param/Jhm2ParamMsgRequest.h>
#include <jhm2/Jhm2Param/Jhm2ParamMsg.h>
#include <jhm2/Jhm2Param/Jhm2ImageMsg.h>
#include <jhm2/Jhm2Param/Jhm2CmdResponseMsg.h>

#include <interfaces/CalibrationRequestUI/CalibrationUIClientInf.hpp>

#include <interfaces/LpsSaListMgr/TxInterface.hpp>
#include <interfaces/LpsSaListMgr/TxInterfaceInputChannel.h>

#include <interfaces/LpsSaListMgr/PostInterfaceOutputChannel.h>

#include <interfaces/LpsSaLoadRecordChannel/Channel/Input/channel.h>

#include <interfaces/LpsSaTotals/TotalsInterface.hpp>
#include <interfaces/LpsSaTotals/TotalsInterfaceInputChannel.h>

#include <interfaces/LpsSaTotals/RequestHelper.hpp>

#include <interfaces/LpsSaTotals/PrinterCnfgInterfaceInputChannel.h>
#include <interfaces/LpsSaTotals/PrinterCnfgInterfaceOutputChannel.h>

#include <interfaces/SerialPrinter/RequestInterfaceOutputChannel.h>

#include <interfaces/LpsSaUI/DisplaySettingsInterfaceInputChannel.h>
#include <interfaces/LpsSaUI/DisplayStateInterfaceOutputChannel.h>
#include <interfaces/LpsSaUI/BEMSimStateInterfaceOutputChannel.h>


#include <interfaces/LpsSaWeighReqstChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrReqstChannel/InterfaceTypes.h>
#include <interfaces/AisJhm2TxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrReqstChannel/LpsSaJobMgrRequestHelper.hpp>
#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp>
#include <interfaces/AutonomyConditionDiagnostics/RequestHelper.hpp>

#include <interfaces/DemoAppReqstChannel/InterfaceTypes.h>

#include <interfaces/ShmClock/InterfaceTypes.h>
#include "../../../../../apps/LpsSaListMgrApp/OperatorIdListMgr.hpp"

#include <interfaces/DataLinkData/InterfaceTypes.h>
#include <interfaces/PartNumbers/InterfaceTypes.h>
#include <interfaces/PartNumbers/InterfaceTypes.h>
#include <interfaces/GenericRequest/InterfaceTypes.h>
#include <interfaces/SEAStatus/InterfaceTypes.h>

#include <interfaces/EventDiagnosticData/InterfaceTypes.h>
#include <interfaces/LpsSaListMgr/NotifyInterface.hpp>
#include <interfaces/LpsSaListMgr/PostInterface.hpp>
#include <interfaces/LpsSaListMgr/ListMgrRequestHelper.hpp>

#include <interfaces/WorkOrderAssist/SettingsInterfaceInputChannel.h>

#include <interfaces/LpsSaTotals/USBSaveProgressInterface.hpp>
#include <interfaces/LpsSaTotals/USBSaveProgressInterfaceInputChannel.h>

#include "AisJhm2ScsOutputs.h"
#include "AisJhm2ScsInputs.h"
#include "AisJhm2FileMsg.h"
#include "AisJhm2DisplayState.h"
#include "ShowHideConfig.h"
#include "AisJhm2TargetWtSelection.h"
#include "AisJhm2CANProcess.h"
#include "BEMSim/BEMSimCnfgMgr.h"
#include "AisJhm2Keypad.h"
#include "AisJhm2FrequentList.h"

/*
 * Info Popup Flags
 * This is an example of a portable bitset instead of the non-portable union/bitfield pattern.
 */
namespace AisJhmInfoPopUp {
    enum {
        // The first 4 bits (0 through 3) are unused
        PAYLOAD_REWEIGH_WARMUP_LIFT = 4,
        PAYLOAD_STORE_REJECTED_NO_GBW,
        PAYLOAD_REWEIGH_EXCESSIVE_PITCH,
        TOA_DISARMED_LIFT_LEVER_COMMAND,
        TOA_DISARMED_EXCESSIVE_MACHINE_SPEED,
        PAYLOAD_CAL_WT_ENTRY_REQUIRED,
        TOA_ASSIST_COMPLETE,
        TOA_ARMED,
        TOA_DISARMED_END_OF_TRAVEL,
        TOA_DISARMED_TILT_LEVER_RELEASED,
        TOA_DISARMED_TRUCK_TARGET_NOT_SET,
        TOA_DISARMED_NOT_IN_PILE_TIP_OFF_MODE,
        TOA_DISARMED_TILT_LEVER_RACKED,
        TOA_DISARMED_PAYLOAD_NOT_ACHIEVABLE,
        TOA_DISARMED_DUE_TO_TIMEOUT,
        DISPATCH_COMMUNICATION_LOSS,
        PAYLOAD_TOO_HEAVY_TO_ZERO,
        PAYLOAD_NOT_ZEROED,
        PAYLOAD_CAL_ACCEPTED,
        PAYLOAD_ZERO_ACCEPTED,
        PAYLOAD_REWEIGH_LIFT_TOO_SLOW,
        PAYLOAD_REWEIGH_STOPPED_IN_RANGE,
        PAYLOAD_REWEIGH_NOT_RACKED,
        PAYLOAD_REWEIGH_SPEED_CHANGING,
        PAYLOAD_REWEIGH_INCONSISTENT,
        PAYLOAD_REWEIGH_PRESSURE_CHANGING,
        PAYLOAD_RAISE_STALL,
        PAYLOAD_LOWER_STALL,
        NUM_BITS
    };
    static_assert(NUM_BITS == 32, "AisJhmInfoPopUp::NUM_BITS != 32");

    typedef std::bitset<NUM_BITS> type;
}

/*
 * Diag Popup Flags
 */
namespace AisJhmDiagPopUp {
    enum {
        J1939_DATALINK_2_ABNORMAL = 1,
        J1939_DATALINK_2_SPECIAL_INSTRUCTION,
        J1939_DATALINK_1_ABNORMAL,
        J1939_DATALINK_1_SPECIAL_INSTRUCTION,
        ETHERNET_DATALINK_1_ABNORMAL,
        CAT_DATALINK_ABNORMAL,
        CAT_DATALINK_SPECIAL_INSTRUCTION,

        TILT_RE_FREQ_ABNORMAL,
        TILT_RE_VOLTAGE_BELOW_NORMAL,
        TILT_RE_VOLTAGE_ABOVE_NORMAL,

        TILT_HE_FREQ_ABNORMAL,
        TILT_HE_VOLTAGE_BELOW_NORMAL,
        TILT_HE_VOLTAGE_ABOVE_NORMAL,

        HYDRAULIC_OIL_TEMP_BAD,
        MACHINE_MODEL_NOT_SET,
        PAYLOAD_SYSTEM_NOT_INSTALLED,
        PAYLOAD_SYSTEM_OUT_OF_CAL,

        LIFT_RE_FREQ_ABNORMAL,
        LIFT_RE_VOLTAGE_BELOW_NORMAL,
        LIFT_RE_VOLTAGE_ABOVE_NORMAL,

        LIFT_HE_FREQ_ABNORMAL,
        LIFT_HE_VOLTAGE_BELOW_NORMAL,
        LIFT_HE_VOLTAGE_ABOVE_NORMAL,

        LIFT_LINK_OUT_OF_CAL,
        LIFT_LINK_FREQ_ABNORMAL,
        LIFT_LINK_VOLTAGE_BELOW_NORMAL,
        LIFT_LINK_VOLTAGE_ABOVE_NORMAL,

        TILT_LINK_OUT_OF_CAL,
        TILT_LINK_FREQ_ABNORMAL,
        TILT_LINK_VOLTAGE_BELOW_NORMAL,
        TILT_LINK_VOLTAGE_ABOVE_NORMAL,

        NUM_BITS,
    };
    static_assert(NUM_BITS == 32, "AisJhmDiagPopUp::NUM_BITS != 32");

    typedef std::bitset<NUM_BITS> type;
}

/*
 * Event Popup Flags
 */
namespace AisJhmEventPopUp {
    enum {
        LOW_ECU_VOLTAGE = 24,
        HIGH_ECU_VOLTAGE = 25,
        PAYLOAD_LFT_NOT_SEALED = 26,
        PAYLOAD_MEMORY_FULL = 27,
        PAYLOAD_MEMORY_LOW = 28,
        BEMSIM_LEVEL_NEED_TO_CHARGE_WARNING = 29,
        BEMSIM_LEVEL_BELOW_DERATE_THRESHOLD = 30,
        OVERLOAD_LIMIT_EXCEEDED = 31,
        NUM_BITS,
    };
    static_assert(NUM_BITS == 32, "AisJhmEventPopUp::NUM_BITS != 32");

    typedef std::bitset<NUM_BITS> type;
}

namespace BucketIcon {
    enum {
        NONE = 0,
        IN_WEIGH_RANGE = 1,
        EXCESSIVE_PITCH = 2,
        NOT_RACKED = 3
    };
}

class AisJhm2RequestProcessor: public Jhm2RequestProcessor
{
  public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    AisJhm2RequestProcessor();
    
    virtual ~AisJhm2RequestProcessor();

    virtual bool configure( TaskParser& tp);
    virtual bool update( );
    virtual void cleanup( );

    virtual Jhm2Msg* getResponse( const Jhm2ParamMsgRequest& req );

    void postUpdate(); // Do things after the message processing.

    bool bemSimDerateSetEvent;
    bool bemSimDerateClrEvent;

    bool bemSimWarningSetEvent;
    bool bemSimWarningClrEvent;

  protected:
    inline std::string makeStoragePath(const std::string& fileName) const {
        return (storageRoot_ / fileName).string();
    }

    inline std::string makeTempPath(const std::string& fileName) const {
        return (tempRoot_ / fileName).string();
    }



    float updateRate_;

    boost::filesystem::path storageRoot_;
    boost::filesystem::path tempRoot_;
    boost::filesystem::path msnLinkageMappingAU2020Path_;
    
    bool publishDisplayState_;
    bool displaySettingsChanged_;
    bool showHideReady_;
    bool displayBrightnessSupported_;
    bool payloadRecipesSupported_;
    uint32_t displayStatePublishCounter_;
    AisJhm2DisplayState displayState_;
    AisJhm2TargetWtSelection targetWeights_;
    AisJhm2FrequentList materialFreqList_;
    AisJhm2FrequentList truckFreqList_;
    AisJhm2FrequentList custom1FreqList_;
    AisJhm2FrequentList custom2FreqList_;
    AisJhm2FrequentList custom3FreqList_;
    AisJhm2FrequentList custom4FreqList_;
    unsigned int prevStoreCount_;
    unsigned int prevSubtotalCount_;

    LpsSaUIDisplayStateInterfaceOutputChannel* displayStateOutputChannel_;
    LpsSaUIDisplaySettingsInterfaceInputChannel* displaySettingsInputChannel_;

    ShmClockInput* shmClockInput_;
    struct {
        int32_t offset;
        int32_t index;
    } tzInfo_;

    DataLinkDataInput* dataLinkDataInput_;

    DataLinkDataOutput* dataLinkDataOutput_;
    DataLinkData dlData_;

    AisJhm2ScsInputs  m_scsInputs;
    AisJhm2ScsOutputs m_scsOutputs;

    CalibrationUIClientInf calUIClientInf_;

    // For SEA
    SEA seaStatusLevel1_;
    SEA seaStatusLevel2_;
    SEA seaStatusLegalForTrade_;
    bool legalForTradeInstalled_;
    bool legalForTradeInstalledStatusReceived_;

    // For receiving part numbers
    PartNumbers partNumbers_;
    PartNumbersInput* partInputChannel_;

    // For receiving lifetime and trip totals
    TimePoint totalsReceivedTime_;
    LpsSaTotalsInterface totals_;
    LpsSaTotalsInterfaceInputChannel* totalsInputChannel_;
    LpsSaLoadRecordChannelInputChannel* lpsSaLoadRecordChannelInput_;

    // For sending requests to the totals app
    LpsSaTotalsRequestHelper totalsRequestHelper_;

    // For receiving the current printer configuration
    LpsSaTotalsPrinterCnfgInterfaceInputChannel* printerCnfgInput_;

    // For writing the new printer configuration
    LpsSaTotalsPrinterCnfgInterfaceOutputChannel* writePrinterCnfgOutput_;

    // For making requests to the printer
    SerialPrinterRequestInterfaceOutputChannel* printerRequestChannelOutput_;

    AisJhm2TxChannelOutput* aisJhm2TxOutput_;

    // For writing to Demo App
    DemoAppReqstChannelOutput* DemoAppRequestOutput_;

    // For restarting the system when the MSN changes
    GenericRequestOutput* systemRestartRequestOutput_;

    // The current printer cnfg
    TimePoint printerCnfgReceivedTime_;
    LpsSaTotalsPrinterCnfgInterface printerCnfg_;

    // Dispatch (Work Order Assist) Settings
    WorkOrderAssistSettingsInterfaceInputChannel* dispatchSettingsInput_;

    // USB-Download Save Progress
    LpsSaTotalsUSBSaveProgressInterfaceInputChannel* USBSaveProgressInput_;

    std::string detailedDiagnosticsFilePath_;
    std::string ecmSummaryFilePath_;
    std::string displayServiceDashboardFilePath_;
    std::string telematicsServiceDashboardFilePath_;

    // For fast lookup of the request handler, a map is used instead of if->else if->else if string compares.
    typedef bool (AisJhm2RequestProcessor::*ParamMsgRequestHandler)(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    typedef std::unordered_map<std::string, ParamMsgRequestHandler> ParamMsgRequestHandlerMap;
    ParamMsgRequestHandlerMap paramMsgRequestHandlerMap_;

    // For fast lookup of the command handler, a map is used instead of if->else if->else if string compares.
    typedef bool (AisJhm2RequestProcessor::*CmdMsgRequestHandler)(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    typedef std::unordered_map<std::string, CmdMsgRequestHandler> CmdMsgRequestHandlerMap;
    CmdMsgRequestHandlerMap cmdMsgRequestHandlerMap_;

    // For fast lookup of the file handler, a map is used instead of if->else if->else if string compares.
    typedef bool (AisJhm2RequestProcessor::*FileMsgRequestHandler)(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    typedef std::unordered_map<std::string, FileMsgRequestHandler> FileMsgRequestHandlerMap;
    FileMsgRequestHandlerMap fileMsgRequestHandlerMap_;

    bool operatorIdCsv_;
    OperatorIdListMgr operatorIdListMgr_;

    LpsSaJobMgrTxChannelStorage jobMgrTxData_;
    LpsSaJobMgrRequestHelper jobMgrHelper_;

    AutonomyConditionDiagnosticsTxInterface autonomyConditionDiagnosticsTxData_;
    AutonomyConditionDiagnosticsRequestHelper autonomyConditionDiagnosticsHelper_;

    LpsSaWeighTxChannelStorage weighAppTxData_;
    LpsSaWeighAppInf weighAppInf_;

    bool listMgrTxReceived_;
    LpsSaListMgrTxInterface listMgrTxData_;
    LpsSaListMgrRequestHelper listMgrRequestHelper_;
    AisJhm2CANProcess can11bitProcess_;
    Keypad_t keypad_;

    struct MachineConfig {
        typedef struct {
           uint16_t val = 0;
           bool isOk = false;
        } DataUint16_t;

        MachineConfig() :
                prevProductId(),
                linkage(), prevLinkage(),
                tool(), prevTool(),
                timeToReset(std::chrono::steady_clock::time_point::max()) {}

        std::string prevProductId;

        DataUint16_t linkage;
        DataUint16_t prevLinkage;

        DataUint16_t tool;
        DataUint16_t prevTool;

        std::chrono::steady_clock::time_point timeToReset;
    };

    MachineConfig machineConfig_;

    struct TotalFuel {
            typedef struct {
               double val = 0.0;
               bool isOk = false;
            } FuelData;

            TotalFuel() :
                    lowResFuel(),
                    highResFuel(){}

            double getTotalFuelUsedGallons() {
                if (highResFuel.isOk) {
                    return highResFuel.val;
                }
                else if (lowResFuel.isOk) {
                    return lowResFuel.val;
                }
                else {
                    // not available
                    return 0;
                }
            }

            double getTotalFuelUsedLiters() {
                const double gal_to_liters = 3.78541178f;
                if (highResFuel.isOk) {
                    //  Gal to Liters (1 Gal = 3.78541178 L)
                    return ((highResFuel.val)*(gal_to_liters));
                }
                else if (lowResFuel.isOk) {
                    // Gal to Litters (1 Gal = 3.78541178 L)
                    return ((lowResFuel.val)*(gal_to_liters));
                }
                else {
                    // not available
                    return 0.0f;
                }
            }

            bool isTotalFuelAvailable() {
                return (highResFuel.isOk || lowResFuel.isOk);
            }

            FuelData lowResFuel;
            FuelData highResFuel;
        };

    TotalFuel totalFuel_;

    // Special BEM Simulator.
    BEMSimCnfgMgr bemSimMgr;


    std::chrono::steady_clock::time_point demo_mode_exit_time;

    ShowHideConfig showHideConfig_;

    AisJhmInfoPopUp::type infoState_;
    AisJhmDiagPopUp::type diagState_;
    AisJhmEventPopUp::type eventState_;

    bool loadShowHideConfig(ConfigSection& cs);

  private:

    static void updateNumberOfReleases(const DataLinkParam& param, Keypad_t::Button_t& button) {
        int releases = 0;
        const auto& vec = param.GetdlpDataVector();
        for (const auto & element : vec) {
            if (0 != element.Dsi) {
                button.previouslyDepressed = false;
                continue;
            }

            bool currentlyDepressed = (element.Value == 0x001D);

            if ((!currentlyDepressed) && (button.previouslyDepressed)) {
                // released, check timer and do something if shorter than config depress duration
                auto time_elapsed = std::chrono::steady_clock::now() - button.timePressed;
                auto seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(time_elapsed).count();
                if (seconds_elapsed < BUTTON_PRESS_CONFIG_TIME_IN_SECS) {
                    // increment press count
                    ++releases;
                }
            }
            else if ((currentlyDepressed) && (!button.previouslyDepressed)) {
                button.timePressed = std::chrono::steady_clock::now();
            }

            button.previouslyDepressed = currentlyDepressed;
        }

        if (releases > 0) {
            // do action, as defined by config PID
            button.numberOfReleases = releases;
        }
    }

    typedef enum
    {
        ADVANCED = 0,
        STANDALONE
    } LpsSaApplicationVariant_t;

    void installTotalsHandlers();
    void installTicketHandlers();
    void installLFTHandlers();
    void installWorkToolHandlers();
    void installRecipeHandlers();

    bool processParamRequest(const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestHeartbeat(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestPayloadInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestRemainingWt(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestTargetWeights(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestMaterialInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestTruckInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestLifetimeTotals(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestTripATotals(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestTripBTotals(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestAllTotals(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestPrinterCnfg(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestDisplaySettings(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestServiceParameters(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestKeyboardLayoutSetting(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestPayloadSettings(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestServiceMode(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestLocalTime(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestCalibrationData(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestDefault(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestOperatorId(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestTipoffWeightAdjustData(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestMachImuPitchOffset(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestCustomListSettings(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestCustomInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestDemoAppInputs(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestUDPTransferDetails(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestPayloadLevel2TempInstallCnfg(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestBEMSimInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestBEMSimCnfg(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestUSBSaveProgress(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestLFTSummary(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestLFTSealed(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestLFTComponentIds(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestLFTSettings(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestWorkToolInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestIMUData(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);
    bool processParamRequestTestStatus(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp);

    bool processCmdRequest( const Jhm2ParamMsgRequest& req, Jhm2CmdResponseMsg& resp );
    bool processCmdRequest( const std::string& cmd,Jhm2CmdResponseMsg& resp );
    bool processCmdRequestTargetWtWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestMaterialInfoWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestTruckInfoWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestListWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPrintedReportsClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestTruckReportClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestMappedTotalsClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestMaterialTotalsReq(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestTripATotalsClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestTripBTotalsClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPayloadNextSubtotal(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPrinterCnfgWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPrintTotals(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestDisplaySettingsWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processParamRequestKeyboardLayoutSettingWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPayloadSettingsWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestServiceModeWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestOperatorIdWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestMachImuPitchOffset(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestTipoffWeightAdjustData(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestSelectTask(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestTipoffWeightAdjustReset(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestCustomListSettingsWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestCustomInfoWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestDemoAppInputs(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestUDPTransferDetails(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPayloadLevel2TempInstallCnfgWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestBEMSimCmdWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestBEMSimCnfgWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestUSBSaveWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestLFTSealedWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestLFTSensorIdsWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestLFTSettingsWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPrintTicket(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestLFTEnableWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestWorkToolInfoWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPayloadServiceDashboard(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestPayloadDetailsWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestTest(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestDeleteRecipeWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestSaveRecipeWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestSelectRecipeWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);
    bool processCmdRequestSubtotalInfoWr(const std::string& cmd,  const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp);

    bool processFileRequest(const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestList(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestMappedTotals(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestDetailedDiagnostics(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestECMSummary(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestDisplayServiceDashboard(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestTelematicsServiceDashboard(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestUIShowFeatureConfig(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestTicketHistory(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestTicketDetails(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestServiceHistory(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestPayloadServiceDashboard(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);
    bool processFileRequestPayloadDetails(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp);

    void machineConfigCheck();

    template<typename T>
    std::string weightUnitConvertToDisplay(T metricValue);

    template<typename T>
    T weightUnitConvertFromDisplay(T displayValue);

    std::string weightUnitsString();

    std::string fetchMchnMdlDataFrmFile();
    std::string fetchSimpleCalList();

    void fetchMSNFromrb();

    void processDataLinkInputs();

    void processKeypadInputs();
    void processKeypadButton(Keypad_t::Button_t& button);

    void aisJhmPopUpRead();

    LpsSaApplicationVariant_t getApplicationVariant();

    // Special BEM Simulator.
    bool bemSimCnfgChanged_;
    LpsSaUIBEMSimStateInterfaceOutputChannel* bemSimStateOutputChannel_;

    static constexpr auto m_PARAMSREQUESTED_KEY = "paramsRequested";
    static constexpr auto m_COMMANDREQUESTED_KEY = "commandRequested";
    static constexpr auto m_FILEREQUESTED_KEY = "fileRequested";
    static constexpr auto m_TESTTYPE_KEY = "testType";
    static constexpr auto m_COMMANDID_KEY = "commandId";

    static constexpr float DISPLAY_STATE_PUBLISH_PERIOD_SEC = 5.0f;

};

#endif /* AISJHM2REQUESTPROCESSOR_H_ */

