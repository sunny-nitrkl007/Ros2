///////////////////////////////////////////////////////////////////////////////
// @attention COPYRIGHT (C) 2011-2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
//
// @file    AutonomyConditionDiagnostics.h
//
// @brief   This is the definition include file for the module.
///////////////////////////////////////////////////////////////////////////////
#ifndef AUTONOMYCONDITIONDIAGNOSTICS_H_
#define AUTONOMYCONDITIONDIAGNOSTICS_H_

///////////////////////////////////////////////////////////////////////////////
// -- #Include's --
///////////////////////////////////////////////////////////////////////////////
#include <chrono>

#include "task/AutonomyTask.h"
#include <ais/time/TimeStamp.h>
#include <ais/time/EventTimer.h>
#include <ais/interfaces/AutonomyConditionMessage/InterfaceTypes.h>
#include <interfaces/DiagnosticStatus/InterfaceTypes.h>
#include <interfaces/ShmClock/InterfaceTypes.h>
#include <commTask/commTask.h>
#include "AutoDiagEvent.h"
#include "AutonomyConditionsList.h"
#include "InhibitConditionList.h"
#include "DiagOutFileWriter.h"
#include <boost/filesystem.hpp>
#include "ECMSummaryWriter.hpp"

#include <interfaces/PartNumbers/InterfaceTypes.h>
#include <ais/interfaces/SystemHardwareHealth/InterfaceTypes.h>
#include <interfaces/AisJhm2TxChannel/InterfaceTypes.h>
#include <ais/conditions/MachineSleep.h>
#include "ais/time/EventTimer.h"
#include <scl_prmsw.h>
#include <interfaces/SEAStatus/InterfaceTypes.h>
#include <AcdCalScsInf.h>
#include <interfaces/LpsSaWeighReqstChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighRespChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighTxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrTxChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrReqstChannel/InterfaceTypes.h>
#include <interfaces/LpsSaJobMgrRespChannel/InterfaceTypes.h>
#include <interfaces/SwitchInputScs/InterfaceTypes.h>
#include <interfaces/DataLinkData/InterfaceTypes.h>
#include <interfaces/LpsSaTotals/TotalsInterfaceInputChannel.h>
#include <interfaces/AutonomyConditionDiagnostics/TxInterface.hpp>
#include <interfaces/AutonomyConditionDiagnostics/TxInterfaceInputChannel.h>
#include <interfaces/AutonomyConditionDiagnostics/TxInterfaceOutputChannel.h>
#include <interfaces/AutonomyConditionDiagnostics/RequestInterfaceInputChannel.h>
#include <interfaces/AutonomyConditionDiagnostics/ResponseInterfaceOutputChannel.h>
#include <interfaces/LpsSaTotals/RequestInterfaceOutputChannel.h>
#include <interfaces/LpsSaTotals/PrinterCnfg.hpp>
#include <interfaces/LpsSaTotals/PrinterCnfgInterfaceInputChannel.h>
#include <interfaces/LpsSaTotals/PrinterCnfgInterfaceOutputChannel.h>
#include <interfaces/LpsSaUI/DisplaySettings.hpp>
#include <interfaces/LpsSaUI/DisplayStateInterfaceInputChannel.h>
#include <interfaces/LpsSaUI/DisplaySettingsInterfaceOutputChannel.h>
#include <interfaces/DataLinkData/InterfaceTypes.h>
#include <interfaces/WorkOrderAssist/SettingsInterfaceInputChannel.h>
#include <interfaces/WorkOrderAssist/SettingsInterfaceOutputChannel.h>
#include <interfaces/LpsSaUI/BEMSimStateInterfaceInputChannel.h>
#include <autonomyConditions/conditions/BEMSimDerate.h>
#include <autonomyConditions/conditions/BEMSimLevelWarning.h>
#include <interfaces/CycleSeg/TxInterfaceInputChannel.h>

#include <taskPlugins/base/TaskPluginMgr.h>

#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp>

///////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
	TIP_OFF_TRUCK = 0x0581,
	TIP_OFF_PILE = 0x0582,
	TIP_OFF_DISABLED = 0x000D,
	TIP_OFF_MANUAL = 0x0070,
	TIP_OFF_AUTO = 0x00D7
}TipOff_t;

typedef enum
{
	MINUS_ONE_ENABLED = 0x000C,
	MINUS_ONE_DISABLED = 0x000D
}PloadRemLastPassButtonDispStat_t;

typedef enum
{
	CLEAR_BUTTON_ENABLED = 0x000C,
	CLEAR_BUTTON_DISABLED = 0x000D
}PloadClrButtonDispStat_t;


extern bool HydOilTempEnabled;

///////////////////////////////////////////////////////////////////////////////
// -- Class Definition
///////////////////////////////////////////////////////////////////////////////
class AutonomyConditionDiagnostics: public task::AutonomyTask, public commTask,LpsSaWeighReqstChannelStorage,LpsSaWeighRespChannelStorage, public TaskPluginMgr
{
    public:
        using Clock = std::chrono::system_clock;
        using TimePoint = Clock::time_point;

        AutonomyConditionDiagnostics( const std::string& taskName );
        virtual ~AutonomyConditionDiagnostics( );

        virtual bool autonomyInitialize(void);
        virtual bool autonomyExecutive(void);
        virtual void autonomyCleanup(void);

   protected:

   private:
	  	bool initializeScsChannels( );
        bool initDiagEvent( scl_ci_es_fault_cfg_eddt_t* logConfig, AutoDiagEvent::DiagEventType type, const std::string& name );
        bool initDiagEvent( scl_ci_es_fault_cfg_eddt_t* logConfig, AutoDiagEvent::DiagEventType type, const std::string& name, scl_obd_test_handle_t handle, bool externallyReported );
        bool initializeDLDiagEvents( );
        bool initializeDiagEvents( );
        bool initializeEDDT( );
        bool acceptedHost(const std::string& thisHostName) const;
        int  getDiagEvent( const std::string& name );
        void processHardwareHealthStatus();
        void processDispBrightness();
        //NVM Testing - Diagnostic Control
        void diagnosticControl(void);
        void postInitialize();
        bool getRTCconfig();
        bool getEcmJ1939Name();
        bool loadSEAconfig();
        bool loadDeviceIdConfig();
        bool loadNVMconfig();
        bool loadECMSummaryconfig();
        void updateSEAStatus();
        void publishSHM();
        void publishPartNumbers();
        void IssueCmdToWeighApp(void);
        void CheckForWeighAppCmdResp(void);
        void CreateECMSummaryFile(void);
        void ReadWeighAppScsTx(void);
        void DataLinkDataScsRx(void);
        void CheckEthernetDiags(void);
        void ReadStoreSwitchTx(void);
        void ReadFromUITx();
        void IssueCmdToUIApp();
        void PublishAutonomyConditionsTx();
        void ReadJobMgrAppScsTx();
        void IssueCmdToJobMgrApp(void);
        void ReadSystemParam(void);
        void ReadTotalsScs(void);
        void ReadCycleSegScs(void);
        void IssueCmdToTotalsApp();

        // Dispatch (Work Order Assist) Settings
        void ReceiveDispatchSettings();
        void WriteDispatchSettings();

        void publishNewProductId();
        void ClearPidFlags();
        // TaskPluginMgr
        bool addPlugins() override;
        void appVersionInit();
        void ScsChkForReqst();
        void ReadBEMSimState();

        //Configuration
        bool                            m_enableOutfileWriter;         // Whether or not to write diagnostic details to output file.
        bool                            m_EDDT_init;
        PartNumbersStorage              m_partNumbers;

        //SCS Channels
        AutonomyConditionMessageInput*  m_pAutonomyConditionMessageIn; // Input SCS channel to receive Conditions
        DiagnosticStatusOutput*         m_pDiagStatusOut;              // Output SCS channel for DiagnosticStatus
        SystemHardwareHealthInput*      m_pHealthInput;
        AisJhm2TxChannelInput*          m_pAisJhm2TxChannel;             // Input SCS channel to receive DispBrightness from UI
        DataLinkDataInput*              m_dataLinkDataInput;           // Input SCS channel for datalink object

        // BEM Sim
        LpsSaUIBEMSimStateInterfaceInputChannel* m_BEMSimStateInput;    // Input SCS channel for BEMSimState

        ShmClockOutput*                 m_pSHMOutput;
        PartNumbersOutput*              m_pPartNumbersOutput;
        std::string                     equipmentId_;

        //Internal State
        std::vector<std::string>        m_hostnameWhiteList;          // List of hosts to listen to conditions from
        bool                            m_useHostnameWhiteList;       // True if the hostnames should be checked against the whitelist/false otherwise
        std::vector<AutoDiagEvent>      m_diagEventList;              // List of configured Diagnostics/Events
        AutonomyConditionsList          m_autoCondList;               // Combined list of all incoming autonomyConditions
        InhibitConditionList            m_inhibitList;                // Entire list of available inhibit conditions
        DiagOutFileWriter               m_outfileWriter;              // Writer to send information to diagnostic output file

        //NVM Testing - Diagnostic Control
        bool                            m_initDone;
        EventTimer                      m_seaStatusTimer;
        SEAStatusOutput*                m_seaStatusOutput;
        SEAStatus                       m_seaStatus;
        EventTimer                      m_shmPublishTimer;

        bool                            isConceptSoftware_;

        LpsSaWeighAppInf weighAppInf_;

        LpsSaJobMgrTxChannelInput    	  *LpsSaJobMgrScsTxIn;
        LpsSaJobMgrReqstChannelOutput   *LpsSaJobMgrScsReqstOut;/*get request from UI*/
        LpsSaJobMgrRespChannelInput     *LpsSaJobMgrScsRespIn;/*setRes to UI*/

        // For Dispatch Settings (Work Order Assist)
        WorkOrderAssistSettingsInterfaceStorage dispatchSettings_;
        WorkOrderAssistSettingsInterfaceInputChannel* dispatchSettingsInput_;
        WorkOrderAssistSettingsInterfaceOutputChannel* dispatchSettingsWrOutput_;

        SwitchInputScsInput 		      *LpsSaSwitchInput; /* Receive Store switch input */

        AutonomyConditionDiagnosticsRequestInterfaceInputChannel  *autonomyConditionDiagnosticsScsReqstIn_;/*get request from UI*/
        AutonomyConditionDiagnosticsResponseInterfaceOutputChannel   *autonomyConditionDiagnosticsRespChannelOutput_;

        // For receiving lifetime totals
        LpsSaTotalsInterfaceInputChannel* totalsInputChannel_;
        LpsSaTotalsRequestInterfaceOutputChannel* totalsRequestOutputChannel_;

        // Tx channel
        AutonomyConditionDiagnosticsTxInterfaceOutputChannel *autonomyConditionDiagnosticsTxOutputChannel_;
        AutonomyConditionDiagnosticsTxInterface autonomyConditionDiagnosticsTx_;

        // Printer Configuration
        TimePoint printerCnfgReceivedTime_;
        LpsSaTotalsPrinterCnfg printerCnfg_;
        LpsSaTotalsPrinterCnfgInterfaceInputChannel* printerCnfgInputChannel_;
        LpsSaTotalsPrinterCnfgInterfaceOutputChannel* printerCnfgOutputChannel_;

        // CycleSeg input channel
        CycleSegTxInterfaceInputChannel *cycleSegTxInterfaceInputChannel_;

        // For receiving and writing display state and settings
        LpsSaUIDisplayState displayState_;
        uint32_t displayHeartbeatLastCount_;
        std::chrono::steady_clock::time_point displayHeartbeatLastTime_;
        LpsSaUIDisplayStateInterfaceInputChannel* displayStateInputChannel_;
        LpsSaUIDisplaySettingsInterfaceOutputChannel* displaySettingsOutputChannel_;
        boost::filesystem::path tempRoot_;
        boost::filesystem::path storageRoot_;
        ECMSummaryWriter ecmSummaryAnalysisModule_;
        ECMSummaryWriter ecmSummaryProductLink_;
        ECMSummaryWriter ecmSummaryAnalysisDisplay_;
        ECMSummary ecmSummary;
        DispBestBktWt_t dispBestBktWt;
        bool SEALevel1EssentialsInstalled_ = true;
        bool SEALevel2ProInstalled_ = true;
        bool SEALegalForTradeInstalled_ = false;
        bool LegalForTradeSupported_ = false;
        bool LegalForTradeSealStatus_ = false;
        bool CycleSegStatus_ = false;

        std::chrono::steady_clock::time_point lastClockAdjustCheckTime_;
        std::chrono::steady_clock::time_point previousSteadyClockTime_;
        std::chrono::system_clock::time_point previousSystemClockTime_;

        bool PayloadCalInProgress_;
        std::chrono::steady_clock::time_point imuDiagStartTime_;
};

#endif //AUTONOMYCONDITIONDIAGNOSTICS_H_
