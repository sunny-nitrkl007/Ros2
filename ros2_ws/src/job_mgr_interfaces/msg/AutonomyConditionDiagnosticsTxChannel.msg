# Wire equivalent of AutonomyConditionDiagnosticsTxInterfaceStorage
# (consumed as AutonomyConditionDiagnosticsTxChannelInput, live call site
# LpsSaJobMgrApp.cpp:415 ->get()).
# Source: CPM-Loader-ais/prod/common/interfaces/AutonomyConditionDiagnostics/TxInterface.hpp
#
# `timePoint` IS live: AutonomyConditionDiagnostics/RequestHelper.hpp:270
# declares `AutonomyConditionDiagnosticsTxInterface txData_;` and compares
# its .timePoint the same way LpsSaJobMgrRequestHelper.hpp and
# LpsSaWeighAppInf.hpp do -- same int64-nanosecond representation.

int64 time_point_ns
bool display_ethernet_bad
bool product_link_ethernet_bad
SEA[] sea_list
