# Channel -> .msg mapping (3 of 16 JobMgr channels done)

| # | Real SCS channel | Real C++ struct | Live call site | Primary .msg | Depends on (nested/shared types) |
|---|---|---|---|---|---|
| 1 | `LpsSaJobMgrTxChannelOutput` | `LpsSaJobMgrTxChannelStorage` | `LpsSaJobMgrScs.cpp:1132` `->publish()` | `LpsSaJobMgrTxChannel.msg` | `WeighBktWtAccuracy.msg` (x2 fields), `TipOffTriggerType.msg`, `TipOffState.msg` (x2 fields), `DispBestBktWt.msg`, `SimpleCalData.msg` (array) |
| 2 | `LpsSaJobMgrReqstChannelInput` | `LpsSaJobMgrReqstChannelStorage` | `LpsSaJobMgrScs.cpp:154` `->get()` | `LpsSaJobMgrReqstChannel.msg` | `JobMgrReqstChannelCommand.msg`, `TipOffTriggerType.msg`, `TipOffState.msg`, `LpsSaJobMgrSubtotalInfo.msg`, `LpsSaJobMgrReqst.msg` (array) |
| 3 | `LpsSaJobMgrRespChannelOutput` | `LpsSaJobMgrRespChannelStorage` | `LpsSaJobMgrScs.cpp:630` `->publish()` | `LpsSaJobMgrRespChannel.msg` | `JobMgrReqstChannelCommand.msg` (reused from #2 -- confirmed same enum via `LpsSaJobMgrReqstChannel::Command` in the real header) |

## All files created so far, by role

**Channel messages (1:1 with a real SCS channel, what the shim's `get()`/`publish()` will carry):**
- `LpsSaJobMgrTxChannel.msg`
- `LpsSaJobMgrReqstChannel.msg`
- `LpsSaJobMgrRespChannel.msg`

**Nested messages (sub-structs inside a channel, not channels themselves):**
- `DispBestBktWt.msg` -- used only inside `LpsSaJobMgrTxChannel.msg`
- `SimpleCalData.msg` -- used only inside `LpsSaJobMgrTxChannel.msg` (as an array)
- `LpsSaJobMgrSubtotalInfo.msg` -- used only inside `LpsSaJobMgrReqstChannel.msg`
- `LpsSaJobMgrReqst.msg` -- used only inside `LpsSaJobMgrReqstChannel.msg` (as an array)

**Shared enum-as-message types (reused across 2+ channels, kept single-source rather than duplicated):**
- `WeighBktWtAccuracy.msg` -- PROVISIONAL values (lps_common missing, see file header) -- used in `LpsSaJobMgrTxChannel.msg`; will also be needed later for `LoadRecordOutput` (channel 5)
- `TipOffTriggerType.msg` -- used in `LpsSaJobMgrTxChannel.msg` and `LpsSaJobMgrReqstChannel.msg`
- `TipOffState.msg` -- used in `LpsSaJobMgrTxChannel.msg` (twice) and `LpsSaJobMgrReqstChannel.msg`
- `JobMgrReqstChannelCommand.msg` -- used in `LpsSaJobMgrReqstChannel.msg` and `LpsSaJobMgrRespChannel.msg`

## Why some channels needed more than one file

A "channel" in SCS terms is one `InputInterface<T>`/`OutputInterface<T>` binding to one struct -- but that struct isn't always flat. Where the real struct had nested structs (`DispBestBktWt_t`, `SimpleCalData_t`, `LpsSaJobMgrSubtotalInfo_t`) or a `std::vector`/`std::deque` of a nested type (`requests`, `simpleCalData`), ROS2's `.msg` format requires those as separate message files referenced by name or by `Type[]` array syntax -- there's no way to inline an anonymous struct or a bare array-of-struct the way C++ does. Enums got the same treatment for a different reason: `.msg` has no enum type at all, so each becomes `uintN` constants -- and once a struct's enum type turned out to be reused by a second channel (this happened 3 times already: `TipOffTriggerType`, `TipOffState`, and the `Command` enum), it got promoted out to its own type message so the constants exist in exactly one place instead of being copy-pasted per channel.
