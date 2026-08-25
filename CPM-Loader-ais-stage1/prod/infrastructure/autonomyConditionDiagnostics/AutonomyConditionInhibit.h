///////////////////////////////////////////////////////////////////////////////
/// @file      AutonomyConditionInhibit.h
/// @author    J Struble
/// @date      1/19/2012
/// @brief     InhibitCondition to implement a inhibit based on an active autonomyCondition
///
/// @attention COPYRIGHT (C) 2012 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////

#ifndef AUTONOMYCONDITIONINHIBIT_H_
#define AUTONOMYCONDITIONINHIBIT_H_

#include "InhibitCondition.h"
#include <ais/interfaces/AutonomyConditionMessage/AutonomyConditionMessage.h>
#include "AutonomyConditionsList.h"
#include <ais/log/Logger.h>

class AutonomyConditionInhibit: public InhibitCondition
{
  public:
    AutonomyConditionInhibit( const std::string& name, AutonomyConditionsList& condList ):
      InhibitCondition( name ),
      m_rAutonomyConditionsList( condList )
    {
    }
    virtual ~AutonomyConditionInhibit() {}
  
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Configure the Inhibit
    ///
    /// @param cs (in) - The configSection containing the config values
    /// @return true if the configuration was successful
    ///////////////////////////////////////////////////////////////////////////////  
    virtual bool configure( ConfigSection& cs )
    {
      std::vector<std::string> configuredAutoConds;
      if ( !cs.getArray( "AutonomyConditions", configuredAutoConds ) )
      {
        AIS_LOG_FATAL( "AutonomyConditions configuration required for %s inhibit", m_name.c_str() );
        return false;                 
      }
      
      if ( configuredAutoConds.size() == 0 )
      {
        AIS_LOG_FATAL( "AutonomyConditions config key contains no elements, which are required for %s inhibit", m_name.c_str() );
        return false;                 
      }      

      for ( unsigned int i=0; i < configuredAutoConds.size(); ++i )
        m_configuredAutoConds.insert( configuredAutoConds.at(i) ) ;
        
      return true;            
    }
  
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Return whether the inhibit condition is active
    ///
    /// @return true if the inhibit is active
    ///////////////////////////////////////////////////////////////////////////////  
    virtual bool inhibitActive()
    {
      bool inhibitActive = false;
      std::set<std::string>::const_iterator it;
      for ( it = m_configuredAutoConds.begin(); it != m_configuredAutoConds.end(); ++it )
        inhibitActive |= m_rAutonomyConditionsList.conditionIsActive( *it );
            
      return inhibitActive;            
    }
  
  private:
    AutonomyConditionsList& m_rAutonomyConditionsList;
    std::set<std::string> m_configuredAutoConds;
};

#endif /*AUTONOMYCONDITIONINHIBIT_H_*/
