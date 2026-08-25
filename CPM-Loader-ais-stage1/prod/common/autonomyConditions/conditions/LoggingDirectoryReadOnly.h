///////////////////////////////////////////////////////////////////////////////
/// @file      LoggingDirectoryReadOnly
/// @author    bode
/// @date      8/2/2012
/// @brief     Autonomy Condtion for LoggingDirectoryReadOnly
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef LoggingDirectoryReadOnly_h
#define LoggingDirectoryReadOnly_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class LoggingDirectoryReadOnly : public AutonomyCondition
{
public:  
  static std::string getConditionTypeString() {return "LoggingDirectoryReadOnly";}
  static std::string getQualifiedString(){ return "Autonomous mass storage device is not accessible"; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  LoggingDirectoryReadOnly():
    AutonomyCondition(getConditionTypeString(),
                      "Unable to write to logging directory (read-only) for Autonomy logging.",
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
