///////////////////////////////////////////////////////////////////////////////
/// @file      LackOfGpsTime
/// @author    bode
/// @date      8/5/2012
/// @brief     Autonomy Condtion for LackOfGpsTime
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef LackOfGpsTime_h
#define LackOfGpsTime_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class LackOfGpsTime : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "LackOfGpsTime";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 10000;

  LackOfGpsTime(const std::string& taskName, const std::string& hostName):
    AutonomyCondition(getConditionTypeString(),
                      taskName + " [ " + hostName + " ] : not utilizing GPS Time",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
