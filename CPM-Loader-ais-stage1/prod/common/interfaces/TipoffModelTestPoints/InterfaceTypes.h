#ifndef _TipoffModelTestPointsInterfaceTypes_h_
#define _TipoffModelTestPointsInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "TipoffModelTestPoints.h"

typedef interfaces::baseTypes::InputInterface<TipoffModelTestPoints> TipoffModelTestPointsInput;
typedef interfaces::baseTypes::OutputInterface<TipoffModelTestPoints> TipoffModelTestPointsOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<TipoffModelTestPointsInput>()
  {
    return "TipoffModelTestPointsInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<TipoffModelTestPointsOutput>()
  {
    return "TipoffModelTestPoints";
  }
}

#endif

