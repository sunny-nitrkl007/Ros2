///////////////////////////////////////////////////////////////////////////////
/// @file      NeverRecvGPSTime
/// @author    bode
/// @date      8/5/2012
/// @brief     Autonomy Condtion for NeverRecvGPSTime
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef NeverRecvGPSTime_h
#define NeverRecvGPSTime_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class NeverRecvGPSTime : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "NeverRecvGPSTime";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 10000;

  NeverRecvGPSTime(const std::string& taskName, const std::string& hostName):
    AutonomyCondition(getConditionTypeString(),
                      taskName + " [ " + hostName + " ]: never received GPS time message.",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
