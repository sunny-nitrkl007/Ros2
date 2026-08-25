///////////////////////////////////////////////////////////////////////////////
/// @file      AutonomyConditionsList.h
/// @author    J Struble
/// @date      9/27/2011
/// @brief     Type to hold the total list of AutonomyConditions
///
/// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef AUTONOMYCONDITIONSLIST_H_
#define AUTONOMYCONDITIONSLIST_H_

#include "ais/interfaces/AutonomyConditionMessage/AutonomyConditionMessage.h"
#include <map>
#include <string>
#include <ais/log/Logger.h>

class AutonomyConditionsList
{
  public:
    AutonomyConditionsList() {}
    ~AutonomyConditionsList() {}
    
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Update the list with a new received AutonomyCondition
    ///
    /// @param newAutoCond (in) - The new AutonomyCondition message
    ///////////////////////////////////////////////////////////////////////////////      
    void update( const AutonomyConditionMessage& newAutoCondMessage )
    {
      m_condList[ newAutoCondMessage.getReporterInstanceName() ] = newAutoCondMessage.getActiveConditions();
    }
  
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Check if a condition is currently active
    ///
    /// @param instance (in) - Optional, if included will limit search to only that instance
    /// @param cond (in) - Autonomy condition to check
    /// @param condText (out) - Will add the autonomyCondition detailed text string if the 
    ///                         auto condition is active, otherwise will not modify
    /// @return true if the condition is active, false otherwise
    ///////////////////////////////////////////////////////////////////////////////        
    bool conditionIsActive( const std::string& instance, const std::string& condType, std::string& condText ) const
    {
      ConditionList::const_iterator it;
      it = m_condList.find( instance );
      if ( it == m_condList.end() )
        return false;

      for(std::vector<AutonomyConditionRecord>::const_iterator condIt = it->second.begin(); condIt != it->second.end(); ++condIt)
      {
        if( condIt->isSameCondition(condType) )
        {
          condText = condText + "\n" + condIt->getConditionRef().getConditionMessage();
          return true;
        }
      }

      return false;
    }

    bool conditionIsActive( const std::string& condType, std::string& condText ) const
    {
      ConditionList::const_iterator it;
      for ( it = m_condList.begin(); it != m_condList.end(); ++it )
      {
        for(std::vector<AutonomyConditionRecord>::const_iterator condIt = it->second.begin(); condIt != it->second.end(); ++condIt)
        {
          if( condIt->isSameCondition(condType) )
          {
            condText = condText + "\n" + condIt->getConditionRef().getConditionMessage();
            return true;
          }
        }
      }

      return false;
    }

    bool conditionIsActive( const std::string& condType ) const
    {
      std::string tempStr;
      return conditionIsActive( condType, tempStr );
    }

    std::vector<AutonomyConditionRecord> getActiveConditions() const
    {
      std::vector<AutonomyConditionRecord> retVal;
      ConditionList::const_iterator it;
      for ( it = m_condList.begin(); it != m_condList.end(); ++it )
      {
        for(std::vector<AutonomyConditionRecord>::const_iterator condIt = it->second.begin(); condIt != it->second.end(); ++condIt)
        {
          retVal.push_back( *condIt );
        }
      }
      return retVal;
    }

    std::vector<AutonomyConditionRecord> getActiveConditionsOfInterest( const std::vector<std::string>& condsOfInterest )
    {
      std::vector<AutonomyConditionRecord> retVal;
      ConditionList::const_iterator it;
      for ( it = m_condList.begin(); it != m_condList.end(); ++it )
      {
        for(std::vector<AutonomyConditionRecord>::const_iterator condIt = it->second.begin(); condIt != it->second.end(); ++condIt)
        {
          for ( unsigned int j = 0; j < condsOfInterest.size(); ++j )
          {
            if ( condIt->isSameCondition( condsOfInterest.at(j) ) )
            {
              retVal.push_back( *condIt );
              break;
            }
          }
        }
      }
      return retVal;
    }
  
  private:
    // instance name, 
    typedef std::map<std::string, std::vector<AutonomyConditionRecord> > ConditionList;
    ConditionList  m_condList;

};

#endif /*AUTONOMYCONDITIONSLIST_H_*/
