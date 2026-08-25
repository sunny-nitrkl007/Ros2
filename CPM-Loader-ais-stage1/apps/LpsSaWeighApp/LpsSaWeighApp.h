/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaWeighApp.h
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef _LPS_SA_WEIGHAPP_H_
#define _LPS_SA_WEIGHAPP_H_

#include <mutex>

#include <boost/filesystem.hpp>

#include <ais/task/Task.h>

// ---- ROS2/DDS wrapper layer 
#include "rclcpp/rclcpp.hpp"
#include "RosInputInterface.h"
#include "RosOutputInterface.h"

#include "cpm_common_interfaces/msg/lps_sa_weigh_reqst_channel.hpp"
#include <cpm_common_interfaces/msg/lps_sa_weigh_resp_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_tx_channel.hpp>

#include <interfaces/LpsSaUI/DisplayStateInterfaceInputChannel.h>
#include <interfaces/LpsSaWeighReqstChannel/InterfaceTypes.h>
#include <interfaces/DemoAppTxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighRespChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighTxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighInitDebugChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighDebugChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrTxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrReqstChannel/InterfaceTypes.h>
#include <interfaces/AisJhm2TxChannel/InterfaceTypes.h>
#include <interfaces/PwmInputChannels/InterfaceTypes.h>
#include <interfaces/AutonomyConditionDiagnostics/TxInterfaceInputChannel.h>
#include <interfaces/ShmClock/InterfaceTypes.h>
#include <interfaces/LpsSaTotals/PrinterCnfgInterfaceInputChannel.h>

#include <interfaces/Machine/InterfaceTypes.h>
#include <interfaces/LpsSaNvmCalDataChannel/InterfaceTypes.h>
#include <interfaces/LpsSaNvmCalOnTheFlyDataChannel/InterfaceTypes.h>

#include <interfaces/CalMgrCmdReqst/InterfaceTypes.h>
#include <interfaces/CalMgrCmdResp/InterfaceTypes.h>

#include <autonomyConditions/conditions/PayLdOverLdLimitExceeded.h>

#ifndef _Tpms2DataLinkChannel_h_
#include <interfaces/DataLinkData/InterfaceTypes.h>
#endif

#include <interfaces/DataLinkData/DataLinkData.h>

#include <interfaces/PartNumbers/InterfaceTypes.h>

#include <ais/interfaces/SystemHardwareHealth/InterfaceTypes.h>
#include <ais/interfaces/SystemHardwareHealthRequest/InterfaceTypes.h>

#include <interfaces/ReadyToFlashStatus/InterfaceTypes.h>

#include <mlComms/scsIo/ScsOutputs.h>
#include <scsIOContainer/SCSOutData.h>
#include <ais/config/ConfigSection.h>

#include <catdllib_fid_def.h>

#include <stdio.h>

#include <fstream>


 #include <stdlib.h>


#ifndef __LPS_COMMON_TYPE_DEF_H__
#include <LpsCommonTypeDef.h>
#endif

#ifndef __CPM_FILTER_H__
#include <cpm_filter.h>
#endif

#ifndef __LPS_COMMON_STRUCTURES_H__
#include <LpsCommonStructures.h>
#endif

#ifndef __LPS_PUBLIC_H__
#include <LpsPublic.h>
#endif

#ifndef __LPS_SA_INCLUDES_H__
#include "LpsSaIncludes.h"
#endif

#ifndef  __LPS_COMMON_LOOKUP_TBL_UTILITY_H__
#include <LpsCommonLookUpTblUtility.h>
#endif

#include <commTask/commTask.h>

#include <nvm.h>
#include <scl_prmsw.h>

#include "src_nvm/app_nvm_file_access.h"
#include <time.h>
#ifndef  _LPS_SA_CALMGR_H_
#include "LpsSaCalMgr.h"
#endif


#include <autonomyConditions/conditions/CPMAutonomyGeneric.h>

#ifndef __LPS_CAL_APP_PUBLIC_H__
extern "C" {
#include <LpsCalAppPublic.h>
}
#endif

#ifndef __LPS_CAL_PUBLIC_H__
extern "C" {
#include <LpsCalPublic.h>
}
#endif

#ifndef CAL_MGR_H_
#include <cal_mgr.h>
#endif

#include <autonomyConditions/conditions/TiltLinkVoltageBelow.h>
#include <autonomyConditions/conditions/TiltLinkVoltageAbove.h>
#include <autonomyConditions/conditions/TiltLinkFreqAbnormal.h>
#include <autonomyConditions/conditions/TiltLinkCalibrationOut.h>

#include <autonomyConditions/conditions/LiftLinkVoltageBelow.h>
#include <autonomyConditions/conditions/LiftLinkVoltageAbove.h>
#include <autonomyConditions/conditions/LiftLinkFreqAbnormal.h>
#include <autonomyConditions/conditions/LiftLinkCalibrationOut.h>

#include <autonomyConditions/conditions/LiftREVoltageBelow.h>
#include <autonomyConditions/conditions/LiftREVoltageAbove.h>
#include <autonomyConditions/conditions/LiftREFreqAbnormal.h>


#include <autonomyConditions/conditions/LiftHEVoltageBelow.h>
#include <autonomyConditions/conditions/LiftHEVoltageAbove.h>
#include <autonomyConditions/conditions/LiftHEFreqAbnormal.h>

#include <autonomyConditions/conditions/TiltREVoltageBelow.h>
#include <autonomyConditions/conditions/TiltREVoltageAbove.h>
#include <autonomyConditions/conditions/TiltREFreqAbnormal.h>

#include <autonomyConditions/conditions/TiltHEVoltageBelow.h>
#include <autonomyConditions/conditions/TiltHEVoltageAbove.h>
#include <autonomyConditions/conditions/TiltHEFreqAbnormal.h>

#include <autonomyConditions/conditions/PayloadMonCalibrationOut.h>

#include <autonomyConditions/conditions/MachineModelNotSetOut.h>

#include <autonomyConditions/conditions/PayLdOverLdLimitExceeded.h>

#include <autonomyConditions/conditions/PayloadLegalForTradeNotSealed.h>

#include <autonomyConditions/conditions/BatteryHigh.h>
#include <autonomyConditions/conditions/BatteryLow.h>

#include "adv/TipoffAssist.h"

#include "LpsSaWeighAppCnfg.h"
#include "LpsSaWeighAppNvm.h"
#include "LpsSaWeighAppSeal.h"
#include "LpsSaChassisIMU.h"
#include "LpsSaWeighAppTestFixture.h"

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
class LpsSaWeighApp : public task::Task, public commTask
{
public:

    LpsSaWeighApp( const std::string& taskName );
    virtual ~ LpsSaWeighApp( );

    virtual bool initialize( );
    virtual bool executive( );
    virtual void cleanup( );

    void UpdtCalNvmTbl(const LpsCalNvmTbl_t* pCalNvmTbl);

protected:
    inline std::string makeTempPath(const std::string& fileName) const {
        return (tempRoot_ / fileName).string();
    }

private:

    typedef enum
    {
        ADVANCED = 0,
        STANDALONE
    } LpsSaApplicationVariant_t;

    typedef struct
    {
        vector<float> lift_cyl_ext_pct_axis;
        vector<float> tilt_cyl_ext_pct_axis;
        vector<float> tilt_comp_gain_data;
    } CalibTblRuby_t;

    typedef struct
    {
        /* This structure members may be
         * update in future based on TX channels
         */
        /*TX Param*/
        LpsWeighBktDigStat_t DigStat = LPS_WEIGHT_BKT_DIG_STATE_UNKNOWN; /* Machine Dig Status */
        LpsWeighCalStatus_t CalStat = LPS_WEIGH_SYSTEM_CALIBRATED; /* Lps System Calibration Status */
        LpsWeighBktDumpStat_t DumpStat = LPS_WEIGHT_BKT_DUMP_STATE_UNKNOWN; /* Machine Dump Status */
        float BestBktWtInTonnes = 0.f; /* Best Available Bucket Weight in metric Tonne */
        unsigned short int PayloadCalcMeth = 0; /* Payload Calculation method */
        bool BestBktWtLatched = false;
        bool LatchConditionsOK = false;

        LpsSaPayload_t Payload = {}; /* default constructed */

        LpsWeighRangeIndicator_t Indicator = LPS_BELOW_WEIGH_RANGE;

        LpsSaLiftPosition_t LiftPosition = {}; /* default constructed */
        LpsFloatIO_t LiftCylVel = {LPS_STATUS_OK, 0.f};
        LpsFloatIO_t LiftAngVel = {LPS_STATUS_OK, 0.f};
        LpsFloatIO_t LiftValveCommand = {LPS_STATUS_BAD, 0.f};

        LpsSaTiltPosition_t TiltPosition = {}; /* default constructed */
        LpsFloatIO_t TiltCylVel = {LPS_STATUS_OK, 0.f};
        LpsFloatIO_t TiltValveCommand = {LPS_STATUS_BAD, 0.f};

        /*Tipoff Assist Inputs */
        TipoffAssistInputs TipoffInputs = {}; /* struct is value initialized */
        TipoffAssistOutputs* TipoffOutputsPtr = NULL;
        bool TipOffAssistEnable = false;
        LpsSaWeighTxChannelStorage::ToaAnchorStatus_t ToaAnchorStatus =
                LpsSaWeighTxChannelStorage::ToaAnchorStatus_t::UNCHANGED;

        /* Indicate CAN11-bit message receive timeout */
        bool CAN11MessageTimeoutFlag = false;

        /* Tilt Pressures */
        LpsFloatIO_t     TiltCylHePres = {LPS_STATUS_OK, 0.f};
        LpsFloatIO_t     TiltCylRePres = {LPS_STATUS_OK, 0.f};

        /*RX Param*/
        uint16_t                    passCount = 0;
        float_32                    TruckTargetWeight = 0.f;
        float_32                    TruckStartWeight = 0.f;
        LpsSaJobMgrTipOffState_t    TipOffState = LPS_SA_JOB_MGR_TIP_OFF_PILE_ENABLE;   /*Live state for Pile, Truck, None */
        LpsSaJobMgrTipOffState_t    TipOffStateCfg = LPS_SA_JOB_MGR_TIP_OFF_PILE_ENABLE; /* Configured state for Pile, Truck */

        unsigned char               FilterTauDelay = 0;
        float                       PwmcycleRate = 0.f;
        LpsSaJobMgrStandbyState_t   StandbyState = LPS_SA_JOB_MGR_STANDBY_DEACTIVATED;
        bool                        TipoffActive = false;
        ACDEventPopUp::type         EventState = {}; /* zero initialize */
        ACDDiagPopUp::type          DiagState = {}; /* zero initialize */
        ACDInfoPopUp::type          InfoState = {}; /* zero initialize */
        bool                        ShowExclamationPoint = false;
        float_32                    CycleRate_hz = 0.f;

        /*
         * Payload SEA status
         *  - Initialize Payload installation and Enable status to Installed and Enabled,
         *    we do this so that the user does not see a pop up during initialization before
         *    the SEA status has been received from ACD
         */
        bool SEALevel1EssentialsInstalled = true;

        /* Machine/Model not selected flag */
        bool MachineModelNotSet = false;

        /*CAL resp data*/
        unsigned_8 stepNo = 0;
        uint16_t error = 0;
        uint16_t warning = 0;

        unsigned int FilterSettleDelay = 0;
        
        LpsLeverInfo_t LiftLeverInfo = {};  /* zero initialize */
        bool CalOvrAcknowledge = false;

        /* engine speed for auto-cals */
        int16_t EngineSpeedRPM = 0;

        bool inVerificationMode = false;
        LpsCommonWeightUnits weightUnits = LpsCommonWeightUnits::TONNE;
        float weightInterval = 0.f;
        float weightCapacity = 0.f;
    } LpsSaWeighInfoTbl_t;

    typedef struct
    {
        float TiltLinkageSensorDc = 0.f;
        float LiftLinkageSensorDc = 0.f;
        float LiftCylHePress = 0.f;
        float LiftCylRePress = 0.f;
        float HydOilTemp = 0.f;
        LpsZeroNeededStatus_t PloadSysZeroReqStat = LPS_ZERO_UPDATE_ZERO_NOT_REQUIRED;
        BktPayloadData_t BktPayloadData = BUCKET_PAYLOAD_NOT_AVAILABLE;
        PloadSysCalWtEntryReqStat_t PloadSysCalWtEntryReqStat = CAL_ENTRY_NOT_REQUIRED;
        int16_t QR_HydOilTempMin_C = 0;
        int16_t QR_LiftCylVelMin_mm_sec = 0;
        int16_t QR_LiftCylVelMax_mm_sec = 0;
    } WeighPidTbl_t;

    WeighPidTbl_t WeighPidTbl;
    LinkageMap_t linkage_table_cnfg;
    LpsSaMachineProperties_t machineProperties;
    CalibTblRuby_t CalibTblRuby;

    // These are used in the process of updating simple cal adjustment factor
    unsigned int storePressCount;
    float sumOfAdjustedTruckWts;
    float sumOfZeroedTruckWts;
    float updatedSimpleCalFactor;

    bool demo_mode;

    std::string machineMSN;
    std::chrono::steady_clock::time_point ZeroAcceptedInfoExpireTime = std::chrono::steady_clock::time_point::min();
    std::chrono::steady_clock::time_point TooHeavyToZeroInfoExpireTime = std::chrono::steady_clock::time_point::min();

    // This is passed to the weigh library for it's update.
    LpsUpdtTbl_t weighUpdtTbl;

    LpsSaWeighDebugChannel DebugLpsSaXCPChannels;

    LpsSaInitTbl_t LpsSaInitTbl;

    DERFILT LiftCylVelLpFilt;
    DERFILT TiltCylVelLpFilt;
    DERFILT LiftAngVelLpFilt;

    LPFILT2 TiltLinkageSensorDcLpFilt;
    LPFILT2 LiftLinkageSensorDcLpFilt;

    LpsSaWeighReqstChannelStorage request_;

    // This is data needed by the app, but not the weigh library.
    LpsSaWeighInfoTbl_t LpsSaWeighInfoTbl;

    PwmInputChannels inPwm_;

    struct {
    	float defaultStartOfWeighRange = DEFAULT_WEIGH_RANGE_START;
    	float defaultEndOfWeighRange = DEFAULT_WEIGH_RANGE_END;
    	float minimumWeighRangeSize = DEFAULT_MIN_WEIGH_RANGE_SIZE;
    	float minimumWeighRangeStart = DEFAULT_MIN_WEIGH_RANGE_START;
    	float maximumWeighRangeEnd = DEFAULT_MAX_WEIGH_RANGE_END;
    } WeighRangeConfig;

    LpsSaWeighAppCnfg cnfg_;
    LpsSaTiltCalNvmTbl tiltCalNvmTbl_;
    LpsSaLiftCalNvmTbl liftCalNvmTbl_;
    LpsSaPayloadCalNvmTbl payloadCalNvmTbl_;
    LpsSaChassisIMU chassisImu_;
    LpsSaWeighAppSealTracker sealTracker_;
    LpsSaWeighAppTestFixture testFixture_;

    boost::filesystem::path storageRoot_;
    boost::filesystem::path tempRoot_;

    /* SHM and Time Zone */
    uint_least32_t serviceHourMeter_;
    struct {
        int32_t offset;
        int32_t index;
    } tzInfo_;

    DemoAppTxChannel demoInputs_;

    LpsSaWeighRespChannelOutput* LpsSaWeighScsRespOut;
    LpsSaWeighTxChannelOutput* LpsSaWeighScsTxOut;
    ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>* LpsSaWeighScsReqstIn;
    ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>* LpsSaWeighScsRespOut_ROS2;
    ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>* LpsSaWeighScsTxOut_ROS2;

    // ROS2/DDS shared node + executor
    rclcpp::Node::SharedPtr rosNode_;
    rclcpp::executors::SingleThreadedExecutor executor_;

    LpsSaJobMgrReqstChannelOutput* LpsSaJobMgrScsReqstOut;
    ReadyToFlashStatusOutput *ReadyToFlashStatusOutput;
    PwmInputChannelsInput* PwmIn;
    LpsSaJobMgrTxChannelInput* LpsSaJobMgrScsTxIn;
    DemoAppTxChannelInput* DemoAppTxIn;
    AisJhm2TxChannelInput* AisJhm2TxInputScs;
    AutonomyConditionDiagnosticsTxInterfaceInputChannel* AutonomyConditionDiagnosticsTxInputChannel;
    MachineInput* MachineIn;
    LpsSaWeighInitDebugChannelOutput* LpsSaWeighScsInitDebugOut;
    LpsSaWeighDebugChannelOutput* LpsSaWeighScsDebugOut;
    CalMgrCmdReqstInput* LpsCalCmdScsReqstIn;
    CalMgrCmdRespOutput* LpsCalCmdScsRespOut;
    LpsSaNvmCalDataChannelOutput* LpsNvmDumpChanOut;
    LpsSaNvmCalOnTheFlyDataChannelOutput* LpsNvmOnTheFlyDumpChanOut;
    DataLinkDataInput* DataLinkDataInput_;
    PartNumbersInput* PartNumbersInput_;
    SystemHardwareHealthInput* SystemHardwareHealthInput_;
    SCSOutData<SystemHardwareHealthRequest> SystemHardwareHealthRequestOutput_;
    LpsSaUIDisplayStateInterfaceInputChannel* displayStateInput_;

    // For receiving the current printer configuration
    LpsSaTotalsPrinterCnfgInterfaceInputChannel* printerCnfgInput_;

    ShmClockInput* shmClockInput_;

    bool LinkageCalInProgress;

    LpsSaApplicationVariant_t getApplicationVariant();
    void PwmInputRead();
    void LpsJobMgrTxRead( );
    void LpsSaWeighingScsRx( );
    void LpsSaWeighScsDebugTx();
    void LpsSaWeighScsInitDebugTx();
    void AisJhmDataServerTxRead();
    void LpsSaSEAStatus();
    bool LpsSaWeighingScsTx();
    void LpsSaScsSendZeroRqst();

    bool LpsSaScsSendReqstResponse(LpsSaWeighReqstChannel::Command command, bool success);
    bool LpsSaScsSendReqstResponse(const cpm_common_interfaces::msg::LpsSaWeighReqstChannel& request, bool success, const std::string& arg1 = "");
    void LpsSaScsChkForReqst();
    LpsSaInitErrorType_t LpsSaInit(void);
    LpsSaUpdtErrorType_t LpsSaUpdt(void);
    LpsUpdtErrorTypes_t LpsSaUpdateWeighingLib();
    LpsHydOilTemp_t LpsSaHydOilTempTransferFunc(void);
    static LpsFloatIO_t UpdateVelocity(DERFILT& filt, float value, LpsStat_t status);
    LpsFloatIO_t LiftHeadEndPresTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout);
    LpsFloatIO_t LiftRodEndPresTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout);
    LpsFloatIO_t TiltHeadEndPresTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout);
    LpsFloatIO_t TiltRodEndPresTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout);
    LpsUpdtErrorTypes_t ProcessInputs();

    float lpsSaGetLiftAngle(float liftLinkageDc, bool liftOk);
    LpsSaLiftPosition_t lpsSaGetLiftPosition(float liftAngle, bool liftOk);
    LpsSaLiftPosition_t LpsSaLiftSensorTransferFunc(int width, int period, PwmInputSwitchStatus SwitchStatus, PwmInputTimeout Timeout);

    float lpsSaGetNormalizedTiltPosition(float tiltLinkageDc, bool tiltOk);
    float lpsSaGetTiltAngle(float tiltNormalizedPosition, bool tiltOk);
    LpsSaTiltPosition_t lpsSaGetTiltRotaryPosition(float liftAngle, bool liftOk, float tiltAngle, bool tiltOk);
    LpsSaTiltPosition_t lpsSaGetTiltInlinePosition(float liftAngle, float liftCylinderLength, bool liftOk, float tiltCylinderLength, bool tiltOk);
    LpsSaTiltPosition_t LpsSaTiltSensorTransferFunc(const LpsSaLiftPosition_t& liftPosition, int tiltSensorPulseWidth, int tiltSensorPeriod, PwmInputSwitchStatus tiltSwitchStatus, PwmInputTimeout tiltTimeout);

    bool LpsSaInitWeighTbl(void);
    void NvmInitialize(void);
    bool LpsSaLoadKinematicsTbl(void);
    bool LpsSaLoadMachineProperties(void);
    void LpsSaBmiRead(void);
    void LpsSaBattVoltageRead(void);
    void LpsSaLoadDefaultCalibrationTbl(void);

    void CheckForFilterSettleTime(void);
    void UpdateCalibrationDiagnostics(void);
    void LpsSaWeighGetAllInfoPopUp();

    void UpdateEvents(void);
    unsigned int GetSysMonotonicTime();
    PloadSysZeroStat_t GetZeroStat(void);

    void setPayloadNotCalibrated();

    void setLiftLinkageSensorCalibrated(float fullLowerDutyCycle, float fullRaiseDutyCycle);
    bool setLiftLinkageSensorFullLowerCalValue(float fullLowerDutyCycle);
    bool setLiftLinkageSensorFullRaiseCalValue(float fullRaiseDutyCycle);
    void setLiftLinkageSensorNotCalibrated();

    void setTiltLinkageSensorCalibrated(float fullDumpDutyCycle, float fullRackDutyCycle);
    bool setTiltLinkageSensorFullDumpCalValue(float fullDumpDutyCycle);
    bool setTiltLinkageSensorFullRackCalValue(float fullRackDutyCycle);
    void setTiltLinkageSensorCalAngleLimits(float fullDumpAngle, float fullRackAngle);
    void setTiltLinkageSensorNotCalibrated(void);

    /* Audible Tone for Payload */
    LpsSaWeighTxChannel::AudibleAnnunciationPriority_t getAudibleCommand();
    LpsSaWeighTxChannel::AudibleAnnunciationPriority_t audibleTriggered_;
    std::chrono::steady_clock::time_point audibleTriggerTimepoint_;
    std::chrono::steady_clock::time_point flashUpdateTimepoint_;
    LpsWeighRangeIndicator_t prevWeighRangeIndicator_;

    void ReInitWeighLib( void );

    /*
     * Calibration functions
     *  There is a mutex for the lps_cal library to make it thread safe.
     *  The executive/periodic thread calls "CalLibUpdt" which will collect the calibration data.
     *  The callback thread updates the calibration state machine for cal manager.
     */
    std::mutex calLibMtx_;
    boolean CalLibInit(void);
    void CalLibUpdt(void);
    void LpsSaWeighCalReqstCallback();

    CAL_MGR_MR_E LpsSaTiltRotaryLinkageSensorCalibration (CAL_MGR_MC_E Cmd,
            uint16_t CalId, unsigned_8* StepNo, uint16_t* CalError);
    CAL_MGR_MR_E LpsSaTiltInlineLinkageSensorCalibration (CAL_MGR_MC_E Cmd,
            uint16_t CalId, unsigned_8* StepNo, uint16_t* CalError);
    CAL_MGR_MR_E LpsSaLiftLinkageSensorCalibration (CAL_MGR_MC_E Cmd,
            uint16_t CalId, unsigned_8* StepNo, uint16_t* CalError);

    bool GetPayloadMonSysCalStatus(void);
    bool GetLiftCylCalStatus(void);
    bool GetTiltCylCalStatus(void);
    bool GetCalWtRequired(void);
    bool GetPayloadCalStatus(void);

    void copyCalNVMToWeighInitTable(LpsCalNvmTbl_t& to);

    bool setHydOilTempEnableStatus(bool enabled);
    bool setIMUCompEnableStatus(bool enabled);
    bool setLoadCheckValveInstallStatus(bool installed);
    bool setAudibleWeightEnableStatus(bool enabled);
    void setIMUPitchCalOffsetNVM(float imu_cal_offset);
    void setWeighRange(float weighRangeBottom, float weighRangeSize);

    bool setFlashEnableStatus(bool enabled);

    void flashEnablerUpdate();

    void resetToaAnchorValues(void);

    void PublishCalFromNvmPayload( void ); // Populates some custom data from calibration table into an SCS object and publishes it for XCP server

    void saveIMUCalResultsToFile(LpsCalIMUResults_t imu_cal_results, uint16_t cal_id);
    void readIMUCalResultsFromFile(LpsCalIMUResults_t& imu_cal_results);

    void logWeighRangeWeighInit() const;
    void logWeighRangeWeighUpdate() const;
    bool publishRecentWeighResults(const boost::filesystem::path& filePath) const;
    void logWeighCalResults() const;
    void logWeighProductIdFile(const std::string& newProductId) const;

    // period variables to run transmit info less frequently in host transmit thread
    // transmitPeriodTime_ = (transmitPeriodSubCycles_)/(host thread period)
    float transmitPeriodTime_; // Tx period Time in Seconds
    int32_t transmitPeriodCount_; // period running loop count to subdivide the host thread period


    // Private scheduler to run transmit info less frequently in host transmit thread
    // transmitPeriodTime_ = (transmitPeriodSubCycles_)/(host thread period)
    inline bool transmitScheduler() {
        bool transmit = false;

        //DECREMENT period count for each host cycle
        --transmitPeriodCount_;
        if (transmitPeriodCount_ <= 0) {
            // Transmit data.
            transmit = true;

            // RESET period count to start new subcycle
            transmitPeriodCount_ = static_cast<decltype(transmitPeriodCount_)>(LpsSaWeighInfoTbl.CycleRate_hz*transmitPeriodTime_);
        }

        return(transmit);
    }
};

#if defined (__cplusplus)
extern "C"
{
#endif

extern bool CalNVMReinitFlag;
void AppGetCalOvCtrl(boolean enable);
void AppEnableDisableQualRead(unsigned_8 Enable, signed_16* minvalue);
void AppUpdtCalNvmTbl(const LpsCalNvmTbl_t* pCalData);

#if defined (__cplusplus)
}
#endif

/*******************************************************************************
** -- Function Prototypes --
*******************************************************************************/

/*******************************************************************************
** -- Data Declarations --
*******************************************************************************/
// This is the one and only one instance of this task.
LpsSaWeighApp *temp_thisTask;

#endif
