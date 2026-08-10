======================================================================
Stage 1 minimal file set -- callback / real-blocking-wait variant
======================================================================
Same scope as CPM-Loader-ais-stage1/: legs 1-3 only (LpsSaWeighReqstChannel,
LpsSaWeighRespChannel, LpsSaWeighTxChannel -- the weighAppInf trio).

This is a TEST/COMPARISON folder, not a replacement for
CPM-Loader-ais-stage1/. Everything is inline, same structure as that
folder (rosNode_/executor_/weighAppInf_ as plain members, no separate
"channels" class -- that idea was tried and dropped). The one real
difference: weighAppInf_ is DDSWeighAppInfCb, backed by a real background
spin thread (spinThread_) instead of poll-once-per-tick, so
waitForResponse()/getLastResponse()/sendRequestGetResponse()/
sendRequestWaitForTxData() are real again, same as the original
LpsSaWeighAppInf.hpp -- see RosInputInterfaceCb.h and DDSWeighAppInfCb.hpp
for how.

----------------------------------------------------------------------
Files in this folder
----------------------------------------------------------------------
  apps/LpsSaJobMgrApp/LpsSaJobMgrApp.h/.cpp   -- weighAppInf_ typed
                                                  DDSWeighAppInfCb; adds
                                                  spinThread_ member,
                                                  started in initialize(),
                                                  joined in cleanup().
                                                  executive() no longer
                                                  calls spin_some().
  apps/LpsSaJobMgrApp/LpsSaJobMgrScs.cpp      -- unchanged from
                                                  CPM-Loader-ais-stage1/ --
                                                  weighAppInf_.waitForTxData()/
                                                  sendRequest() keep the
                                                  same signatures either way.

  apps/LpsSaWeighApp/*                        -- unchanged from
                                                  CPM-Loader-ais-stage1/ --
                                                  WeighApp has no
                                                  "weighAppInf" of its own,
                                                  so it isn't affected by
                                                  this variant at all.

  prod/common/ros2_wrapper/RosInputInterfaceCb.h
                                               -- callback-capable sibling
                                                  of RosInputInterface.h.
  prod/common/interfaces/LpsSaWeighReqstChannel/DDSWeighAppInfCb.hpp
                                               -- callback-capable sibling
                                                  of DDSWeighAppInf.hpp.
  prod/common/interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp
                                               -- unchanged, byte-identical
                                                  to the original, same
                                                  reasoning as
                                                  CPM-Loader-ais-stage1/.
  prod/common/ros2_wrapper/RosOutputInterface.h
                                               -- unchanged.

----------------------------------------------------------------------
Not build-tested
----------------------------------------------------------------------
Same as everywhere else in this project -- no real AIS SDK/rclcpp
available in this checkout to compile against.
