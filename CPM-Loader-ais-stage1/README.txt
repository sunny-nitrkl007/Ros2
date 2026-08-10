======================================================================
Stage 1 minimal file set -- legs 1-3 only
======================================================================
Scope: LpsSaWeighReqstChannel, LpsSaWeighRespChannel, LpsSaWeighTxChannel
(the weighAppInf_ trio). NOT legs 4/5 -- those are tangled with unrelated
channels (SwitchInputScs, displayStateInput_) inside shared functions
and were left out of this narrower cut.

Every file here is either:
  (a) copied byte-for-byte from the original pre-conversion baseline
      (commit cad392b) with zero edits, or
  (b) that same baseline with ONLY the confirmed leg-1/2/3-relevant
      hunks spliced in from the fully-converted version -- every other
      channel in these files is untouched, original raw SCS.

How to use: copy each file below over the matching path in your real
site-2 checkout.

  apps/LpsSaJobMgrApp/LpsSaJobMgrApp.h     -- rosNode_/executor_ added;
                                               no channel member retyped
                                               (weighAppInf_ was already
                                               unchanged in the header)
  apps/LpsSaJobMgrApp/LpsSaJobMgrApp.cpp   -- weighAppInf_ construction
                                               block converted to DDS +
                                               rosNode_/executor_ setup +
                                               executor_.spin_some() in
                                               executive()
  apps/LpsSaJobMgrApp/LpsSaJobMgrScs.cpp   -- CORRECTED (was wrongly
                                               claimed byte-identical to
                                               baseline). weighAppInf_'s
                                               method NAMES didn't change
                                               (sendRequest/waitForTxData),
                                               but their PARAMETER TYPES
                                               did (raw SCS struct -> ROS2
                                               .msg type) -- that part was
                                               missed originally. 2 real
                                               edits needed, both now
                                               applied: (1)
                                               LpsSaWeighScsTxParamRead()'s
                                               local `rxParam` was typed as
                                               the old raw LpsSaWeighTxChannel
                                               (wrong -- wouldn't compile
                                               against
                                               DDSWeighAppInf::waitForTxData(),
                                               and its ~10 CamelCase field
                                               reads afterward were stale
                                               too) -- retyped to
                                               cpm_common_interfaces::msg::
                                               LpsSaWeighTxChannel with all
                                               field accesses converted to
                                               snake_case (dig_stat,
                                               best_bkt_wt_in_tonnes, etc.,
                                               matching the already-working
                                               full CPM-Loader-ais/ copy);
                                               (2) LpsSaJobMgrScsSendCmd()'s
                                               local `request` was typed as
                                               the old raw
                                               LpsSaWeighReqstChannel (same
                                               problem) -- retyped to
                                               cpm_common_interfaces::msg::
                                               LpsSaWeighReqstChannel,
                                               `request.command = command`
                                               -> `request.command.value =
                                               static_cast<uint8_t>(command)`.
                                               The function's own PARAMETER
                                               type (LpsSaWeighReqstChannel::
                                               Command) correctly stays
                                               unchanged, since
                                               LpsSaJobMgrProcess.cpp
                                               (excluded from Stage 1, real
                                               baseline) calls it with that
                                               real enum type -- confirmed
                                               against the reference tree.

  apps/LpsSaWeighApp/LpsSaWeighApp.h       -- 3 member types converted
                                               (ReqstIn/RespOut/TxOut) +
                                               rosNode_/executor_ added
  apps/LpsSaWeighApp/LpsSaWeighApp.cpp     -- 3 construction lines
                                               converted + rosNode_/
                                               executor_ setup +
                                               executor_.spin_some() in
                                               executive()
  apps/LpsSaWeighApp/LpsSaScs.cpp          -- only LpsSaScsChkForReqst(),
                                               LpsSaScsSendReqstResponse()
                                               (all overloads), and
                                               LpsSaWeighingScsTx()
                                               converted; every other
                                               function in this file is
                                               original baseline

  prod/common/ros2_wrapper/RosInputInterface.h   -- new, as-is
  prod/common/ros2_wrapper/RosOutputInterface.h  -- new, as-is
  prod/common/interfaces/LpsSaWeighReqstChannel/DDSWeighAppInf.hpp
                                                  -- new file, this IS
                                                     legs 1-3
  prod/common/interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp
                                                  -- NOT changed, byte-
                                                     identical to the
                                                     original. Left
                                                     alone deliberately:
                                                     it's shared common/
                                                     interfaces code, and
                                                     AisJhm2RequestProcessor
                                                     (legacy UI infra,
                                                     still SCS-only)
                                                     depends on its
                                                     original form.
                                                     weighAppInf_ uses
                                                     DDSWeighAppInf
                                                     instead -- see that
                                                     file's own comment.

NOT included: LpsSaJobMgrProcess.cpp, adv/TipoffAssist.* (neither touches
legs 1-3), and every other channel's conversion in LpsSaJobMgrApp.h/.cpp/
LpsSaWeighApp.h/.cpp/LpsSaScs.cpp -- those stay on their original raw SCS
types in the files above.

NOT included, but DOES touch legs 1-3 -- apps/LpsSaWeighApp/LpsSaProcess.cpp:
calls LpsSaScsSendReqstResponse(request_.command, success) (the 2-arg,
Command-typed overload in LpsSaScs.cpp) 5 times, for
RESET_BEST_BUCKET_WEIGHT/ZERO/CAPTURE_CYLINDER_EXTENSION_REFERENCE/
CLEAR_REWEIGH_WARNING -- responses for these 4 commands are deliberately
deferred until the actual physical operation completes in this file's
weigh-library update cycle, rather than answered immediately in
LpsSaScsChkForReqst(). This is why LpsSaScs.cpp has what looks like
duplicate near-identical overloads of LpsSaScsSendReqstResponse (one
taking the old LpsSaWeighReqstChannelStorage&, one taking the new
cpm_common_interfaces::msg::LpsSaWeighReqstChannel&) -- they are NOT
redundant, each has a real, different caller. LpsSaProcess.cpp needs ZERO
code changes for this Stage 1 cut: it only touches request_ (kept on its
real old type) and the Command-typed overload (signature unchanged) --
both were correctly left alone by the conversion. Do NOT "simplify" or
remove either LpsSaScsSendReqstResponse overload, or the 2-arg
Command-typed one, or request_ itself -- doing so would compile fine
inside this narrow file set but silently break LpsSaProcess.cpp the
moment this tree is copied back over the real repo.

Verified: brace-balance sanity check across all 9 files, and confirmed
by grep that only cpm_common_interfaces::msg::{LpsSaWeighReqstChannel,
LpsSaWeighRespChannel, LpsSaWeighTxChannel, WeighReqstChannelCommand,
ProdMeasureSensorStatus, WeighPidData} (the last two are nested fields
of LpsSaWeighTxChannel itself, not scope creep) appear anywhere in the
WeighApp files -- no other channel's converted type leaked in.

Also required, not part of this file set (already covered separately):
  - colcon build of cpm_common_interfaces ONLY. Verified via grep across
    every real file in this folder: nothing here references
    job_mgr_interfaces:: or weigh_app_interfaces:: at all -- the only
    "job_mgr" string anywhere is the rclcpp::Node NAME "job_mgr_node"
    in LpsSaJobMgrApp.cpp, unrelated to the interfaces package.
    (CORRECTION: this used to also list job_mgr_interfaces as required
    -- that was wrong for this narrow Stage 1 cut; job_mgr_interfaces
    is only needed once legs 4-5 or other job_mgr_interfaces-typed
    channels are converted, which Stage 1 doesn't touch.)
  - SConscript wiring (see Stage1-Real-Build-SConscript-Steps.txt)

CORRECTION 1 (found by re-reading this file against DDSWeighAppInf.hpp's
actual method signatures, not just checking method names existed):
LpsSaJobMgrScs.cpp was NOT actually byte-identical/zero-edit as originally
claimed above -- 2 real type-mismatch bugs existed (would not have
compiled) and are now fixed. See the LpsSaJobMgrScs.cpp entry above for
detail. Lesson: "the method name didn't change" is not the same
verification as "the call site still type-checks" -- a method can keep
its name while its parameter type moves from the old SCS struct to the
new ROS2 .msg type, and only checking the WeighApp side's field-name grep
(see "Verified" above) never exercised the JobMgr side at all.

CORRECTION 2 (the LpsSaProcess.cpp / "is this duplication?" finding --
see the "NOT included, but DOES touch legs 1-3" note above for the full
story of why the 2 response-building overloads both need to exist):
apps/LpsSaWeighApp/LpsSaWeighApp.h was ALSO missing the declaration for
the 3rd LpsSaScsSendReqstResponse overload entirely (the one taking
cpm_common_interfaces::msg::LpsSaWeighReqstChannel&) -- it was DEFINED in
LpsSaScs.cpp but never declared in the header. Every call site in
LpsSaScsChkForReqst() (which passes the ROS2 .msg-typed request) would
have failed to compile with "no matching function" -- confirmed by
comparing against the full working copy's LpsSaWeighApp.h, which has all
3 declared. Fixed in both stage1 folders. Separately, the 2 overloads
that build a response (old-type and new-type) had ~7 lines of verbatim
duplicated boilerplate (timestamp, publish, log, return) -- genuinely
reducible, unlike the overloads' own existence (both still needed).
Factored into a new private helper, publishWeighResponse(), called by
both. Neither overload's own signature changed, so LpsSaProcess.cpp's
real call to the 2-arg Command-typed overload is unaffected.

UPDATE: the old-type response-building overload was later removed
entirely (the Command-typed overload now builds a small partial new-type
request and routes through the new-type overload instead) -- see git
history. With only one caller left, publishWeighResponse() no longer had
a real duplication to avoid, so it was inlined back into the one
remaining overload.
