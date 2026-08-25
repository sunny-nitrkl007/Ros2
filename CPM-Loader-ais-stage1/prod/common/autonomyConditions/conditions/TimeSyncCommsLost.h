///////////////////////////////////////////////////////////////////////////////
/// @file      TimeSyncCommsLost
/// @author    garrake
/// @date      9/24/2013
/// @brief     Autonomy Condition for TimeSyncCommsLost
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef TimeSyncCommsLost_h
#define TimeSyncCommsLost_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class TimeSyncCommsLost : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "TimeSyncCommsLost";}
  static std::string getQualifiedString(){ return "Machine is reporting communications loss with Time Sync"; }
  static const unsigned int activationDebounce_ms = 1000;
  static const unsigned int deactivationDebounce_ms = 5000;
  static const unsigned int timeToLive_ms = 0;

  TimeSyncCommsLost():
    AutonomyCondition(getConditionTypeString(),
                      "Loss of Comm with Time Sync",
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
