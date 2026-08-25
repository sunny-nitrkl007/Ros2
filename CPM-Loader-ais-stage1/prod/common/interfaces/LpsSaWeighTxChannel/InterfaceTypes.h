#ifndef _LpsSaWeighTxChannelInterfaceTypes_h_
#define _LpsSaWeighTxChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaWeighTxChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaWeighTxChannel> LpsSaWeighTxChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaWeighTxChannel> LpsSaWeighTxChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighTxChannelInput>()
  {
    return "LpsSaWeighTxChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighTxChannelOutput>()
  {
    return "LpsSaWeighTxChannelOutput";
  }
}

#endif
