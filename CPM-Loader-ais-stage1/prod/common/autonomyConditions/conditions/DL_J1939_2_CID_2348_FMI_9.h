///////////////////////////////////////////////////////////////////////////////
/// @file      DL_J1939_2_CID_2348_FMI_9
/// @author    pf
/// @date      10/10/2019
/// @brief     Autonomy Condition for DL_J1939_2_CID_2348_FMI_9
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef DL_J1939_2_CID_2348_FMI_9_h
#define DL_J1939_2_CID_2348_FMI_9_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class DL_J1939_2_CID_2348_FMI_9 : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "DL_J1939_2_CID_2348_FMI_9";}
  static const unsigned int activationDebounce_ms = 3000;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  DL_J1939_2_CID_2348_FMI_9(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "DL_J1939_2_CID_2348_FMI_9",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
