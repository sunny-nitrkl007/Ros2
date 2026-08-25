///////////////////////////////////////////////////////////////////////////////
/// @file      ExternalHDNotMounted
/// @author    bode
/// @date      8/2/2012
/// @brief     Autonomy Condtion for ExternalHDNotMounted
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef ExternalHDNotMounted_h
#define ExternalHDNotMounted_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class ExternalHDNotMounted : public AutonomyCondition
{
public:
  static std::string getConditionTypeString() {return "ExternalHDNotMounted";}
  static std::string getQualifiedString(){ return "Autonomous mass storage device is not accessible"; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  ExternalHDNotMounted():
    AutonomyCondition(getConditionTypeString(),
                      "The external hard drive is not mounted.",
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
