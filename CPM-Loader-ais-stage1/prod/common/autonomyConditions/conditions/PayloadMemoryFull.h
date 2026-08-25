#ifndef PayloadMemoryFull_h
#define PayloadMemoryFull_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class PayloadMemoryFull : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "PayloadMemoryFull";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  PayloadMemoryFull(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "Payload:Memory Full",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
