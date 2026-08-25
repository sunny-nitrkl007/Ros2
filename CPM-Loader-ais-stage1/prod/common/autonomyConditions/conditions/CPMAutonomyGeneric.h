///////////////////////////////////////////////////////////////////////////////
/// @file      CPMAutonomyGeneric
/// @author    pf
/// @date      10/4/2017
/// @brief     Autonomy Condition for CPMAutonomyGeneric
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef CPMAutonomyGeneric_h
#define CPMAutonomyGeneric_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class CPMAutonomyGeneric : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "CPMAutonomyGeneric";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  CPMAutonomyGeneric(const std::string& conditionType):
    AutonomyCondition(conditionType,
    		          conditionType,
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
