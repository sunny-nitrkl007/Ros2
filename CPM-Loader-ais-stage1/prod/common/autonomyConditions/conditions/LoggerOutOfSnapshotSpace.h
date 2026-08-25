///////////////////////////////////////////////////////////////////////////////
/// @file      LoggerOutOfSnapshotSpace
/// @author    dtascion
/// @date      10/22/2012
/// @brief     Autonomy Condtion for LoggerOutOfSnapshotSpace
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef LoggerOutOfSnapshotSpace_h
#define LoggerOutOfSnapshotSpace_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class LoggerOutOfSnapshotSpace : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "LoggerOutOfSnapshotSpace";}
  static std::string getQualifiedString(){ return "Configured available snapshot space has been exhausted. No new snapshots will allowed to be generated."; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 1000;
  static const unsigned int timeToLive_ms = 0;

  LoggerOutOfSnapshotSpace(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      getQualifiedString(),
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
