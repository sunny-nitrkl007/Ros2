# Wire equivalent of LpsSaJobMgrRespChannelStorage (published as
# LpsSaJobMgrRespChannelOutput, live call site LpsSaJobMgrScs.cpp:630).
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrRespChannel/LpsSaJobMgrRespChannel.h
#
# `timePoint` dropped -- same reasoning as LpsSaJobMgrTxChannel.msg.
# `command` reuses LpsSaJobMgrReqstChannel::Command (LpsSaJobMgrRespChannel.h:21)
# -- same JobMgrReqstChannelCommand type as LpsSaJobMgrReqstChannel.msg, not
# duplicated locally. Now in cpm_common_interfaces (moved there since
# LpsSaJobMgrReqstChannel.msg itself moved -- WeighApp publishes to that
# channel directly).

string app_name
uint32 app_request_id
cpm_common_interfaces/JobMgrReqstChannelCommand command
bool success
