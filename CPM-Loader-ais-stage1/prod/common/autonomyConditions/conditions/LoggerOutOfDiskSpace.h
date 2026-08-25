///////////////////////////////////////////////////////////////////////////////
/// @file      LoggerOutOfDiskSpace
/// @author    bode
/// @date      8/2/2012
/// @brief     Autonomy Condtion for LoggerOutOfDiskSpace
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef LoggerOutOfDiskSpace_h
#define LoggerOutOfDiskSpace_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class LoggerOutOfDiskSpace : public AutonomyCondition
{
public:
  static std::string getConditionTypeString() {return "LoggerOutOfDiskSpace";}
  static std::string getQualifiedString(){ return "Logger is out of disk space."; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 1000;
  static const unsigned int timeToLive_ms = 10000;
  
  LoggerOutOfDiskSpace():
    AutonomyCondition(getConditionTypeString(),
                      getQualifiedString(),
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
