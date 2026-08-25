///////////////////////////////////////////////////////////////////////////////
/// @file      TimeSynchronizationFailed
/// @author    bode
/// @date      8/5/2012
/// @brief     Autonomy Condtion for TimeSynchronizationFailed
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef TimeSynchronizationFailed_h
#define TimeSynchronizationFailed_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class TimeSynchronizationFailed : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "TimeSynchronizationFailed";}
  static std::string getQualifiedString(){ return "Time synchronization failed on {0}"; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 10000;

  TimeSynchronizationFailed(const std::string& hostName, const std::string& taskName, const std::string& errStr):
    AutonomyCondition(getConditionTypeString(),
                      taskName + " [ " + hostName + " ]: " + errStr,
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {
    m_auxilaryParameters.push_back(hostName);
  }
};

#endif
