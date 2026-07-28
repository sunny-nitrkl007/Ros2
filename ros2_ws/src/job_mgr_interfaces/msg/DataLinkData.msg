# PARTIAL RECONSTRUCTION -- DataLinkDataInput (live call site
# LpsSaJobMgrScs.cpp:771 ->get()). Real struct not in this checkout, see
# DataLinkParam.msg for the full evidence trail and scoping rationale.
#
# `dlData.GetParams()` (LpsSaJobMgrScs.cpp:772) is iterated with a
# range-for -- confirms a real collection, modeled as an array on that basis.

DataLinkParam[] params
