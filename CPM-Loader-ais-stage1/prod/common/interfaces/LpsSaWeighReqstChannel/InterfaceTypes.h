#ifndef _LpsSaWeighReqstChannelInterfaceTypes_h_
#define _LpsSaWeighReqstChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaWeighReqstChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaWeighReqstChannel> LpsSaWeighReqstChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaWeighReqstChannel> LpsSaWeighReqstChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighReqstChannelInput>()
  {
    return "LpsSaWeighReqstChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaWeighReqstChannelOutput>()
  {
    return "LpsSaWeighReqstChannelOutput";
  }
}

#endif
