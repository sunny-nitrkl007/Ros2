# Wire equivalent of LpsSaWeighReqstChannelStorage (published by JobMgr as
# LpsSaWeighReqstChannelOutput, live call site LpsSaJobMgrScs.cpp:1168 via
# weighAppInf_.sendRequest(); consumed by WeighApp as
# LpsSaWeighReqstChannelInput, live call site LpsSaScs.cpp:49 ->get()).
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaWeighReqstChannel/LpsSaWeighReqstChannel.h
#
# DIRECT DDS CHANNEL (Development-Plan.txt Step 6.1.1): JobMgr -> WeighApp,
# no Bridge involved. Lives in cpm_common_interfaces since both apps
# construct/consume this exact type directly.
#
# `timePoint`: this struct doesn't have one (unlike most other channels) --
# confirmed by reading the full header, not assumed.

string app_name
uint32 app_request_id

# Command (LpsSaWeighReqstChannel.h:23-60) -- promoted to WeighReqstChannelCommand
# since LpsSaWeighRespChannel.msg also needs it (reused via
# LpsSaWeighReqstChannel::Command in LpsSaWeighRespChannel.h:21).
WeighReqstChannelCommand command

# `arg` sub-struct (LpsSaWeighReqstChannel.h:67-74) -- flattened with an
# arg_ prefix, same reasoning as LpsSaJobMgrReqstChannel.msg's data_ prefix.
bool arg_b
float32 arg_f1
float32 arg_f2
string arg_s
uint32 arg_u
FloatPair[] arg_map
