#ifndef _LpsSaJobMgrTxChannelInterfaceTypes_h_
#define _LpsSaJobMgrTxChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaJobMgrTxChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaJobMgrTxChannel> LpsSaJobMgrTxChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaJobMgrTxChannel> LpsSaJobMgrTxChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaJobMgrTxChannelInput>()
  {
    return "LpsSaJobMgrTxChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaJobMgrTxChannelOutput>()
  {
    return "LpsSaJobMgrTxChannelOutput";
  }
}

#endif
