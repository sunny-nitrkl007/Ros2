///////////////////////////////////////////////////////////////////////////////
/// @file      TimeSyncIntermittent
/// @author    garrake
/// @date      9/24/2013
/// @brief     Autonomy Condition for TimeSyncIntermittent
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef TimeSyncIntermittent_h
#define TimeSyncIntermittent_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class TimeSyncIntermittent : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "TimeSyncIntermittent";}
  static std::string getQualifiedString(){ return "Machine is reporting intermittent data from Time Sync.  Communication is erratic"; }
  static const unsigned int activationDebounce_ms = 3600000;
  static const unsigned int deactivationDebounce_ms = 5000;
  static const unsigned int timeToLive_ms = 0;

  TimeSyncIntermittent():
    AutonomyCondition(getConditionTypeString(),
                      "Intermittent data from Time Sync",
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
