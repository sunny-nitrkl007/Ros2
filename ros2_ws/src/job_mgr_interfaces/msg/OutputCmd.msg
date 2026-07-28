# PARTIAL RECONSTRUCTION -- OutputChannel::OutputCmd. Real struct (folder
# interfaces/OutputChannel/, confirmed via LpsSaJobMgrApp.h:37) does not
# exist in this checkout. Reconstructed from the one real construction site
# that exists anywhere in the tree, LpsSaJobMgrScs.cpp:922-927:
#   OutputChannel::OutputCmd command;
#   command.OutputPort = HORN_PORT_NUMBER;              // = OutputChannel::Port::OUTPUT_SINK_3
#   command.InitialState = OutputChannel::State::PORT_ON;
#   command.StateChangeDuration = OutputChannel::ChangeDuration::NO_FLASH;
#   command.TotalDuration = 2;
#   command.FinalState = OutputChannel::State::PORT_OFF;
#
# Only this ONE field combination is ever constructed anywhere in the tree --
# no other Port/State/ChangeDuration enumerator names appear anywhere. So
# only PORT_ON/PORT_OFF (State), NO_FLASH (ChangeDuration), and
# OUTPUT_SINK_3 (Port) are known to exist at all -- the real enums almost
# certainly have more values (e.g. a flashing ChangeDuration, other ports),
# but there's zero usage evidence for them, so none are invented here.
# All three carried as raw uint8 -- real numeric values unknown, not even
# a single literal integer visible for any of them (unlike WeighBktWtAccuracy,
# no indirect mirror evidence found either).

uint8 output_port      # OutputChannel::Port -- only OUTPUT_SINK_3 seen, value unknown
uint8 initial_state     # OutputChannel::State -- PORT_ON/PORT_OFF seen, values unknown
uint8 state_change_duration  # OutputChannel::ChangeDuration -- only NO_FLASH seen, value unknown
uint8 final_state       # OutputChannel::State
int32 total_duration    # plain int in source (command.TotalDuration = 2) -- likely seconds, unconfirmed unit
