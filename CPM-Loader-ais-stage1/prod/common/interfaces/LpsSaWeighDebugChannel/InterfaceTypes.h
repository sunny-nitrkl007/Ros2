#ifndef _LpsSaWeighDebugChannelInterfaceTypes_h_
#define _LpsSaWeighDebugChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaWeighDebugChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaWeighDebugChannel> LpsSaWeighDebugChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaWeighDebugChannel> LpsSaWeighDebugChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighDebugChannelInput>()
  {
    return "LpsSaWeighDebugChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighDebugChannelOutput>()
  {
    return "LpsSaWeighDebugChannelOutput";
  }
}

#endif
