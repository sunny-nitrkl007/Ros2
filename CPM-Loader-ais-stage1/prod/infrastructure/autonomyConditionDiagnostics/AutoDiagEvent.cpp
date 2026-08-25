///////////////////////////////////////////////////////////////////////////////
// @file    AutoDiagEvent.cpp
// @author  J Struble
// @date    9/27/2011
//
// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- #Include's --
////////////////////////////////////////////////////////////////////////////////
#include "AutoDiagEvent.h"
#include <ais/log/Logger.h>

#include "src_scl_info/app_scl_obd_es_config.h"

////////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- Function Prototypes --
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- Symbols --
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- Start of code for this file --
////////////////////////////////////////////////////////////////////////////////
AutoDiagEvent::AutoDiagEvent( const std::string& name, scl_ci_es_fault_cfg_eddt_t* logConfig, DiagEventType type, scl_obd_test_handle_t handle, bool externallyReported ):
  InhibitCondition( name ),
  m_type( type ),
  m_triggerAutonomyConditions(),
  m_instanceNameIncl( false ), 
  m_instanceName(),
  m_latchUntilNewAssignment( false ),
  m_waitingForNewAssignment( false ),
  m_conditionString(),
  m_handle( handle ),
  m_pLogConfig( logConfig ), 
  m_status_db(),
  m_currStatus( false ),
  m_oldStatus( false ),
  m_enableLog( true ),
  m_inhibitConditions(),
  m_isInhibited( false ),
  m_diagName(name),
  m_externallyReported(externallyReported)
{
    if (!m_externallyReported) {
        scl_obd_es_error_t err;

        if (scl_obd_es_register_test(app_scl_obd_es,
                handle,
                logConfig->es_config
                ) == SCL_OBD_ES_SUCCESS) {
            err = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                    handle,
                    logConfig->eddt_config);
            if (0 == err) {
                AIS_LOG_DEBUG("SUCCESS!! registering %s (handle %d)", name.c_str(), handle);
            }
            else {
                AIS_LOG_DEBUG("scl_diag_pj1939_register_fault ERROR = %d for %s!", err, name.c_str());
            }
        }
        else {
            AIS_LOG_DEBUG( "ERROR registering test %s!", name.c_str() );
        }
    }
}


AutoDiagEvent::~AutoDiagEvent()
{
  
}

bool AutoDiagEvent::configure( ConfigSection& cs)
{
  if ( !InhibitCondition::configure( cs ) )
    return false;

  //Get the array of strings noting the triggered autonomy conditions
  if ( cs.hasParameter("AutonomyConditions"))
  {
     cs.getArray( "AutonomyConditions", m_triggerAutonomyConditions );
  }
  else
  {
     AIS_LOG_FATAL( "Unable to retrieve list of triggerd AutonomyCondition for DiagEvent %s!", 
                                                     name().c_str() );
     return false;
  }

  //Debounces are optional, so set them to 0 by default
  double dbI2A_sec = 0.0;
  cs.get( "Debounce_I2A_sec", dbI2A_sec );

  double dbA2I_sec = 0.0;
  cs.get( "Debounce_A2I_sec", dbA2I_sec );

  m_status_db.setDebounceTimes( dbI2A_sec, dbA2I_sec );

  m_instanceNameIncl = cs.get( "InstanceName", m_instanceName );
  cs.get( "WriteToLogfile", m_enableLog );
  
  cs.get( "LatchUntilNewAssignment", m_latchUntilNewAssignment );

  cs.getArray( "InhibitConditions", m_inhibitConditions );

  //We need to check that this condition isn't inhibiting on itself
  for ( unsigned int i = 0; i < m_inhibitConditions.size(); ++i )
  {
    if ( m_inhibitConditions.at(i) == name() )
    {
      AIS_LOG_FATAL( "%s is inhibiting on itself.  This is invalid.", name().c_str() );
      return false;      
    }
  }
  
  return true;
}
 
///////////////////////////////////////////////////////////////////////////////
/// @brief Look at the current active autonomyConditions to determine if any of this event's triggered conditions are active!
///
/// @param autoConds 
/// @param inhibitList 
/// @param asmNum  - assignment number indicates when a new assignment is received.
/// 
///////////////////////////////////////////////////////////////////////////////
void AutoDiagEvent::update( const AutonomyConditionsList& autoConds, const InhibitConditionList& inhibitList , const unsigned int &asmNum, const bool &inAutoMode, const std::vector<AutoDiagEvent>& diagEventList, bool disable)
{
    m_conditionString.clear();
    m_isInhibited = isInhibited(inhibitList);

    if (!m_externallyReported) {
        bool condActive = false;
        bool active = false;

        for (const auto& triggerCondition : m_triggerAutonomyConditions) {
            if (m_instanceNameIncl) {
                condActive = condActive || autoConds.conditionIsActive(m_instanceName, triggerCondition, m_conditionString);
            }
            else {
                condActive = condActive || autoConds.conditionIsActive(triggerCondition, m_conditionString);
            }

            for (const auto& diagEvent : diagEventList) {
                if (triggerCondition == diagEvent.getDiagName() && diagEvent.active()) {
                    condActive = true;
                    m_conditionString = m_conditionString + diagEvent.getConditionString();
                }
            }
        }
    
        // Check if we need to latch the diagnostic until a new assignment is received.
        if (m_latchUntilNewAssignment) {
            static unsigned int activeAsmNum;
            static bool condWasActive = false;
            if (condActive) {
                // Save the assignment number
                activeAsmNum = asmNum;
                // Activate the diagnostic
                active = true;
                // Set the latched diagnostic flag
                condWasActive = true;
                m_waitingForNewAssignment = false;
            }
            else if ((asmNum == activeAsmNum) && condWasActive && inAutoMode) {
                // Latch active because we have not received a new assignment yet and we are still in auto mode.
                active = true;
                m_waitingForNewAssignment = true;
            }
            else {
                // Clear diag/event
                active = false;
                // Reset latched flag
                condWasActive = false;
                m_waitingForNewAssignment = false;
            }
        }
        else {
            // The diagnostic being active or inactive depends only on the autonomy conditions.
            active = condActive;
        }

        // Update the raw status in the debounce handler if we have autonomyConditions to update from
        // This allows an event/diagnostic to be updated internally from the ACD task using the setStatus method
        if (m_triggerAutonomyConditions.size() > 0) {
            m_status_db.updateCurrentStatus( active );
        }

        if (disable) {
            // This diag/event is disabled, clear it.
            scl_obd_es_notify_test_results(
                    app_scl_obd_es,
                    m_handle,
                    SCL_OBD_ES_TEST_RESULTS_PASS);
        }
        else if (m_isInhibited) {
            // Don't report any results if we are being inhibited.
        }
        else if (0 != m_status_db.getDebouncedStatus()) {
            scl_obd_es_notify_test_results(
                    app_scl_obd_es,
                    m_handle,
                    SCL_OBD_ES_TEST_RESULTS_FAIL);
        }
        else {
            scl_obd_es_notify_test_results(
                    app_scl_obd_es,
                    m_handle,
                    SCL_OBD_ES_TEST_RESULTS_PASS);
        }
    }
}

void AutoDiagEvent::setStatus( bool status )
{
  //Update the raw status in the debounce handler  
  m_status_db.updateCurrentStatus( status );  
}
  
bool AutoDiagEvent::active() const
{
  //Use the fault report to determine if this event is active or not
  bool ret_val = false;
  scl_obd_es_fault_rep_t fault_report;

  /* get the current fault status */
  scl_obd_es_get_fault_rep(app_scl_obd_es,
                           m_handle,
                           &fault_report);

  if ( (SCL_OBD_ES_REP_ACTIVE_DIAG == (SCL_OBD_ES_REP_ACTIVE_DIAG & fault_report)) ||
       (SCL_OBD_ES_REP_ACTIVE_EVENT == (SCL_OBD_ES_REP_ACTIVE_EVENT & fault_report)) )
  {
     ret_val = true;
  }

  return ret_val;
}

bool AutoDiagEvent::inhibitActive()
{
  return active();
}
 
bool AutoDiagEvent::firstActivated()
{
  m_currStatus = active();
  bool firstActivated = (m_currStatus && !m_oldStatus); 
  m_oldStatus = m_currStatus;
  return firstActivated;      
}

bool AutoDiagEvent::changedActivated()
{
  m_currStatus = active();
  bool changedActivated = (m_currStatus == !m_oldStatus); 
  m_oldStatus = m_currStatus;
  return changedActivated;      
}

AutoDiagEvent::DiagEventType AutoDiagEvent::getType() const {
    return m_type;
}

scl_obd_test_handle_t AutoDiagEvent::getHandle() const {
    return m_handle;
}

std::string AutoDiagEvent::getId() const
{
  std::stringstream tempSs;
  if ( m_pLogConfig )
  {  
    tempSs << getCid() << "-" << static_cast<unsigned int>( getFmi() );
  }
  return tempSs.str();
}

std::string AutoDiagEvent::getDesc() const
{
  std::string tempStr;
  if ( m_pLogConfig )     
    tempStr = m_pLogConfig->eddt_config->ascii_text;
  return tempStr;
}

unsigned_16 AutoDiagEvent::getCid() const
{
  return m_pLogConfig->eddt_config->eid_or_cid;
}

unsigned_8 AutoDiagEvent::getFmi() const
{
  return m_pLogConfig->eddt_config->fmi;
}

unsigned_8 AutoDiagEvent::getWci() const
{
  return m_pLogConfig->eddt_config->wci;
}

std::string AutoDiagEvent::getConditionString() const
{
  return m_conditionString;
}

std::string AutoDiagEvent::getDiagName() const
{
  return m_diagName;
}

void AutoDiagEvent::setConditionString( const std::string& condStr )
{
  m_conditionString = condStr;
}

bool AutoDiagEvent::logEnabled() const
{
  return m_enableLog; 
}

void AutoDiagEvent::getDiagEventStatus( DiagnosticStatus::DiagEventStatus& ds ) const
{
  ds.m_type = m_type;
  ds.m_cid = getCid();
  ds.m_fmi = getFmi();
  ds.m_wci = getWci();
  ds.m_desc = getDesc();
  ds.m_active = active();
  if ( ds.m_active )
    ds.m_errorTxt = getConditionString();
  else
    ds.m_errorTxt = "";

  ds.m_isInhibited = m_isInhibited;
  ds.m_latchedUntilNewAssignment = m_waitingForNewAssignment;
  ds.m_rawStatus = m_status_db.getCurrentStatus();
}

bool AutoDiagEvent::isInhibited( const InhibitConditionList& inhibitList )
{
  for ( unsigned int i = 0; i < m_inhibitConditions.size(); ++i )
  {
    InhibitCondition* inhib = inhibitList.get( m_inhibitConditions.at( i ) ); 
    if ( inhib )
    {
      if ( inhib->inhibitActive() )
        return true;
    }
  }
  return false;
}

bool AutoDiagEvent::waitingForNewAssignment() const
{
  return m_waitingForNewAssignment;
}

