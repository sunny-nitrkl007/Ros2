#ifndef PayloadLegalForTradeNotSealed_h
#define PayloadLegalForTradeNotSealed_h

#include <string>
#include <ais/task/AutonomyCondition.h>

class PayloadLegalForTradeNotSealed : public AutonomyCondition
{
public:

  static std::string getConditionTypeString() {return "PayloadLegalForTradeNotSealed";}
  static const unsigned int activationDebounce_ms = 0;
  static const unsigned int deactivationDebounce_ms = 0;
  static const unsigned int timeToLive_ms = 0;

  PayloadLegalForTradeNotSealed(/*insert any needed arguments for message string*/):
    AutonomyCondition(getConditionTypeString(),
                      "Payload Legal For Trade:Not Sealed",
                      activationDebounce_ms,
                      deactivationDebounce_ms,
                      timeToLive_ms)
  {}
};

#endif
