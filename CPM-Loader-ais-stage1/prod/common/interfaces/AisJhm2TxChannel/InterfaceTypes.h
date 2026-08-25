#ifndef _AisJhm2TxChannelInterfaceTypes_h_
#define _AisJhm2TxChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "AisJhm2TxChannel.h"

typedef interfaces::baseTypes::InputInterface<AisJhm2TxChannel> AisJhm2TxChannelInput;
typedef interfaces::baseTypes::OutputInterface<AisJhm2TxChannel> AisJhm2TxChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<AisJhm2TxChannelInput>()
  {
    return "AisJhm2TxChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<AisJhm2TxChannelOutput>()
  {
    return "AisJhm2TxChannelOutput";
  }
}

#endif
