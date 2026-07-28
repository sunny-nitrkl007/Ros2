# Channel -> .msg mapping (13 of 16 JobMgr channels done, 3 blocked)

## Part 1 — How to manually trace one channel, step by step

This is the exact repeatable process used for all 16 channels. Anyone can
follow it starting from nothing but the channel name.

**Step 1 — Find the channel name in the app's config.**
Open `CPM-Loader-ais/config/LpsSaJobMgrApp.rb`. Every channel JobMgr binds is
listed in its `Interfaces` hash, e.g.:
```ruby
"LpsSaJobMgrTxChannelOutput" => InterfaceDefs_Loaders::LpsSaJobMgrTxChannelOutput,
```
The left-hand string is the SCS channel name. The right-hand side tells you
which `InterfaceDefs*` module (`InterfaceDefs_Loaders`, `InterfaceDefs_CpmCommon`,
or plain `InterfaceDefs`) actually defines it -- this matters for step 2,
since it hints at where the real header lives.

**Step 2 — Find the `#include` in the app source that pulls in the type.**
Open `LpsSaJobMgrApp.h` and grep for the channel's base name (strip the
trailing `Input`/`Output`). Every channel has a line like:
```cpp
#include <interfaces/LpsSaJobMgrTxChannel/InterfaceTypes.h>
```
The path segment right after `interfaces/` (`LpsSaJobMgrTxChannel` here) is
the real folder name to go looking for next -- it is NOT always the same as
the channel string from step 1 (e.g. the channel `DisplayStateInput` maps to
folder `LpsSaUI`, not `DisplayStateInput`).

**Step 3 — Find the real folder. Check BOTH locations, not just one.**
Two places, and both must be checked before concluding a struct is missing:
- `CPM-Loader-ais/prod/common/interfaces/<FolderName>/` -- where most
  JobMgr/WeighApp/UI-specific channels live.
- `eta-ais/prod/machineCommon/content/prod/common/interfaces/<FolderName>/`
  -- where lower-level, more platform-wide channels live instead (this is
  where `ShmClockInput`, `AutonomyConditionDiagnosticsTxChannel`, and
  `EventDiagnosticData` actually turned out to be -- all three were wrongly
  marked blocked at first because only the first location was checked).
If neither location has it, do a repo-wide `Glob`/`Grep` for the struct's
class name before giving up -- `InterfaceTypes.h`'s `#include` path is a
strong hint, not a guarantee, of where the real definition sits.

**Step 4 — Read the real header in full, not a summary.**
Inside the folder, `InterfaceTypes.h` is just a typedef
(`typedef Datum<XStorage> X;`) -- the actual fields live in a sibling file
named after the channel (e.g. `LpsSaJobMgrTxChannel.h`). Read the whole
`...Storage` class: constructor defaults, member declarations, and
critically the `serialize()` method.

**Step 5 — Use `serialize()` as the authoritative field list, not the member
declarations.** A class can declare a member that's never actually written
to the wire (never touched by `ar & field`). `serialize()` is ground truth
for what crosses SCS today. Two things to watch for:
- Version-gated blocks (`if (version >= N)`) are almost always backwards
  file-compat scaffolding for reading OLD stored blobs -- for a NEW wire
  format, just take the current max version's full field set (check
  `BOOST_CLASS_VERSION(X, N)` at the bottom of the file).
- A field CAN be archived (crosses the wire in the old system) but never
  read back downstream -- `timePoint` is the recurring example. Don't
  assume dead just because a member looks unused inside the app's own
  folder; grep the WHOLE tree, and specifically check for a
  `...RequestHelper.hpp`/`...AppInf.hpp`-style helper class, since those are
  where `timePoint` correlation logic actually lives (see Part 3).

**Step 6 — Chase every non-primitive field type before writing anything.**
For each field that isn't a plain int/float/bool/string:
- Nested struct (`Foo_t`, `struct Foo`) -> needs its own `.msg` file.
- `std::vector`/`std::deque<Foo>` -> same nested `.msg`, referenced as
  `Foo[]`.
- Enum -> `.msg` has no enum type at all. Find the real values: explicit
  literals in the same file are done; a name that just aliases ANOTHER
  named constant (e.g. `ZEROED = TIP_OFF_TRIGGER_AUTO`) means tracing into
  that constant's own header; unlabeled sequential enumerators follow plain
  C rules (first = 0 unless stated, else previous + 1).
- If the enum/struct's defining header genuinely can't be found anywhere
  (checked both locations in step 3, repo-wide grep, no luck) -- see Part 4.

**Step 7 — Write the `.msg`, citing the source file and line range in a
header comment.** Every field name gets mechanically converted
`camelCase`/`PascalCase` -> `snake_case` (required by `rosidl`, not a style
choice). Nothing else changes -- no renaming for clarity, no reinterpreting
meaning, no unit conversion.

**Step 8 — Register it in `CMakeLists.txt`, in dependency order** (a message
referencing another message must be listed after the one it depends on),
commit, push.

## Part 2 — New `.msg` file vs. reuse an existing one: the actual rule

The test applied every time, in order:

1. **Is this exact enum/struct already used by a second CHANNEL** (a second
   `.msg` file, anywhere in this workspace, not just the one currently being
   written)? If yes, it does not get inlined a second time -- it gets its
   own file and both channels reference it. This happened three times within
   `job_mgr_interfaces` alone: `TipOffTriggerType`, `TipOffState`, and the
   `Command` enum shared between Reqst/Resp channels (-> `JobMgrReqstChannelCommand`).
2. **Is it used directly by a DIFFERENT APP's own code**, not just via this
   channel's struct (checked by grepping the real C++ type name across the
   whole tree)? If yes, it doesn't belong in `job_mgr_interfaces` at all --
   it moves to `cpm_common_interfaces`, the package with no dependents of
   its own, so the dependency direction stays one-way (`job_mgr_interfaces`
   and `weigh_app_interfaces` both depend on `cpm_common_interfaces`, never
   the reverse). This is why `WeighBktWtAccuracy`, `StandbyState`, and the
   entire `LpsSaJobMgrReqstChannel` family (WeighApp publishes to it
   directly) live in `cpm_common_interfaces` instead of `job_mgr_interfaces`.
3. **Otherwise**, the enum/struct is inlined as local constants (enum) or
   defined as its own file used only within the one channel (nested struct)
   -- no premature sharing. `LpsSaJobMgrManualTipOffState_t` is the example:
   checked, confirmed used nowhere else, kept as local constants in
   `LpsSaJobMgrTxChannel.msg` rather than promoted.
4. **A "channel" is not always one flat struct.** Where the real struct
   nests other structs or holds a `vector`/`deque` of one, ROS2 has no
   anonymous-struct or bare-array-of-struct equivalent -- each nested type
   becomes its own `.msg`, referenced by name (or `Type[]` for the array
   case). This is why some channels produced 5+ files from what looks like
   "one channel."

This check is re-run for every new field, every time -- it's why several
already-pushed files got corrected mid-session (`WeighBktWtAccuracy` and
friends moved out of `job_mgr_interfaces` after the cross-app usage was
found; `timePoint` was added back to two files after being wrongly dropped).

## Part 3 — `timePoint`: the one recurring trap

Nearly every channel struct has a `std::chrono::steady_clock::time_point
timePoint` field. It is NOT safe to assume it's dead weight:
- On plain Tx-style channels with no request/response helper
  (`LpsSaJobMgrTxChannel`, checked properly the second time), it can
  genuinely be dead -- constructed, serialized, never read back.
- On any channel that has a matching `...RequestHelper.hpp` /
  `...AppInf.hpp` companion class, `timePoint` is almost certainly live --
  used to detect whether fresh response/Tx data reflects the most recent
  request (`nextTxTimePoint_ = response.timePoint`, then compared against
  another channel's `timePoint`). Confirmed live this way for
  `LpsSaJobMgrTxChannel`, `LpsSaJobMgrRespChannel`, `LpsSaWeighRespChannel`,
  `LpsSaWeighTxChannel`, and `AutonomyConditionDiagnosticsTxChannel`.
Wire representation for the live (steady_clock) ones: `int64` nanosecond
count -- not wall-clock time, only meaningful compared against another
value of the same field family. Separately, a handful of channels carry a
genuine wall-clock timestamp (`std::chrono::system_clock`/POSIX `TimeStamp`
wrapping `struct timeval`) -- `LftSealStatus.seal_time_ns`,
`LoadRecordTimeStamp.utc_time_ns`, `ShmClockInput.utc_sec_us`,
`Diagnostic.gps_time_sec_us` -- these ARE real calendar time, carried as
epoch nanoseconds/microseconds.

## Part 4 — When a struct genuinely can't be found

Applies to 3 of 16 channels so far: `SwitchInputScsInput`,
`OutputChannelOutput`, `DataLinkDataInput`. The rule: don't invent a
plausible-looking struct from method-call evidence alone. A usage site like
`obj.get_STG_value(STG4)` proves an accessor exists, not what the class
stores internally. These stay undrafted, flagged, and skipped rather than
published looking as verified as the other 13 -- separate in kind from
fields where the struct IS known but one enum's numeric values are blocked
(see `WeighBktWtAccuracy`/`FloatIO.stat`/etc.), which ARE drafted, as raw
integers with no invented constants, because the shim casts the real C++
enum value through at runtime regardless of whether the mapping is
documented yet.

## Part 5 — Full 16-channel status

| # | Real SCS channel | Consumer app(s) | Real struct | Where it actually lives | `.msg` | Status |
|---|---|---|---|---|---|---|
| 1 | `LpsSaJobMgrTxChannelOutput` | UI/DisplayApp (+ aisXcpServer snoop) | `LpsSaJobMgrTxChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrTxChannel/` | `LpsSaJobMgrTxChannel.msg` | Done |
| 2 | `LpsSaJobMgrReqstChannelInput` | UI/DisplayApp, WorkOrderAssistApp, aisXcpServer, **+ WeighApp (direct producer, found this session)** | `LpsSaJobMgrReqstChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrReqstChannel/` | `LpsSaJobMgrReqstChannel.msg` (`cpm_common_interfaces`) | Done |
| 3 | `LpsSaJobMgrRespChannelOutput` | UI/DisplayApp | `LpsSaJobMgrRespChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrRespChannel/` | `LpsSaJobMgrRespChannel.msg` | Done |
| 4 | `LpsSaJobMgrDebugChannelOutput` | aisXcpServer / diagnostic tooling | `LpsSaJobMgrDebugChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrDebugChannel/` | `LpsSaJobMgrDebugChannel.msg` | Done |
| 5 | `LpsSaWeighReqstChannelOutput` | WeighApp | `LpsSaWeighReqstChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/LpsSaWeighReqstChannel/` | `LpsSaWeighReqstChannel.msg` (`cpm_common_interfaces`, direct DDS) | Done |
| 6 | `LpsSaWeighRespChannelInput` | WeighApp | `LpsSaWeighRespChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/LpsSaWeighRespChannel/` | `LpsSaWeighRespChannel.msg` (`cpm_common_interfaces`, direct DDS) | Done |
| 7 | `LpsSaWeighTxChannelInput` | WeighApp | `LpsSaWeighTxChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/LpsSaWeighTxChannel/` | `LpsSaWeighTxChannel.msg` (`cpm_common_interfaces`, direct DDS) | Done -- 6 fields raw-int (`LpsPublic.h` missing) |
| 8 | `LoadRecordOutput` | LpsSaTotalsApp / WorkOrderAssistApp | `LpsSaLoadRecordChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/LpsSaLoadRecordChannel/` | `LpsSaLoadRecordChannel.msg` | Done -- `weight_units` raw-int (`LpsCommonWeight.h` missing) |
| 9 | `SwitchInputScsInput` | SwitchInputScs app | unknown | not found anywhere in checkout | -- | **Blocked** |
| 10 | `OutputChannelOutput` | OutputApp (horn relay) | unknown | not found anywhere in checkout | -- | **Blocked** |
| 11 | `AisJhm2TxChannelInput` | AisJhm2 data server | `AisJhm2TxChannelStorage` | `CPM-Loader-ais/prod/common/interfaces/AisJhm2TxChannel/` | `AisJhm2TxChannel.msg` | Done |
| 12 | `DisplayStateInput` | UI/DisplayApp | `LpsSaUIDisplayState` | `CPM-Loader-ais/prod/common/interfaces/LpsSaUI/` | `LpsSaUIDisplayState.msg` | Done -- `weight_units`/`weight_precision` raw-int |
| 13 | `ShmClockInput` | ACD/ShmClock service | `ShmClockStorage` | `eta-ais/prod/machineCommon/content/prod/common/interfaces/ShmClock/` | `ShmClockInput.msg` | Done |
| 14 | `DataLinkDataInput` | DataLink / other ECMs | `DataLinkDataStorage` (uncertain match) | `eta-ais/.../legacy/common/interfaces/CpmDataLinkData/` (legacy) | -- | **Blocked** -- complex map-based struct, legacy folder |
| 15 | `AutonomyConditionDiagnosticsTxChannelInput` | SEA licensing broadcaster | `AutonomyConditionDiagnosticsTxInterfaceStorage` | `CPM-Loader-ais/prod/common/interfaces/AutonomyConditionDiagnostics/` | `AutonomyConditionDiagnosticsTxChannel.msg` | Done |
| 16 | `EventDiagnosticDataInput` | Other ECM's event/diagnostic server | `EventDiagnosticDataStorage` | `eta-ais/prod/machineCommon/content/prod/common/interfaces/EventDiagnosticData/` | `EventDiagnosticData.msg` | Done |

## All files created so far, by role

**Channel messages (1:1 with a real SCS channel):**
`LpsSaJobMgrTxChannel.msg`, `LpsSaJobMgrReqstChannel.msg`,
`LpsSaJobMgrRespChannel.msg`, `LpsSaJobMgrDebugChannel.msg`,
`LpsSaWeighReqstChannel.msg`, `LpsSaWeighRespChannel.msg`,
`LpsSaWeighTxChannel.msg`, `LpsSaLoadRecordChannel.msg`,
`AisJhm2TxChannel.msg`, `LpsSaUIDisplayState.msg`, `ShmClockInput.msg`,
`AutonomyConditionDiagnosticsTxChannel.msg`, `EventDiagnosticData.msg`

**Nested messages (sub-structs, not channels themselves):**
`DispBestBktWt.msg`, `SimpleCalData.msg`, `LpsSaJobMgrSubtotalInfo.msg`,
`LpsSaJobMgrReqst.msg`, `FloatIO.msg`, `LiftPosition.msg`, `TiltPosition.msg`,
`Payload.msg`, `LftSealStatus.msg`, `WeighRange.msg`,
`ProdMeasureSensorStatus.msg`, `LinkSensorCalLim.msg`, `WeighPidData.msg`,
`AcdInfoPopUp.msg`, `AcdDiagPopUp.msg`, `AcdEventPopUp.msg`,
`LoadRecordTimeStamp.msg`, `LoadRecordPass.msg`, `LoadRecordSubtotal.msg`,
`SimpleCalDataT.msg`, `TipoffWeightAdjustData.msg`,
`LpsSaUIDisplaySettings.msg`, `SEA.msg`, `J1939Name.msg`, `Diagnostic.msg`

**Shared type messages, in `cpm_common_interfaces` (reused across channels
and/or a different app's own code):**
`WeighBktWtAccuracy.msg`, `TipOffTriggerType.msg`, `TipOffState.msg`,
`StandbyState.msg`, `JobMgrReqstChannelCommand.msg`,
`WeighReqstChannelCommand.msg`, `FloatPair.msg`
