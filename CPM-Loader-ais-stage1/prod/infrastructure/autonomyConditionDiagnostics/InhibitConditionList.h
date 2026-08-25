///////////////////////////////////////////////////////////////////////////////
/// @file      InhibitConditionList.h
/// @author    J Struble
/// @date      10/13/2011
/// @brief     List of Inhibit Conditions
///
/// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef INHIBITCONDITIONLIST_H_
#define INHIBITCONDITIONLIST_H_

#include <vector>
#include <ais/config/TaskParser.h>
#include "InhibitCondition.h"
#include "AutonomyConditionsList.h"

class InhibitConditionList: public std::vector<InhibitCondition*>
{
  public:
    InhibitConditionList();
    ~InhibitConditionList();
  
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Configure the Inhibit
    ///
    /// @param tp (in) - The taskParser containing the config values
    /// @return true if the configuration was successful
    ///////////////////////////////////////////////////////////////////////////////    
    bool configure( TaskParser& tp, AutonomyConditionsList& condList );

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get an inhibitCondition from the list
    ///
    /// @param name (in) - The unique name of the inhibitCondition
    /// @return pointer to the inhibitCondition, or NULL if none exist with that name
    ///////////////////////////////////////////////////////////////////////////////      
    InhibitCondition* get( const std::string& name ) const;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Add an inhibitCondition to the list
    ///
    /// @param newCondition (in) - Pointer to the inhibitCondition to add
    /// @return true if the add was successfull, and there were no duplicates
    ///////////////////////////////////////////////////////////////////////////////      
    bool addCondition( InhibitCondition* newCondition );

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Are any startup inhibits active
    ///
    /// @param timeRemaining_sec(out) amount of time remaining on the longest inhibit
    /// @return true if any startup inhibit is active
    ///////////////////////////////////////////////////////////////////////////////      
    bool startupInhibitActive( double& timeRemaining_sec );
};

#endif /*INHIBITCONDITIONLIST_H_*/
