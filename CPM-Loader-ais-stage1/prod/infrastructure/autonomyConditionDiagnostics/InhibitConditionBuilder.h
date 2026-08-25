///////////////////////////////////////////////////////////////////////////////
/// @file      InhibitConditionBuilder.h
/// @author    J Struble
/// @date      10/13/2011
/// @brief     Builder class for inhibitConditions
///
/// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef INHIBITCONDITIONBUILDER_H_
#define INHIBITCONDITIONBUILDER_H_

#include "InhibitCondition.h"
#include "StartupInhibit.h"
#include "AutonomyConditionInhibit.h"
#include "AutonomyConditionsList.h"
#include <string>

class InhibitConditionBuilder
{
  public:
    enum InhibitType
    {
      STARTUP_INHIBIT = 0,        
      AUTONOMYCONDITION_INHIBIT,        
    };
  
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get a new instance of the inhibitCondition with the referenced type
    ///
    /// @param inhibitType (in) - The type of inhibitCondition to create
    /// @param name (in) - The name used to initialize the inhibitCondition
    /// @return pointer to the newly created inhibitCondition, or NULL if one could 
    ///         not be created or the inhibitType was invalid
    ///////////////////////////////////////////////////////////////////////////////    
    static InhibitCondition* get( unsigned int inhibitType, const std::string& name, AutonomyConditionsList& condList )
    {
      switch( inhibitType )
      {
        case STARTUP_INHIBIT: return new StartupInhibit( name );
        case AUTONOMYCONDITION_INHIBIT: return new AutonomyConditionInhibit( name, condList );
      }
      return NULL;      
    }
  
  private:
    InhibitConditionBuilder();  //Private to prevent use
    ~InhibitConditionBuilder(); //Private to prevent use
};

#endif /*INHIBITCONDITIONBUILDER_H_*/
