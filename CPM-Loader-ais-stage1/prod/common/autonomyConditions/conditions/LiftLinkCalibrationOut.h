///////////////////////////////////////////////////////////////////////////////
/// @file      LiftLinkCalibrationOut
/// @author    pf
/// @date      11/14/2017
/// @brief     Autonomy Condition for LiftLinkCalibrationOut
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef LiftLinkCalibrationOut_h
#define LiftLinkCalibrationOut_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class LiftLinkCalibrationOut : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "LiftLinkCalibrationOut";}
  static const unsigned int activationDebounce_ms = 1000;
  static const unsigned int deactivationDebounce_ms = 200;
  static const unsigned int timeToLive_ms = 0;

  LiftLinkCalibrationOut(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "/* insert condition message here*/",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
