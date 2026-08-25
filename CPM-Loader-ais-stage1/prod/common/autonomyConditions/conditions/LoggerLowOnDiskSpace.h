///////////////////////////////////////////////////////////////////////////////
/// @file      LoggerLowOnDiskSpace
/// @author    bode
/// @date      8/2/2012
/// @brief     Autonomy Condtion for LoggerLowOnDiskSpace
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef LoggerLowOnDiskSpace_h
#define LoggerLowOnDiskSpace_h

#include <ais/task/AutonomyCondition.h>
#include <string>
#include "ais/util/stringUtils.h"

class LoggerLowOnDiskSpace : public AutonomyCondition
{
public:
  static std::string getConditionTypeString() {return "LoggerLowOnDiskSpace";}
  static std::string getQualifiedString(){ return "Machine logger has low disk space.  {0}% remaining"; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 1000;
  static const unsigned int timeToLive_ms = 10000;

  LoggerLowOnDiskSpace(double percentRemaining):
    AutonomyCondition(getConditionTypeString(),
                      "Logging disk is nearing capacity, " + 
                      stringUtils::typeToString(percentRemaining) + 
                      " %% remaining.",
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {
    m_auxilaryParameters.push_back(stringUtils::typeToString(percentRemaining,1));
  }
};

#endif
