#ifndef _DemoAppReqstChannelInterfaceTypes_h_
#define _DemoAppReqstChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "DemoAppReqstChannel.h"

typedef interfaces::baseTypes::InputInterface<DemoAppReqstChannel> DemoAppReqstChannelInput;
typedef interfaces::baseTypes::OutputInterface<DemoAppReqstChannel> DemoAppReqstChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<DemoAppReqstChannelInput>()
  {
    return "DemoAppReqstChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<DemoAppReqstChannelOutput>()
  {
    return "DemoAppReqstChannelOutput";
  }
}

#endif
