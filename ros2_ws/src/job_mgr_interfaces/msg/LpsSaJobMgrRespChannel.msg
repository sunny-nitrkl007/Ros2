# Wire equivalent of LpsSaJobMgrRespChannelStorage (published as
# LpsSaJobMgrRespChannelOutput, live call site LpsSaJobMgrScs.cpp:630).
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrRespChannel/LpsSaJobMgrRespChannel.h
#
# CORRECTION: `timePoint` is NOT dead -- see LpsSaJobMgrTxChannel.msg's header
# for the full correction. LpsSaJobMgrRequestHelper.hpp:239,246 reads
# `response.timePoint` (this struct) and assigns it to `nextTxTimePoint_`,
# which is then compared against LpsSaJobMgrTxChannel's timePoint to detect
# freshness -- both fields are part of the same live correlation mechanism.
# Same wire representation as LpsSaJobMgrTxChannel.msg: int64 nanosecond
# count from std::chrono::steady_clock, not wall-clock time.
#
# `command` reuses LpsSaJobMgrReqstChannel::Command (LpsSaJobMgrRespChannel.h:21)
# -- same JobMgrReqstChannelCommand type as LpsSaJobMgrReqstChannel.msg, not
# duplicated locally. Now in cpm_common_interfaces (moved there since
# LpsSaJobMgrReqstChannel.msg itself moved -- WeighApp publishes to that
# channel directly).

string app_name
uint32 app_request_id
int64 time_point_ns
cpm_common_interfaces/JobMgrReqstChannelCommand command
bool success
