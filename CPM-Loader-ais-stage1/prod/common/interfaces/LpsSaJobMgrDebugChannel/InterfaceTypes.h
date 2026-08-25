#ifndef _LpsSaJobMgrDebugChannelInterfaceTypes_h_
#define _LpsSaJobMgrDebugChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaJobMgrDebugChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaJobMgrDebugChannel> LpsSaJobMgrDebugChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaJobMgrDebugChannel> LpsSaJobMgrDebugChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaJobMgrDebugChannelInput>()
  {
    return "LpsSaJobMgrDebugChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaJobMgrDebugChannelOutput>()
  {
    return "LpsSaJobMgrDebugChannelOutput";
  }
}

#endif
