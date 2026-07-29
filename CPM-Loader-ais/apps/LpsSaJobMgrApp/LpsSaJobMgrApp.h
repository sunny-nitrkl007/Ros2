/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaJobMgrApp.h
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef LPSSAJOBMGRAPP_H
#define LPSSAJOBMGRAPP_H

#include <string>
#include <deque>
#include <chrono>

#include <boost/filesystem.hpp>

#ifndef __LPS_COMMON_TYPE_DEF_H__
#include <LpsCommonTypeDef.h>
#endif

#ifndef __LPS_COMMON_STRUCTURES_H__
#include <LpsCommonStructures.h>
#endif

#ifndef __LPS_PT_PUBLIC_H__
#include <LpsPtPublic.h>
#endif
#include <LpsPrivate.h>
#include <ais/task/Task.h>

#include <interfaces/LpsSaJobMgrDebugChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrTxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrReqstChannel/InterfaceTypes.h>
#include <interfaces/SwitchInputScs/InterfaceTypes.h>
#include <interfaces/OutputChannel/InterfaceTypes.h>
#include <interfaces/AisJhm2TxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaLoadRecordChannel/Channel/Output/channel.h>
#include <interfaces/LpsSaLoadRecordChannel/LpsSaLoadRecordChannel.h>
#include <interfaces/LpsSaJobMgrRespChannel/LpsSaJobMgrRespChannel.h>
#include <interfaces/LpsSaJobMgrReqstChannel/LpsSaJobMgrReqstChannel.h>
#include <interfaces/LpsSaJobMgrRespChannel/InterfaceTypes.h>
#include <interfaces/LpsSaUI/DisplayStateInterfaceInputChannel.h>
#include <interfaces/ShmClock/InterfaceTypes.h>
#include <interfaces/DataLinkData/InterfaceTypes.h>
#include <interfaces/AutonomyConditionDiagnostics/TxInterfaceInputChannel.h>
#include <interfaces/EventDiagnosticData/InterfaceTypes.h>

#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp>

// ---- ROS2/DDS shim layer (Development-Plan.txt Step 4.1) ----------------
// NOTE: the interface-type includes above are KEPT, not removed -- they
// still supply real struct/enum types this file and LpsSaJobMgrSimpleCal.h
// depend on directly (SimpleCalData_t, DispBestBktWt_t, the tip-off/manual-
// add enums, etc.). Only the raw SCS channel POINTER types below are being
// replaced; the underlying AIS struct definitions are untouched.
#include <rclcpp/rclcpp.hpp>
#include <ros_shim/RosInputInterface.h>
#include <ros_shim/RosOutputInterface.h>

#include <job_mgr_interfaces/msg/lps_sa_job_mgr_tx_channel.hpp>
#include <job_mgr_interfaces/msg/lps_sa_job_mgr_resp_channel.hpp>
#include <job_mgr_interfaces/msg/lps_sa_job_mgr_debug_channel.hpp>
#include <job_mgr_interfaces/msg/switch_input_scs.hpp>
#include <job_mgr_interfaces/msg/output_channel.hpp>
#include <job_mgr_interfaces/msg/ais_jhm2_tx_channel.hpp>
#include <job_mgr_interfaces/msg/lps_sa_ui_display_state.hpp>
#include <job_mgr_interfaces/msg/lps_sa_ui_display_state_interface.hpp>
#include <job_mgr_interfaces/msg/shm_clock_input.hpp>
#include <job_mgr_interfaces/msg/data_link_data.hpp>
#include <job_mgr_interfaces/msg/lps_sa_load_record_channel.hpp>
#include <job_mgr_interfaces/msg/autonomy_condition_diagnostics_tx_channel.hpp>
#include <job_mgr_interfaces/msg/event_diagnostic_data.hpp>
#include <cpm_common_interfaces/msg/lps_sa_job_mgr_reqst_channel.hpp>
// weighAppInf_'s 3 channels (WeighReqst/Resp/Tx) are NOT converted here --
// LpsSaWeighAppInf lives in a shared header (interfaces/LpsSaWeighReqstChannel/
// LpsSaWeighAppInf.hpp), outside this file. Deferred to Step 6 (Direct DDS
// Wiring) as its own decision, not silently folded into this edit.

#include "LpsSaJobMgrTasks.h"
#include "LpsSaJobMgrCnfg.h"
#include "LpsSaJobMgrStats.h"
#include "LpsSaJobMgrSimpleCal.h"

/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/
//#define DEBUG

#define SUCCESS           (boolean) 1
#define FAIL              (boolean) 0

#define TEMP_STORAGE_ROOT (R"(/tmp/appdata/CPM/LpsSaJobMgrApp/)")

typedef enum
{
    LPS_SA_JOB_MGR_SCS_RESP_RECVD = 0,
    LPS_SA_JOB_MGR_SCS_RESP_INCORRECT,
    LPS_SA_JOB_MGR_SCS_RESP_NOT_RECVD,
    LPS_SA_JOB_MGR_SCS_RESP_UNKNOWN_ERR
} LpsSaJobMgrScsChkCmdResponseStat_t;

typedef enum
{
    LPS_SA_JOB_MGR_INIT_SUCCESS = 0,
    LPS_SA_JOB_MGR_SCS_INIT_ERROR,
    LPS_SA_JOB_MGR_PT_INIT_ERROR,
} LpsSaJobMgrInitErrorType_t;


class LpsSaJobMgrApp: public task::Task
{
public:
    LpsSaJobMgrApp( const std::string& taskName );
    virtual ~LpsSaJobMgrApp( );

    virtual bool initialize( );
    virtual bool executive( );
    virtual void cleanup( );

protected:
    inline std::string makeStoragePath(const std::string& fileName) const {
        return (storageRoot_ / fileName).string();
    }

private:

    struct LpsJobMgrJobTrackerInfoTbl_t {
        LpsSaJobMgrManualTipOffState_t ManualTipOffState = LPS_SA_JOB_MGR_MAN_TIP_OFF_UNAVAILABLE;
        LpsSaJobMgrTipOffState_t TipOffState = LPS_SA_JOB_MGR_TIP_OFF_UNAVAILABLE;
        LpsSaJobMgrOperationMode_t OperationMode = LPS_SA_JOB_MGR_WEIGH_MODE;
        DispBestBktWt_t DispBestBktWt = { 0.f, false };
        LpsWeighBktDigStat_t DigStat = LPS_WEIGHT_BKT_DIG_STATE_UNKNOWN; /* Machine Dig Status */
        LpsWeighCalStatus_t CalStat = LPS_WEIGH_SYSTEM_CALIBRATED; /* Lps System Calibration Status */
        LpsWeighBktDumpStat_t DumpStat = LPS_WEIGHT_BKT_DUMP_STATE_UNKNOWN; /* Machine Dump Status */
        ReqPloadCtrlSysStat_t ReqPloadCtrlSysStat = ReqPloadCtrlSysStat_t::NONE;
        bool TipoffAssistActive = false;
        bool TipoffAssistPIDActive = false;
        uint_fast16_t TipoffAssistActiveEid = 0;
        bool ManualAddPIDActive = false;
        bool ManualAddPIDPreviouslyDepressed = false;
        bool StorePIDActive = false;
        bool StorePIDPreviouslyDepressed = false;
        unsigned int storePressCount = 0;
        float zeroWeight = 0.f;
        float simpleCalAdjust = 1.f;
        bool splitModeNextPayloadCmd = false;
        uint16_t subtotalIndex;
        bool selectSubtotalCmd = false;
        bool lftSealed = false;
        bool memoryFull = false;
        bool inVerificationMode = false;
    };

    /* SHM and Time Zone */
    uint_least32_t serviceHourMeter_;
    struct {
        int32_t offset;
        int32_t index;
    } tzInfo_;

    LpsPtInputs_t                    LpsSaJobMgrWmInput;
    LpsPtOutputs_t                   LpsSaJobMgrWmOutput;

    LpsJobMgrJobTrackerInfoTbl_t     LpsJobMgrJobTrackerInfoTbl;

    /* ROS2/DDS shims -- replace the raw SCS channel objects. Member names
       kept identical to the originals; only the declared type changes.
       ->get()/->publish() call syntax is unchanged at every call site. */
    rclcpp::Node::SharedPtr rosNode_;
    rclcpp::executors::SingleThreadedExecutor executor_;

    ros_shim::RosOutputInterface<job_mgr_interfaces::msg::LpsSaJobMgrTxChannel>        *LpsSaJobMgrScsTxOut;/* from job manager write param to UI*/
    ros_shim::RosInputInterface<cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel>   *LpsSaJobMgrScsReqstIn;/*get request from UI*/
    ros_shim::RosOutputInterface<job_mgr_interfaces::msg::LpsSaJobMgrDebugChannel>     *LpsSaJobMgrScsDebugOut;/*Debug symbols*/
    ros_shim::RosOutputInterface<job_mgr_interfaces::msg::LpsSaJobMgrRespChannel>      *LpsSaJobMgrRespChannelOutput_;

    bool weighAppTxDataReceived_;
    LpsSaWeighAppInf weighAppInf_; // WeighApp Interface -- NOT yet converted, see Step 6 note above

    ros_shim::RosInputInterface<job_mgr_interfaces::msg::SwitchInputScs>               *LpsSaSwitchInput;
    ros_shim::RosOutputInterface<job_mgr_interfaces::msg::OutputChannel>               *LpsSaOutputChannelOut;

    ros_shim::RosInputInterface<job_mgr_interfaces::msg::AisJhm2TxChannel>             *AisJhm2TxInputScs;

    // Type corrected during Step 4.2 -- see LpsSaUIDisplayStateInterface.msg
    // header for why this needs the wrapper type, not LpsSaUIDisplayState
    // directly.
    ros_shim::RosInputInterface<job_mgr_interfaces::msg::LpsSaUIDisplayStateInterface>* displayStateInput_;

    ros_shim::RosInputInterface<job_mgr_interfaces::msg::ShmClockInput>                *ShmClockInputScs;

    ros_shim::RosInputInterface<job_mgr_interfaces::msg::DataLinkData>                 *dataLinkDataInput_;

    ros_shim::RosOutputInterface<job_mgr_interfaces::msg::LpsSaLoadRecordChannel>* loadRecordOutputChannel_;

    LpsSaJobMgrTasks tasks_;

    LpsSaJobMgrCnfg config_;

    LpsSaJobMgrStats stats_;

    LpsSaJobMgrSimpleCal simpleCal_;

    boost::filesystem::path storageRoot_;

    float defaultTargetWeight_;

    std::string machineMSN;

    std::chrono::steady_clock::time_point storeRejectedExpireTime;

    ros_shim::RosInputInterface<job_mgr_interfaces::msg::AutonomyConditionDiagnosticsTxChannel>* autonomyConditionDiagnosticsTxInputChannel_;
    bool SEALevel1EssentialsInstalled_;
    bool SEALevel2ProInstalled_;
    bool SEALegalForTradeInstalled_;

    ros_shim::RosInputInterface<job_mgr_interfaces::msg::EventDiagnosticData>* eddtInputChannel_;

    bool loadOldLoadRecord(LpsSaLoadRecordChannel& loadRecord);

    bool saveConfig(void);

    bool parseUiConfigurableFeatures(void);	// check if Show/Hide config has tip-off disabled

    void LpsSaJobMgrPtInit(void);/* Pass tracker lib Initialization*/
    boolean LpsSaJobMgrPtUpdate(void);/*Job Manager App Pt Lib Update*/
    void LpsSaJobMgrPtRestoreTruck(void);

    /*SCS  Functions*/
    void LpsSaWeighScsTxParamRead(void);/*get parameters from weighing app*/
    void LpsSaJobMgrScsChkForReqst(void);/*get request from UI*/

    boolean LpsSaJobMgrScsRx(void);/*get parameters from weighing app*/
    boolean LpsSaJobMgrScsTx(void);/*write param to UI*/
    void LpsSaJobMgrScsSendCmd(LpsSaWeighReqstChannel::Command command); /* send command to weighing app */
    void LpsSaJobMgrSendCmdToWeighApp(void);
    void LpsSaJobMgrHandleTipOffBtnStates(void);
    void AisJhmDataServerTxRead(void);
    void LpsSaJobMgrScsSHMRead(void);
    void LpsSaJobMgrScsDataLinkDataRead(void);
    void LpsSaJobMgrScsEddtRead();
    bool sendReqstResponse(const LpsSaJobMgrReqstChannel& request, bool success);
    ReqPloadCtrlSysStat_t GetActiveButtonStatus (void);
    void LpsSaJobMgrScsChkHornAction(void);
    bool LpsSaJobMgrHornOnStoreAction(void);

    // Reset LpsPtInputs flags
    void resetLpsPtInputFlags(void);

    LpsWeighBktWtAccuracy_t totalWeightAccuracy_;

    enum class LftStoreAllowedStatus_t { ALLOW_STORE, REJECT_STORE_MEMORY_FULL, REJECT_STORE_LOW_ACCURACY };

    // Disable store for LFT when accuracy below High accuracy OR memory is full
    // When LFT is not sealed accuracy can not exceed ACCURACY_MED
    inline LftStoreAllowedStatus_t getLftStoreAllowedStatus() {
        if (SEALegalForTradeInstalled_) {
            if (LpsJobMgrJobTrackerInfoTbl.memoryFull) {
                return LftStoreAllowedStatus_t::REJECT_STORE_MEMORY_FULL;
            }
            else if (tasks_.currentTaskGetLFTDisable()) {
                return LftStoreAllowedStatus_t::ALLOW_STORE;
            }
            else if (totalWeightAccuracy_ < LPS_WEIGH_BUCKET_WEIGHT_ACCURACY_HIGH) {
                return LftStoreAllowedStatus_t::REJECT_STORE_LOW_ACCURACY;
            }
            else {
                return LftStoreAllowedStatus_t::ALLOW_STORE;
            }
        }
        else {
            return LftStoreAllowedStatus_t::ALLOW_STORE;
        }
    }
};

#endif
