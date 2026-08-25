///////////////////////////////////////////////////////////////////////////////
/// @file      TiltLinkVoltageBelow
/// @author    pf
/// @date      10/4/2017
/// @brief     Autonomy Condition for TiltLinkVoltageBelow
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef TiltLinkVoltageBelow_h
#define TiltLinkVoltageBelow_h
#include <ais/task/AutonomyCondition.h>
#include <string>

class TiltLinkVoltageBelow : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "TiltLinkVoltageBelow";}
  static const unsigned int activationDebounce_ms = 3000;
  static const unsigned int deactivationDebounce_ms = 200;
  static const unsigned int timeToLive_ms = 0;

  TiltLinkVoltageBelow(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "Tilt Linkage Position Sensor: Voltage Below Normal",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
