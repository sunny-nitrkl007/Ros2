#ifndef PayloadMemoryLow_h
#define PayloadMemoryLow_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class PayloadMemoryLow : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "PayloadMemoryLow";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  PayloadMemoryLow(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "Payload:Memory Low",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
