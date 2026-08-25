#ifndef _LpsSaWeighRespChannelInterfaceTypes_h_
#define _LpsSaWeighRespChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaWeighRespChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaWeighRespChannel> LpsSaWeighRespChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaWeighRespChannel> LpsSaWeighRespChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighRespChannelInput>()
  {
    return "LpsSaWeighRespChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighRespChannelOutput>()
  {
    return "LpsSaWeighRespChannelOutput";
  }
}

#endif
