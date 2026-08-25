#ifndef _LpsSaNvmCalOnTheFlyDataChannelInterfaceTypes_h_
#define _LpsSaNvmCalOnTheFlyDataChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaNvmCalOnTheFlyDataChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaNvmCalOnTheFlyDataChannel> LpsSaNvmCalOnTheFlyDataChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaNvmCalOnTheFlyDataChannel> LpsSaNvmCalOnTheFlyDataChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaNvmCalOnTheFlyDataChannelInput>()
  {
    return "LpsSaNvmCalOnTheFlyDataChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaNvmCalOnTheFlyDataChannelOutput>()
  {
    return "LpsSaNvmCalOnTheFlyDataChannelOutput";
  }
}

#endif
