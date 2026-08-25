///////////////////////////////////////////////////////////////////////////////
/// @file      LoggerLowOnCache
/// @author    dtascion
/// @date      12/13/2012
/// @brief     Autonomy Condition for LoggerLowOnCache
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef LoggerLowOnCache_h
#define LoggerLowOnCache_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class LoggerLowOnCache : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "LoggerLowOnCache";}
  static std::string getQualifiedString(){ return "Autonomous mass storage device is not accessible"; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 5000;

  LoggerLowOnCache(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "The Production Logger is running low on cache. Data will soon be dropped from the data logs.",
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
