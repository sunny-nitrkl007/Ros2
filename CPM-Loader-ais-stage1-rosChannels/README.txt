======================================================================
Stage 1 minimal file set -- ROS2-plumbing-as-a-separate-class variant
======================================================================
Same scope as CPM-Loader-ais-stage1/: legs 1-3 only
(LpsSaWeighReqstChannel, LpsSaWeighRespChannel, LpsSaWeighTxChannel --
the weighAppInf trio). Nothing about WHAT is converted changes here --
only HOW the ROS2/DDS plumbing is organized within each app.

This is a TEST/COMPARISON folder, not a replacement for
CPM-Loader-ais-stage1/. It exists to answer: "can node creation,
subscription/publisher setup, and other ROS2-specific things be kept in
their own file(s) instead of inlined into each app's own .h/.cpp?" --
see the chat discussion this folder came from.

----------------------------------------------------------------------
What's different from CPM-Loader-ais-stage1/
----------------------------------------------------------------------
CPM-Loader-ais-stage1/ inlines all ROS2 setup directly into each app:
  - LpsSaJobMgrApp.h/.cpp: rosNode_, executor_, weighAppInf_ as 3
    separate members; rclcpp::init()/node construction/executor setup
    all written directly in initialize(); executor_.spin_some() written
    directly in executive(); rclcpp::shutdown() written directly in
    cleanup().
  - LpsSaWeighApp.h/.cpp: same idea, 5 separate members
    (LpsSaWeighScsReqstIn/RespOut/TxOut, rosNode_, executor_).

This folder pulls all of that into one small class per app:
  - apps/LpsSaJobMgrApp/LpsSaJobMgrRosChannels.h (NEW)
      Owns rosNode_, executor_, and weighAppInf. init()/spinSome()/
      shutdown() replace what used to be written inline.
  - apps/LpsSaWeighApp/LpsSaWeighAppRosChannels.h (NEW)
      Owns rosNode_, executor_, and the 3 channel wrappers
      (LpsSaWeighScsReqstIn/RespOut/TxOut). Same init()/spinSome()/
      shutdown() pattern.

Each app now holds ONE member, rosChannels_, instead of 3-5 separate
ROS2-related members. Every call site that used to touch
weighAppInf_.<method>() or LpsSaWeighScsXxx->method() directly now goes
through rosChannels_.weighAppInf.<method>() or
rosChannels_.LpsSaWeighScsXxx->method() instead -- purely mechanical,
no logic changes anywhere.

----------------------------------------------------------------------
Files in this folder
----------------------------------------------------------------------
  apps/LpsSaJobMgrApp/LpsSaJobMgrRosChannels.h
                                             -- NEW. Owns rosNode_,
                                                executor_, weighAppInf
                                                (a DDSWeighAppInf).
  apps/LpsSaJobMgrApp/LpsSaJobMgrApp.h      -- weighAppInf_/rosNode_/
                                                executor_ (3 members)
                                                replaced by one
                                                rosChannels_ member.
                                                Include swapped from the
                                                old rclcpp/ros2_wrapper/
                                                DDSWeighAppInf includes
                                                to "LpsSaJobMgrRosChannels.h".
                                                LpsSaWeighAppInf.hpp
                                                include KEPT (unrelated
                                                reason -- see file
                                                comment: LpsSaJobMgrScsSendCmd()
                                                still needs the real
                                                LpsSaWeighReqstChannel::Command
                                                type, outside this
                                                variant's scope same as
                                                the original Stage 1).
  apps/LpsSaJobMgrApp/LpsSaJobMgrApp.cpp    -- initialize(): the
                                                rclcpp::init() guard +
                                                node construction + 3
                                                wrapper constructions +
                                                weighAppInf_.start() all
                                                collapse into
                                                rosChannels_.init(
                                                "job_mgr_node", getTaskName()).
                                                executive():
                                                executor_.spin_some() ->
                                                rosChannels_.spinSome().
                                                cleanup(): the
                                                rclcpp::shutdown() guard
                                                -> rosChannels_.shutdown().
                                                Constructor init list
                                                updated to match (single
                                                rosChannels_() instead of
                                                weighAppInf_()/rosNode_(nullptr)/
                                                executor_()).
  apps/LpsSaJobMgrApp/LpsSaJobMgrScs.cpp    -- 2 call sites got the
                                                mechanical rosChannels_.
                                                prefix (weighAppInf_.
                                                waitForTxData(...) ->
                                                rosChannels_.weighAppInf.
                                                waitForTxData(...); same
                                                for sendRequest). BUT that
                                                prefix pass alone copied
                                                forward 2 real, pre-
                                                existing type-mismatch
                                                bugs from
                                                CPM-Loader-ais-stage1/'s
                                                own LpsSaJobMgrScs.cpp
                                                (present there too, now
                                                fixed in both places -- see
                                                that folder's README.txt
                                                for full detail): (1)
                                                LpsSaWeighScsTxParamRead()'s
                                                local `rxParam` was typed
                                                as the old raw
                                                LpsSaWeighTxChannel instead
                                                of cpm_common_interfaces::
                                                msg::LpsSaWeighTxChannel --
                                                wouldn't compile against
                                                DDSWeighAppInf::
                                                waitForTxData()'s real
                                                signature, and its ~10
                                                CamelCase field reads
                                                afterward were stale;
                                                fixed, all fields now
                                                snake_case, 3 fields
                                                static_cast back to the old
                                                SCS enum types
                                                LpsJobMgrJobTrackerInfoTbl
                                                still uses. (2)
                                                LpsSaJobMgrScsSendCmd()'s
                                                local `request` had the
                                                same problem, fixed the
                                                same way; the function's
                                                own parameter type
                                                (LpsSaWeighReqstChannel::
                                                Command) correctly stays
                                                unchanged, since
                                                LpsSaJobMgrProcess.cpp
                                                (excluded from Stage 1)
                                                calls it with that real
                                                enum type.

  apps/LpsSaWeighApp/LpsSaWeighAppRosChannels.h
                                             -- NEW. Owns rosNode_,
                                                executor_, and the 3
                                                channel wrappers
                                                (LpsSaWeighScsReqstIn/
                                                RespOut/TxOut), public.
  apps/LpsSaWeighApp/LpsSaWeighApp.h        -- Same 5-members-to-1
                                                collapse as JobMgr's
                                                side. Include swapped to
                                                "LpsSaWeighAppRosChannels.h".
                                                ALSO: added the 3rd
                                                LpsSaScsSendReqstResponse
                                                overload's declaration
                                                (cpm_common_interfaces::
                                                msg::LpsSaWeighReqstChannel&
                                                -typed) -- it was defined
                                                in LpsSaScs.cpp but never
                                                declared here, inherited
                                                from the same gap in
                                                CPM-Loader-ais-stage1/
                                                (would not have compiled;
                                                see that folder's
                                                README.txt). Also added
                                                publishWeighResponse()'s
                                                declaration -- new private
                                                helper factored out of the
                                                2 response-building
                                                overloads' duplicated
                                                boilerplate.
  apps/LpsSaWeighApp/LpsSaWeighApp.cpp      -- initialize(): the
                                                rclcpp::init() guard +
                                                node construction + all
                                                3 wrapper constructions
                                                (including what was
                                                LpsSaWeighScsReqstIn's
                                                separate construction
                                                further down in the
                                                original -- moved to
                                                happen together here;
                                                harmless reorder, no
                                                init step in between
                                                depends on the order
                                                these 3 are created in)
                                                all collapse into
                                                rosChannels_.init(
                                                "weigh_app_node").
                                                executive():
                                                executor_.spin_some() ->
                                                rosChannels_.spinSome().
                                                cleanup(): the
                                                rclcpp::shutdown() guard
                                                -> rosChannels_.shutdown().
                                                Constructor init list
                                                updated to match.
  apps/LpsSaWeighApp/LpsSaScs.cpp           -- 7 call sites changed,
                                                all mechanical prefix
                                                additions: LpsSaWeighScsReqstIn
                                                -> rosChannels_.
                                                LpsSaWeighScsReqstIn (1x),
                                                LpsSaWeighScsRespOut ->
                                                rosChannels_.
                                                LpsSaWeighScsRespOut (4x,
                                                both LpsSaScsSendReqstResponse
                                                overloads), LpsSaWeighScsTxOut
                                                -> rosChannels_.
                                                LpsSaWeighScsTxOut (2x, in
                                                LpsSaWeighingScsTx()).
                                                Nothing else in this
                                                file changed. NOTE: the 3
                                                overloads of
                                                LpsSaScsSendReqstResponse
                                                in this file (2-arg
                                                Command-typed, old
                                                LpsSaWeighReqstChannelStorage&-
                                                typed, new
                                                cpm_common_interfaces::msg::
                                                LpsSaWeighReqstChannel&-typed)
                                                look like duplication but
                                                are NOT -- each has a real,
                                                distinct caller. In
                                                particular,
                                                apps/LpsSaWeighApp/
                                                LpsSaProcess.cpp (not
                                                included in this file set,
                                                and not previously
                                                documented as touching
                                                legs 1-3) calls the 2-arg
                                                Command-typed overload 5
                                                times, deferring the
                                                response for RESET_BEST_
                                                BUCKET_WEIGHT/ZERO/
                                                CAPTURE_CYLINDER_EXTENSION_
                                                REFERENCE/CLEAR_REWEIGH_WARNING
                                                until the actual physical
                                                operation completes in its
                                                own update cycle. It needs
                                                ZERO code changes here
                                                (request_ and the
                                                Command-typed overload
                                                were correctly left on
                                                their old types), but do
                                                NOT remove either
                                                overload, or request_, as
                                                "duplication cleanup" --
                                                that would silently break
                                                LpsSaProcess.cpp once this
                                                tree is copied back over
                                                the real repo. See
                                                CPM-Loader-ais-stage1/
                                                README.txt for full
                                                detail.

  prod/common/ros2_wrapper/RosInputInterface.h   -- unchanged, copied
                                                     as-is from
                                                     CPM-Loader-ais-stage1/.
  prod/common/ros2_wrapper/RosOutputInterface.h  -- unchanged, copied
                                                     as-is.
  prod/common/interfaces/LpsSaWeighReqstChannel/DDSWeighAppInf.hpp
                                                  -- unchanged, copied
                                                     as-is.
  prod/common/interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp
                                                  -- unchanged, copied
                                                     as-is -- still
                                                     byte-identical to
                                                     the original;
                                                     AisJhm2RequestProcessor
                                                     still depends on it
                                                     unmodified, same
                                                     reasoning as
                                                     CPM-Loader-ais-stage1/.

----------------------------------------------------------------------
Verified (static checks only -- no Docker/compiler run yet)
----------------------------------------------------------------------
  - Brace/paren balance checked on all 8 touched/new files; all balanced
    except LpsSaWeighApp.cpp, which carries the exact same +2 brace /
    +3 paren offset as the ORIGINAL CPM-Loader-ais-stage1/ file (confirmed
    by diffing counts against the original) -- a pre-existing string-
    literal artifact, not something introduced here.
  - grep swept the whole folder for weighAppInf_. / bare rosNode_ /
    bare executor_ / LpsSaWeighScsReqstIn / LpsSaWeighScsRespOut /
    LpsSaWeighScsTxOut outside the 2 new RosChannels classes -- no
    leftover unprefixed call sites found.
  - NOT yet verified: an actual compile. Try a Docker build the same way
    CPM-Loader-ais-stage1/ was tested (Dockerfile.stage1 /
    docker-compose), pointed at this folder instead, once Docker is
    available.

----------------------------------------------------------------------
Also required, not part of this file set (identical to CPM-Loader-ais-stage1/)
----------------------------------------------------------------------
  - colcon build of cpm_common_interfaces + job_mgr_interfaces (only
    these 2 -- legs 1-3 use no weigh_app_interfaces types)
  - SConscript wiring (see Stage1-Real-Build-SConscript-Steps.txt) --
    unaffected by this variant: LpsSaJobMgrRosChannels.h/
    LpsSaWeighAppRosChannels.h are header-only, so nothing new needs to
    be added to either app's SConscript Glob('*.cpp') pattern.
