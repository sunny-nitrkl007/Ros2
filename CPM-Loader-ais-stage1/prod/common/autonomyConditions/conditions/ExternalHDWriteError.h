///////////////////////////////////////////////////////////////////////////////
/// @file      ExternalHDWriteError
/// @author    dtascion
/// @date      12/10/2012
/// @brief     Autonomy Condition for ExternalHDWriteError
///                                                                            
/// @attention Copyright (C) 2012                                              
/// @attention National Robotics Engineering Center                            
/// @attention Carnegie Mellon University                                      
/// @attention All rights reserved                                             
///////////////////////////////////////////////////////////////////////////////
#ifndef ExternalHDWriteError_h
#define ExternalHDWriteError_h

#include <ais/task/AutonomyCondition.h>
#include <string>

class ExternalHDWriteError : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "ExternalHDWriteError";}
  static std::string getQualifiedString(){ return "Autonomous mass storage device is not accessible"; }
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 2000;

  ExternalHDWriteError(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "An unspecified error occurred while flushing data to the disk. Check the human readable logger status message for more details.",
                      getQualifiedString(),
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
