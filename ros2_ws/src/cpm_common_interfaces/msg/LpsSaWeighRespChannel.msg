# Wire equivalent of LpsSaWeighRespChannelStorage (published by WeighApp as
# LpsSaWeighRespChannelOutput; consumed by JobMgr as
# LpsSaWeighRespChannelInput, live call site LpsSaJobMgrApp.cpp:278 via
# weighAppInf_.start()).
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaWeighRespChannel/LpsSaWeighRespChannel.h
#
# DIRECT DDS CHANNEL (Development-Plan.txt Step 6.1.2): WeighApp -> JobMgr,
# no Bridge involved.
#
# `timePoint` (real field this time -- LpsSaWeighRespChannel.h:20) IS live:
# LpsSaWeighAppInf.hpp:261 reads `response.timePoint` and assigns it to
# `nextTxTimePoint_`, compared against LpsSaWeighTxChannel's own timePoint
# to detect whether fresh Tx data reflects the latest request. Same
# int64-nanosecond wire representation as LpsSaJobMgrTxChannel.msg /
# LpsSaJobMgrRespChannel.msg -- not wall-clock time.
#
# `arg1` kept as a raw field (real struct exposes it only via a filePath()
# accessor, LpsSaWeighRespChannel.h:34-35 -- convenience wrapper, not
# separate wire data).

string app_name
uint32 app_request_id
int64 time_point_ns
WeighReqstChannelCommand command
bool success
string arg1
