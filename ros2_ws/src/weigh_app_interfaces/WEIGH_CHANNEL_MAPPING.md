# Channel -> .msg mapping (WeighApp)

Companion to `job_mgr_interfaces/CHANNEL_MAPPING.md` -- same manual-trace
methodology (read that file's Part 1/2/3/4 for the step-by-step process,
the new-vs-reuse rule, the `timePoint` trap, and the reconstruct-from-usage
fallback). This file only tracks WeighApp's own channel inventory and status.

## Channel count: 24, not ~22/23

The Development-Plan.txt estimate ("~22 in LpsSaWeighApp.cpp + 1 adv-only")
undercounted. The authoritative source -- `LpsSaWeighApp.cpp`'s
`initialize()` -- has 23 `task::InterfaceDb::bind()` calls plus 1
`SCSOutData<T>::initPublishInterface()` call (a different, non-InterfaceDb
binding pattern used only by `SystemHardwareHealthRequestOutput_`), for 24
total channel handles. Plus the 1 adv-only channel in `adv/TipoffAssist.cpp`
(`TipoffModelTestPointsOutput`, not yet traced) = 25 total.

`demoInputs_` (`DemoAppTxChannel`, `LpsSaWeighApp.h:373`) and `inPwm_`
(`PwmInputChannels`, `LpsSaWeighApp.h:345`) are NOT separate channels --
they're local `Datum` storage variables the real bound channels
(`DemoAppTxIn`, `PwmIn`) drain into via `get()`. Same pattern as any other
`while (channel->get(local)) {...}` drain loop; not double-counted.

## Head start: 9 of 24 channels already fully defined

These are the JobMgr<->WeighApp direct-DDS channels (Development-Plan.txt
Step 6.1) plus channels JobMgr already fully traced that WeighApp also
consumes verbatim (same real struct, same topic) -- confirmed by grepping
each real C++ type name across both apps' trees, not assumed from name
similarity alone. WeighApp's `.cpp` files will `#include` the existing
`cpm_common_interfaces`/`job_mgr_interfaces` message headers directly; no
new `.msg` needed for these 9.

| WeighApp member | Real SCS channel | Direction | Reused `.msg` | Package |
|---|---|---|---|---|
| `LpsSaWeighScsReqstIn` | `LpsSaWeighReqstChannelInput` | Input, direct DDS from JobMgr | `LpsSaWeighReqstChannel.msg` | `cpm_common_interfaces` |
| `LpsSaWeighScsRespOut` | `LpsSaWeighRespChannelOutput` | Output, direct DDS to JobMgr | `LpsSaWeighRespChannel.msg` | `cpm_common_interfaces` |
| `LpsSaWeighScsTxOut` | `LpsSaWeighTxChannelOutput` | Output, direct DDS to JobMgr | `LpsSaWeighTxChannel.msg` | `cpm_common_interfaces` |
| `LpsSaJobMgrScsReqstOut` | `LpsSaJobMgrReqstChannelOutput` | Output, direct DDS to JobMgr (HYBRID -- see Step 2.5/6.1.4) | `LpsSaJobMgrReqstChannel.msg` | `cpm_common_interfaces` |
| `LpsSaJobMgrScsTxIn` | `LpsSaJobMgrTxChannelInput` | Input, subscribes to JobMgr's own published topic directly | `LpsSaJobMgrTxChannel.msg` | `job_mgr_interfaces` |
| `AisJhm2TxInputScs` | `AisJhm2TxChannelInput` | Input, same external broadcaster JobMgr already subscribes to | `AisJhm2TxChannel.msg` | `job_mgr_interfaces` |
| `AutonomyConditionDiagnosticsTxInputChannel` | `AutonomyConditionDiagnosticsTxChannelInput` | Input, same SEA broadcaster JobMgr already subscribes to | `AutonomyConditionDiagnosticsTxChannel.msg` | `job_mgr_interfaces` |
| `shmClockInput_` | `ShmClockInput` | Input, same ACD/ShmClock service JobMgr already subscribes to | `ShmClockInput.msg` | `job_mgr_interfaces` |
| `displayStateInput_` | `DisplayStateInput` | Input, same UI broadcaster JobMgr already subscribes to | `LpsSaUIDisplayStateInterface.msg` | `job_mgr_interfaces` |

**`DataLinkDataInput_` -- kept as its own separate weigh_app_interfaces
file rather than reusing job_mgr_interfaces', but see the correction
below: this IS genuinely the same real channel, not a rejected match.**
JobMgr's `DataLinkData.msg` is deliberately scoped to only the 3 PIDs
JobMgr itself reads (see `job_mgr_interfaces/msg/DataLinkParam.msg`).
WeighApp's own consumption is at `LpsSaWeighApp.cpp:1304-1954` -- roughly
650 lines, a much larger PID surface than JobMgr's. Kept separate
because merging the two independently-scoped field sets means touching
already-committed code in both apps -- a deliberate, recorded scope
decision (Challenges-And-Decisions.txt 6.15), not a "these are
different real channels" finding (that part was wrong, see below).

**Real header found this time.** Unlike JobMgr's channel (real struct
never located, usage-scoped reconstruction with unconfirmed enum values),
WeighApp's `DataLinkParamInfo`/`DataLinkParam`/`DataLinkData` classes ARE
present in this checkout, at
`eta-ais/prod/machineCommon/content/legacy/common/interfaces/CpmDataLinkData/`
-- confirmed genuinely real (not a name coincidence) by matching every
single accessor WeighApp calls (`GetSid`, `GetParamId`,
`GetParamIdentifierType`, `GetUnits`, `GetScaling`, `GetOffset`,
`GetLastValueDsi`, `GetLastValue<T>`, `GetLastValueEng`,
`GetLastGoodValueEng`, `GetLastValueVector`, `GetVarParamBlockLength`,
`GetVarParamBlock`, `GetVarLengthParamType`, `GetVarLengthParamDsi`,
`IsPIDDataReceived`) against the real header's exact signatures -- a
100% match, not indirect/coincidental. All 3 enums involved
(`DlpParamIdentifierType_t`, `DlpUnits_t`,
`VarLengthDataLinkParamFactory::VarLengthParamType`) have explicit
confirmed numeric values, unlike JobMgr's channel.

CORRECTION (found in the top-level msg-package audit, Challenges-And-
Decisions.txt 6.15 -- this section's original claim below was wrong,
kept struck-through rather than deleted so the mistake and its fix are
both visible):
~~Important: JobMgr's own channel uses a *different*, still-unlocated
type -- literally `DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PID`
(`LpsSaJobMgrScs.cpp:778`), not `DataLinkParamInfo::...` like WeighApp.
Same enumerator naming convention, different C++ class -- do NOT use
this finding to "fix" JobMgr's `DataLinkParam.msg`... Confirmed by
grepping the actual original JobMgr source, not assumed.~~
That grep found the names but never located either app's real header
to actually compare classes -- JobMgr's `interfaces/DataLinkData/`
still doesn't exist, so nothing to compare against at the time. Found
this pass, at `eta-ais/prod/machineCommon/content/legacy/common/
interfaces/CpmDataLinkData/DataLinkParam.h`: `class DataLinkParam :
public DataLinkParamInfo`. It's ONE real class hierarchy -- JobMgr's
`DataLinkParam::` reference is the derived class name, WeighApp's
`DataLinkParamInfo::` reference is the inherited base class's own
scope for the same enum. Genuinely the same real channel both apps
subscribe to independently (same `interfaces/DataLinkData/
InterfaceTypes.h` include path on both sides, same drain-loop
structure, same `DataLinkData dlData; dlData.GetParams()` pattern).
This caused 2 real field-type bugs in job_mgr_interfaces'
`DataLinkParam.msg` (`last_value_dsi` was uint32, real type per
`DataLinkParam.h:467` is uint16; `last_value_eng` was float32, real
type per `DataLinkParam.h:227` is float64) -- both now fixed. The
channel itself is left un-merged/misclassified on purpose (user's
explicit choice, given the fix would touch already-committed business
logic in both apps) -- see Challenges-And-Decisions.txt 6.15 for the
full writeup and the deferred full-merge option.

`weigh_app_interfaces/msg/DataLinkParam.msg` is still scoped to exactly
what WeighApp itself reads (17 fields) -- not the full real class's API
surface (which also serves `autonomyConditionDiagnostics` and others with
fields WeighApp never touches, e.g. `GetDataLinkType()`,
`GetFaultDataCount()`, `GetParameterName()`) -- same scoping discipline as
before, just with far more precise types/values now that the real
header exists.

## Remaining: 15 channels needing their own trace (14 in LpsSaWeighApp.cpp/.h + 1 adv-only)

Real `#include` path is cited directly from `LpsSaWeighApp.h` (lines
19-54) -- step 2 of the JobMgr methodology, already done here so the next
session can jump straight to step 3 (locate the real header).

| # | Real SCS channel | Member | Direction | `#include` path (`LpsSaWeighApp.h`) | `.msg` | Status |
|---|---|---|---|---|---|---|
| 1 | `PrinterCnfgInput` | `printerCnfgInput_` | Input | `interfaces/LpsSaTotals/PrinterCnfgInterfaceInputChannel.h` | `LpsSaTotalsPrinterCnfgInterfaceStorage.msg` + 4 nested | Done |
| 2 | `SystemHardwareHealthInput` | `SystemHardwareHealthInput_` | Input | `ais/interfaces/SystemHardwareHealth/InterfaceTypes.h` | `SystemHardwareHealthStorage.msg` + 4 nested | Done |
| 3 | `PartNumbersInput` | `PartNumbersInput_` | Input | `interfaces/PartNumbers/InterfaceTypes.h` | `PartNumbers.msg` | Done -- real struct/folder not in this checkout (same class of gap as JobMgr's SwitchInputScs/OutputChannel/DataLinkData), usage-scoped reconstruction. Producer confirmed at `AutonomyConditionDiagnostics.cpp:935-3216` (real type `PartNumbersStorage`, 7 real fields via Set*() calls), but WeighApp itself only reads 3 (`LpsSaWeighApp.cpp:667-690`: product ID, sw group part number, equipment ID, each via a `IsXValid()/IsXSet()` + `GetX()` pair) -- scoped to exactly those 3, not the producer's full field set |
| 4 | `DataLinkDataInput` | `DataLinkDataInput_` | Input | `interfaces/DataLinkData/InterfaceTypes.h` + `DataLinkData/DataLinkData.h` | `DataLinkData.msg` + `DataLinkParam.msg` | Done -- see note below, real header found this time (unlike JobMgr's channel) |
| 5 | `ReadyToFlashStatusOutput` | `ReadyToFlashStatusOutput` | Output | `interfaces/ReadyToFlashStatus/InterfaceTypes.h` | `ReadyToFlashStatus.msg` | Done -- real enum `rpa_application_ready_code_e` confirmed (19 values, -1..18) plus one WeighApp-local `#define` extension (`APP_READY_CODE_PAYLOAL_LEGAL_FOR_TRADE_IS_SEALED=22`, typo preserved verbatim from source, `LpsSaWeighApp.cpp:52`) |
| 6 | `LpsSaWeighInitDebugChannelOutput` | `LpsSaWeighScsInitDebugOut` | Output | `interfaces/LpsSaWeighInitDebugChannel/InterfaceTypes.h` | `LpsSaWeighInitDebugChannel.msg` | Done -- 8 fields, a scoped slice of the much larger (missing) `LpsInitTbl_t`; only the fields `serialize()` actually archives are modeled, not the full struct |
| 7 | `LpsSaWeighDebugChannelOutput` | `LpsSaWeighScsDebugOut` | Output | `interfaces/LpsSaWeighDebugChannel/InterfaceTypes.h` | `LpsSaWeighDebugChannel.msg` | Done -- large (~150 fields). Top-level fields and `TipoffAssistInputs`/`TipoffAssistOutputs` (from `adv/TipoffAssist.h`, found fully typed locally) are fully confirmed. `m_LpsWrk.*` (~45 fields) sits on `LpsWrkTbl_t`, confirmed genuinely missing from this checkout (same class of gap as `LpsPublic.h`) -- field NAMES are ground truth from `serialize()`, but TYPES are naming-inferred (float32 default, bool/uint8 where evidenced), not read from a real header. Revisit if `lps_weighing` is ever sourced (Development-Plan.txt Step 0.2). |
| 8 | `PwmInputChannelsInput` | `PwmIn` | Input | `interfaces/PwmInputChannels/InterfaceTypes.h` | `PwmInputChannels.msg` | Done -- real struct's own drain call site (`PwmIn->get()`/`PwmInputRead()`) is declared in the header but has NO body anywhere in this checkout -- a genuine gap, not a scoping choice. Reconstructed instead from `inPwm_.PwmData.{Period,Width,SwitchStatus,Timeout}[idx]` usage in the "sa/" build-variant file (`LpsSaProcessInputs.cpp`, real business logic, left unchanged) -- 4 parallel dynamic arrays, exact channel count intentionally left unbounded rather than guessed (References/weighapp.txt's diagram claims `[4]` but that contradicts 6 distinct `*_CH_NUM` constants seen in real code, so not trusted as authoritative) |
| 9 | `MachineInput` | `MachineIn` | Input | `interfaces/Machine/InterfaceTypes.h` | `Machine.msg` | Done -- real header fully present and fully typed (`Machine.h`), zero blocked fields |
| 10 | `DemoAppTxChannelInput` | `DemoAppTxIn` | Input | `interfaces/DemoAppTxChannel/InterfaceTypes.h` | `DemoAppTxChannel.msg` | Done -- drains into local `demoInputs_`; real header fully present and typed, zero blocked fields |
| 11 | `CalMgrCmdReqstInput` | `LpsCalCmdScsReqstIn` | Input | `interfaces/CalMgrCmdReqst/InterfaceTypes.h` | `CalMgrCmdReqst.msg` + `CalibrationRequest.msg` | Done -- real folder/header not in this checkout, nor is `cal_mgr.h` (where `CAL_MGR_MC_E` is actually defined) -- usage-scoped from `LpsSaWeighCalReqstCallback()` (`LpsSaWeighApp.cpp:911-925`). `calcmd`/enum values unconfirmed (BLOCKED, raw uint8). `cal_iterm` element type CORRECTED during Step 5 (`uint8[]` -> `uint32[]`): found the real local target `unsigned_32 cal_iterm[(CAL_LAST_ITERM_BIT>>5)+1];` (`LpsSaCalibration.cpp:24`, an unchanged file) while converting the actual call site -- the memcpy's target element size proves the source field is an `unsigned_32` array, not raw bytes. Array length still left unbounded rather than guessing a fixed count. |
| 12 | `CalMgrCmdRespOutput` | `LpsCalCmdScsRespOut` | Output | `interfaces/CalMgrCmdResp/InterfaceTypes.h` | `CalMgrCmdResp.msg` + `CalMgrCalibrationResp.msg` | Done -- same gap as channel 11, same call site (`LpsSaWeighApp.cpp:922-989`). `resp_code`/`cal_resp` BLOCKED (raw uint8, real `CAL_MGR_MR_E` values unconfirmed); `error`/`step_no`/`warning` types cross-confirmed against `LpsSaWeighInfoTbl_t`'s own matching fields |
| 13 | `LpsSaNvmCalDataChannelOutput` | `LpsNvmDumpChanOut` | Output | `interfaces/LpsSaNvmCalDataChannel/InterfaceTypes.h` | `LpsSaNvmCalDataChannel.msg` + `LpsSaNvmCalDataChannelDataMain.msg` + `LpsSaNvmCalDataChannelDataDebug.msg` + `LpsCalIMUResults.msg` | Done -- real header fully present and typed, zero blocked fields. `LpsCalIMUResults` (16 float fields) also found in `lps_app_common/include/LpsCalAppPublic.h`, shared/reused by channel 14 |
| 14 | `LpsSaNvmCalOnTheFlyDataChannelOutput` | `LpsNvmOnTheFlyDumpChanOut` | Output | `interfaces/LpsSaNvmCalOnTheFlyDataChannel/InterfaceTypes.h` | `LpsSaNvmCalOnTheFlyDataChannel.msg` + 6 nested messages | Done -- real header fully present and typed. `serialize()` cross-check caught 4 declared-but-never-archived struct members (`LpsCalAppInfRackCmdLmt`/`DumpCmdLmt`/`EngSpdLmt`, a second unarchived `LeverInfoAvailable` inside `Overrides` distinct from the archived `MiscUpdates.LiftLeverInfo_LeverInfoAvailable`) -- correctly omitted, same class of finding as `ReqPloadOpMode_t` earlier this session. `IMUCalResults`' 16 fields are split across two version gates (v3 archives 7, v5 archives the other 9) but together cover the *entire* real struct, so the full shared `LpsCalIMUResults.msg` type is referenced as-is |
| 15 | `SystemHardwareHealthRequestOutput` | `SystemHardwareHealthRequestOutput_` | Output | `ais/interfaces/SystemHardwareHealthRequest/InterfaceTypes.h` | `SystemHardwareHealthRequest.msg` | Done -- genuinely empty struct (pure trigger/ping); different binding pattern (`SCSOutData<T>` + `initPublishInterface()`/`send()` at `LpsSaScs.cpp:581`, not `InterfaceDb::bind()`+plain `publish()`) flagged for Step 5 |

**Adv-only (16th, separate file):** `TipoffModelTestPointsOutput`, bound
and published in `adv/TipoffAssist.cpp` (Development-Plan.txt 5.3) --
Done. Real header (`TipoffModelTestPoints.h`) fully present and typed,
82 fields, every one archived in `serialize()` -- confirmed against the
real `publish()` call site (`TipoffAssist.cpp:127-214`), field-for-field
match. `.msg`: `TipoffModelTestPoints.msg`.

## Status

**Step 2.2 for WeighApp is fully complete -- all 16 of 16 channels
done** (34 `.msg` files total, plus the 9 reused from
`cpm_common_interfaces`/`job_mgr_interfaces`).

**Step 5 (5.1-5.4) is now fully complete.** `LpsSaWeighApp.h/.cpp` (22
channels, 8 call sites), `LpsSaScs.cpp` (13 call sites), and
`adv/TipoffAssist.cpp` (the adv-only `TipoffModelTestPointsOutput`
channel, its own dedicated `rclcpp::Node` since `TipoffAssist` is
instantiated standalone in the unchanged `adv/LpsAdvProcessInputs.cpp`)
are all converted to shim types, and `executor_.spin_some()` is wired
into the 50Hz `executive()`. Remaining: 5.5 (SConscript build-wiring)
and 5.6-5.8 (build/link verification, real-time budget check,
component testing) -- all blocked on real build infrastructure, same
status as JobMgr's equivalent Step 4.5-4.7.

**Step 6.1-6.2 are now complete.** All 4 JobMgr<->WeighApp channel legs
are wired native DDS-to-DDS, no Bridge involved on either side:
`LpsSaWeighReqstChannel` (JobMgr publishes/WeighApp subscribes),
`LpsSaWeighRespChannel` and `LpsSaWeighTxChannel` (WeighApp
publishes/JobMgr subscribes, via the redesigned `LpsSaWeighAppInf`,
see Challenges-And-Decisions.txt 6.11), and the hybrid
`LpsSaJobMgrReqstChannel` leg (WeighApp publishes onto the same
`lps_sa_job_mgr_reqst_channel` topic JobMgr already subscribes to for
the legacy UI path -- confirmed matching topic/message type on both
sides, no code change needed there since Step 5). QoS matches by
default (`qosDepth=10` on both ends, neither side overrides it).
Remaining: 6.3 (component tests running together) -- blocked on real
build/runtime infrastructure, same status class as the other testing
steps. Next up: Step 7 (Bridge, `cpm_scs_bridge`).
