///////////////////////////////////////////////////////////////////////////////
/// @file      DL_J1939_2_CID_2348_FMI_14
/// @author    pf
/// @date      10/10/2019
/// @brief     Autonomy Condition for DL_J1939_2_CID_2348_FMI_14
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef DL_J1939_2_CID_2348_FMI_14_h
#define DL_J1939_2_CID_2348_FMI_14_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class DL_J1939_2_CID_2348_FMI_14 : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "DL_J1939_2_CID_2348_FMI_14";}
  static const unsigned int activationDebounce_ms = 3000;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  DL_J1939_2_CID_2348_FMI_14(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "DL_J1939_2_CID_2348_FMI_14",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
