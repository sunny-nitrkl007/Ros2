#ifndef _LpsSaJobMgrReqstChannelInterfaceTypes_h_
#define _LpsSaJobMgrReqstChannelInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "LpsSaJobMgrReqstChannel.h"

typedef interfaces::baseTypes::InputInterface<LpsSaJobMgrReqstChannel> LpsSaJobMgrReqstChannelInput;
typedef interfaces::baseTypes::OutputInterface<LpsSaJobMgrReqstChannel> LpsSaJobMgrReqstChannelOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<LpsSaJobMgrReqstChannelInput>()
  {
    return "LpsSaJobMgrReqstChannelInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<LpsSaJobMgrReqstChannelOutput>()
  {
    return "LpsSaJobMgrReqstChannelOutput";
  }
}

#endif
