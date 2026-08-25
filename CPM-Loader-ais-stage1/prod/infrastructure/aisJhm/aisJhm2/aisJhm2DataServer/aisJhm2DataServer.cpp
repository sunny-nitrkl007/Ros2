///////////////////////////////////////////////////////////////////////////////
/// @file      aisJhm2DataServer.cpp
/// @author    Josh Struble
/// @date      Aug 15, 2014
/// @brief     App to serve data to CGI web script
///
/// @attention Copyright (C) 2013
/// @attention Caterpillar, Inc.
///////////////////////////////////////////////////////////////////////////////
#include "aisJhm2DataServer.h"
#include "../AisJhm2RequestProcessor/AisJhm2RequestProcessor.h"
#include "../AisJhm2RequestProcessor/machineModelList.h"

using namespace task;
using namespace std;
int msn;
char *msn1;
///////////////////////////////////////////////////////////////////////////////
/// @brief This is the task creation method.
/// @return Task* Pointer to the instance of the task.
///////////////////////////////////////////////////////////////////////////////
AbstractTaskCore* task::getTaskImplementation(void)
{
  static aisJhm2DataServer thisTask("AisJhm2DataServer");
  return &thisTask;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Constructor
///
/// @param taskName Unique name for this task
///////////////////////////////////////////////////////////////////////////////
aisJhm2DataServer::aisJhm2DataServer( const std::string& taskName ):
  Task( taskName ),
  m_requestServer(),
  m_requestProc()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Destructor
///////////////////////////////////////////////////////////////////////////////
aisJhm2DataServer::~aisJhm2DataServer( )
{

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize method.
///
/// This method initializes the task's interfaces and configurable parameters.
///
/// @return True if successful, False otherwise
///////////////////////////////////////////////////////////////////////////////
bool aisJhm2DataServer::initialize( )
{
  if ( !m_requestProc.configure( getTaskParser() ) )
  {
    AIS_LOG_FATAL("Could not configure task parameters");
    return false;
  }

  ConfigSection machineData;

  if(!getTaskParser().get("MachineType", machineData))
  {
    getLogger().log_error("Couldnt find MachineType config in Robot_CAT_972M_STD.rb for Machine model");
    return false;
  }
  
  machineData.get( "InternalMsn", machineMSN );

  std::string machineMake;
  std::string machineModel;
  std::string machineLinkage;

  if(!machineData.get("Make", machineMake))
  {
    getLogger().log_fatal("Couldnt find MachineType->Make in ruby file");
    return false;
  }

  if(machineMake.empty())
  {
    getLogger().log_fatal("machine make in ruby is empty");
    return false;
  }

  if(!machineData.get("Model", machineModel))
  {
    getLogger().log_fatal("Couldnt find MachineType->Model in ruby file");
    return false;
  }

  if(machineModel.empty())
  {
    getLogger().log_fatal("machine model in ruby is empty");
    return false;
  } 

  if(!machineData.get("Linkage", machineLinkage))
  {
    getLogger().log_fatal("Couldnt find MachineType->Linkage in ruby file");
    return false;
  }

  if(machineLinkage.empty())
  {
    getLogger().log_fatal("machine linkage in ruby is empty");
    return false;
  } 

  MM_Selected_Make_Rd = machineMake;
  MM_Selected_Machine_Rd = machineModel;
  MM_Selected_Linkage_Rd = machineLinkage;

  getLogger().log_error( " Selected Machine model from ruby file %s",machineMake.c_str() );
  getLogger().log_error( " Selected Machine model from ruby file %s",machineModel.c_str() );
  getLogger().log_error( " Selected Machine model from ruby file %s",machineLinkage.c_str() );

  return true;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief This is the task's executive method.
///
/// This method will process user input from the keyboard.  Each key press
/// event is possibly a command to be relayed to the other PCM processes.
///
/// @return True if the task should continue executing, False otherwise.
///////////////////////////////////////////////////////////////////////////////
bool aisJhm2DataServer::executive( )
{
    m_requestProc.update();

using namespace task;

    if(m_requestProc.bemSimDerateSetEvent) {
        setAutonomyCondition(BEMSimDerate());
        m_requestProc.bemSimDerateSetEvent = false;

    }

    if(m_requestProc.bemSimDerateClrEvent) {
        clearAutonomyCondition<BEMSimDerate>();
        m_requestProc.bemSimDerateClrEvent = false;

    }

    if (m_requestProc.bemSimWarningSetEvent) {
        setAutonomyCondition(BEMSimLevelWarning());
        m_requestProc.bemSimWarningSetEvent = false;

    }

    if (m_requestProc.bemSimWarningClrEvent) {
            clearAutonomyCondition<BEMSimLevelWarning>();
            m_requestProc.bemSimWarningClrEvent = false;

    }

    if (m_requestServer.initialized()) {
        m_requestServer.update( m_requestProc );
    }
    else if (!m_requestServer.initialize()) {
        AIS_LOG_CRIT("Failed to initialize request server");
    }

    m_requestProc.postUpdate();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Cleanup task
///////////////////////////////////////////////////////////////////////////////
void aisJhm2DataServer::cleanup( )
{
	m_requestProc.cleanup();
}

