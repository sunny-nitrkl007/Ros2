///////////////////////////////////////////////////////////////////////////////
// @attention COPYRIGHT (C) 2011-2015 CATERPILLAR INC. ALL RIGHTS RESERVED.
//
// @file    AutonomyConditionDiagnostics.cpp
//
// @brief   This is the autonomyCondition diagnostics and events application.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// -- #Include's --
///////////////////////////////////////////////////////////////////////////////

#include "AutonomyConditionDiagnostics.h"
#include <clock_tm_zone_proto.h>
#include <std_types.h>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <sys/types.h>

#include <chrono/tz.hpp>
#include <chrono/print.hpp>

#include <LoaderdiagnosticEventConfig/Loader_autonomy_diagnostics_config.h>
#include <LoaderdiagnosticEventConfig/Loader_autonomy_event_config.h>

#include <ais/task/load_version.h>
#include <version/EcmHwVersion.h>

#include <version/legal_for_trade_identifiers.h>

#include <lps_sea_defs.h>

#include <hal_types.h>
#include <hal_boot_proto.h>
#include "src_scl_info/app_scl_obd_es_config.h"
#include "src_j1939/app_can_init.h"
#include "src_j1939/app_j1939_map.h"
#include "src_j1939/app_health_j39.h"
#include "src_nvm/app_nvm_file_init.h"
#include "src_j1939/app_et_j1939_support.h"
#include "src_catdl/app_catdl_init.h"
#include "src_hour/app_clock_init.h"
#include <ecminfolib_private.h>
#include "src_prmsw/app_prmsw_config.h"
#include <datalink_communication_fault2.h>
#include <cdl2_proto.h>
#include <scl_j1939_proto.h>
#include <clock_tm_zone_proto.h>
#include <clock_struct.h>
#include <taskPlugins/interfaces/PlatformDefinesC.h>
#if CSF_PGT_SERVER_SUPPORTED
#include "src_pgt_server/app_pgt.h"
#endif

#include "src_catdl/app_disp_brightness.h"
#include <sys/stat.h>

#include "DetailedDiagEventsWriter.hpp"

#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp>

#include <interfaces/LpsSaTotals/TotalsInterface.hpp>
#include <interfaces/LpsSaTotals/TotalsInterfaceInputChannel.h>
#include <interfaces/LpsSaTotals/RequestInterface.hpp>
#include <interfaces/LpsSaTotals/RequestInterfaceOutputChannel.h>
#include <boost/filesystem.hpp>

using namespace std;
using namespace task;

///////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --
#define DEFAULT_TEMP_ROOT (R"(/tmp/AutonomyConditionDiagnostics/temp)")
#define DEFAULT_STORAGE_ROOT (R"(/opt/appdata/CPM/ACD/nvm)")

#define THREE_SECONDS_IN_MICROSECONDS    (3*1000*1000)

#define CONVERT_TO_PT5_RES(value) (round((value) * 2.0))
#define CONVERT_TO_PT1_RES(value) (round((value) * 10.0))
#define CONVERT_TO_PT01_RES(value) (round((value) * 100.0))
#define CONVERT_TO_PT001_RES(value) (round((value) * 1000.0))
#define CONVERT_TO_PT0001_RES(value) (round((value) * 10000.0))


/******************************************************************************
 ***
 ***    Data types defined for this file
 ***    -- Struct's, Typedef's, Enum's --
 ***
 ******************************************************************************/
// PIDF9D1 uses 1 for January, etc, while tm struct uses 0 for January
constexpr int_16 PidF9D1MonOffset = 1;

// PIDF9D1 bases year 1985 as 0, while tm struvc bases year 1900 as 0, 2000 as 100
// clock_tm_to_seconds() subtracts 100 from year before calculating
constexpr int_16 PidF9D1BaseYear = 85;

bool_t lft_disable_write_pids = false;
bool_t dl_diag_disabled = true;
bool HydOilTempEnabled = false;
int can_port1; // This is to export port configurations to app_can_init()
unsigned_8 EngineRunning = 1; // This is to export EngineRunning State to clock_nvm
unsigned_8 app_master_mid;
unsigned_8 rtc_master_mid;
unsigned_8 rtc_type;
unsigned_8 rtc_security;

bool_t can_init_completed = FALSE; //Used to make sure can is initialized before initializing EDDT

// Needed for scl_info diag/evnt support
scl_obd_es_t              *app_scl_obd_es = nullptr;
bool_t                    nvm_reset_flag = FALSE;

// Needed for EDDT over J1939 support
scl_diag_pj1939_t         *app_pj1939_diag_obj = nullptr;
scl_obd_handle_gen_t      *app_handle_gen = nullptr;

int prmsw_feat_size;
scl_prmsw_free_use_cfg_t* prmsw_free_use_cfg = nullptr;
scl_prmsw_feat_cfg_t* prmsw_feat_cfg = nullptr;
//scl_prmsw_free_use_cfg_t* prmsw_free_use_cfg{ new scl_prmsw_free_use_cfg_t[prmsw_feat_size]{} };
//scl_prmsw_feat_cfg_t* prmsw_feat_cfg{ new scl_prmsw_feat_cfg_t[prmsw_feat_size]{} };

int nvm_file_block_size = 0;
nvm_file_block_cfg_t* app_nvm_file_cfg = nullptr;
//nvm_file_block_cfg_t* app_nvm_file_cfg{ new nvm_file_block_cfg_t[nvm_file_block_size]{} };

vector<std::string> block_filename;

struct stat nvm ={0};

unsigned_8  mid_D6_display = 0x54;  // MID of the D6 display.. used to send CDL over J1939 message toset brightness to 100%
unsigned_8 cycleRate_hz = 10;        //  frequency of ACD task used to set periodic timer to send D^ brightness write request to 100%
extern int_16 LiftCylinderVelocity_val;

///////////////////////////////////////////////////////////////////////////////
// -- Symbols --
///////////////////////////////////////////////////////////////////////////////
extern "C" boolean get_time(scl_obd_real_time_stamp_t *real_time_stamp);

///////////////////////////////////////////////////////////////////////////////
// -- Start of code for this file --
///////////////////////////////////////////////////////////////////////////////

AbstractTaskCore* task::getTaskImplementation( )
{
   static AutonomyConditionDiagnostics thisTask("AutonomyConditionDiagnostics");
   return dynamic_cast<Task *>(&thisTask);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief AutonomyConditionDiagnostics Constructor
///////////////////////////////////////////////////////////////////////////////
AutonomyConditionDiagnostics::AutonomyConditionDiagnostics( const std::string& taskName ):
   AutonomyTask(taskName),
   TaskPluginMgr(static_cast<task::AutonomyTask&>(*this)),
   m_enableOutfileWriter( false ),
   m_EDDT_init( false ),
   m_partNumbers(),
   m_pAutonomyConditionMessageIn( NULL ),
   m_pDiagStatusOut( NULL ),
   m_pHealthInput(NULL),
   m_pAisJhm2TxChannel(NULL),
   m_dataLinkDataInput(nullptr),
   m_BEMSimStateInput(nullptr),
   m_pSHMOutput(nullptr),
   m_pPartNumbersOutput(nullptr),
   equipmentId_(),
   m_hostnameWhiteList(),
   m_useHostnameWhiteList(false),
   m_diagEventList(),
   m_autoCondList(),
   m_inhibitList(),
   m_outfileWriter(),
   m_initDone(false),
   m_seaStatusTimer(),
   m_seaStatusOutput(nullptr),
   m_shmPublishTimer(),
   isConceptSoftware_(false),
   weighAppInf_(),
   LpsSaJobMgrScsTxIn(nullptr),
   LpsSaJobMgrScsReqstOut(nullptr),
   LpsSaJobMgrScsRespIn(nullptr),
   dispatchSettings_(),
   dispatchSettingsInput_(nullptr),
   dispatchSettingsWrOutput_(nullptr),
   LpsSaSwitchInput(nullptr),
   autonomyConditionDiagnosticsScsReqstIn_(nullptr),
   autonomyConditionDiagnosticsRespChannelOutput_(nullptr),
   totalsInputChannel_(nullptr),
   totalsRequestOutputChannel_(nullptr),
   autonomyConditionDiagnosticsTxOutputChannel_(nullptr),
   autonomyConditionDiagnosticsTx_(),
   printerCnfgReceivedTime_(TimePoint::min()),
   printerCnfg_(),
   printerCnfgInputChannel_(nullptr),
   printerCnfgOutputChannel_(nullptr),
   cycleSegTxInterfaceInputChannel_(nullptr),
   displayState_(),
   displayHeartbeatLastCount_(0),
   displayHeartbeatLastTime_(std::chrono::steady_clock::now()),
   displayStateInputChannel_(nullptr),
   displaySettingsOutputChannel_(nullptr),
   tempRoot_(DEFAULT_TEMP_ROOT),
   storageRoot_(DEFAULT_STORAGE_ROOT),
   ecmSummaryAnalysisModule_(),
   ecmSummaryProductLink_(),
   ecmSummaryAnalysisDisplay_(),
   ecmSummary(),
   dispBestBktWt(),
   SEALevel1EssentialsInstalled_(true),
   SEALevel2ProInstalled_(true),
   SEALegalForTradeInstalled_(false),
   LegalForTradeSupported_(false),
   LegalForTradeSealStatus_(false),
   CycleSegStatus_(false),
   lastClockAdjustCheckTime_(),
   previousSteadyClockTime_(),
   previousSystemClockTime_(),
   PayloadCalInProgress_(false),
   imuDiagStartTime_()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief AutonomyConditionDiagnostics Destructor
///////////////////////////////////////////////////////////////////////////////
AutonomyConditionDiagnostics::~AutonomyConditionDiagnostics( )
{
    free(prmsw_free_use_cfg);
    free(prmsw_feat_cfg);
    free(app_nvm_file_cfg);
}

bool AutonomyConditionDiagnostics::loadECMSummaryconfig()
{
    bool ret = true;
    ConfigSection section;
    std::string filename;
    std::string ipaddress;
    std::string ecmname;

    std::string temp_root;
    if (!getTaskConfig().get("tempRoot", temp_root)) {
        tempRoot_ = DEFAULT_TEMP_ROOT;
    }
    else {
        tempRoot_ = temp_root;
    }

    /* Get machine make model linkage */
    ConfigSection machineData;
    if(getTaskParser().get("MachineType", machineData))
    {
        std::string machineMake;
        if(machineData.get("Make", machineMake))
        {
            ecmSummary.machine_make = machineMake;
        }
        
        std::string machineModel;
        if(machineData.get("Model", machineModel))
        {
            ecmSummary.machine_model = machineModel;
        }

        std::string machineLinkage;
        if(machineData.get("Linkage", machineLinkage))
        {
            ecmSummary.machine_linkage = machineLinkage;
        }

        std::string machineModelDetail;
        if (machineData.get("ModelDetail", machineModelDetail))
        {        
            /* Limit data to 26 alpha numeric characters */ 
            /* NOTE: c_str() is null terminating */
            std::strncpy(pidf9b4_pdata, machineModelDetail.c_str(), PIDF9B4_LENGTH);   
        }

        unsigned_16 machineLinkageNum;
        if (machineData.get("LinkageConfigurationNumber", machineLinkageNum))
        {
            pidd10049_pdata = machineLinkageNum;  
        }
    }

    if( getTaskParser().getSection( "ECM_SUMMARY", section ))
    {
        list<std::string>  ecms = section.getParameterNames();

        for( list<std::string>::const_iterator it = ecms.begin(); it != ecms.end(); ++it )
        {
            vector<std::string> configArray;
            section.getArray(*it, configArray);
            ecmname = it->c_str();
            filename = configArray.at(0).c_str();
            ipaddress = configArray.at(1).c_str();

            if ("Analysis Module" == ecmname) {
                ecmSummary.ecm_name = ecmname;
                /* create ecm summary writer object */
                ecmSummaryAnalysisModule_ = ECMSummaryWriter(tempRoot_, filename);
            }
            else if ("Analysis Display" == ecmname) {
                /* create ecm summary writer object */
                ecmSummaryAnalysisDisplay_ = ECMSummaryWriter(tempRoot_, filename, ipaddress);
            }
            else if ("Product Link" == ecmname) {
                /* create ecm summary writer object */
                ecmSummaryProductLink_ = ECMSummaryWriter(tempRoot_, filename, ipaddress);
            }
            else {
                AIS_LOG_ERROR("Unsupported ECM: %s", ecmname.c_str());
            }
        }
    }
    else
    {
        AIS_LOG_ERROR("Could not find ECM_SUMMARY");
        ret = false;
    }

    return ret;
}

bool AutonomyConditionDiagnostics::loadNVMconfig()
{
    bool ret = true;
    ConfigSection section;

    std::string filepath(getenv("HOME"));
    filepath += "/appdata/CPM/ACD/nvm/";

    //Create location for NVM log, if it doesn't already exist.
    if(stat(filepath.c_str(), &nvm) == -1)
    {
        int r = system(("mkdir --mode=755 -p " + filepath).c_str());
        (void)r; // unused return
    }

    if( getTaskParser().getSection( "NVM", section ))
    {
        list<std::string>  keys = section.getParameterNames();

        nvm_file_block_size = 0;
        for( list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it )
        {
            nvm_file_block_size++;
        }

        if (nvm_file_block_size > APP_NVM_MAX_NUM_BLOCKS) {
            AIS_LOG_FATAL("Cannot configure more NVM file blocks than %d", APP_NVM_MAX_NUM_BLOCKS);
            ret = false;
        }
        else {
            // Make sure the filename vector has an entry for each configured block
            // The file names are stored here just to they continue to have a non-temporary memory location.
            block_filename.resize(nvm_file_block_size);

            app_nvm_file_cfg = static_cast<nvm_file_block_cfg_t*>(malloc(nvm_file_block_size * sizeof(nvm_file_block_cfg_t)));

            for( list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it )
            {
                vector<std::string> configArray;
                section.getArray(*it, configArray);
                int indx = atoi(it->c_str());
                if (indx < nvm_file_block_size) {
                    uint_least32_t  blockid = strtol(configArray.at(0).c_str(), NULL, 16);
                    uint_least32_t  blocksize = atoi( configArray.at(1).c_str() );
                    uint_least8_t  block_int = atoi( configArray.at(2).c_str() );
                    std::string filename = configArray.at(3);
                    app_nvm_file_cfg[indx].data_id = blockid;
                    app_nvm_file_cfg[indx].block_size = blocksize;
                    app_nvm_file_cfg[indx].block_integrity = block_int;
                    if (block_filename[indx].empty()) {
                        block_filename[indx] = filepath + filename;
                    }
                    else {
                        AIS_LOG_FATAL("NVM Key %d filename cannot be set twice", indx);
                        ret = false;
                    }
                    app_nvm_file_cfg[indx].filepathname1 = const_cast<char*>(block_filename[indx].c_str());
                    app_nvm_file_cfg[indx].filepathname2 = NULL;
                    AIS_LOG_INFO("NVM Key %d has filepathname1 = %s", indx, app_nvm_file_cfg[indx].filepathname1);
                }
                else {
                    AIS_LOG_FATAL("Cannot configure an NVM key, %d, >= size %d", indx, nvm_file_block_size);
                    ret = false;
                }
            }
        }
    }

    return ret;
}

bool AutonomyConditionDiagnostics::loadDeviceIdConfig()
{
    bool ret = true;
    ConfigSection section;

    if( getTaskParser().getSection( "DeviceID_J1939", section ))
    {
        if ( !section.get("MID", j1939_mid) )
        {
            AIS_LOG_FATAL("Could not find MID");
            ret = false;
        }
        if ( !section.get("ST_app_num", st_app_num) )
        {
            AIS_LOG_FATAL("Could not find ST_app_num");
            ret = false;
        }
        if ( !section.get("ST_chg_lvl", st_chg_lvl) )
        {
            AIS_LOG_FATAL("Could not find ST_chg_lvl");
            ret = false;
        }
    }
    else
    {
        ret = false;
    }

    return ret;
}

bool AutonomyConditionDiagnostics::loadSEAconfig()
{
    bool ret = true;
    ConfigSection section;

    if( getTaskParser().getSection( "SEA", section ))
    {
        list<std::string>  keys = section.getParameterNames();

        prmsw_feat_size = 0;
        for( list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it )
        {
            prmsw_feat_size++;
        }

        prmsw_free_use_cfg = static_cast<scl_prmsw_free_use_cfg_t*>(malloc(prmsw_feat_size * sizeof(scl_prmsw_free_use_cfg_t)));
        prmsw_feat_cfg = static_cast<scl_prmsw_feat_cfg_t*>(malloc(prmsw_feat_size * sizeof(scl_prmsw_feat_cfg_t)));

        int prmsw_ind = 0;
        for( list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it, prmsw_ind++ )
        {
            vector<std::string> configArray;
            section.getArray(*it, configArray);
            int indx = atoi(it->c_str());
            uint_least32_t nvid = app_nvm_file_cfg[indx].data_id;
            uint_least16_t tempcnt = (uint_least16_t) atoi(configArray.at(6).c_str());
            scl_security_sec_level_t seclvl = (scl_security_sec_level_t) atoi(configArray.at(3).c_str());
            scl_prmsw_free_use_cfg_t *ptr_prmsw_free_use_cfg;

            if(0 == tempcnt)
            {
                ptr_prmsw_free_use_cfg = NULL;
            }
            else
            {
                prmsw_free_use_cfg[prmsw_ind] = SCL_PRMSW_FREE_USE_CFG_I1(
                        (scl_prmsw_data_id_type_t) SCL_PRMSW_DATA_ID_TYPE_CDL,     //FREE_USE_DATA_ID_TYPE
                        (uint_least32_t) strtol(configArray.at(7).c_str(), NULL, 16),                        //FREE_USE_DATA_ID
                        (scl_prmsw_data_id_type_t) SCL_PRMSW_DATA_ID_TYPE_CDL,     //FREE_USE_COUNT_DATA_ID_TYPE
                        (uint_least32_t) strtol(configArray.at(8).c_str(), NULL, 16),                        //FREE_USE_COUNT_DATA_ID
                        tempcnt                                                    //FREE_USE_COUNT_MAX
                );
                ptr_prmsw_free_use_cfg = &prmsw_free_use_cfg[prmsw_ind];
            }

            if(0 == seclvl)
            {
                prmsw_feat_cfg[prmsw_ind] = SCL_PRMSW_FEAT_CFG_I1(
                        (uint_least16_t) atoi(configArray.at(0).c_str()),                        //REASON_CODE
                        (uint_least16_t) atoi(configArray.at(4).c_str()),                        //ENCODING_BYTE_POS
                        (uint_least8_t) atoi(configArray.at(5).c_str()),                         //ENCODING_BIT_INDEX
                        (uint_least32_t) nvid,                            //NVM_BLOCK_ID
                        (scl_prmsw_data_id_type_t) SCL_PRMSW_DATA_ID_TYPE_CDL,     //PERM_FEAT_INSTALL_DATA_ID_TYPE
                        (uint_least32_t) strtol(configArray.at(1).c_str(), NULL, 16),                        //PERM_FEAT_INSTALL_DATA_ID
                        (scl_prmsw_data_id_type_t) SCL_PRMSW_DATA_ID_TYPE_CDL,     //PERM_FEAT_ENABLE_DATA_ID_TYPE
                        (uint_least32_t) strtol(configArray.at(2).c_str(), NULL, 16),                        //PERM_FEAT_ENABLE_DATA_ID
                        ptr_prmsw_free_use_cfg,                                    //FREE_USE_CFG
                        app_prmsw_get_feat_status,                                 //FEAT_STATUS_CHANGE_FN_PTR
                        reinterpret_cast<void*>(atoi(configArray.at(0).c_str()))   //FEAT_STATUS_CHANGE_CONTEXT_PTR
                );
            }
            else if (1 == seclvl)
            {
                prmsw_feat_cfg[prmsw_ind] = SCL_PRMSW_FEAT_CFG_I2(
                        (uint_least16_t) atoi(configArray.at(0).c_str()),                        //REASON_CODE
                        (uint_least16_t) atoi(configArray.at(4).c_str()),                        //ENCODING_BYTE_POS
                        (uint_least8_t) atoi(configArray.at(5).c_str()),                         //ENCODING_BIT_INDEX
                        (uint_least32_t) nvid,                            //NVM_BLOCK_ID
                        (scl_prmsw_data_id_type_t) SCL_PRMSW_DATA_ID_TYPE_CDL,     //PERM_FEAT_INSTALL_DATA_ID_TYPE
                        (uint_least32_t) strtol(configArray.at(1).c_str(), NULL, 16),                        //PERM_FEAT_INSTALL_DATA_ID
                        (scl_prmsw_data_id_type_t) SCL_PRMSW_DATA_ID_TYPE_CDL,     //PERM_FEAT_ENABLE_DATA_ID_TYPE
                        (uint_least32_t) strtol(configArray.at(2).c_str(), NULL, 16),                        //PERM_FEAT_ENABLE_DATA_ID
                        ptr_prmsw_free_use_cfg,                                    //FREE_USE_CFG
                        app_prmsw_get_feat_status,                                 //FEAT_STATUS_CHANGE_FN_PTR
                        reinterpret_cast<void*>(atoi(configArray.at(0).c_str())),  //FEAT_STATUS_CHANGE_CONTEXT_PTR
                        seclvl                                                     //SEC_LEVEL_ENABLE_PID
                );
            }
            else if (2 == seclvl)
            {
                prmsw_feat_cfg[prmsw_ind] = SCL_PRMSW_FEAT_CFG_I3(
                        (uint_least16_t) atoi(configArray.at(0).c_str()),                        //REASON_CODE
                        (uint_least16_t) atoi(configArray.at(4).c_str()),                        //ENCODING_BYTE_POS
                        (uint_least8_t) atoi(configArray.at(5).c_str()),                         //ENCODING_BIT_INDEX
                        (uint_least32_t) nvid,                            //NVM_BLOCK_ID
                        (scl_prmsw_data_id_type_t) SCL_PRMSW_DATA_ID_TYPE_CDL,     //PERM_FEAT_INSTALL_DATA_ID_TYPE
                        (uint_least32_t) strtol(configArray.at(1).c_str(), NULL, 16),                        //PERM_FEAT_INSTALL_DATA_ID
                        (scl_prmsw_data_id_type_t) SCL_PRMSW_DATA_ID_TYPE_CDL,     //PERM_FEAT_ENABLE_DATA_ID_TYPE
                        (uint_least32_t) strtol(configArray.at(2).c_str(), NULL, 16),                        //PERM_FEAT_ENABLE_DATA_ID
                        ptr_prmsw_free_use_cfg,                                    //FREE_USE_CFG
                        app_prmsw_get_feat_status,                                 //FEAT_STATUS_CHANGE_FN_PTR
                        reinterpret_cast<void*>(atoi(configArray.at(0).c_str())),  //FEAT_STATUS_CHANGE_CONTEXT_PTR
                        seclvl,                                                    //SEC_LEVEL_ENABLE_PID
                        1                                                          //is_tracking_enabled
                );
            }

            AIS_LOG_DEBUG("prmsw_feat_cfg[prmsw_ind].perm_install_data_id: %x", prmsw_feat_cfg[prmsw_ind].perm_install_data_id);
            AIS_LOG_DEBUG("prmsw_feat_cfg[prmsw_ind].perm_enable_data_id: %x", prmsw_feat_cfg[prmsw_ind].perm_enable_data_id);
            if(prmsw_feat_cfg[prmsw_ind].free_use_cfg != NULL)
            {
                AIS_LOG_DEBUG("prmsw_feat_cfg[prmsw_ind].free_use_cfg->free_use_data_id: %x", prmsw_feat_cfg[prmsw_ind].free_use_cfg->free_use_data_id);
                AIS_LOG_DEBUG("prmsw_feat_cfg[prmsw_ind].free_use_cfg->free_use_cnt_data_id: %x", prmsw_feat_cfg[prmsw_ind].free_use_cfg->free_use_cnt_data_id);
                AIS_LOG_DEBUG("prmsw_feat_cfg[prmsw_ind].free_use_cfg->free_use_count_max: %d", prmsw_feat_cfg[prmsw_ind].free_use_cfg->free_use_count_max);
            }
        }

        ret = ret & true;
    }
    else
    {
        ret = ret & false;
    }

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief AutonomyConditionDiagnostics Initialize
///
/// @return
///
///////////////////////////////////////////////////////////////////////////////
bool AutonomyConditionDiagnostics::autonomyInitialize( )
{
  if (!getTaskConfig().get("EDDT_CAN_port", can_port1))
  {
    can_port1 = 1; //default
    AIS_LOG_FATAL("EDDT_CAN_port NOT found");
  }

  // Check for invalid port configuration
  if ( can_port1 > 3 || can_port1 < 0 )
  {
    AIS_LOG_FATAL("Invalid port configuration. Configure ports in the range (0-3)");
    return false;
  }

  if (!getTaskConfig().get("MID_D6_Display", mid_D6_display))
  {
      mid_D6_display = 0x54; //default
      AIS_LOG_FATAL("mid_D6_display NOT found, set to default 0x54");
  }

  if (!getTaskConfig().get("cycleRate_hz", cycleRate_hz))
  {
      cycleRate_hz = 10; //default
      AIS_LOG_FATAL("cycleRate_hz NOT found, set to default 10");
  }

  if( !getEcmJ1939Name() )
  {
     AIS_LOG_FATAL("ECM Name not found");
     return false;
  }

  if( !getRTCconfig() )
  {
       AIS_LOG_FATAL("RTCconfig not found");
       return false;
  }

  if( !loadNVMconfig() )
  {
       AIS_LOG_FATAL("NVMconfig not found");
       return false;
  }

  if( !loadSEAconfig() )
  {
       AIS_LOG_FATAL("SEAconfig not found");
       return false;
  }

    { // Determine if legal for trade or cycle seg is supported
        ConfigSection rubyCfg;
        if (!getTaskParser().getSection("MachineSpecificConfig", rubyCfg)) {
            AIS_LOG_ERROR("MachineSpecificConfig section not found");
            LegalForTradeSupported_ = false;
            CycleSegStatus_ = false;
        }

        else {
            // LegalForTradeSupported
            if (rubyCfg.get("LegalForTradeSupported", LegalForTradeSupported_)) {
                AIS_LOG_INFO("LegalForTradeSupported: %d", LegalForTradeSupported_);
            }
            else {
                // Default to not supported.
                LegalForTradeSupported_ = false;
                AIS_LOG_INFO("LegalForTradeSupported missing, default to: %d", LegalForTradeSupported_);
            }

            // CycleSegAppNumber
            uint16_t cycleSegAppNumber = 0;

            if (!rubyCfg.get("CycleSegAppNumber", cycleSegAppNumber)) {
                cycleSegAppNumber = 0;
            }

            if (0 == cycleSegAppNumber) {
                CycleSegStatus_ = false;
            }
            else {
                CycleSegStatus_ = true;
            }

            AIS_LOG_INFO("CycleSegStatus: %d", CycleSegStatus_);
        }
    }

  if ( !initializeScsChannels() )
  {
    AIS_LOG_FATAL("Could not initialize SCS channels");
    return false;
  }

  m_useHostnameWhiteList = true;
  if (!getTaskConfig().getArray("hostnameWhiteList", m_hostnameWhiteList))
  {
    AIS_LOG_WARN("Could not initialize hostname whitelist - accepting conditions from all hosts");
    m_useHostnameWhiteList = false;
  }

  if ( !m_inhibitList.configure( getTaskParser(), m_autoCondList ) )
  {
    AIS_LOG_FATAL("Cound not initilize inhibits");
    return false;
  }

  if( !getTaskConfig().get( "EnableDiagnosticOutputFileWriter", m_enableOutfileWriter ))
  {
    AIS_LOG_FATAL("Could not get EnableDiagnosticOutputFileWriter from task config.");
    return false;
  }
  else if( m_enableOutfileWriter )
  {
    if ( !m_outfileWriter.configure( getTaskConfig() ) )
    {
      AIS_LOG_FATAL("Could not initialize output file writer");
      return false;
    }
  }

  std::string tempRoot;
  if (getTaskConfig().get("tempRoot", tempRoot)) {
      tempRoot_ = tempRoot;
  }
  else {
      tempRoot_ = DEFAULT_TEMP_ROOT;
      AIS_LOG_ERROR("No tempRoot defined in rb, initializing tempRoot directory to default directory, some functionality may not be available");
  }

  // Read the software version library info
  appVersionInit();

  if( !loadECMSummaryconfig() )
  {
      AIS_LOG_ERROR("ECMSummary Config not found");
      return false;
  }

  // Initialize plugins after J1939 communication stack initialization is complete
  if (!initializePlugins())
  {
    AIS_LOG_DEBUG("initializePlugins FAILED");
    return false;
  }

  AIS_LOG_DEBUG("initializePlugins PASSED");

  // Initialize SHM publish Timer
  m_shmPublishTimer.setInterval(1000);
  m_shmPublishTimer.connect(boost::bind(&AutonomyConditionDiagnostics::publishSHM, this));
  m_shmPublishTimer.start();

  if (!getTaskConfig().get("ID_Master", app_master_mid))
  {
    app_master_mid = 0x1C; //default MID
    AIS_LOG_FATAL("ID_Master NOT found");
  }
  

  { // Read the Machine Type - Machine Model Linkage
        ConfigSection machineType;
        std::string machineMSN;

        if (!getTaskParser().getSection("MachineType", machineType)) {
            AIS_LOG_ERROR("MachineType section not found");
            return false;
        }
        else {
            machineType.get("InternalMsn", machineMSN);

            // disable datalink CID 248 if machine model is NOT_SET (default)
            if ("NOT00000" == machineMSN) {
                dl_diag_disabled = TRUE;
            }
            else {
                // Machine model has been selected
                dl_diag_disabled = FALSE;
            }
        }
  }
  
  // These must be initialized before oel and common services are initialized
  // so that we can make sure we capture the time points before any clock
  // adjustments are possible.  These variables are used to detect a clock
  // adjustment so that we can update the HW RTC.
  {
      auto steadyNow = std::chrono::steady_clock::now();
      lastClockAdjustCheckTime_ = steadyNow;
      previousSteadyClockTime_ = steadyNow;
      previousSystemClockTime_ = std::chrono::system_clock::now();
  }

  // Initialize j1939 communication stack. Call this at the end of autonomyInitialize()
  // so that j1939 stack can accept config parameteres from ruby file
  commInitialize();

  /* wait for initialization to complete */
  {
      auto timeStart = std::chrono::steady_clock::now();
      while ((FALSE == can_init_completed) || (FALSE == AppStartupInitFlag)) {
          static constexpr auto sleepTime = std::chrono::seconds(1);
          std::this_thread::sleep_for(sleepTime);

          /* if we are here for more than 10secs, return false to exit process */
          auto timeNow = std::chrono::steady_clock::now();
          auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(timeNow - timeStart);
          if (timeDiff.count() > 10) {
              AIS_LOG_FATAL("commInitialize taking too long?");
          }
      }
  }

  // Device ID config
  if( !(loadDeviceIdConfig()) )
  {
      AIS_LOG_FATAL("DeviceID_J1939 not found");
      return false;
  }

  {   // Set up storage root for Flash Info Log File
      ConfigSection& configs = getTaskConfig();

      std::string temp;
      if (configs.get("storageRoot", temp)) {
          storageRoot_= temp;
      }
      else {
          storageRoot_= DEFAULT_STORAGE_ROOT;
      }

      if (!tes_common_ais::directory::create(storageRoot_)) {
          AIS_LOG_ERROR("Directory Created %s NOT created!", storageRoot_.c_str());
          return false;
      }
  }

  postInitialize();

  // Initialize any plugins that require J1939 communication stack initialization
  if (!postInitializePlugins())
  {
    AIS_LOG_DEBUG("postInitializePlugins FAILED");
    return false;
  }
  
  AIS_LOG_DEBUG("postInitializePlugins PASSED");

  // Print the Ethernet PGT initialization error code if there is one.
#if CSF_PGT_SERVER_SUPPORTED
  if (APP_PGT_INIT_ERROR_CODE_NONE != app_pgt_init_error_code) {
      AIS_LOG_ERROR("app_pgt_init_error_code = %d", app_pgt_init_error_code);
  }
#endif

  return true;
}

void AutonomyConditionDiagnostics::updateSEAStatus( )
{
    if (!m_initDone) {
        return;
    }

    autonomyConditionDiagnosticsTx_.seaList.clear();

    for (int indx = 0; indx < prmsw_feat_size; ++indx) {
        SEA status;

        status.reason_code = prmsw_feat_cfg[indx].reason_code;

        if (!scl_prmsw_get_free_use_cnt( prmsw_feat_cfg[indx].reason_code, &status.free_count)) {
            status.free_count = 0;
        }

        status.status = scl_prmsw_get_feat_status(prmsw_feat_cfg[indx].reason_code);

        switch (status.reason_code) {

        case (LPS_SEA_REASON_CODE_149): {
            // inhibit diagnostics if SEA Level1 is not installed.
            // For AU2020, we uninstall Level 1 when Level2 is not installed
            if (autonomyConditionDiagnosticsTx_.checkSEAEnableStatus(status.status)) {
                SEALevel1EssentialsInstalled_ = true;
            }
            else {
                SEALevel1EssentialsInstalled_ = false;
            }
            break;
        }

        case (LPS_SEA_REASON_CODE_245): {
            // Save off the SEA Level2 status
            if (autonomyConditionDiagnosticsTx_.checkSEAEnableStatus(status.status)) {
                SEALevel2ProInstalled_ = true;
            }
            else {
                SEALevel2ProInstalled_ = false;
            }
            break;
        }

        case (LPS_SEA_LFT_REASON_CODE_312): {
            // Payload Legal For Trade Measurement Install SEA
            if (LegalForTradeSupported_) {
                if (autonomyConditionDiagnosticsTx_.checkSEAEnableStatus(status.status)) {
                    SEALegalForTradeInstalled_ = true;
                }
                else if (autonomyConditionDiagnosticsTx_.checkSEAInstallStatus(status.status)) {
                    // Enable LFT if it is supported and installed
                    scl_prmsw_set_feat_cmd(LPS_SEA_LFT_REASON_CODE_312, SCL_PRMSW_FEAT_PERM_ENABLE);
                    status.status = scl_prmsw_get_feat_status(LPS_SEA_LFT_REASON_CODE_312);
                    SEALegalForTradeInstalled_ = true;
                }
                else {
                    SEALegalForTradeInstalled_ = false;
                }
            }
            else {
                // Legal for trade is not supported
                if (autonomyConditionDiagnosticsTx_.checkSEAEnableStatus(status.status)) {
                    // Disable LFT if it is not supported but enabled.
                    scl_prmsw_set_feat_cmd(LPS_SEA_LFT_REASON_CODE_312, SCL_PRMSW_FEAT_PERM_DISABLE);
                    status.status = scl_prmsw_get_feat_status(LPS_SEA_LFT_REASON_CODE_312);
                }
                SEALegalForTradeInstalled_ = false;
            }

            if (autonomyConditionDiagnosticsTx_.checkSEAInstallStatus(status.status)) {
                PidD11E8D = 0x0010; // Installed
            }
            else {
                PidD11E8D = 0x0011; //Uninstalled
            }

            break;
        }

        default: {
            // This is not an SEA we are looking for.
            break;
        }

        }

        autonomyConditionDiagnosticsTx_.seaList.push_back(status);

        AIS_LOG_DEBUG("Reason code:%d, Status:=%d, Free Use:%d", status.reason_code, status.status, status.free_count);
    }
}

void AutonomyConditionDiagnostics::publishSHM()
{
    //SHM and Local time offset publish
    {
        ShmClock shmStatus;
        shmStatus.attributes.originationTime = commonNow();
        shmStatus.set_SHM(currentHour_Sec);
        shmStatus.set_UTC(shmStatus.attributes.originationTime);
        shmStatus.set_UTC_offset(clock_tzone_get_offset());

        { // Set tzone info
            tzone_tx_comm_struct tx_comm_struct;
            bool_least_t sec = FALSE;
            if (clock_tzone_get(&tx_comm_struct, &sec) == clock_RESULT_SUCCESS) {
                tes_common_ais::get_tz_data(shmStatus, tx_comm_struct);
            }
        }

        if (m_pSHMOutput) {
            m_pSHMOutput->publish(shmStatus);
        }

        AIS_LOG_INFO("LocalTime : %s Delta (minutes):%d", shmStatus.get_Local_time_str().c_str(), shmStatus.get_UTC_offset());
    }
}

void AutonomyConditionDiagnostics::publishPartNumbers() {
    // Set Product ID
    m_partNumbers.SetProductIdNum(std::string(pidF82D_pdata, PIDF82D_LENGTH));

    //Trim spaces (if it has changed) and set Equipment ID
    if (PidWrF81AFlag) {
        std::string str(std::string(pidF81A_pdata, PIDF81A_LENGTH));
        str.erase(std::find_if(str.rbegin(), str.rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), str.end());
        equipmentId_ = str;
        PidWrF81AFlag = false;
    }

    m_partNumbers.SetEquipmentId(equipmentId_);

    // Publish the data
    if (nullptr != m_pPartNumbersOutput) {
        PartNumbers data(m_partNumbers);
        m_pPartNumbersOutput->publish(data);
    }
}

void AutonomyConditionDiagnostics::postInitialize( )
{
    if (TRUE == can_init_completed && FALSE == m_EDDT_init)
    {
        m_EDDT_init = initializeEDDT();
        m_initDone = m_EDDT_init;
        AIS_LOG_DEBUG("Post Init: %d",m_initDone);
    }
}

bool AutonomyConditionDiagnostics::initializeScsChannels( )
{
    bool everythingOk = true;

  //Initialize the Calibration channels
  AcdCalScsInf::AcdCalGetInstance()->AcdCalScsInitInterface();

  // Initialize the AutonomyConditions channel
  m_pAutonomyConditionMessageIn = fetch<AutonomyConditionMessageInput>("AutonomyConditionMessageInput");


  // Initialize the SCS output channel for DiagnosticStatus
  m_pDiagStatusOut  = fetch<DiagnosticStatusOutput>("DiagnosticStatusOutput");

  // Initialize the SCS input channel for SystemHardwareHealth
  m_pHealthInput  = fetch<SystemHardwareHealthInput>("SystemHardwareHealthInput");
  if (!m_pHealthInput )
  {
      AIS_LOG_FATAL("Failed to create SystemHardwareHealthInput");
      everythingOk = false;
  }

  m_seaStatusOutput  = fetch<SEAStatusOutput>("SEAStatusOutput");
  
  // Initialize the SCS input channel for AisJhm2TxChannelInput 
  m_pAisJhm2TxChannel  = fetch<AisJhm2TxChannelInput>("AisJhm2TxChannelInput");
  if (!m_pAisJhm2TxChannel)
  {
      AIS_LOG_FATAL("Failed to create AisJhm2TxChannelInput");
      everythingOk = false;
  }

  // Initialize the SCS input channel for DatalinkData object
  m_dataLinkDataInput  = fetch<DataLinkDataInput>("DataLinkDataInput");
  if (!m_dataLinkDataInput)
  {
      AIS_LOG_FATAL("Failed to create DataLinkDataInput");
      everythingOk = false;
  }

  // Initialize the SCS input channel for BEMSimStateInput object
    m_BEMSimStateInput = fetch<LpsSaUIBEMSimStateInterfaceInputChannel>("BEMSimStateInput");
    if (!m_BEMSimStateInput)
    {
        AIS_LOG_FATAL("Failed to create BEMSimStateInput");
        everythingOk = false;
    }

   // request channel
   autonomyConditionDiagnosticsScsReqstIn_  = dynamic_cast<AutonomyConditionDiagnosticsRequestInterfaceInputChannel*>( InterfaceDb::fetch("AutonomyConditionDiagnosticsRequestChannelInput") );
   if (nullptr == autonomyConditionDiagnosticsScsReqstIn_) {
       AIS_LOG_FATAL("No ACD request input channel defined.");
       everythingOk = false;
   }

   // response channel
   autonomyConditionDiagnosticsRespChannelOutput_ = dynamic_cast<AutonomyConditionDiagnosticsResponseInterfaceOutputChannel*>(InterfaceDb::fetch("AutonomyConditionDiagnosticsResponseChannelOutput"));
   if (nullptr == autonomyConditionDiagnosticsRespChannelOutput_) {
       AIS_LOG_FATAL("No ACD response output channel defined.");
       everythingOk = false;
   }

   LpsSaSwitchInput = dynamic_cast<SwitchInputScsInput*>( InterfaceDb::fetch("SwitchInputScsInput") );
   LpsSaJobMgrScsTxIn  = dynamic_cast<LpsSaJobMgrTxChannelInput*>( InterfaceDb::fetch("LpsSaJobMgrTxChannelInput") );
   LpsSaJobMgrScsReqstOut  = dynamic_cast<LpsSaJobMgrReqstChannelOutput*>( InterfaceDb::fetch("LpsSaJobMgrReqstChannelOutput"));
   LpsSaJobMgrScsRespIn  = dynamic_cast<LpsSaJobMgrRespChannelInput*>( InterfaceDb::fetch("LpsSaJobMgrRespChannelInput"));

    // Initialize Dispatch (Work Order Assist) Settings Input
    if (!task::InterfaceDb::bind("DispatchSettingsInput", dispatchSettingsInput_)) {
        everythingOk = false;
    }

    // Initialize Dispatch (Work Order Assist) Settings Output
    if (!task::InterfaceDb::bind("DispatchSettingsWrOutput", dispatchSettingsWrOutput_)) {
        everythingOk = false;
    }

    totalsInputChannel_ = dynamic_cast<LpsSaTotalsInterfaceInputChannel*>(task::InterfaceDb::fetch("TotalsInput"));
    if (nullptr == totalsInputChannel_) {
        AIS_LOG_FATAL("No totals input channel defined.");
        everythingOk = false;
    }

    autonomyConditionDiagnosticsTxOutputChannel_ = dynamic_cast<AutonomyConditionDiagnosticsTxInterfaceOutputChannel*>
                                                        (task::InterfaceDb::fetch("AutonomyConditionDiagnosticsTxOutput"));
    if (nullptr == autonomyConditionDiagnosticsTxOutputChannel_) {
        AIS_LOG_FATAL("No autonomy condition diagnostics tx output channel defined.");
        everythingOk = false;
    }

    totalsRequestOutputChannel_ = dynamic_cast<LpsSaTotalsRequestInterfaceOutputChannel*>(task::InterfaceDb::fetch("TotalsRequestOutput"));
    if (nullptr == totalsRequestOutputChannel_) {
        AIS_LOG_FATAL("No totals request output channel defined.");
        everythingOk = false;
    }

    // Initialize the printer cnfg stuff.
    printerCnfgReceivedTime_ = TimePoint::min();

    printerCnfgInputChannel_ = dynamic_cast<LpsSaTotalsPrinterCnfgInterfaceInputChannel*>(task::InterfaceDb::fetch("PrinterCnfgInput"));
    if (nullptr == printerCnfgInputChannel_) {
        AIS_LOG_FATAL("No printer config input channel defined.");
        everythingOk = false;
    }

    printerCnfgOutputChannel_ = dynamic_cast<LpsSaTotalsPrinterCnfgInterfaceOutputChannel*>(task::InterfaceDb::fetch("PrinterCnfgOutput"));
    if (nullptr == printerCnfgOutputChannel_) {
        AIS_LOG_FATAL("No printer config output channel defined.");
        everythingOk = false;
    }

    cycleSegTxInterfaceInputChannel_ = dynamic_cast<CycleSegTxInterfaceInputChannel*>(task::InterfaceDb::fetch("CycleSegTxInterfaceInputChannel"));
    if (nullptr == cycleSegTxInterfaceInputChannel_) {
        AIS_LOG_FATAL("No cycleSegTxInterfaceInputChannel_ defined.");
        everythingOk = false;
    }

    displayStateInputChannel_ = dynamic_cast<LpsSaUIDisplayStateInterfaceInputChannel*>(task::InterfaceDb::fetch("DisplayStateInput"));
    if (nullptr == displayStateInputChannel_) {
        AIS_LOG_FATAL("No display state input channel defined.");
        everythingOk = false;
    }

    displaySettingsOutputChannel_ = dynamic_cast<LpsSaUIDisplaySettingsInterfaceOutputChannel*>(task::InterfaceDb::fetch("DisplaySettingsOutput"));
    if (nullptr == displaySettingsOutputChannel_) {
        AIS_LOG_FATAL("No display settings output channel defined.");
        everythingOk = false;
    }

    // Initialize the SCS output channel for ShmClockOutput
    m_pSHMOutput  = fetch<ShmClockOutput>("ShmClockOutput");
    if (!m_pSHMOutput)
    {
        AIS_LOG_FATAL("Failed to create ShmClockOutput");
        everythingOk = false;
    }

    // Initialize the SCS output channel for PartNumbersOutput
    if (!task::InterfaceDb::bind("PartNumbersOutput", m_pPartNumbersOutput)) {
        everythingOk = false;
    }

    { // Initialize the Weigh App Interface
        LpsSaWeighReqstChannelOutput* requestOutput = dynamic_cast<LpsSaWeighReqstChannelOutput*>(InterfaceDb::fetch("LpsSaWeighReqstChannelOutput"));
        if (nullptr == requestOutput) {
            everythingOk = false;
            AIS_LOG_ERROR("LpsSaWeighReqstChannelOutput Interface not configured.");
        }

        LpsSaWeighRespChannelInput* responseInput = dynamic_cast<LpsSaWeighRespChannelInput*>(InterfaceDb::fetch("LpsSaWeighRespChannelInput"));
        if (nullptr == responseInput) {
            AIS_LOG_ERROR("LpsSaWeighRespChannelInput Interface not configured.");
            everythingOk = false;
        }

        LpsSaWeighTxChannelInput* txInput = dynamic_cast<LpsSaWeighTxChannelInput*>(InterfaceDb::fetch("LpsSaWeighTxChannelInput"));
        if (nullptr == txInput) {
            AIS_LOG_ERROR("LpsSaWeighTxChannelInput Interface not configured.");
            everythingOk = false;
        }

        if (!weighAppInf_.start(getTaskName(), requestOutput, responseInput, txInput)) {
            AIS_LOG_ERROR("Failed to start weigh app interface.");
            everythingOk = false;
        }
    }

    if ( LpsSaSwitchInput )
    {
       AIS_LOG_INFO( "LpsSaSwitchInput  configured as a producer/sender." );
    }
    else
    {
       AIS_LOG_ERROR( "\n LpsSaSwitchInput Interface  not configured." );
       everythingOk = false;
    }

    return everythingOk;
}




bool AutonomyConditionDiagnostics::initDiagEvent(scl_ci_es_fault_cfg_eddt_t* logConfig, AutoDiagEvent::DiagEventType type, const std::string& name)
{
    scl_obd_test_handle_t app_test_handle = scl_obd_handle_gen_get(app_handle_gen);
    return initDiagEvent(logConfig, type, name, app_test_handle, false);
}

bool AutonomyConditionDiagnostics::initDiagEvent(scl_ci_es_fault_cfg_eddt_t* logConfig, AutoDiagEvent::DiagEventType type, const std::string& name, scl_obd_test_handle_t handle, bool externallyReported)
{
    AutoDiagEvent diagEvent(name, logConfig, type, handle, externallyReported);
    ConfigSection cs;

    if (!getTaskParser().getSection(name, cs)) {
        AIS_LOG_FATAL("Could not find %s ConfigSection", name.c_str());
        return false;
    }

    if (!diagEvent.configure(cs)) {
        AIS_LOG_FATAL("Could not configure %s", name.c_str());
        return false;
    }

    m_diagEventList.push_back(diagEvent);
    return true;
}

bool AutonomyConditionDiagnostics::initializeDiagEvents( )
{
  // AIS diags/events
  if ( !initDiagEvent( &app_evntcfg_taskS_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "TaskStalledEvent" ) ) return false;
  if ( !initDiagEvent( &app_evntcfg_taskC_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "TaskCrashedEvent" ) ) return false;
  if ( !initDiagEvent( &app_evntcfg_low_ecu_power_input_voltage_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "Battery_Low_Event" ) ) return false;
  if ( !initDiagEvent( &app_evntcfg_high_ecu_power_input_voltage_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "Battery_High_Event" ) ) return false;

  //CPM diags/events
  if ( !initDiagEvent( &app_diagcfg_tilt_link_pos_vlt_abv_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltLinkVoltageAboveDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_tilt_link_pos_vlt_blw_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltLinkVoltageBelowDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_tilt_link_pos_abnorm_pwm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltLinkFreqAbnormalDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_tilt_link_pos_out_calib_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltLinkCalibrationOutDiagnostic" ) ) return false;

  if ( !initDiagEvent( &app_diagcfg_lift_link_pos_vlt_abv_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftLinkVoltageAboveDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_lift_link_pos_vlt_blw_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftLinkVoltageBelowDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_lift_link_pos_abnorm_pwm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftLinkFreqAbnormalDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_lift_link_pos_out_calib_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftLinkCalibrationOutDiagnostic" ) ) return false;

  if ( !initDiagEvent( &app_diagcfg_lift_re_pos_vlt_abv_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftREVoltageAboveDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_lift_re_pos_vlt_blw_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftREVoltageBelowDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_lift_re_pos_abnorm_pwm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftREFreqAbnormalDiagnostic" ) ) return false;

  if ( !initDiagEvent( &app_diagcfg_lift_he_pos_vlt_abv_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftHEVoltageAboveDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_lift_he_pos_vlt_blw_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftHEVoltageBelowDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_lift_he_pos_abnorm_pwm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "LiftHEFreqAbnormalDiagnostic" ) ) return false;

  if ( !initDiagEvent( &app_diagcfg_tilt_re_pos_vlt_abv_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltREVoltageAboveDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_tilt_re_pos_vlt_blw_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltREVoltageBelowDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_tilt_re_pos_abnorm_pwm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltREFreqAbnormalDiagnostic" ) ) return false;

  if ( !initDiagEvent( &app_diagcfg_tilt_he_pos_vlt_abv_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltHEVoltageAboveDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_tilt_he_pos_vlt_blw_norm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltHEVoltageBelowDiagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_tilt_he_pos_abnorm_pwm_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "TiltHEFreqAbnormalDiagnostic" ) ) return false;

  if ( !initDiagEvent( &app_diagcfg_pay_load_mon_sys_out_calib_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "PayloadMonCalibrationOutDiagnostic" ) ) return false;

  if ( !initDiagEvent( &app_diagcfg_mach_not_set_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "MachineModelNotSetOutDiagnostic" ) ) return false;

  if ( !initDiagEvent( &app_evntcfg_PayLd_OverLd_LimitExceeded_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "PayLdOverLdLimitExceededEvent" ) ) return false;

  if ( !initDiagEvent( &app_evntcfg_BEMSim_derate_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "BEMSimDerateEvent" ) ) return false;

  if ( !initDiagEvent( &app_evntcfg_BEMSim_chargeLevel_Warning_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "BEMSimLevelWarningEvent" ) ) return false;

  if ( !initDiagEvent( &app_evntcfg_payload_memory_low_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "PayloadMemoryLowEvent" ) ) return false;
  if ( !initDiagEvent( &app_evntcfg_payload_memory_full_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "PayloadMemoryFullEvent" ) ) return false;

  if ( !initDiagEvent( &app_evntcfg_payload_lft_not_sealed_evnt_lv1, AutoDiagEvent::AUTO_EVENT, "PayloadLegalForTradeNotSealedEvent" ) ) return false;

  //APP_AUTONOMY_DIAG_SCL_OBD_MAX_FAULTS must be updated when any diagnostics are added/removed,
  //And APP_AUTONOMY_EVENT_SCL_OBD_MAX_FAULTS must be updated for events

  // TODO We are currently not utilizing the autogen diags/events support in AIS. Also current autogen events are example events that we do not want
  //if ( !initializeAutoGenDiagEvents()) return false;

  return true;
}

bool AutonomyConditionDiagnostics::initializeDLDiagEvents( )
{
  if ( !initDiagEvent( &app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_9_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_ETHERNET_1_CID_3900_FMI_9_Diagnostic" ) ) return false;
  if ( !initDiagEvent( &app_diagcfg_DL_ETHERNET_1_CID_3900_FMI_14_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_ETHERNET_1_CID_3900_FMI_14_Diagnostic" ) ) return false;

  AIS_LOG_DEBUG("DL_J1939_1_CID_247_FMI_9_Diagnostic, %d", app_test_handle_CID247_FMI9);
  if ( !initDiagEvent( &app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_J1939_1_CID_247_FMI_9_Diagnostic", app_test_handle_CID247_FMI9, true ) ) return false;

  AIS_LOG_DEBUG("DL_J1939_3_CID_5856_FMI_9_Diagnostic, %d", app_test_handle_CID5856_FMI9);
  if ( !initDiagEvent( &app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_J1939_3_CID_5856_FMI_9_Diagnostic", app_test_handle_CID5856_FMI9, true ) ) return false;

  AIS_LOG_DEBUG("DL_J1939_2_CID_2348_FMI_9_Diagnostic, %d", app_test_handle_CID2348_FMI9);
  if ( !initDiagEvent( &app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_J1939_2_CID_2348_FMI_9_Diagnostic", app_test_handle_CID2348_FMI9, true ) ) return false;

  AIS_LOG_DEBUG("DL_CDL_CID_248_FMI_9_Diagnostic, %d", app_test_handle_CID248_FMI9);
  if ( !initDiagEvent( &app_diagcfg_DL_CDL_CID_248_FMI_9_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_CDL_CID_248_FMI_9_Diagnostic", app_test_handle_CID248_FMI9, true ) ) return false;

  AIS_LOG_DEBUG("DL_J1939_1_CID_247_FMI_14_Diagnostic, %d", app_test_handle_CID247_FMI14);
  if ( !initDiagEvent( &app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_J1939_1_CID_247_FMI_14_Diagnostic", app_test_handle_CID247_FMI14, true ) ) return false;

  AIS_LOG_DEBUG("DL_J1939_3_CID_5856_FMI_14_Diagnostic, %d", app_test_handle_CID5856_FMI14);
  if ( !initDiagEvent( &app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_J1939_3_CID_5856_FMI_14_Diagnostic", app_test_handle_CID5856_FMI14, true ) ) return false;

  AIS_LOG_DEBUG("DL_J1939_2_CID_2348_FMI_14_Diagnostic, %d", app_test_handle_CID2348_FMI14);
  if ( !initDiagEvent( &app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_J1939_2_CID_2348_FMI_14_Diagnostic", app_test_handle_CID2348_FMI14, true ) ) return false;

  AIS_LOG_DEBUG("DL_CDL_CID_248_FMI_14_Diagnostic, %d", app_test_handle_CID248_FMI14);
  if ( !initDiagEvent( &app_diagcfg_DL_CDL_CID_248_FMI_14_lv1, AutoDiagEvent::AUTO_DIAGNOSTIC, "DL_CDL_CID_248_FMI_14_Diagnostic", app_test_handle_CID248_FMI14, true ) ) return false;

  //APP_AUTONOMY_DIAG_SCL_OBD_MAX_FAULTS must be updated when any diagnostics are added/removed,
  //And APP_AUTONOMY_EVENT_SCL_OBD_MAX_FAULTS must be updated for events

  // TODO We are currently not utilizing the autogen diags/events support in AIS. Also current autogen events are example events that we do not want
  //if ( !initializeAutoGenDiagEvents()) return false;

  return true;
}

int AutonomyConditionDiagnostics::getDiagEvent( const std::string& name )
{
  for ( unsigned int i = 0; i < m_diagEventList.size(); ++i )
  {
    if ( m_diagEventList.at( i ).name() == name )
      return i;
  }

  // This diag/event was not found
  return -1;
}

bool AutonomyConditionDiagnostics::getRTCconfig()
{
    ConfigSection RTC_TZName;
    bool ret = true;
    if (!getTaskParser().getSection("RTC_TZ",RTC_TZName))
    {
        AIS_LOG_ERROR("RTC_TZ config section not found");
        return false;
    }

    if (!getTaskConfig().get("EDDT_CAN_port", can_port1))
    {
      can_port1 = 1; //default
      AIS_LOG_FATAL("EDDT_CAN_port NOT found");
    }

    if ( !RTC_TZName.get("RTC_Security", rtc_security) )
    {
        rtc_security = 0; //security disabled
        AIS_LOG_FATAL("Could not find RTC_TZ::RTC_Security");
        ret = false;
    }

    if ( !RTC_TZName.get("RTC_Type", rtc_type) )
    {
        rtc_type = 0; //Standalone
        AIS_LOG_FATAL("Could not find RTC_TZ::RTC_Type");
        ret = false;
    }

    if(RTC_TYPE_DISTRIBUTED_CLIENT == rtc_type || RTC_TYPE_PID_CLIENT == rtc_type) //for client/pid client, server address is needed
    {
        if ( !RTC_TZName.get("RTC_Master", rtc_master_mid) )
        {
            AIS_LOG_FATAL("Could not find RTC_TZ::RTC_Master");
            ret = false;
        }
    }

    return ret;
}

bool AutonomyConditionDiagnostics::getEcmJ1939Name()
{
    ConfigSection EcmJ1939Name;
    bool ret = true;
    if (!getTaskParser().getSection("Ecm_J1939_Name",EcmJ1939Name))
    {
        AIS_LOG_ERROR("Ecm_J1939_Name config section not found");
        return false;
    }

    if ( !EcmJ1939Name.get("Industry_Group", j1939_name_address.Mu8_industry_group) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::Industry_Group info");
        ret = false;
    }

    if ( !EcmJ1939Name.get("Vehicle_System", j1939_name_address.Mu8_vehicle_system) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::Vehicle_System info");
        ret = false;
    }

    if ( !EcmJ1939Name.get("Vehicle_System_Instance",j1939_name_address.Mu8_vehicle_system_instance) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::Vehicle_System_Instance info");
        ret = false;
    }

    if ( !EcmJ1939Name.get("Function", j1939_name_address.Mu8_function) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::Function info");
        ret = false;
    }

    if ( !EcmJ1939Name.get("Function_Instance", j1939_name_address.Mu8_function_instance) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::Function_Instance info");
        ret = false;
    }

    if ( !EcmJ1939Name.get("ECU_Instance", j1939_name_address.Mu8_ecu_instance) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::ECU_Instance info");
        ret = false;
    }

    if ( !EcmJ1939Name.get("Manufacturer_Code", j1939_name_address.Mu16_manufacturer_code) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::Manufacturer_Code info");
        ret = false;
    }

    if ( !EcmJ1939Name.get("Identity_Number", j1939_name_address.Mu32_identity_number) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::Identity_Number info");
        ret = false;
    }

    if ( !EcmJ1939Name.get("Preferred_Address", j1939_name_address.Mu8_preferred_address) )
    {
        AIS_LOG_FATAL("Could not find Ecm_J1939_Name::Preferred_Address info");
        ret = false;
    }

    return ret;
}

bool AutonomyConditionDiagnostics::initializeEDDT( )
{
    bool ret_val = true;

    // This needs to be added after commInitialize because it requires j1939 to be initialized
    if ( !initializeDiagEvents() )
    {
        AIS_LOG_FATAL("Could not initialize diagnostics/events");
        ret_val = false;
    }

    // This needs to be added after commInitialize because it requires j1939 to be initialized
    if ( !initializeDLDiagEvents() )
    {
        AIS_LOG_FATAL("Could not initialize diagnostics/events");
        ret_val = false;
    }

    //Add of the diagnostics and events to the inhibit list as well
    for ( unsigned int i = 0; i < m_diagEventList.size(); ++i )
    {
        if ( !m_inhibitList.addCondition( &(m_diagEventList.at(i)) ) )
        {
            AIS_LOG_FATAL( "Error adding %s as an inhibit condition", m_diagEventList.at(i).name().c_str() );
            ret_val = false;
        }
    }

    return ret_val;
}

////////////////////////////////////////////////////////////////////////////////////
/// @brief Returns true if the passed hostname is in the whitelist or if the whitelist is not being used
/// @return true if the passed hostname is in the whitelist or if the whitelist is not being used, false otherwise
/// @param thisHostName the hostname to check
////////////////////////////////////////////////////////////////////////////////////
bool AutonomyConditionDiagnostics::acceptedHost(const std::string& thisHostName) const
{
  if(m_useHostnameWhiteList)
  {
    for(unsigned int hni = 0; hni < m_hostnameWhiteList.size(); ++hni)
    {
      if(m_hostnameWhiteList[hni] == thisHostName)
      {
        return true;  // a match in the whitelist was found
      }
    }
    return false;  // no match in the whitelist was found
  }
  else
  {
    return true;  // all hosts are accepted if not using whitelist
  }
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Update the gui based in the current list of messages to process
///////////////////////////////////////////////////////////////////////////////
void AutonomyConditionDiagnostics::processHardwareHealthStatus()
{
    SystemHardwareHealth data;

    if (m_pHealthInput->get(data))
    {
        PidF42A = CONVERT_TO_PT001_RES(data.battery_voltage); /* System Voltage */
    }
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Update the gui based in the current list of messages to process
///////////////////////////////////////////////////////////////////////////////
void AutonomyConditionDiagnostics::processDispBrightness()
{
    static int  dispbrightness_write_request_timer = 0;

    /* Send CDL Parameter Write Request with Param Data Resp(0x9E) for Disp Brightness PID D0022B */
    unsigned_8 tx_message[6] = {0x9E, 0xD0, 0x02, 0x2B, 0xFF, 0xFF};  // D0022B

    /* DispBrightness code always set to 100%  */
    dispbrightness_write_request_timer --;  // used to send message every 10 seconds to D6

    if (0 >=  dispbrightness_write_request_timer)
   {
        dispbrightness_write_request_timer = (unsigned_16)(cycleRate_hz * 10);  // every 10 seconds loops send write request
        // cycleRate_hz = 20 ==   50ms * 200 = 10000 = 10 seconds
        // cycleRate_hz = 10 == 100ms * 100 = 10000 = 10 seconds
        // cycleRate_hz = 5 ==   200ms *   50 = 10000 = 10 seconds
        // cycleRate_hz = 1 ==  1000ms *   10 = 10000 = 10 seconds

        /* Send a write request */
        tx_message[4] = (0x0064 & 0x00FF);      /* Set to 100% brightness */
        tx_message[5] = (0x0000 & 0xFF00);       /* Set to 100% brightness */

        scl_j1939_que_tx_msg_by_address(
                Ph_Link_app,                /* Handle identifying J1939 connection */
                0x00EF00,                   /* Proprietary PGN */
                6,                          /* priority identifier field */
                mid_D6_display,          /* destination address used in the PS field */
                0,                          /* Index to ecm in address table */
                tx_message,                 /* pointer to the data field that will be transmitted */
                6                           /* the length of the data field in number of bytes */
        );
    }
  
    return;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Main loop for AutonomyConditionDiagnostics.
///
/// @return true/false
///
///////////////////////////////////////////////////////////////////////////////
bool AutonomyConditionDiagnostics::autonomyExecutive( )
{
    if(!m_initDone)
    {
        return true;
    }

    // Set Machine Sleep AutonomyCondition (from J1939)
    if (dm13_receive_suspend) {
        setAutonomyCondition(MachineSleep());
    }
    else {
        clearAutonomyCondition<MachineSleep>();
    }

    // check for requests on scs
    ScsChkForReqst();

    /* check for ethernet diagnostics */
    CheckEthernetDiags();

    //Update Plugins
    if(!updatePlugins())
    {
        // We don't want to stop executive function in case plugin update fails
        AIS_LOG_ERROR("Error updating plugins");
        //return false;
    }
  
    //struct clock_tm* clock_rtc_local_time;
    //clock_rtc_local_time = clock_tzone_get_local_tm();
    //printf("SA Local RTC:%d/%d/%d %d:%d:%d\n", clock_rtc_local_time->tm_mday, clock_rtc_local_time->tm_mon + 1, clock_rtc_local_time->tm_year + clock_TM_0_YEAR, clock_rtc_local_time->tm_hour,clock_rtc_local_time->tm_min,clock_rtc_local_time->tm_sec);

    if ( m_pAutonomyConditionMessageIn )
    {
        bool newMsgRx=true;
        while( newMsgRx )
        {
            AutonomyConditionMessage msg;
            newMsgRx = m_pAutonomyConditionMessageIn->get( msg );

            if ( newMsgRx )
            {
                if(acceptedHost(msg.getReporterHostname()))
                {
                    m_autoCondList.update( msg );
                }
            }
        }
    }

    if (TRUE == m_EDDT_init)
    {
        int dummy=0;

        // Update the evnt/diag code based on the autonomy conditions
        for (auto& diagEvent : m_diagEventList) {
            uint16_t cid = diagEvent.getCid();
            // Disable the engineering events for production software
            if (((TASKS_EID == cid) || (TASKC_EID == cid)) && (!isConceptSoftware_) ) {
                diagEvent.update(m_autoCondList, m_inhibitList, dummy, dummy, m_diagEventList, true);
            }
            else if ((DL_J19393_CID == cid) || (DL_J19392_CID == cid)) {
                // Enable datalink diagnostics even if payload sea is not installed for TES CAN. This
                // will only be active on AU2020 machine when we are not able to talk to either Imple
                // or IMU on TES CAN even when SEA is not installed
                diagEvent.update(m_autoCondList, m_inhibitList, dummy, dummy, m_diagEventList, false);
            }
            else {
                // Disable diagnostics if sea level 1 is not installed
                diagEvent.update(m_autoCondList, m_inhibitList, dummy, dummy, m_diagEventList, !SEALevel1EssentialsInstalled_);
            }

            // Only for purposes of printing all supported diags/events to console
            //AIS_LOG_ERROR("%s, %d, %d, %d", diagEvent.name().c_str(), diagEvent.getCid(), diagEvent.getFmi(), diagEvent.getWci());
        }

        /*
         * Update datalink diagnostics
         * - Only looking for inhibits configured for J1939_1 and J1939_3 CIDs
         */
        {
            bool dl_diag_inhibited = false;
            for (auto& diagEvent : m_diagEventList) {
                uint16_t cid = diagEvent.getCid();
                if ((DL_J19391_CID == cid) || (DL_J19393_CID == cid) ||
                        (DL_J19392_CID == cid) || (DL_CDL_CID == cid)) {
                    if (diagEvent.isInhibited(m_inhibitList)) {
                        dl_diag_inhibited = true;
                        break;
                    }
                }
            }

            isDatalinkDiagInhibited = dl_diag_inhibited;
        }

        /* OBD Event System related periodic tasks */
        scl_obd_es_update(app_scl_obd_es);

        /* PJ1939 diag update to support EDDT on J1939 link */
        scl_diag_pj1939_update(app_pj1939_diag_obj);

        { // Here we are detecting is anything has changed
            static bool diagEventChanged = true;
            static DiagnosticStatus diagStat;

            TimeStamp timestamp = commonNow();

            // Make sure stats are the same size as the diag/event list.
            diagStat.m_diagEventStatList.resize(m_diagEventList.size());

            for (unsigned int ii = 0; ii < m_diagEventList.size(); ++ii) {
                AutoDiagEvent& diagEvent = m_diagEventList[ii];
                DiagnosticStatus::DiagEventStatus& diagEventStat = diagStat.m_diagEventStatList[ii];

                // Grab the information for this diag/event
                diagEvent.getDiagEventStatus(diagEventStat);

                // This will "consume" the change detection.
                // Any subsequnt call to this function will not detect the change.
                if (diagEvent.changedActivated()) {
                    diagEventChanged = true;

                    if (diagEvent.active()) {
                        // It changed, and it is active. Set the time.
                        diagEventStat.m_gpsTime = timestamp;

                        if (diagEvent.logEnabled()) {
                            if (m_enableOutfileWriter) {
                                //Print to output file
                                m_outfileWriter.log(diagEvent, 0, 0 );
                            }
                        }
                    }
                }
            }

            if (diagEventChanged) { // something changed to re-write files & scs objects
                diagEventChanged = false;

                // Publish DiagnosticStatus because they changed.
                if (m_pDiagStatusOut) {
                    // Update Origination Time
                    diagStat.attributes.originationTime = timestamp;

                    // Send it out.
                    m_pDiagStatusOut->publish(diagStat);
                }

                std::vector<DetailedDiagnostics> diag_list;
                DetailedDiagEventsWriter diag_writer(tempRoot_);

                // Add all diagnostics
                for (auto& diagEvent : m_diagEventList) {
                    scl_obd_test_handle_t handle = diagEvent.getHandle();

                    // Get the occurrence information
                    scl_obd_es_occur_info_t  occur_info;
                    scl_obd_es_get_occur_info(app_scl_obd_es, handle, &occur_info);

                    DetailedDiagnostics diag;
                    diag.name = diagEvent.name();
                    diag.ID = diagEvent.getCid();
                    diag.count = occur_info.occur_cnt;
                    diag.first = occur_info.first_shm;
                    diag.last = occur_info.last_shm;
                    diag.active = diagEvent.active();
                    diag.MID_name = "Analysis Module"; // TODO - Fixed for A6N2 Blue-key ECM
                    diag.MID_ID = 0x37; // TODO - Fixed for A6N2 Blue-key ECM
                    if (AutoDiagEvent::AUTO_DIAGNOSTIC == diagEvent.getType()) {
                        diag.type = "DIAG";
                        diag.FMI = diagEvent.getFmi();
                        diag.WCI = diagEvent.getWci();
                    }
                    else {
                        diag.type = "EVENT";
                        diag.FMI = 0;
                        diag.WCI = diagEvent.getWci();
                    }

                    AIS_LOG_INFO("%s  CID:%d  FMI:%d active:%d count:%d first-shm:%d last-shm:%d", diag.name.c_str(), diag.ID, diag.FMI, diag.active, diag.count, diag.first, diag.last);

                    diag_list.push_back(diag);
                }

                // Create the json detailed diagnostic file
                diag_writer.createFile(diag_list);
            }
        }
    }


    { // Startup Event
        double timeRemaining_sec;
        bool startingUp = m_inhibitList.startupInhibitActive( timeRemaining_sec );

        //get the vector index of the startup in progress event
        int startupEventInd = getDiagEvent( "StartupInProgress" );
        //if the startup in progress event exists, set it's activation if the startup inhibit is active
        if ( startupEventInd >= 0 )
        {
            if ( startingUp )
            {
                std::stringstream ss;
                ss.setf(ios::fixed,ios::floatfield);   // floatfield set to fixed
                ss << "There are " << setprecision (2) << timeRemaining_sec << " Seconds Remaining." << std::endl;
                m_diagEventList.at( startupEventInd ).setConditionString( ss.str() );
                m_diagEventList.at( startupEventInd ).setStatus( true );
            }
            else
            {
                m_diagEventList.at( startupEventInd ).setConditionString( "" );
                m_diagEventList.at( startupEventInd ).setStatus( false );
            }
        }
    }

    { // Create ECM Summary File every 3 seconds
        using namespace std::chrono;
        static auto last_file_created_timestamp = high_resolution_clock::now();
        static bool init_flag = false;
        auto now = high_resolution_clock::now();
        auto duration_usec = duration_cast<microseconds>(now - last_file_created_timestamp);

        if ((duration_usec.count() > THREE_SECONDS_IN_MICROSECONDS) || !init_flag ) {
            CreateECMSummaryFile();
            init_flag = true;
            last_file_created_timestamp = high_resolution_clock::now();
        }
    }
    
    DataLinkDataScsRx();
    ReadWeighAppScsTx();
    ReadJobMgrAppScsTx();
    ReadStoreSwitchTx();
    ReadTotalsScs();
    ReadCycleSegScs();
    ReceiveDispatchSettings();

    IssueCmdToTotalsApp();
    IssueCmdToWeighApp();
    IssueCmdToJobMgrApp();
    WriteDispatchSettings();

    processHardwareHealthStatus();
    publishPartNumbers();
    PublishAutonomyConditionsTx();

    /* TODO: Brightness will be revisited, commented this out for now so we don't write
     * incorrectly to the vision display as we do today
     * */
    //processDispBrightness();

    ReadSystemParam();/* Which will read System parameter PIDs */
    ReadFromUITx();
    IssueCmdToUIApp();
    ClearPidFlags();

    ReadBEMSimState();

    // Disable certain PID writes when LFT is installed and sealed.
    if ((SEALegalForTradeInstalled_) && (LegalForTradeSealStatus_)) {
         lft_disable_write_pids = true;
    } 
    else {
         lft_disable_write_pids = false;
    }          

    AIS_LOG_DEBUG("Service Hour: %d",currentHour_Sec);

    { // Every 60 seconds, check for a deviation between system clock and steady clock
        auto steadyNow = std::chrono::steady_clock::now();
        if ((steadyNow - lastClockAdjustCheckTime_) >= std::chrono::minutes(1)) {
            auto systemNow = std::chrono::system_clock::now();

            auto steadyDiff = std::chrono::duration_cast<std::chrono::seconds>(steadyNow - previousSteadyClockTime_);
            auto systemDiff = std::chrono::duration_cast<std::chrono::seconds>(systemNow - previousSystemClockTime_);

            if (std::abs(systemDiff.count() - steadyDiff.count()) >= 2) {
                static bool oneTimeSync = false;
                if (!oneTimeSync) {
                    // The two clocks have changed by different amounts, this must mean
                    // a system clock adjustment so write it to HW RTC. 
                    // Only set the RTC hw once if it drifts after powerup since 
                    // setting RTC hw consumes time. We will set RTC again in cleanup      
                    std::tm localTime;
                    std::time_t currentTime = time(0);
                    if (nullptr != gmtime_r(&currentTime, &localTime)) {
                        int fd = open("/dev/rtc", O_RDONLY);
                        if (fd >= 0) {
                            if (ioctl(fd, RTC_SET_TIME, localTime) >= 0) {
                                oneTimeSync = true;
                            }
                            close(fd);
                        }
                    }
                }

                // Reset the time points in order to detect the next time change.
                previousSteadyClockTime_ = steadyNow;
                previousSystemClockTime_ = systemNow;                         
                AIS_LOG_ALERT("RTC Change of %lld seconds detected", (systemDiff.count() - steadyDiff.count()));    
            }

            // Check again later.
            lastClockAdjustCheckTime_ = steadyNow;
        }
    }

    return true;
}

/******************************************************************************
FUNCTION AutonomyConditionDiagnostics::ScsChkForReqst
DESCRIPTION: It will check the request from UI through SCS channel by polling method
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AutonomyConditionDiagnostics::ScsChkForReqst()
{
    AutonomyConditionDiagnosticsRequestInterface reqIn;

    /* Retrieve SCS channel data */
    while (autonomyConditionDiagnosticsScsReqstIn_->get(reqIn)) {
        bool success = true;

        /*
         * Check the request and handle them
         */
        switch (reqIn.command) {
        case (AutonomyConditionDiagnosticsRequestInterfaceCommand::SEA_LEVEL2_TEMP_INSTALL): {
            if (reqIn.data.installed) {
                // attempt to install Temp SEA Level2
                scl_prmsw_set_feat_cmd_status(LPS_SEA_REASON_CODE_245, SCL_PRMSW_FEAT_STATUS_CMD_FU_INSTALL);
            }
            else {
                // uninstall Temp SEA Level2
                scl_prmsw_set_feat_cmd_status(LPS_SEA_REASON_CODE_245, SCL_PRMSW_FEAT_STATUS_CMD_FU_UNINSTALL);
            }
            break; // out of switch-case
        }
        case (AutonomyConditionDiagnosticsRequestInterfaceCommand::NONE):
        default: {
            break; // out of switch-case
        }
        }

        // Build the response
        AutonomyConditionDiagnosticsResponseInterface response; // Default timepoint is now
        response.appName = reqIn.appName;
        response.appRequestId = reqIn.appRequestId;
        response.command = reqIn.command;
        response.success = success;

        if (autonomyConditionDiagnosticsRespChannelOutput_) {   /* Publish Response */
            if (autonomyConditionDiagnosticsRespChannelOutput_->publish(response)) {
                AIS_LOG_INFO("Published response, command=%d, success=%d", reqIn.command, success);
            }
            else {
                AIS_LOG_ERROR("Failed to publish response, command=%d, success=%d", reqIn.command, success);
            }
        }
        else {
            AIS_LOG_ERROR("Failed to publish response, channel is null, command=%d, success=%d", reqIn.command, success);
        }
    }
}


/******************************************************************************
FUNCTION AutonomyConditionDiagnostics::ReadBEMSimState
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AutonomyConditionDiagnostics::ReadBEMSimState()
{
    LpsSaUIBEMSimStateInterface dataIn;

    /* Retrieve SCS channel data */
    while (m_BEMSimStateInput->get(dataIn)) {
        bem_sim_derate_enable = dataIn.derateEngineCmd;
        bem_sim_derate_rpm = dataIn.derateEngineRPM;
    }
}

/******************************************************************************
FUNCTION ReadSystemParam
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AutonomyConditionDiagnostics::ReadSystemParam(void)
{
    PidF2C4 = 0x01 ;/*ECM Location Code*/
    PidF2C5 = 0x02;/*Desired ECM Location Code*/
}

/******************************************************************************
FUNCTION IssueCmdToWeighApp
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AutonomyConditionDiagnostics::IssueCmdToWeighApp(void)
{
    LpsSaWeighReqstChannel request;

    bool publish = true;

    if ((TRUE == WeighRangeBottomRqstFlg) || (TRUE == WeighRangeSizeRqstFlg)) {
        WeighRangeBottomRqstFlg = FALSE;
        WeighRangeSizeRqstFlg = FALSE;
        request.command = LpsSaWeighReqstChannel::Command::WRITE_WEIGH_RANGE;
        request.arg.f1 = (float)PidWrD00C9F / 10.f;
        request.arg.f2 = (float)PidWrD0102D / 10.f;
    }
    else if (PidWrD009A7Flag) {
        request.command = LpsSaWeighReqstChannel::Command::WRITE_CALIBRATION_WEIGHT;
        request.arg.f1 = (float)PidWrD009A7 / 1000.f; // Payload Control System Calibration Weight [1]
        PidWrD009A7Flag = false;
    }
    else if (PidWrD10ACCFlag) {
        PidWrD10ACCFlag = false;
        request.command = LpsSaWeighReqstChannel::Command::WRITE_OVERLOAD_WARNING_ENABLE;
        switch (PidWrD10ACC)
        {
        case (0x000D): { // Disabled
            request.arg.b = false;
            break;
        }
        case (0x000C): { // Enabled
            request.arg.b = true;
            break;
        }
        default: {
            publish = false;
            break;
        }
        }
    }
    else if (PidWrD10F96Flag) {
        // Payload Weight Calculation Acceptable Audible Alarm Configuration [1] (RECM)
        request.command = LpsSaWeighReqstChannel::Command::WRITE_AUDIBLE_WEIGHT_ENABLE;

        if (PidWrD10F96) {
            request.arg.b = true;
        }
        else {
            request.arg.b = false;
        }

        PidWrD10F96Flag = false;
    }
    else if (PidWrD009F5Flag) {
        request.command = LpsSaWeighReqstChannel::Command::WRITE_BUCKET_PAYLOAD_TARGET_WEIGHT;
        request.arg.f1 = (float)PidWrD009F5 / 10.f; // Loader Bucket Payload Target Weight [1]
        PidWrD009F5Flag = false;
    }
    else if (PidWrD10972Flag)
    {
        auto reqPloadCtrlSysStat = static_cast<ReqPloadCtrlSysStat_t>(PidWrD10972);
        switch (reqPloadCtrlSysStat)
        {
        case ReqPloadCtrlSysStat_t::ZERO:
            request.command = LpsSaWeighReqstChannel::Command::ZERO;
            break;
        default:
            publish = false;
            break;
        }
    }
    else if (PidD11B9A.wrFlag) {
        PidD11B9A.wrFlag = false;
        request.command = LpsSaWeighReqstChannel::Command::WRITE_HYD_OIL_TEMP_ENABLE;
        switch (PidD11B9A.wr) {
        case (0x000C): { // Enabled
            request.arg.b = true;
            break;
        }
        case (0x000D): { // Disabled
            request.arg.b = false;
            break;
        }
        default: {
            publish = false;
            break;
        }
        }
    }
    else if (PidWrD11F99Flag) {
        PidWrD11F99Flag = false;
        request.command = LpsSaWeighReqstChannel::Command::WRITE_LFT_SEALED;
        switch (PidWrD11F99)
        {
        case (0x0AAA ): { // Not Sealed
            request.arg.b = false;
            break;
        }
        case (0x0AA9): { // Sealed
            request.arg.b = true;
            break;
        }
        default: {
            publish = false;
            break;
        }
        }
    }
    else if (PidWrD1207AFlag) {
            PidWrD1207AFlag = false;
            request.command = LpsSaWeighReqstChannel::Command::WRITE_LFT_SEALED_FLASH_ENABLE;
            switch (PidWrD1207A)
            {
            case (0x000C ): { // Enabled
                request.arg.b = true;
                break;
            }
            case (0x000D): { // Disabled
                request.arg.b = false;
                break;
            }
            default: {
                publish = false;
                break;
            }
            }
    }
    else {
        /* Do nothing */
        publish = false;
    }

    if (publish) {
        weighAppInf_.sendRequest(request);
        NewLpsSaWeighScsTxIn_ = 0;
    }
    PendingLpsSaWeighScsReqOut_ = 0;
}


/******************************************************************************
FUNCTION IssueCmdToJobMgrApp
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AutonomyConditionDiagnostics::IssueCmdToJobMgrApp(void)
{
    std::vector<LpsSaJobMgrReqstChannel> requests;

    if (PidWrD112B2Flag) {
        PidWrD112B2Flag = false;

        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TIPOFF_TRIGGER_TYPE;

        switch (PidWrD112B2) {
        case TIP_OFF_AUTO: {
            req.data.tipoffTriggerType = LPS_SA_JOB_MGR_TIP_OFF_TRIGGER_AUTO;
            break;
        }
        case TIP_OFF_MANUAL: {
            req.data.tipoffTriggerType = LPS_SA_JOB_MGR_TIP_OFF_TRIGGER_MANUAL;
            break;
        }
        default: {
            req.data.tipoffTriggerType = LPS_SA_JOB_MGR_TIP_OFF_TRIGGER_DISABLED;
            break;
        }
        }

        requests.push_back(std::move(req));
    }
    else if (PidWrD10E4EFlag) {
        PidWrD10E4EFlag = false;

        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TIPOFF_MODE;

        switch (PidWrD10E4E) {
        case TIP_OFF_PILE: {
            req.data.tipoffMode = LPS_SA_JOB_MGR_TIP_OFF_PILE_ENABLE;
            break;
        }
        case TIP_OFF_TRUCK: {
            req.data.tipoffMode = LPS_SA_JOB_MGR_TIP_OFF_TRUCK_ENABLE;
            break;
        }
        default: {
            req.command = LpsSaJobMgrReqstChannel::Command::NONE;
            break;
        }
        }

        if (LpsSaJobMgrReqstChannel::Command::NONE != req.command) {
            requests.push_back(std::move(req));
        }
    }
    else if (PidWrD106C4Flag) {
        PidWrD106C4Flag = false;

        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_HORN_ON_STORE_ENABLED;

        switch (PidWrD106C4) {
        case StorePloadHornStat_t::SOUND_HORN: {
            req.data.enabled = true;
            break;
        }
        case StorePloadHornStat_t::NOT_SOUND_HORN: {
            req.data.enabled = false;
            break;
        }
        default: {
            req.command = LpsSaJobMgrReqstChannel::Command::NONE;
            break;
        }
        }

        if (LpsSaJobMgrReqstChannel::Command::NONE != req.command) {
            requests.push_back(std::move(req));
        }
    }
    else if (PidD0273B.wrFlag) { /* Payload Auto Store Pass Count Threshold */
        PidD0273B.wrFlag = false;

        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_STORE_PASS_COUNT;
        req.data.autoStorePassCount = PidD0273B.wr;
        requests.push_back(std::move(req));
    }
    /* PCS Features Configuration #2 Write Request */
    else if (PidF2CA.wrFlag) {
        PidF2CA.wrFlag = false;

        { // Check Auto Truck Id
            LpsSaJobMgrReqstChannel req;
            req.command = LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_TRUCK_ID_ENABLED;

            if (0 != (PidF2CA.wr & 0x10)) {
                req.data.enabled = true;
            }
            else {
                req.data.enabled = false;
            }

            requests.push_back(std::move(req));
        }
        { // Check Auto Material Id
            LpsSaJobMgrReqstChannel req;
            req.command = LpsSaJobMgrReqstChannel::Command::WRITE_AUTO_MATERIAL_ID_ENABLED;

            if (0 != (PidF2CA.wr & 0x08)) {
                req.data.enabled = true;
            }
            else {
                req.data.enabled = false;
            }

            requests.push_back(std::move(req));
        }
    }
    else if (PidWrD10972Flag) {
        PidWrD10972Flag = false;
        auto reqPloadCtrlSysStat = static_cast<ReqPloadCtrlSysStat_t>(PidWrD10972);

        LpsSaJobMgrReqstChannel req;

        switch (reqPloadCtrlSysStat) {
        case ReqPloadCtrlSysStat_t::CLEAR: {
            req.command = LpsSaJobMgrReqstChannel::Command::CLEAR;
            break;
        }
        case ReqPloadCtrlSysStat_t::REWEIGH: {
            req.command = LpsSaJobMgrReqstChannel::Command::REWEIGH;
            break;
        }
        case ReqPloadCtrlSysStat_t::STORE: {
            req.command = LpsSaJobMgrReqstChannel::Command::STORE;
            break;
        }
        case ReqPloadCtrlSysStat_t::MINUS_ONE: {
            req.command = LpsSaJobMgrReqstChannel::Command::MINUS_ONE;
            break;
        }
        default: {
            req.command = LpsSaJobMgrReqstChannel::Command::NONE;
            break;
        }
        }

        if (LpsSaJobMgrReqstChannel::Command::NONE != req.command) {
            requests.push_back(std::move(req));
        }
    }
    else if (PidWrD10748Flag) {
        PidWrD10748Flag = false;
        auto reqPayloadOperatingMode = static_cast<ReqPloadOpMode_t>(PidWrD10748);

        LpsSaJobMgrReqstChannel req;

        switch (reqPayloadOperatingMode) {
        case ReqPloadOpMode_t::NORMAL: {
            req.command = LpsSaJobMgrReqstChannel::Command::STANDBY_DEACTIVATE;
            break;
        }
        case ReqPloadOpMode_t::STANDBY: {
            req.command = LpsSaJobMgrReqstChannel::Command::STANDBY_ACTIVATE;
            break;
        }
        default: {
            req.command = LpsSaJobMgrReqstChannel::Command::NONE;
            break;
        }
        }

        if (LpsSaJobMgrReqstChannel::Command::NONE != req.command) {
            requests.push_back(std::move(req));
        }
    }
    else if (PidD11919.wrFlag) {
        PidD11919.wrFlag = false;
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_MANUAL_ADD_ENABLED;
        req.data.enabled = (0x000C == PidD11919.wr);
        requests.push_back(std::move(req));
    }
    else {
        /* Do nothing */
    }

    for (const auto& req : requests) {
        if (!LpsSaJobMgrScsReqstOut->publish(req)) {
            AIS_LOG_ERROR("Failed to publish job manager request.");
        }
        else {
            /* reset the Tx In flag */
            NewLpsSaJobMgrScsTxIn_ = 0;
        }
    }

    /* clear pending requests flag */
    PendingLpsSaJobMgrScsReqOut_ = 0;
}


void AutonomyConditionDiagnostics::CreateECMSummaryFile(void) {
    ecmSummary.ecm_partnumber.assign(ecm_part_number, ECM_PART_NUMBER_MAX_SIZE);
    ecmSummary.ecm_serialnumber.assign(ecm_serial_number, ECM_SERIAL_NUMBER_MAX_SIZE);
    ecmSummary.sw_partnumber.assign(software_group_part_number, SOFTWARE_GROUP_PN_LENGTH);
    ecmSummary.sw_description.assign(app_description_dl_tx_pie, APPLICATION_TEXT_MAX_SIZE );
    ecmSummary.sw_releasedate.assign(software_group_date_code, SOFTWARE_GROUP_DATE_CODE_LENGTH);
    ecmSummary.product_id.assign(pidF82D_pdata, PIDF82D_LENGTH);

    ecmSummaryAnalysisModule_.createFile(ecmSummary);

    ecmSummaryAnalysisDisplay_.getServiceDashboard();
    ecmSummaryProductLink_.getServiceDashboard();
}

void AutonomyConditionDiagnostics::ReadWeighAppScsTx(void)
{
    LpsSaWeighTxChannel WeighParam;
    unsigned_16 zeroReqStat[3]= {0x001B,0x03D4,0x03D3};

    if (weighAppInf_.waitForTxData(WeighParam)) {
        AIS_LOG_WARN("waitForTxData did not time out");
    }
    else {
        AIS_LOG_ERROR("waitForTxData timed out");
        return;
    }

    // We have data, update our variables with it
    PidD00C9F = (unsigned_16) (WeighParam.WeighRange.WeighRangeBottom/0.1);
    PidD0102D = (unsigned_16) (WeighParam.WeighRange.WeighRangeSize/0.1);
    PidD00E9B = static_cast<unsigned_16>(WeighParam.PidData.ProdMeasureWeighStatus.to_ulong());
    PidF13D = (unsigned_8) WeighParam.PidData.ProdMeasureSensorStatus.LiftLinkDC;/*Lift Linkage Position Sensor Duty Cycle*/

    /*Lift Cylinder Position*/
    if (WeighParam.PidData.ProdMeasureSensorStatus.LiftCylPos >= UNKNOWN2U) {
        PidF5AE = WeighParam.PidData.ProdMeasureSensorStatus.LiftCylPos; /*fault exists*/
    }
    else {
        PidF5AE = (unsigned_16) CONVERT_TO_PT1_RES(WeighParam.PidData.ProdMeasureSensorStatus.LiftCylPos);/*Lift Cylinder Position*/
    }

    PidF47B = (unsigned_16) WeighParam.PidData.ProdMeasureSensorStatus.LiftCylHEPres;/*Lift Cylinder Head End Pressure*/
    PidF5AF = (unsigned_16) WeighParam.PidData.ProdMeasureSensorStatus.LiftCylREPres;/*Lift Cylinder Rod End Pressure*/
    PidF4E0 = (unsigned_16) WeighParam.PidData.ProdMeasureSensorStatus.TiltCylHEPres;/*Tilt Cylinder Head End Pressure*/
    PidF47C = (unsigned_16) WeighParam.PidData.ProdMeasureSensorStatus.TiltCylREPres;/*Tilt Cylinder Rod End Pressure*/
    PidF13E = (unsigned_8) WeighParam.PidData.ProdMeasureSensorStatus.TiltLinkDC;/*Tilt Linkage Position Sensor Duty Cycle*/
    Pid00D00639 = (unsigned_16) (WeighParam.PidData.ProdMeasureSensorStatus.TiltLinkDC/0.1);/*Tilt Cylinder Position Sensor Duty Cycle*/

    if (WeighParam.PidData.HydOilTempEnabled) {
        HydOilTempEnabled = true;
        PidD11B9A.tx = 0x000C; // Enabled
        Pid0045 = (int_16) WeighParam.PidData.ProdMeasureSensorStatus.HydOilTemp;
        if ((IS_KNOWN2S(Pid0045)) && (Pid0045 < -273)) {
            Pid0045 = UNKNOWN2S + FMICNM;
        }
    }
    else {
        Pid0045 = UNKNOWN2S + FMIDNI;
        HydOilTempEnabled = false;
        PidD11B9A.tx = 0x000D; // Disabled
    }

    // Loader Tilt Linkage Position Sensor Configuration
    PidD10046.tx = WeighParam.PidData.ProdMeasureSensorStatus.TiltSensorConfig;

    { // Lift cylinder extension
        float cylinderExtension = WeighParam.LiftPosition.cylinderExtension;
        if (cylinderExtension < 0) {
            cylinderExtension = 0.f;
        }
        else if (cylinderExtension >= UNKNOWN2U) {
            cylinderExtension = UNKNOWN2U - 1;
        }

        Pid00D0018B = static_cast<unsigned_16>(cylinderExtension + 0.5f); /* Lift Cylinder Extension (Absolute) */

        if (WeighParam.LiftPosition.status == LPS_STATUS_BAD) {
            Pid00D0018B = UNKNOWN2U + FMIIUA;
        }
    }

    { // Tilt cylinder extension
        float cylinderExtension = WeighParam.TiltPosition.cylinderExtension;
        if (cylinderExtension < 0) {
            cylinderExtension = 0.f;
        }
        else if (cylinderExtension >= UNKNOWN2U) {
            cylinderExtension = UNKNOWN2U - 1;
        }

        Pid00D0018C = static_cast<unsigned_16>(cylinderExtension + 0.5f); /* Tilt Cylinder Extension (Absolute) */
        if (WeighParam.TiltPosition.status == LPS_STATUS_BAD) {
            Pid00D0018C = UNKNOWN2U + FMIIUA;
        }
    }

    { // Loader Tilt Linkage Angle
        float tiltAngle = WeighParam.TiltPosition.angle;
        if (tiltAngle >= 0.f) {
            // Datalink definition says it cannot go above 327.67 degrees.
            if (tiltAngle > 327.67) {
                tiltAngle = 327.67;
            }

            tiltAngle = (tiltAngle * 100.f) + 0.5f; // Scale and add 0.5 for rounding
        }
        else {
            // Datalink definition says it cannot go below -327.36 degrees.
            if (tiltAngle < -327.36) {
                tiltAngle = -327.36;
            }

            tiltAngle = (tiltAngle * 100.f) - 0.5f; // Scale and subtract 0.5 for rounding
        }

        PidF47A = static_cast<int_16>(tiltAngle);
        if (LPS_STATUS_BAD == WeighParam.TiltPosition.status) {
            PidF47A = UNKNOWN2S + FMIIUA;
        }
    }

    { // Bucket Angle
        float bucketAngle = WeighParam.TiltPosition.bucketAngle;
        if (bucketAngle >= 0.f) {
            // Datalink definition says it cannot go above 180 degrees.
            if (bucketAngle > 180.f) {
                bucketAngle = 180.f;
            }

            bucketAngle = (bucketAngle * 10.f) + 0.5f; // Scale and add 0.5 for rounding
        }
        else {
            // Datalink definition says it cannot go below -180 degrees.
            if (bucketAngle < -180.f) {
                bucketAngle = -180.f;
            }

            bucketAngle = (bucketAngle * 10.f) - 0.5f; // Scale and subtract 0.5 for rounding
        }

        Pid00D00A70 = static_cast<int_16>(bucketAngle); /* Loader Bucket Angle */
        if (WeighParam.TiltPosition.status == LPS_STATUS_BAD)  {
            Pid00D00A70 = UNKNOWN2S + FMIIUA;
        }
    }

    PidD010FA = (unsigned_16) CONVERT_TO_PT01_RES(WeighParam.PidData.LinkSensorCalLim.LiftPosSensorFullRaiseDC);/*Lift Linkage Position Sensor Full Raise Duty Cycle*/
    PidD010FB = (unsigned_16) CONVERT_TO_PT01_RES(WeighParam.PidData.LinkSensorCalLim.LiftPosSensorFullLowerDC);/*Lift Linkage Position Sensor Full Lower Duty Cycle*/
    PidD010FC = (unsigned_16) CONVERT_TO_PT01_RES(WeighParam.PidData.LinkSensorCalLim.TiltPosSensorFullRackDC);/*Tilt Linkage Position Sensor Full Rackback Duty Cycle*/
    PidD010FD = (unsigned_16) CONVERT_TO_PT01_RES(WeighParam.PidData.LinkSensorCalLim.TiltPosSensorFullDumpDC);/*Tilt Linkage Position Sensor Full Dump Duty Cycle*/
    PidD01788 = WeighParam.PayloadCalcMeth; /*Payload Calculation Method*/
    PidD10ACC = WeighParam.PidData.OverloadWarningEnabled ? 0x000C : 0x000D; /* Payload Overload Warning Enable [1] (RECM)*/
    PidD01B24 = (int_16) CONVERT_TO_PT01_RES(WeighParam.PidData.LastPloadWt);
    if(WeighParam.PidData.LastPloadWt <= 0)
    {
        PidD01B24 = UNKNOWN2S + FMICNM;
    }
    PidD106DF = (unsigned_16) WeighParam.Indicator;
    PidD10822 = (unsigned_16) WeighParam.PidData.PloadSysZeroStat;

    PidD009A7 = (unsigned_16) CONVERT_TO_PT001_RES(WeighParam.CalWt);/* - Payload Control System Calibration Weight [1]*/
    PidD009F5 = (unsigned_16) CONVERT_TO_PT1_RES(WeighParam.PidData.LoaderBktPloadTgtWt);/* - Loader Bucket Payload Target Weight [1]*/
    PidD109A5 = (unsigned_16) WeighParam.PidData.PloadSysCalWtEntryReqStat;/*Payload System Calibration Weight Entry Requirement Status*/
    PidD10933 = (unsigned_16) zeroReqStat[WeighParam.PidData.PloadSysZeroReqStat];/*Payload System Zero Requirement Status*/
    PidD00D67 = (unsigned_16) CONVERT_TO_PT1_RES(WeighParam.PidData.LoaderBktPloadTgtWtPer);

    if(WeighParam.PidData.LoaderBktPloadTgtWtPer > 100)
    {
        PidD00D67 = UNKNOWN2U + FMICNM;
    }

    LiftCylinderVelocity_val = (int_16)WeighParam.LiftCylVel.Val;

    /* Audible Alarm if level 2*/
    if (SEALevel2ProInstalled_) {
        PidD02923 = (unsigned_16) WeighParam.PidData.AudibleWeightCommand;/*Desired Audible Annunciation Priority*/
        PidD10F96 = (unsigned_16) WeighParam.PidData.AudibleWeightEnabled;/*Payload Weight Calculation Acceptable Audible Alarm Configuration*/
    }
    else {
        /* disabled or not installed */
        PidD02923 = UNKNOWN2U + FMIDNI;
        PidD10F96 = UNKNOWN2U + FMIDNI;
    }

    /*Production Measurement Loading Status [-75716]*/
    /*Bucket Payload Data Status */
    PidF2C6 = WeighParam.PidData.BktPayloadData;

    PidD00C8D = (int_16)CONVERT_TO_PT01_RES(WeighParam.BestBktWtInTonnes);/*Bucket Payload*/
    if ((BUCKET_PAYLOAD_AVAILABLE != WeighParam.PidData.BktPayloadData) ||
        (false == dispBestBktWt.isOk))
    {
        PidD00C8D = UNKNOWN2S + FMI14;
    }

    /* IMU data */
    // Datalink definition says it cannot be greater 32.767 degree
    // or smaller -32.736 degree
    if ( (WeighParam.PidData.TipoffPitchCalOffset <= 32.767) &&
         (WeighParam.PidData.TipoffPitchCalOffset >= -32.736) ) {
         
         pidd02bb6_pdata = (int_least16_t) CONVERT_TO_PT001_RES(WeighParam.PidData.TipoffPitchCalOffset);
    }         
    else {

         pidd02bb6_pdata = UNKNOWN2S + FMIDNR;  /*faulted*/   
    } 

    /* IMU data */
    // Datalink definition says it cannot be greater 32.767 tonne
    // or smaller -32.736 tonne
    if ( (WeighParam.ToaAnchoredZeroOffset <= 32.767) && 
         (WeighParam.ToaAnchoredZeroOffset >= -32.736) ) {
          
          pidd02bb7_pdata = (int_least16_t) CONVERT_TO_PT001_RES(WeighParam.ToaAnchoredZeroOffset);
    }
    else {
         pidd02bb6_pdata = UNKNOWN2S + FMIDNR;  /*faulted*/ 
    }
    
    /* IMU data */
    pidd02bb8_pdata = (unsigned_16) CONVERT_TO_PT0001_RES(WeighParam.ToaAnchoredFactor);
    PidD019A1 = WeighParam.PidData.MachineRearLateralAcceleration;
    PidD019A3 = WeighParam.PidData.MachineRearLongitudinalAcceleration;
    PidD019A5 = WeighParam.PidData.MachineRearVerticalAcceleration;
    PidF58B = WeighParam.PidData.MachinePitch;
    PidF533 = WeighParam.PidData.MachineSlope;
    PidD01AFF = WeighParam.PidData.MachineRearRoll;
    PidD01FC9 = WeighParam.PidData.MachineRearSideSlope;
    PidD00A64 = WeighParam.PidData.MachineRoll;
    PidD00409 = WeighParam.PidData.MachineSideSlope;

    PayloadCalInProgress_ = WeighParam.PayloadCalInProgress;

    { // Calibration Identification Number
        auto str = std::to_string(WeighParam.LftSealStatus.sealId);
        auto length = str.copy(&pidF958_pdata[0], PIDF958_LENGTH, 0);
        if (length < PIDF958_LENGTH) {
            pidF958_pdata[length] = '\0';
        }
    }


    
    // Legal For Trade
    {
        // Set PidD11F99 to dsi20 (aka hide) if LFT is not installed. 
        if (!SEALegalForTradeInstalled_)
        {
            PidD11F99 = UNKNOWN2U+FMIDNI;
            PidD1207A = UNKNOWN2U+FMIDNI;
        }
        else
        {    
            if(WeighParam.LftSealStatus.sealed)
            {
                // Sealed
                PidD11F99 = 0x0AA9 ;
                LegalForTradeSealStatus_ = true;
            }
            else 
            {    
                //Not Sealed
                PidD11F99 = 0x0AAA;
                LegalForTradeSealStatus_ = false;
            }

            // Flash enable status
            if (WeighParam.flashEnabled)
            {
                PidD1207A = 0x000C;
            }
            else
            {
                PidD1207A = 0x000D;
            }
        }    
        
        {
            std::string str = std::to_string(WeighParam.LftSealStatus.sealId);
            std::strncpy(pidf9d0_pdata, str.c_str(), PIDF9D0_LENGTH);
        }

        {
            std::tm tmLocal;
            getCalendarTimeFromTimePoint(WeighParam.LftSealStatus.sealTime, tmLocal);
            if (tmLocal.tm_year < PidF9D1BaseYear)
            {
                AIS_LOG_INFO("Year is less than 1985 %d", tmLocal.tm_year);
                memset(&pidf9d1_pdata[0], 0 , PIDF9D1_LENGTH);
            }
            else
            {
                /* Seconds Resolution: 0.25 sec/bit */
                pidf9d1_pdata[0] = (uint8_t) (tmLocal.tm_sec*4);

                /* Min */
                pidf9d1_pdata[1] = (uint8_t) tmLocal.tm_min;

                /* Hour */
                pidf9d1_pdata[2] = (uint8_t) tmLocal.tm_hour;

                /* Month */
                // PIDF9D1 uses 1 for January, etc, while tm_mon uses 0 for January
                pidf9d1_pdata[3] = (uint8_t) (tmLocal.tm_mon + PidF9D1MonOffset);  // change 0-11 to 1-12

                /* Day Resolution: 0.25 day/bit */
                pidf9d1_pdata[4] = (uint8_t) (tmLocal.tm_mday*4);

                /* Year */
                //  PIDF9D1 bases year 1985 as 0, while tm_year bases year 1900 as 0, 2000 as 100
                pidf9d1_pdata[5] = (uint8_t) (tmLocal.tm_year - PidF9D1BaseYear); // change from 1900 to 1985
            }
        }

        if (WeighParam.SimpleCalAdjust <= 0.f) {
            PidD023EA = 0;
        }
        else if (WeighParam.SimpleCalAdjust >= 655.03f) {
            PidD023EA = 65503;
        }
        else {
            PidD023EA = (unsigned_16)CONVERT_TO_PT01_RES(WeighParam.SimpleCalAdjust);
        }
        
        if (WeighParam.ZeroWeight <= -327.36) {
            PidD02B3B = -32736;
        }
        else if (WeighParam.ZeroWeight >= 327.67) {
            PidD02B3B = 32767;
        }
        else {
            PidD02B3B = (int_16)CONVERT_TO_PT01_RES(WeighParam.ZeroWeight);
        }
    }
    
    NewLpsSaWeighScsTxIn_ = 1;
}

void AutonomyConditionDiagnostics::ReadCycleSegScs() {
    if (nullptr != cycleSegTxInterfaceInputChannel_) {
        CycleSegTxInterface CycleSegData;
            while (cycleSegTxInterfaceInputChannel_->get(CycleSegData)) {
                switch ( CycleSegData.digState )//0 = Inactive, 1 = Tentative, 2 = Active
                {
                    case 0:
                        PidD11907 = 0x0808; // Not active
                        break;
                    case 1:
                        PidD11907 = 0x0829; // Tentative
                        break;
                    case 2:
                        PidD11907 = 0x000E ; // Active
                        break;
                    default:
                        PidD11907 = UNKNOWN2U + FMI14;
                        break;
                }

                PidD0272F = CycleSegData.activeSegmentId;
            }
    }
}

void AutonomyConditionDiagnostics::ReadTotalsScs() {
    if (nullptr != totalsInputChannel_) {
        LpsSaTotalsInterface totals;
        while (totalsInputChannel_->get(totals)) {
            PidFCF8 = (uint32_t)(totals.weightTonnes + 0.5); // Round when cast to uint32_t
            PidFC06 = totals.loadCount;
            PidFCF7 = totals.passCount;
            PidFE22 = PidFCF7;
            PidD001A4 = (uint16_t)CONVERT_TO_PT5_RES(totals.memoryRemainingPct);
        }
    }

    if (nullptr != printerCnfgInputChannel_) {
        LpsSaTotalsPrinterCnfgInterface printerCnfg;
        while (printerCnfgInputChannel_->get(printerCnfg)) {
            printerCnfg_ = printerCnfg.config;
            printerCnfgReceivedTime_ = Clock::now();

            // Fill in the PIDs
            PidD10E4D.tx = printerCnfg_.installed ? 0x0010 : 0x0011;
            PidD00CA0.tx = printerCnfg_.truckTicket.numCopies;
            PidD01D2A.tx = printerCnfg_.leadingBlankLines;
            PidD01D2B.tx = printerCnfg_.trailingBlankLines;
            PidD01D2C.tx = printerCnfg_.truckTicket.ticketNumber > 65503 ? 65503 : printerCnfg_.truckTicket.ticketNumber;
            PidD11132.tx = printerCnfg_.truckTicket.headerEnabled ? 0x000C : 0x000D;
            PidD03093.tx = printerCnfg_.truckTicket.retentionPeriod;

            std::strncpy(&(PidF933.tx[0]), printerCnfg_.truckTicket.headerLine1.c_str(), APP_PID_STRING_BUF_SIZE-1);
            std::strncpy(&(PidF934.tx[0]), printerCnfg_.truckTicket.headerLine2.c_str(), APP_PID_STRING_BUF_SIZE-1);
            std::strncpy(&(PidF935.tx[0]), printerCnfg_.truckTicket.headerLine3.c_str(), APP_PID_STRING_BUF_SIZE-1);

            PidD11133.tx = printerCnfg_.truckTicket.bucketWeightsEnabled ? 0x000C : 0x000D;
            PidD11134.tx = printerCnfg_.truckTicket.truckWeightEnabled ? 0x000C : 0x000D;
            PidD11135.tx = printerCnfg_.truckTicket.truckIdEnabled ? 0x000C : 0x000D;
            PidD11136.tx = printerCnfg_.truckTicket.materialIdEnabled ? 0x000C : 0x000D;
            PidD11137.tx = UNKNOWN2U+FMIDNI;
            PidD11130.tx = LpsSaTotalsPrinterCnfgDateFormat_Base_t(printerCnfg_.dateFormat);
            PidD11131.tx = LpsSaTotalsPrinterCnfgDateSeparator_Base_t(printerCnfg_.dateSeparator);
            PidD11138.tx = printerCnfg_.truckTicket.storeTimeEnabled ? 0x000C : 0x000D;
            PidD11139.tx = printerCnfg_.materialReport.resetTimeEnabled ? 0x000C : 0x000D;
            PidD1113A.tx = printerCnfg_.truckReport.resetTimeEnabled ? 0x000C : 0x000D;
        }
    }
}

void AutonomyConditionDiagnostics::IssueCmdToTotalsApp() {
    if (PidWrFC06Flag) {
        if (nullptr != totalsRequestOutputChannel_) {
            LpsSaTotalsRequestInterface out;
            out.appName = getTaskName();
            out.command = LpsSaTotalsRequestInterfaceCommand::WRITE_LIFETIME_TOTAL_LOAD_COUNT;
            out.loadCount(PidWrFC06);
            totalsRequestOutputChannel_->publish(out);
            AIS_LOG_INFO("Totals request published for writing total load count.");
        }
        PidWrFC06Flag = 0;
    }
    else if (PidWrFCF8Flag) {
        if (nullptr != totalsRequestOutputChannel_) {
            LpsSaTotalsRequestInterface out;
            out.appName = getTaskName();
            out.command = LpsSaTotalsRequestInterfaceCommand::WRITE_LIFETIME_TOTAL_WEIGHT;
            out.weight(static_cast<double>(PidWrFCF8));
            totalsRequestOutputChannel_->publish(out);
            AIS_LOG_INFO("Totals request published for writing total weight.");
        }
        PidWrFCF8Flag = 0;
    }
    else if (PidWrFCF7Flag) {
        if (nullptr != totalsRequestOutputChannel_) {
            LpsSaTotalsRequestInterface out;
            out.appName = getTaskName();
            out.command = LpsSaTotalsRequestInterfaceCommand::WRITE_LIFETIME_TOTAL_PASS_COUNT;
            out.passCount(PidWrFCF7);
            totalsRequestOutputChannel_->publish(out);
            AIS_LOG_INFO("Totals request published for writing total pass count.");
        }
        PidWrFCF7Flag = 0;
    }

    // Check for printer config writes.
    if ((printerCnfgReceivedTime_ > TimePoint::min()) && (nullptr != printerCnfgOutputChannel_)) {
        bool publishPrinterCnfg = false;

        // Printer Installation Status
        if (PidD10E4D.wrFlag) {
            switch (PidD10E4D.wr) {
            case(0x0010):
            case(0x003D):
            case(0x067A): {
                printerCnfg_.installed = true;
                publishPrinterCnfg = true;
                break;
            }
            case(0x0011): {
                printerCnfg_.installed = false;
                publishPrinterCnfg = true;
                break;
            }
            default: {
                break;
            }
            }
            PidD10E4D.wrFlag = 0;
        }

        // Number of Copies
        if (PidD00CA0.wrFlag) {
            printerCnfg_.truckTicket.numCopies = PidD00CA0.wr > 3 ? 3 : PidD00CA0.wr;
            publishPrinterCnfg = true;
            PidD00CA0.wrFlag = 0;
        }

        // Preceding Blank Lines
        if (PidD01D2A.wrFlag) {
            printerCnfg_.leadingBlankLines = PidD01D2A.wr > 3 ? 3 : PidD01D2A.wr;
            publishPrinterCnfg = true;
            PidD01D2A.wrFlag = 0;
        }

        // Trailing Blank Lines
        if (PidD01D2B.wrFlag) {
            printerCnfg_.trailingBlankLines = PidD01D2B.wr > 3 ? 3 : PidD01D2B.wr;
            publishPrinterCnfg = true;
            PidD01D2B.wrFlag = 0;
        }

        // Truck Ticket Number
        if (PidD01D2C.wrFlag) {
            printerCnfg_.truckTicket.ticketNumber = PidD01D2C.wr;
            publishPrinterCnfg = true;
            PidD01D2C.wrFlag = 0;
        }

        // Ticket Retention Period
        if (PidD03093.wrFlag) {
            printerCnfg_.truckTicket.retentionPeriod = PidD03093.wr;
            publishPrinterCnfg = true;
            PidD03093.wrFlag = 0;
        }

        // Ticket Header Enable Status
        if (PidD11132.wrFlag) {
            printerCnfg_.truckTicket.headerEnabled = (0x000C == PidD11132.wr);
            publishPrinterCnfg = true;
            PidD11132.wrFlag = 0;
        }

        // Header Line #1
        if (PidF933.wrFlag) {
            PidF933.wr[APP_PID_STRING_BUF_SIZE-1] = '\0';
            printerCnfg_.truckTicket.headerLine1.assign(&(PidF933.wr[0]));
            publishPrinterCnfg = true;
            PidF933.wrFlag = 0;
        }

        // Header Line #2
        if (PidF934.wrFlag) {
            PidF934.wr[APP_PID_STRING_BUF_SIZE-1] = '\0';
            printerCnfg_.truckTicket.headerLine2.assign(&(PidF934.wr[0]));
            publishPrinterCnfg = true;
            PidF934.wrFlag = 0;
        }

        // Header Line #3
        if (PidF935.wrFlag) {
            PidF935.wr[APP_PID_STRING_BUF_SIZE-1] = '\0';
            printerCnfg_.truckTicket.headerLine3.assign(&(PidF935.wr[0]));
            publishPrinterCnfg = true;
            PidF935.wrFlag = 0;
        }

        // Bucket Weights Enable Status
        if (PidD11133.wrFlag) {
            printerCnfg_.truckTicket.bucketWeightsEnabled = (0x000C == PidD11133.wr);
            publishPrinterCnfg = true;
            PidD11133.wrFlag = 0;
        }

        // Truck Weight Enable Status
        if (PidD11134.wrFlag) {
            printerCnfg_.truckTicket.truckWeightEnabled = (0x000C == PidD11134.wr);
            publishPrinterCnfg = true;
            PidD11134.wrFlag = 0;
        }

        // Truck ID Enable Status
        if (PidD11135.wrFlag) {
            printerCnfg_.truckTicket.truckIdEnabled = (0x000C == PidD11135.wr);
            publishPrinterCnfg = true;
            PidD11135.wrFlag = 0;
        }

        // Material ID Enable Status
        if (PidD11136.wrFlag) {
            printerCnfg_.truckTicket.materialIdEnabled = (0x000C == PidD11136.wr);
            publishPrinterCnfg = true;
            PidD11136.wrFlag = 0;
        }

        // Operator ID Enable Status
        // Not Writable
        if (PidD11137.wrFlag) {
            PidD11137.tx = PidD11137.wr = UNKNOWN2U+FMIDNI;
            PidD11137.wrFlag = 0;
        }

        // Date Format Configuration
        if (PidD11130.wrFlag) {
            switch (PidD11130.wr) {
            case(0x0670):
            case(0x0671):
            case(0x0672):
            case(0x0673):
            case(0x0674):
            case(0x0675):
            case(0x0676):
            case(0x0677): {
                printerCnfg_.dateFormat = static_cast<LpsSaTotalsPrinterCnfgDateFormat>(PidD11130.wr);
                publishPrinterCnfg = true;
                break;
            }
            default: {
                break;
            }
            }
            PidD11130.wrFlag = 0;
        }

        // Date Format Separator Configuration
        if (PidD11131.wrFlag) {
            switch (PidD11131.wr) {
            case (0x0678):
            case (0x0679): {
                printerCnfg_.dateSeparator = static_cast<LpsSaTotalsPrinterCnfgDateSeparator>(PidD11131.wr);
                publishPrinterCnfg = true;
                break;
            }
            default: {
                break;
            }
            }
            PidD11131.wrFlag = 0;
        }

        // Truck Ticket Store Time Enable Status
        if (PidD11138.wrFlag) {
            printerCnfg_.truckTicket.storeTimeEnabled = (0x000C == PidD11138.wr);
            publishPrinterCnfg = true;
            PidD11138.wrFlag = 0;
        }

        // Material Report Reset Time Enable Status
        if (PidD11139.wrFlag) {
            printerCnfg_.materialReport.resetTimeEnabled = (0x000C == PidD11139.wr);
            publishPrinterCnfg = true;
            PidD11139.wrFlag = 0;
        }

        // Truck Report Reset Time Enable Status
        if (PidD1113A.wrFlag) {
            printerCnfg_.truckReport.resetTimeEnabled = (0x000C == PidD1113A.wr);
            publishPrinterCnfg = true;
            PidD1113A.wrFlag = 0;
        }

        if (publishPrinterCnfg) {
            LpsSaTotalsPrinterCnfgInterface printerCnfg;
            printerCnfg.config = printerCnfg_;
            printerCnfgOutputChannel_->publish(printerCnfg);
            AIS_LOG_ALERT("Printer configuration change published.");
        }
    }
}

/*
 * Receive dispatch settings and update PIDs
 */
void AutonomyConditionDiagnostics::ReceiveDispatchSettings()
{
    if (nullptr != dispatchSettingsInput_) {
        WorkOrderAssistSettingsInterface settings;
        while (dispatchSettingsInput_->get(settings)) {
            if (settings.installed) {
                PidD11879.tx = 0x0010; // Installed
                PidD11878.tx = settings.enabled ? 0x000C : 0x000D; // Enabled : Disabled
            }
            else {
                PidD11879.tx = 0x0011; // Not Installed
                PidD11878.tx = UNKNOWN2U + FMIDNI; // Disabled or Not Installed
            }

            dispatchSettings_ = settings;
        }
    }
}

/*
 * Write any dispatch settings changes to the dispatch app
 */
void AutonomyConditionDiagnostics::WriteDispatchSettings()
{
    bool publishSettings = false;
    WorkOrderAssistSettingsInterface settings(dispatchSettings_);

    // Dispatch Installation Status
    if (PidD11879.wrFlag) {
        PidD11879.wrFlag = false;

        // Write not supported
        PidD11879.tx = settings.installed ? 0x0010 : 0x0011; // Installed : Not Installed
        PidD11879.wr = PidD11879.tx;
    }

    // Dispatch Enable Status
    if (PidD11878.wrFlag) {
        PidD11878.wrFlag = false;

        if (settings.installed) {
            settings.enabled = (0x000C == PidD11878.wr);
            publishSettings = true;
        }
        else {
            // Write not supported
            PidD11878.tx = UNKNOWN2U + FMIDNI; // Disabled or Not Installed
            PidD11878.wr = PidD11878.tx;
        }
    }

    if ((publishSettings) && (nullptr != dispatchSettingsWrOutput_)) {
        dispatchSettingsWrOutput_->publish(settings);
        AIS_LOG_INFO("Dispatch settings published.");
    }
}


void AutonomyConditionDiagnostics::ReadJobMgrAppScsTx()
{
    LpsSaJobMgrTxChannel JobMgrParam;  

    if ( LpsSaJobMgrScsTxIn)
    {
        while(LpsSaJobMgrScsTxIn->get( JobMgrParam ))
        {
            const unsigned short int TipOffConfPIDMap[3] = {TIP_OFF_AUTO, TIP_OFF_MANUAL, TIP_OFF_DISABLED};
            const unsigned short int TipOffModPIDMap[3] = {TIP_OFF_PILE, TIP_OFF_TRUCK, TIP_OFF_DISABLED};

            PidFD39 = (unsigned_32) CONVERT_TO_PT001_RES(JobMgrParam.truckWeight) ;/*Weigh Cycle Total Truck Payload*/
            PidFE2E = (int_32) CONVERT_TO_PT001_RES(JobMgrParam.remainingWeight);/*Remaining Payload to Load*/
            PidFD38 = (unsigned_32) (PidFD39 + PidFE2E);/*Truck Payload Target Weight*/
            Pid00D00BD3 = JobMgrParam.passCount;/*Load Cycle Pass Count*/
            PidD10748  = (unsigned_16) JobMgrParam.OperationMode;/*Payload Operating Mode*/

            /*Loader Payload State*/
            PidD10972 = (unsigned_16) JobMgrParam.ReqPloadCtrlSysStat;/*Requested Payload Control System Status*/

            if(LPS_SA_JOB_MGR_CLEAR_BTN_ENABLED == JobMgrParam.ClearMinusOneEnableStat)
            {
                PidD10FAD = (unsigned_16) MINUS_ONE_DISABLED ;
                PidD10FAC = (unsigned_16) CLEAR_BUTTON_ENABLED;/* Payload Clear Button Display Status */
            }
            else
            {
                PidD10FAD = (unsigned_16) MINUS_ONE_ENABLED ;
                PidD10FAC = (unsigned_16) CLEAR_BUTTON_DISABLED ;/* Payload Clear Button Display Status */

            }

            /*Configurable Parameters*/
            PidD10E4E = (unsigned_16) TipOffModPIDMap[JobMgrParam.TipOffStateCfg];/* - Tip-Off Mode [1] (RECM)*/
            PidD112B2 = (unsigned_16) TipOffConfPIDMap[JobMgrParam.TipOffTriggerType];
            PidD106C4 = (unsigned_16) JobMgrParam.HornStoreState;

            {   // PCS Features Configuration #2
                // 1 1 0 x x 0 0 1
                // | | | | | | | |
                // | | | | | | | - Zero Reminder Feature (1 - Always Enabled)
                // | | | | | | - Warmup Lifts Feature (0 - Always Disabled)
                // | | | | | - Automatic Operator ID Feature (0 - Always Disabled)
                // | | | | - Automatic Material ID Feature (Configurable)
                // | | | - Automatic Truck ID Feature (Configurable)
                // | | - Not Used (0)
                // | - Lift Cylinder Rod End Pressure Sensor (1 - Always Enabled)
                // - Tilt Sensor (1 - Always Enabled for now)
                uint8_t data = 0xC1; // Start with default
                if (JobMgrParam.AutoTruckIdEnabled) {
                    data |= 0x10; // Set Automatic Truck ID Feature
                }

                /* store the Best Bucket Weight val and status */
                dispBestBktWt = JobMgrParam.DispBestBktWt;

                if (JobMgrParam.AutoMaterialIdEnabled) {
                    data |= 0x08; // Set Automatic Material ID Feature
                }

                PidF2CA.tx = data;
            }

            { // Payload Manual Add Configuration
                PidD11919.tx = JobMgrParam.manualAddEnabled ? 0x000C : 0x000D;
            }

            { // Payload Auto Store Pass Count Threshold
                PidD0273B.tx = JobMgrParam.AutoStorePassCount;
            }

            /* received new data */
            NewLpsSaJobMgrScsTxIn_++;
        }
    }

    /* Hide level2 (Pro) parameters by setting dsi if not installed */
    if (!SEALevel2ProInstalled_) {
        PidD106C4 = UNKNOWN2U + FMIDNI; /* Store Payload Data Horn Configuration */
        PidD11919.tx = UNKNOWN2U + FMIDNI; /* Payload Manual Add Configuration */   
    }
}

/********************************************************************************
FUNCTION NAME:
DESCRIPTION:
PARAMETER DESCRIPTION: No parameter
RETURN VALUE: void
*******************************************************************************/
void AutonomyConditionDiagnostics::ReadFromUITx() {
    if (nullptr != displayStateInputChannel_) {
        bool newInput = false;
        LpsSaUIDisplayStateInterface in;
        while (displayStateInputChannel_->get(in)) {
            newInput = true;
        }

        if (newInput) {
            displayState_ = in.state;
            const LpsSaUIDisplaySettings& displaySettings = displayState_.getSettings();
            PidD0022C = static_cast<uint16_t>(displaySettings.language);
            PidD00144 = displaySettings.serviceModeEnableCode;
            PidF25B = static_cast<uint8_t>(displaySettings.units);
            PidD10938.tx = static_cast<uint16_t>(displaySettings.weightUnits);
            PidD11B8D.tx = static_cast<uint16_t>(displaySettings.weightPrecision);
            PidD01396 = displayState_.getDocumentId();
            AIS_LOG_INFO("Received new display state.");
        }
    }
}

/********************************************************************************
FUNCTION NAME:
DESCRIPTION:
PARAMETER DESCRIPTION: No parameter
RETURN VALUE: void
*******************************************************************************/
void AutonomyConditionDiagnostics::IssueCmdToUIApp() {
    bool newOutput = false;

    // Language
    if (PidWrD0022CFlag) {
        displayState_.setLanguage(static_cast<LpsSaUIDisplaySettingsLanguage>(PidWrD0022C));
        PidWrD0022CFlag = false;
        newOutput = true;
    }

    // Service Mode Enable Code
    if (PidWrD00144Flag) {
        displayState_.setServiceModeEnableCode(PidWrD00144);
        PidWrD00144Flag = false;
        newOutput = true;
    }

    // Information Units
    if (PidWrF25BFlag) {
        displayState_.setUnits(static_cast<LpsSaUIDisplaySettingsUnits>(PidWrF25B));
        PidWrF25BFlag = false;
        newOutput = true;
    }

    // Display Weight Units
    if (PidD10938.wrFlag) {
        PidD10938.wrFlag = false;
        displayState_.setWeightUnits(static_cast<LpsCommonWeightUnits>(PidD10938.wr));
        newOutput = true;
    }

    // Display Weight Precision Configuration
    if (PidD11B8D.wrFlag) {
        PidD11B8D.wrFlag = false;
        displayState_.setWeightPrecision(static_cast<LpsCommonWeightPrecision>(PidD11B8D.wr));
        newOutput = true;
    }

    if ((nullptr != displaySettingsOutputChannel_) && newOutput) {
        LpsSaUIDisplaySettingsInterface out;
        out.settings = displayState_.getSettings();
        displaySettingsOutputChannel_->publish(out);
        AIS_LOG_INFO("New display settings published.");
    }
}


/********************************************************************************
FUNCTION NAME:
DESCRIPTION:
PARAMETER DESCRIPTION: No parameter
RETURN VALUE: void
*******************************************************************************/
void AutonomyConditionDiagnostics::PublishAutonomyConditionsTx() {
    if (nullptr != autonomyConditionDiagnosticsTxOutputChannel_) {

        /* Update ethernet status if diag is active */
        for (auto& diagEvent : m_diagEventList) {
            if (DL_ETHERNET1_CID == diagEvent.getCid()) {
                if (!diagEvent.isInhibited(m_inhibitList)) {
                    autonomyConditionDiagnosticsTx_.displayEthernetBad = ecmSummaryAnalysisDisplay_.isConnectionBad();
                    autonomyConditionDiagnosticsTx_.productLinkEthernetBad = ecmSummaryProductLink_.isConnectionBad();
                }
                break;
            }
        }

        // update SEA status
        updateSEAStatus();

        // update timepoint with time of publish
        autonomyConditionDiagnosticsTx_.setTimePoint();

        autonomyConditionDiagnosticsTxOutputChannel_->publish(autonomyConditionDiagnosticsTx_);
        AIS_LOG_INFO("AutonomyConditionDiagnosticsTx published");
    }
}

/******************************************************************************
FUNCTION ClearPidFlags
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AutonomyConditionDiagnostics::ClearPidFlags()
{
    PidWrD10972Flag = false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Cleanup for the AutonomyConditionDiagnostics
///////////////////////////////////////////////////////////////////////////////
void AutonomyConditionDiagnostics::autonomyCleanup( )
{
   AIS_LOG_INFO( "AutonomyConditionDiagnostics::cleanup" );

   // Send the DM13 suspend message
   app_dm13_send_sleep_msg();

   // Set RTC HW      
   std::tm localTime;
   std::time_t currentTime = time(0);
   if (nullptr != gmtime_r(&currentTime, &localTime)) {
       int fd = open("/dev/rtc", O_RDONLY);
       if (fd >= 0) {
           ioctl(fd, RTC_SET_TIME, localTime);
           close(fd);
       }
   }

   /* give it a second for dm13 sleep message to go out */
   sleep(1);

   /* stop the EventTimer threads, to shutdown threads gracefully */
   m_shmPublishTimer.stop();
   m_seaStatusTimer.stop();
   cleanupPlugins();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize ecm and software version data
///////////////////////////////////////////////////////////////////////////////
//extern "C" void app_version_init(oel_rtos_init_object_t const* never_used);

void AutonomyConditionDiagnostics::appVersionInit()
{
    { // Get ecm serial number
        hal_ecm_serialno_t serialNum = hal_boot_get_ecm_serialno();
        std::string serialNumStr = std::string(reinterpret_cast<const char*>(serialNum.string), serialNum.length);
        std::strncpy(ecm_serial_number, serialNumStr.c_str(), ECM_SERIAL_NUMBER_MAX_SIZE);
        m_partNumbers.SetECMSerialNumber(serialNumStr);
    }

    { // Get ecm part number
        hal_cat_partno_t partNum = hal_boot_get_ecm_partno();
        std::string partNumStr = std::string(reinterpret_cast<const char*>(partNum.string), partNum.length);
        std::strncpy(ecm_part_number, partNumStr.c_str(), ECM_PART_NUMBER_MAX_SIZE);
        m_partNumbers.SetECMPartNum(ecm_part_number);
    }

    { const auto& s = get_softwareGroupPartNumber();   std::memset(software_group_part_number, '\0', SOFTWARE_GROUP_PN_LENGTH);        std::memcpy(software_group_part_number, s.c_str(), std::min(s.size(), static_cast<std::size_t>(SOFTWARE_GROUP_PN_LENGTH))); }
    { const auto& s = get_softwareGroupReleaseDate();  std::memset(software_group_date_code, '\0', SOFTWARE_GROUP_DATE_CODE_LENGTH);    std::memcpy(software_group_date_code, s.c_str(), std::min(s.size(), static_cast<std::size_t>(SOFTWARE_GROUP_DATE_CODE_LENGTH))); }
    { const auto& s = get_softwareGroupDescription();  std::memset(app_description_dl_tx_pie, '\0', APPLICATION_TEXT_MAX_SIZE);         std::memcpy(app_description_dl_tx_pie, s.c_str(), std::min(s.size(), static_cast<std::size_t>(APPLICATION_TEXT_MAX_SIZE))); }

    m_partNumbers.SetSwGroupPartNum(software_group_part_number);
    m_partNumbers.SetSwGroupReleaseDate(software_group_date_code);
    m_partNumbers.SetSwGroupDescription(app_description_dl_tx_pie);

    { // Is this production or concept software?
        std::string str;
        str.assign(software_group_part_number, SOFTWARE_GROUP_PN_LENGTH);
        if (str.substr(str.length()-2, 2) == "00") {
            // Software Group Part Number ends in "00", it it production
            isConceptSoftware_ = false;
        }
        else {
            isConceptSoftware_ = true;
        }
        AIS_LOG_INFO("isConceptSoftware_: %d", isConceptSoftware_);
    }

    { // Payload Legal For Trade Measurement Software Identifier
        std::string version = legal_for_trade_software_id_version + "." + legal_for_trade_software_id_build;
        std::strncpy(PidF9C0, version.c_str(), PID_F9C0_STRING_BUF_SIZE-1);
    }
}

/*
 * Allow c code to log errors
 */
void app_log_alert(const char* file, int lineNum, const char* message) {
    logger::SingletonLogger::Instance()->logxx_alert(file, lineNum, message);
}

boolean get_time(scl_obd_real_time_stamp_t *real_time_stamp)
{
    std::tm tmp;
    std::time_t tt= commonNow().getTime_t() + 315964800; //adding the offset to convert from gps time to unix time

    if (nullptr != gmtime_r(&tt, &tmp)) {
        real_time_stamp->second = tmp.tm_sec;
        real_time_stamp->minute = tmp.tm_min;
        real_time_stamp->hour = tmp.tm_hour;
        real_time_stamp->day = tmp.tm_mday;
        real_time_stamp->month = (tmp.tm_mon)+1;
        real_time_stamp->year = tmp.tm_year + 1900;
        return true;
    }

    return false;
}
