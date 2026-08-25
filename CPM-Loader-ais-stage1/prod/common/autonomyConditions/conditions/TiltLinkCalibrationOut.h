///////////////////////////////////////////////////////////////////////////////
/// @file      TiltLinkCalibrationOut
/// @author    pf
/// @date      10/4/2017
/// @brief     Autonomy Condition for TiltLinkCalibrationOut
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef TiltLinkCalibrationOut_h
#define TiltLinkCalibrationOut_h
#include <ais/task/AutonomyCondition.h>
#include <string>

class TiltLinkCalibrationOut : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "TiltLinkCalibrationOut";}
  static const unsigned int activationDebounce_ms = 1000;
  static const unsigned int deactivationDebounce_ms = 200;
  static const unsigned int timeToLive_ms = 0;

  TiltLinkCalibrationOut(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "Tilt Linkage Position Sensor: Out of Calibration",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
