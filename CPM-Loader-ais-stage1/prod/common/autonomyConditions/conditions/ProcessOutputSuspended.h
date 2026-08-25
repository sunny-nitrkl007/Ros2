///////////////////////////////////////////////////////////////////////////////
/// @file      ProcessOutputSuspended
/// @author    bode
/// @date      8/2/2012
/// @brief     Autonomy Condtion for ProcessOutputSuspended
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef ProcessOutputSuspended_h
#define ProcessOutputSuspended_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class ProcessOutputSuspended : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "ProcessOutputSuspended";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 10000;

  ProcessOutputSuspended(const std::string& taskName):
    AutonomyCondition(getConditionTypeString(),
                      "Process " + taskName + " has suspended its output due to the file exceeding the maximum threshold",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
