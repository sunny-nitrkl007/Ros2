///////////////////////////////////////////////////////////////////////////////
/// @file      LoggerLowOnSnapshotSpace
/// @author    dtascion
/// @date      10/22/2012
/// @brief     Autonomy Condtion for LoggerLowOnSnapshotSpace
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef LoggerLowOnSnapshotSpace_h
#define LoggerLowOnSnapshotSpace_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class LoggerLowOnSnapshotSpace : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "LoggerLowOnSnapshotSpace";}
  static std::string getQualifiedString(){ return "Configured available snapshot space is nearly exhausted. No new snapshots will be allowed to be generated once this limit is exceeded."; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 1000;
  static const unsigned int timeToLive_ms = 0;

  LoggerLowOnSnapshotSpace(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      getQualifiedString(),
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
