#ifndef _LpsSaJobMgrRespChannelInterfaceTypes_h_
#define _LpsSaJobMgrRespChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaJobMgrRespChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaJobMgrRespChannel> LpsSaJobMgrRespChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaJobMgrRespChannel> LpsSaJobMgrRespChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaJobMgrRespChannelInput>()
  {
    return "LpsSaJobMgrRespChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaJobMgrRespChannelOutput>()
  {
    return "LpsSaJobMgrRespChannelOutput";
  }
}

#endif
