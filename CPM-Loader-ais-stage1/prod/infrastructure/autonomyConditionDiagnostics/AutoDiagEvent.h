///////////////////////////////////////////////////////////////////////////////
/// @file      AutoDiagEvent.h
/// @author    J Struble
/// @date      9/27/2011
/// @brief     Type to represent an autonomy layer diagnostic or event
///
/// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef AUTODIAGEVENT_H_
#define AUTODIAGEVENT_H_
//#include "events_common.h" /* for LogHandle_t, LogEntryConfig_t */ 

#include "scl_ci_es.h" /* for scl_ci_es_fault_cfg_eddt_t */

#include "AutonomyConditionsList.h"
#include <string>
#include <vector>
#include "ais/config/ConfigSection.h"
#include <std_types.h>
#include "ais/time/TimeStamp.h"
#include <ais/util/DebounceAssistant.h>
#include "InhibitConditionList.h"
#include "InhibitCondition.h"
#include "interfaces/DiagnosticStatus/DiagnosticStatus.h"

class AutoDiagEvent: public InhibitCondition
{
  public:

    enum DiagEventType
    {
      AUTO_DIAGNOSTIC = 0,
      AUTO_EVENT
    };

    AutoDiagEvent( const std::string& name, scl_ci_es_fault_cfg_eddt_t* logConfig, DiagEventType type, scl_obd_test_handle_t handle, bool externallyReported );
    ~AutoDiagEvent();

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Configure the diag/event
    ///
    /// @param cs (in) - The configSection containing the config values
    /// @return true if the configuration was successful
    ///////////////////////////////////////////////////////////////////////////////  
    bool configure( ConfigSection& cs );
   
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Update the status of this event using autonomy conditions
    ///
    /// @param autoConds (in) - The latest set of autonomy conditions and events
    ///////////////////////////////////////////////////////////////////////////////   
    void update( const AutonomyConditionsList& autoConds, const InhibitConditionList& inhibitList, const unsigned int &asmNum, const bool &inAutoMode,  const std::vector<AutoDiagEvent>& diagEventList, bool disable);

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Set the status of this event/diagnostic
    ///
    /// @param status (in) - The status to update to
    ///////////////////////////////////////////////////////////////////////////////   
    void setStatus( bool status );
    
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Is this diag/event currently active
    ///
    /// @return true if diag/event is currently active
    ///////////////////////////////////////////////////////////////////////////////     
    bool active() const;
    virtual bool inhibitActive();

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Did this diag/event just recently become active
    ///
    /// @return true if diag/event was previously not active and now is
    ///////////////////////////////////////////////////////////////////////////////   
    bool firstActivated();

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Did this diag/event just recently become active
    ///
    /// @return true if diag/event changes state   inactive->active     active->inactive
    ///////////////////////////////////////////////////////////////////////////////   
    bool changedActivated();

    DiagEventType getType() const;
    scl_obd_test_handle_t getHandle() const;
   
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get text string of the CID/FMI for this diag/event
    ///
    /// @return string containing cid-fmi
    ///////////////////////////////////////////////////////////////////////////////      
    std::string getId() const;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get the VIMS description for this diag/event
    ///
    /// @return string containing VIMS description
    ///////////////////////////////////////////////////////////////////////////////    
    std::string getDesc() const;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get the VIMS CID for this event/diag
    ///
    /// @return the CID
    ///////////////////////////////////////////////////////////////////////////////    
    unsigned_16 getCid() const;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get the VIMS FMI for this event/diag
    ///
    /// @return the FMI
    ///////////////////////////////////////////////////////////////////////////////    
    unsigned_8  getFmi() const;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get the VIMS WCI for this event
    ///
    /// @return the WCI
    ///////////////////////////////////////////////////////////////////////////////
    unsigned_8  getWci() const;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get the details of the autonomy condition that triggered this diag/event
    ///
    /// @return string containing the autonomyCondition details
    ///////////////////////////////////////////////////////////////////////////////    
    std::string getConditionString() const;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get the name of this diag/event
    ///
    /// @return string containing the name
    /////////////////////////////////////////////////////////////////////////////// 
    std::string getDiagName() const;
	
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Set the details of what triggered this event
    ///
    /// @param condStr(in) string containing the autonomyCondition details
    ///////////////////////////////////////////////////////////////////////////////
    void setConditionString( const std::string& condStr );

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Is this diag/event configured to be sent to the diagnostic logfile
    ///
    /// @return true if this is to be logged
    ///////////////////////////////////////////////////////////////////////////////    
    bool logEnabled() const;
    
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Is this diag/event active and currently waiting for a new assignment before clearing.
    ///
    /// @return true if it is currently latched and waiting for new assignment.
    ///////////////////////////////////////////////////////////////////////////////
    bool waitingForNewAssignment() const;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get the DiagnosticStatus for this diagnostic/event
    ///
    /// @param ds(out) Diagnostic Status that will be populated
    ///////////////////////////////////////////////////////////////////////////////
    void getDiagEventStatus( DiagnosticStatus::DiagEventStatus& ds ) const;

    friend class DiagOutFileWriter;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Return whether or not this diagnostic is currently being inhibited
    ///
    /// @return true if being inhibited
    ///////////////////////////////////////////////////////////////////////////////     
    bool isInhibited( const InhibitConditionList& inhibitList );

  private:

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief forces default values to be set up on the LogHandle_t object
    ///////////////////////////////////////////////////////////////////////////////
    void setDefaultsOnHandle();

    DiagEventType                 m_type;                   // Is this a Diagnostic or an event?
    std::vector<std::string>      m_triggerAutonomyConditions;  // Autonomy Condition(s) that triggers this event
    bool                          m_instanceNameIncl;       // Whether or not to consider instance name for this autonomyCondition
    std::string                   m_instanceName;           // The instance name to match (if included)
    bool                          m_latchUntilNewAssignment;// Whether or not to latch the diagnostic until a new assignment is received.
    bool                          m_waitingForNewAssignment;// Whether or not we are correctly waiting for a new assignment before clearing diag/event.
    std::string                   m_conditionString;        // String containing the details of this autonomyCondition
    unsigned_16                   m_handle;                 // SCL Info handle for this event/diagnostic
    scl_ci_es_fault_cfg_eddt_t*   m_pLogConfig;             // SCL Info config pointer for this event/diagnostic 
    DebounceAssistant             m_status_db;              // debounce structure
    bool                          m_currStatus;             // current active status (used for the firstActive trigger)
    bool                          m_oldStatus;              // old active status (used for the firstActive trigger)
    bool                          m_enableLog;              // Whether or not to output this event to diagnostic logfile when it triggers
    std::vector<std::string>      m_inhibitConditions;      // Conditions used to inhibit this diagnostic
    bool                          m_isInhibited;            // Whether or not this condition is currently inhibited
    std::string                   m_diagName;               // String containing the name of the diag/event
    bool                          m_externallyReported;     // true if reporting to event system is done externally
};  

#endif /*AUTODIAGEVENT_H_*/
