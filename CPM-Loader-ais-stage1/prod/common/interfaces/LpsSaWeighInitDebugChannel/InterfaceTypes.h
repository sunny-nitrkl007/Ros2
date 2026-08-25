#ifndef _LpsSaWeighInitDebugChannelInterfaceTypes_h_
#define _LpsSaWeighInitDebugChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaWeighInitDebugChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaWeighInitDebugChannel> LpsSaWeighInitDebugChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaWeighInitDebugChannel> LpsSaWeighInitDebugChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighInitDebugChannelInput>()
  {
    return "LpsSaWeighInitDebugChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighInitDebugChannelOutput>()
  {
    return "LpsSaWeighInitDebugChannelOutput";
  }
}

#endif
