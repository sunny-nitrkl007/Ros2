# Wire equivalent of LpsSaJobMgrDebugChannelStorage (published as
# LpsSaJobMgrDebugChannelOutput, live call site LpsSaJobMgrApp.cpp:463).
# Also snooped by aisXcpServer and hornOnStoreTest per the boundary diagram
# in References/jobmngr_new.txt -- both stay unmigrated, consume this via
# the Bridge.
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrDebugChannel/LpsSaJobMgrDebugChannel.h

uint32 current_state
uint32 tipoff_assist_activation_count
