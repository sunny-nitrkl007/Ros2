#ifndef _DemoAppTxChannelInterfaceTypes_h_
#define _DemoAppTxChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "DemoAppTxChannel.h"

typedef interfaces::baseTypes::InputInterface<DemoAppTxChannel> DemoAppTxChannelInput;
typedef interfaces::baseTypes::OutputInterface<DemoAppTxChannel> DemoAppTxChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<DemoAppTxChannelInput>()
  {
    return "DemoAppTxChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<DemoAppTxChannelOutput>()
  {
    return "DemoAppTxChannelOutput";
  }
}

#endif
