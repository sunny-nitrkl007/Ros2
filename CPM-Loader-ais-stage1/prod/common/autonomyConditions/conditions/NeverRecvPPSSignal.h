///////////////////////////////////////////////////////////////////////////////
/// @file      NeverRecvPPSSignal
/// @author    bode
/// @date      8/5/2012
/// @brief     Autonomy Condtion for NeverRecvPPSSignal
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef NeverRecvPPSSignal_h
#define NeverRecvPPSSignal_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class NeverRecvPPSSignal : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "NeverRecvPPSSignal";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 10000;

  NeverRecvPPSSignal(const std::string& taskName, const std::string& hostName):
    AutonomyCondition(getConditionTypeString(),
                      taskName + " [ " + hostName + " ]: never received PPS signal.",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
