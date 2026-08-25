///////////////////////////////////////////////////////////////////////////////
/// @file      PayLdOverLdLimitExceeded
/// @author    pf
/// @date      11/13/2017
/// @brief     Autonomy Condition for PayLdOverLdLimitExceeded
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef PayLdOverLdLimitExceeded_h
#define PayLdOverLdLimitExceeded_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class PayLdOverLdLimitExceeded : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "PayLdOverLdLimitExceeded";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  PayLdOverLdLimitExceeded(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "/* insert condition message here*/",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
