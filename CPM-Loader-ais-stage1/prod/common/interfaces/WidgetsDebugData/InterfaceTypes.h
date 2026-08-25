#ifndef _WidgetsDebugDataInterfaceTypes_h_
#define _WidgetsDebugDataInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "WidgetsDebugData.h"

typedef interfaces::baseTypes::InputInterface<WidgetsDebugData> WidgetsDebugDataInput;
typedef interfaces::baseTypes::OutputInterface<WidgetsDebugData> WidgetsDebugDataOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<WidgetsDebugDataInput>()
  {
    return "WidgetsDebugDataInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<WidgetsDebugDataOutput>()
  {
    return "WidgetsDebugDataOutput";
  }
}

#endif
