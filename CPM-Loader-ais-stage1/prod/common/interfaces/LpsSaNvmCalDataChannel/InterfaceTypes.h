#ifndef _LpsSaNvmCalDataChannelInterfaceTypes_h_
#define _LpsSaNvmCalDataChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaNvmCalDataChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaNvmCalDataChannel> LpsSaNvmCalDataChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaNvmCalDataChannel> LpsSaNvmCalDataChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaNvmCalDataChannelInput>()
  {
    return "LpsSaNvmCalDataChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaNvmCalDataChannelOutput>()
  {
    return "LpsSaNvmCalDataChannelOutput";
  }
}

#endif
