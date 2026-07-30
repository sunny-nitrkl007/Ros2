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

**Explicitly checked and rejected for reuse:** `DataLinkDataInput_`.
JobMgr's `DataLinkData.msg` is deliberately scoped to only the 3 PIDs
JobMgr itself reads (see `job_mgr_interfaces/msg/DataLinkParam.msg` header).
WeighApp's own consumption is at `LpsSaWeighApp.cpp:1304-1954` -- roughly
650 lines, a much larger PID surface than JobMgr's. Needs its own
independently-scoped `.msg` in `weigh_app_interfaces`, not a reuse.

## Remaining: 15 channels needing their own trace (14 in LpsSaWeighApp.cpp/.h + 1 adv-only)

Real `#include` path is cited directly from `LpsSaWeighApp.h` (lines
19-54) -- step 2 of the JobMgr methodology, already done here so the next
session can jump straight to step 3 (locate the real header).

| # | Real SCS channel | Member | Direction | `#include` path (`LpsSaWeighApp.h`) | `.msg` | Status |
|---|---|---|---|---|---|---|
| 1 | `PrinterCnfgInput` | `printerCnfgInput_` | Input | `interfaces/LpsSaTotals/PrinterCnfgInterfaceInputChannel.h` | `LpsSaTotalsPrinterCnfgInterface.msg` + 4 nested | Done |
| 2 | `SystemHardwareHealthInput` | `SystemHardwareHealthInput_` | Input | `ais/interfaces/SystemHardwareHealth/InterfaceTypes.h` | `SystemHardwareHealth.msg` + 4 nested | Done |
| 3 | `PartNumbersInput` | `PartNumbersInput_` | Input | `interfaces/PartNumbers/InterfaceTypes.h` | | Not started |
| 4 | `DataLinkDataInput` | `DataLinkDataInput_` | Input | `interfaces/DataLinkData/InterfaceTypes.h` + `DataLinkData/DataLinkData.h` | | Not started -- large, ~650 lines of usage (`LpsSaWeighApp.cpp:1304-1954`), do NOT reuse job_mgr_interfaces' 3-PID-scoped version; needs its own usage-scoped reconstruction, same Part 4 fallback as JobMgr's but bigger |
| 5 | `ReadyToFlashStatusOutput` | `ReadyToFlashStatusOutput` | Output | `interfaces/ReadyToFlashStatus/InterfaceTypes.h` | | Not started |
| 6 | `LpsSaWeighInitDebugChannelOutput` | `LpsSaWeighScsInitDebugOut` | Output | `interfaces/LpsSaWeighInitDebugChannel/InterfaceTypes.h` | | Not started |
| 7 | `LpsSaWeighDebugChannelOutput` | `LpsSaWeighScsDebugOut` | Output | `interfaces/LpsSaWeighDebugChannel/InterfaceTypes.h` | | Not started |
| 8 | `PwmInputChannelsInput` | `PwmIn` | Input | `interfaces/PwmInputChannels/InterfaceTypes.h` | | Not started -- drains into local `inPwm_` |
| 9 | `MachineInput` | `MachineIn` | Input | `interfaces/Machine/InterfaceTypes.h` | | Not started |
| 10 | `DemoAppTxChannelInput` | `DemoAppTxIn` | Input | `interfaces/DemoAppTxChannel/InterfaceTypes.h` | | Not started -- drains into local `demoInputs_` |
| 11 | `CalMgrCmdReqstInput` | `LpsCalCmdScsReqstIn` | Input | `interfaces/CalMgrCmdReqst/InterfaceTypes.h` | | Not started |
| 12 | `CalMgrCmdRespOutput` | `LpsCalCmdScsRespOut` | Output | `interfaces/CalMgrCmdResp/InterfaceTypes.h` | | Not started |
| 13 | `LpsSaNvmCalDataChannelOutput` | `LpsNvmDumpChanOut` | Output | `interfaces/LpsSaNvmCalDataChannel/InterfaceTypes.h` | | Not started |
| 14 | `LpsSaNvmCalOnTheFlyDataChannelOutput` | `LpsNvmOnTheFlyDumpChanOut` | Output | `interfaces/LpsSaNvmCalOnTheFlyDataChannel/InterfaceTypes.h` | | Not started |
| 15 | `SystemHardwareHealthRequestOutput` | `SystemHardwareHealthRequestOutput_` | Output | `ais/interfaces/SystemHardwareHealthRequest/InterfaceTypes.h` | `SystemHardwareHealthRequest.msg` | Done -- genuinely empty struct (pure trigger/ping); different binding pattern (`SCSOutData<T>` + `initPublishInterface()`/`send()` at `LpsSaScs.cpp:581`, not `InterfaceDb::bind()`+plain `publish()`) flagged for Step 5, see the `.msg` header |

**Adv-only (16th, separate file):** `TipoffModelTestPointsOutput`, bound
and published in `adv/TipoffAssist.cpp` (Development-Plan.txt 5.3) --
not yet located/traced.

## Status

3 of 15 remaining channels done (1 PrinterCnfgInput, 2 SystemHardwareHealthInput,
15 SystemHardwareHealthRequestOutput -- 11 `.msg` files total). Next up:
channel 3, `PartNumbersInput`.
