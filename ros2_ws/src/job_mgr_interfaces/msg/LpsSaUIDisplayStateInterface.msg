# Wire equivalent of LpsSaUIDisplayStateInterfaceStorage -- the ACTUAL type
# bound/gotten via DisplayStateInput (LpsSaJobMgrScs.cpp:150-161 ->get()),
# not LpsSaUIDisplayState directly.
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaUI/DisplayStateInterface.hpp
#
# CORRECTION (Step 4.1 mistake, found while converting the call site in
# Step 4.2): the shim's member declaration in LpsSaJobMgrApp.h originally
# used job_mgr_interfaces::msg::LpsSaUIDisplayState directly, skipping this
# wrapper. DisplayStateInterface.hpp:12-27 shows the real storage class is
# just `{ LpsSaUIDisplayState state; }`, with serialize() only ever
# archiving `state` -- a thin wrapper, but a real one, and the one actually
# named by the channel ("LpsSaUIDisplayStateInterface", not
# "LpsSaUIDisplayState"). Fixed by adding this wrapper message and
# correcting the member type in LpsSaJobMgrApp.h to use it.

LpsSaUIDisplayState state
