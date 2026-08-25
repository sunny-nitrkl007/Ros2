///////////////////////////////////////////////////////////////////////////////
/// @file      BEMSimDerate.h
/// @author    pf
/// @date      11/13/2017
/// @brief     Autonomy Condition for BEMSimDerate
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef BEMSimDerate_h
#define BEMSimDerate_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class BEMSimDerate : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "BEMSimDerate";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  BEMSimDerate(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
            "Battery Electric Simulator:Machine Derate:Recharge Now",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
