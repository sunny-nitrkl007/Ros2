#ifndef _MachineInterfaceTypes_h_
#define _MachineInterfaceTypes_h_

#include <ais/interfaces/baseTypes/InputInterface.h>
#include <ais/interfaces/baseTypes/OutputInterface.h>
#include "Machine.h"

typedef interfaces::baseTypes::InputInterface<Machine> MachineInput;
typedef interfaces::baseTypes::OutputInterface<Machine> MachineOutput;

namespace interfaces
{
  template<>
  inline std::string getDefaultInterfaceName<MachineInput>()
  {
    return "MachineInput";
  }

  template<>
  inline std::string getDefaultInterfaceName<MachineOutput>()
  {
    return "MachineOutput";
  }
}

#endif
