///////////////////////////////////////////////////////////////////////////////
/// @file      TiltHEVoltageAbove
/// @author    pf
/// @date      11/14/2017
/// @brief     Autonomy Condition for TiltHEVoltageAbove
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef TiltHEVoltageAbove_h
#define TiltHEVoltageAbove_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class TiltHEVoltageAbove : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "TiltHEVoltageAbove";}
  static const unsigned int activationDebounce_ms = 3000;
  static const unsigned int deactivationDebounce_ms = 200;
  static const unsigned int timeToLive_ms = 0;

  TiltHEVoltageAbove(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "/* insert condition message here*/",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
