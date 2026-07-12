# LpsSaJobMgrApp — Architecture and Data Flow Reference

## Overview

`LpsSaJobMgrApp` is the **Job Manager** application running on the Caterpillar CPM-Loader ECU. It sits between the UI/Display and the weighing system (`LpsSaWeighApp`), acting as the control and state machine layer for pass tracking, operator requests, tip-off management, and NVM persistence.

**Primary role:** Accept operator commands, maintain payload pass count and truck totals, coordinate with WeighApp, and publish UI display data.

**Cycle rate:** 10 Hz (100 ms per executive cycle)

---

## Architecture — Three Layers

```
┌─────────────────────────────────────────────────────────────────────┐
│  APPLICATION LAYER           LpsSaJobMgrApp                         │
│                                                                     │
│  initialize()  →  executive()  →  cleanup()                         │
│                                                                     │
│  Owns: request handling, state management, NVM flags, horn control  │
└─────────────────────────────────────────────────────────────────────┘
              │
              │  LpsPtInputs_t / LpsPtOutputs_t
              ▼
┌─────────────────────────────────────────────────────────────────────┐
│  BUSINESS LOGIC LAYER        lps_pass_tracker                       │
│  (External library — not part of JobMgr)                            │
│                                                                     │
│  LpsPtInit(), LpsPtUpdate(), weigh_mode()                           │
│                                                                     │
│  State machine: IDLE → DIG → WEIGH → DUMP → STORE → IDLE           │
└─────────────────────────────────────────────────────────────────────┘
              │
              │  SCS channels (InputInterface / OutputInterface)
              ▼
┌─────────────────────────────────────────────────────────────────────┐
│  COMMUNICATION LAYER         SCS Adapters                           │
│                                                                     │
│  LpsSaJobMgrScsRx()  — all input reads                             │
│  LpsSaJobMgrScsTx()  — all output publishes                        │
│  LpsSaJobMgrSendCmdToWeighApp()  — WeighApp commands               │
│  LpsSaJobMgrScsChkHornAction()  — hardware output                  │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Lifecycle

### initialize()

Called once at ECU startup, after NVM is loaded.

| Step | Function | Purpose |
|------|----------|---------|
| 1 | `getTaskConfig()` | Read `.rb` config: `cycleRate_hz`, `DefTargetWeight`, `DefTipOffTriggerType` |
| 2 | `commInitialize()` | Start OEL middleware layer |
| 3 | `NvmInitialize()` | Block until NVM read flags are set |
| 4 | `LpsSaJobMgrPtInit()` | Initialize pass tracker library (`LpsPtInit`) |
| 5 | `InterfaceDb::fetch()` | Acquire pointers to all 10 SCS channels |

**NVM loaded before `initialize()`:**

`app_nvm_jobmgr_machine_spec_config_file_init()`:
- `TargetWeight`
- `TipOffMode` (truck / pile / manual)
- `HornStoreState`

`app_nvm_jobmgr_live_values_file_init()`:
- `PassCount`
- `TruckWeight`
- `BestBktWt`
- `LifetimeTruckLoadCount`
- `LifetimeTotalPayload`

---

### executive() — 10 Hz Loop

The executive is called every 100 ms by the AIS framework. It runs six ordered phases every cycle:

#### Phase 1 — `LpsSaJobMgrScsRx()` : Read All Inputs

Three separate read functions run inside `ScsRx()`:

**`LpsSaJobMgrScsSHMRead()`**
- `ShmClockInputScs->get()` → `TimezoneOffset`, `DSTOffset`

**`LpsSaWeighScsTxParamRead()`**
- `LpsSaWeighScsTxIn->get(rxParam)`:

  | Field | Meaning |
  |-------|---------|
  | `DigStat` | Bucket is in active dig zone |
  | `CalStat` | Current calibration state code |
  | `DumpStat` | Dump detected (bucket tipped) |
  | `BestBktWtInTonnes` | Best available bucket weight from weighing library |
  | `ZeroNotifyStat` | Weighing library is requesting a zero adjust |
  | `PayloadCalcMeth` | Which calculation method was used for this payload |
  | `Indicator` | Weigh indicator (in-range / out-of-range) |
  | `SimpleCalAdjustment` | Simple calibration correction factor |

**`LpsSaJobMgrScsChkForReqst()`**
- `SwitchInputScs->get()` — physical store button STG4: `OPEN` / `CLOSED` / `UNKNOWN`
- `LpsSaJobMgrScsReqstIn->get(reqIn)`:

  | Request Field | Triggered By |
  |--------------|--------------|
  | `ZeroReqst` | Operator zero button |
  | `MinusOneReqst` | Minus-one correction |
  | `ClearReqst` | Clear pass count |
  | `StoreReqst` | Manual store (UI button) |
  | `StandByActReqst` / `StandByDeactReqst` | Standby toggle |
  | `TruckTipOffReqst` / `PileTipOffReqst` | Tip-off mode select |
  | `ManualTipOffActReqst` / `ManualTipOffDeactReqst` | Manual tip-off |
  | `ReweighReqst` | Request reweigh |
  | `TargetWeightReqst` | Set new target weight |
  | `TipOffTrigger` / `TipOffState` | Tip-off state override |
  | `LifetimeTotalsReqst` | Reset lifetime totals |
  | `HornStoreStateReqst` | Enable/disable horn on store |

**`AisJhmDataServerTxRead()`**
- `AisJhm2TxInputScs->get()` → `TruckWeight`, `Timestamp` (purge stale simple cal data from data server)

---

#### Phase 2 — `LpsSaJobMgrPtUpdate()` : Business Logic

Calls the external `lps_pass_tracker` library:

```
weigh_mode(LpsPtInputs_t)  →  LpsPtOutputs_t
```

**Inputs (`LpsPtInputs_t`):**

| Field | Source |
|-------|--------|
| `DigStat` | WeighApp Tx channel |
| `DumpStat` | WeighApp Tx channel |
| `BestBktWt` | WeighApp Tx channel |
| `CalStat` | WeighApp Tx channel |
| `ZeroNotifyStat` | WeighApp Tx channel |
| `Indicator` | WeighApp Tx channel |
| `StoreRequest` | UI reqst channel or STG4 button |
| `ZeroRequest` | UI reqst channel |
| `MinusOneRequest` | UI reqst channel |
| `ClearRequest` | UI reqst channel |
| `TargetWeight` | NVM or UI override |
| `StandbyState` | Current maintained state |
| `TipOffState` | Current maintained state |

**Outputs (`LpsPtOutputs_t`):**

| Field | Effect |
|-------|--------|
| `PassCount` | Incremented each confirmed store |
| `TruckWeight` | Accumulated per-truck total |
| `DispBestBktWt` | Display-ready bucket weight |
| `store_flag` | Triggers NVM write + lifetime update |
| `clear_flag` | Resets pass count + truck weight |
| `unlatch_flag` | Signals WeighApp to reset `BestBktWt` |
| `tipoff_state` | Resolved tip-off mode |
| `standby_state` | Resolved standby state |

---

#### Phase 3 — `LpsSaJobMgrSendCmdToWeighApp()` : Command WeighApp

Publishes to `LpsSaWeighScsCmdOut` based on conditions set during Rx and Pt phases:

| Command | Condition |
|---------|-----------|
| `ZeroReqst` | `ZeroReqst` received from UI |
| `BestBktRstReqst` | `unlatch_flag` set by pass tracker (after store) |
| `CaptCylExtRef` | `DumpStat` rising edge detected |
| `ClearReweighWarnReqst` | `ClearReqst` or `ReweighReqst` received |

---

#### Phase 4 — TipOff + Horn

`LpsSaJobMgrHandleTipOffBtnStates()`: Resolves the active tip-off mode (truck / pile / manual / none) from queued requests and current state.

`LpsSaJobMgrScsChkHornAction()`: Publishes to `OutputChannelOut` (hardware relay `PORT_SINK_3`) if:
- `HornStoreEnable == true` AND
- `PassCount < 50` (short audible feedback on successful store)

---

#### Phase 5 — Store + Simple Cal

Runs only when `store_flag` is set by the pass tracker:

1. Increment `LifetimeTruckLoadCount`
2. Accumulate `LifetimeTotalPayload`
3. Set NVM write flags (live values file)
4. `LpsSaJobMgrAddSimpleCalTruckData()` — push `TruckWeight` + `Timestamp` onto `SimpleCalData` deque (published to UI for manual calibration reference)

---

#### Phase 6 — `LpsSaJobMgrScsTx()` : Publish All Outputs

Three channels published every cycle:

**`LpsSaJobMgrScsTxOut`** (to UI/Display):

| Field | Description |
|-------|-------------|
| `PassCount` | Current number of confirmed stores this truck |
| `TruckWeight` | Accumulated payload this truck |
| `DispBestBktWt` | Display-ready current bucket weight |
| `TargetWeight` | Current target (from NVM or UI override) |
| `TipOffState` | Active tip-off mode |
| `ManualTipOffState` | Manual tip-off active flag |
| `StandbyState` | Current standby state |
| `LifetimeTruckLoadCount` | Total trucks since lifetime reset |
| `LifetimeTotalPayload` | Total payload since lifetime reset |
| `HornStoreState` | Horn-on-store enabled/disabled |
| `ClearMinusOneEnableStat` | Whether clear/minus-one is valid now |
| `OperationMode` | Current operating mode |
| `SimpleCalData` | Deque of recent truck weights + timestamps |

**`LpsSaJobMgrRespOut`** (to UI): Response code (ACK/NAK/status) for the last UI request.

**`LpsSaJobMgrDebugOut`** (to diagnostics): Full internal state snapshot.

---

### cleanup()

Called at ECU shutdown:
1. Check Machine Serial Number (MSN) — if machine MSN has changed, reset NVM to defaults (prevents wrong calibration data on machine swap)
2. Final NVM flush
3. `commCleanup()`

---

## Channel Interface Reference

### Inputs

| Channel | Direction | Publisher | Key Fields |
|---------|-----------|-----------|------------|
| `LpsSaJobMgrReqstChannelInput` | UI → JobMgr | UI/Display | 13 request types (see Phase 1) |
| `LpsSaWeighTxChannelInput` | WeighApp → JobMgr | WeighApp | DigStat, CalStat, DumpStat, BestBktWt, ZeroNotifyStat, PayloadCalcMeth, Indicator, SimpleCalAdj |
| `LpsSaWeighRespChannelInput` | WeighApp → JobMgr | WeighApp | Response code for last WeighApp command |
| `SwitchInputScsInput` | Hardware → JobMgr | STG4 | OPEN / CLOSED / UNKNOWN |
| `AisJhm2TxChannelInput` | DataServer → JobMgr | Data server | TruckWeight, Timestamp |
| `ShmClockInput` | OS → JobMgr | System | TimezoneOffset, DSTOffset |

### Outputs

| Channel | Direction | Subscriber | Key Fields |
|---------|-----------|------------|------------|
| `LpsSaJobMgrTxChannelOutput` | JobMgr → UI | UI/Display | PassCount, TruckWeight, DispBestBktWt, TargetWeight, TipOffState, StandbyState, LifetimeTotals, HornStoreState, SimpleCalData |
| `LpsSaJobMgrRespChannelOutput` | JobMgr → UI | UI/Display | Response code |
| `LpsSaWeighReqstChannelOutput` | JobMgr → WeighApp | WeighApp | ZeroReqst, BestBktRstReqst, CaptCylExtRef, ClearReweighWarnReqst |
| `OutputChannelOutput` | JobMgr → Hardware | PORT_SINK_3 | Horn relay command |
| `LpsSaJobMgrDebugChannelOutput` | JobMgr → Diagnostics | Service tool | Internal state snapshot |

---

## Request Handling

All requests arrive via `LpsSaJobMgrReqstChannelInput`. Processing is synchronous within the executive cycle.

| Request | JobMgr Local Action | WeighApp Command Sent |
|---------|--------------------|-----------------------|
| `ZeroReqst` | None (pass-through) | `ZeroReqst` |
| `MinusOneReqst` | Decrement `PassCount` in pass tracker | None |
| `ClearReqst` | `clear_flag` → reset PassCount + TruckWeight | `ClearReweighWarnReqst` |
| `StoreReqst` | `store_flag` path | None (handled by pt library) |
| `StandByActReqst` | Set `standby_state = ACTIVE` | None |
| `StandByDeactReqst` | Set `standby_state = INACTIVE` | None |
| `TruckTipOffReqst` | Set `tipoff_state = TRUCK` | None |
| `PileTipOffReqst` | Set `tipoff_state = PILE` | None |
| `ManualTipOffActReqst` | Set `manual_tipoff = ACTIVE` | None |
| `ManualTipOffDeactReqst` | Set `manual_tipoff = INACTIVE` | None |
| `ReweighReqst` | Reset best bucket state | `ClearReweighWarnReqst` |
| `TargetWeightReqst` | Update `TargetWeight` + set NVM write flag | None |
| `LifetimeTotalsReqst` | Reset lifetime counters + set NVM write flag | None |
| `HornStoreStateReqst` | Toggle `HornStoreEnable` + set NVM write flag | None |

Physical store button (STG4 `CLOSED`) is treated identically to `StoreReqst` from UI.

---

## Pass Tracker — weigh_mode() State Machine

The `lps_pass_tracker` library manages the core payload accumulation state machine. JobMgr wraps it — JobMgr owns the I/O and NVM; the library owns the weighing logic.

```
IDLE
  │
  │  DigStat becomes true
  ▼
DIG
  │
  │  Bucket leaves dig zone
  ▼
WEIGH    ←── BestBktWt updated by weighing library
  │
  │  DumpStat detected
  ▼
DUMP
  │
  │  StoreReqst received (UI or STG4)
  ▼
STORE    ←── PassCount++, TruckWeight += BestBktWt, store_flag = true
  │
  │  unlatch_flag → send BestBktRstReqst to WeighApp
  ▼
IDLE
```

`MinusOneReqst` decrements `PassCount` and subtracts the last bucket weight from `TruckWeight` at any state.

`ClearReqst` resets `PassCount = 0` and `TruckWeight = 0`.

---

## WeighApp Coordination

JobMgr and WeighApp are **peer applications** on the same ECU, communicating exclusively through SCS channels. They do not call each other's functions.

```
UI  ──[ZeroReqst]──►  JobMgr  ──[ZeroReqst]──►  WeighApp
                                                  │
WeighApp  ──[BestBktWt, DigStat, DumpStat]──►  JobMgr
                                                  │
JobMgr  ──[BestBktRstReqst]──►  WeighApp   (after confirmed store)
```

**Coordination contract:**

| Event | JobMgr sends | WeighApp responds |
|-------|-------------|-------------------|
| Operator presses Zero | `ZeroReqst` | Performs zero adjust, publishes updated `BestBktWt` |
| Bucket dumped (DumpStat) | `CaptCylExtRef` | Captures cylinder extension reference for next dig |
| Pass confirmed (store_flag) | `BestBktRstReqst` | Resets `BestBktWt` to zero for next bucket |
| Operator requests reweigh | `ClearReweighWarnReqst` | Clears reweigh warning state |

---

## Horn Control

`LpsSaJobMgrScsChkHornAction()` fires the horn relay (`OutputChannelOutput → PORT_SINK_3`) when:

```
HornStoreEnable == true
AND
PassCount < 50
```

`HornStoreEnable` is an NVM-persisted operator preference (toggled via `HornStoreStateReqst`). The PassCount < 50 check prevents horn blasts when the truck is near full — avoids nuisance alerts during cleanup passes.

---

## Simple Calibration Tracking

Each time a store is confirmed, `LpsSaJobMgrAddSimpleCalTruckData()` appends an entry to the `SimpleCalData` deque:

```
SimpleCalEntry {
    TruckWeight,    // confirmed payload for this truck
    Timestamp       // local time including timezone/DST offsets from ShmClock
}
```

This deque is published to the UI every cycle via `LpsSaJobMgrTxChannelOutput.SimpleCalData`. It provides field technicians with a recent history of truck weights for manual simple-calibration reference, without requiring a service tool connection.

Data server truck weights (`AisJhm2TxChannelInput`) are read and purged each cycle — stale entries are discarded before being appended to the deque.

---

## NVM Management

JobMgr uses two separate NVM files managed by a **dedicated RTOS task** (`task_1x()`), which runs on an independent infinite loop separate from the 10 Hz executive:

### Machine Config File

**Persisted fields:** `TargetWeight`, `TipOffMode`, `HornStoreState`

**Write trigger:** Set by request handler when operator changes any of these settings. `task_1x()` detects the write flag and calls `app_nvm_jobmgr_machine_spec_config_file_write()`.

### Live Values File

**Persisted fields:** `PassCount`, `TruckWeight`, `BestBktWt`, `LifetimeTruckLoadCount`, `LifetimeTotalPayload`

**Write trigger:** Set by `LpsSaJobMgrProcessStoreRequest()` after each confirmed store, and by `ClearReqst` / `LifetimeTotalsReqst`. `task_1x()` calls `app_nvm_jobmgr_live_values_file_write()`.

### MSN Check (cleanup)

At shutdown, `cleanup()` compares the stored Machine Serial Number against the current ECU's MSN. If the machine has changed (ECU swapped to different loader), NVM is reset to defaults. This prevents calibration and total data from contaminating a different machine's records.

---

## Configuration Parameters (.rb)

The `.rb` config file (`config/LpsSaJobMgrApp.rb`) defines:

| Parameter | Typical Value | Role |
|-----------|--------------|------|
| `cycleRate_hz` | `10.0` | AIS framework executive rate |
| `DefTargetWeight` | Machine-specific | Default target payload weight (tonnes) |
| `DefTipOffTriggerType` | `TRUCK` or `PILE` | Default tip-off mode on fresh NVM |
| Channel names | Various | SCS channel binding for all inputs/outputs |
| Queue sizes | 5–20 | SCS FIFO depth per channel |

---

## Data Flow Summary

```
ShmClock ──────────────────────────────────────────────────────────►┐
DataServer(AisJhm2Tx) ─────────────────────────────────────────────►│
Physical switch (STG4) ─────────────────────────────────────────────►│
UI Requests ────────────────────────────────────────────────────────►│
                                                                     │
WeighApp (BestBktWt, DigStat, DumpStat, CalStat) ──────────────────►│
                                                                     │
                                                ┌────────────────────┘
                                                │
                                                ▼
                              LpsSaJobMgrScsRx()   [collect all inputs]
                                                │
                                                ▼
                              weigh_mode(LpsPtInputs_t)   [lps_pass_tracker]
                                                │
                                                ▼
                              LpsPtOutputs_t   [PassCount, TruckWeight, flags]
                                                │
                               ┌────────────────┼───────────────────┐
                               ▼                ▼                   ▼
                         Send cmd to       Horn relay           NVM flags
                          WeighApp         if needed            if store
                               │
                               ▼
                      LpsSaJobMgrScsTx()   [publish all outputs]
                               │
                               │
               ┌───────────────┼───────────────────┐
               ▼               ▼                   ▼
          UI/Display        WeighApp           Diagnostics
          (Tx channel)      (Cmd channel)      (Debug channel)
```

---

## KPIT Documentation Assessment

The KPIT team's draft documentation is accurate at a conceptual level and provides a solid orientation to the architecture. The following assessment identifies what was correct, what was missing, and one area that needs clarification.

### Correct

- Three-layer architecture (Application → Business Logic → Communication) — correctly described
- Initialize → Rx → Process → Tx execution pattern — correct
- SCS API model (`fetch`, `get`, `publish`) — correctly mapped to ROS2 equivalents
- Primary control flow: UI → JobMgr → WeighApp — correct
- Business logic isolation in `lps_pass_tracker` library — correct
- 10 Hz cycle rate — correct

### Missing from KPIT Documentation

| Missing Item | Impact |
|-------------|--------|
| **NVM task** (`task_1x()`) — separate RTOS thread for persistence | Porting to ROS2 requires a separate thread or timer node for NVM writes |
| **`ShmClockInput`** — timezone and DST from system clock | Timestamps in `SimpleCalData` use this; must be sourced in ROS2 port |
| **`OutputChannelOutput`** — horn relay (hardware output) | Horn-on-store feature is undocumented; requires a hardware output interface in ROS2 |
| **`SimpleCalData` deque** — queue of truck weights + timestamps | A non-trivial output field that the UI depends on for manual calibration |
| **Horn-on-store logic** — `HornStoreEnable + PassCount < 50` | Behavioural requirement invisible from KPIT doc |
| **`cleanup()` MSN check** — reset NVM on machine swap | Critical for machine safety on ECU transplant scenarios |
| **`LpsSaJobMgrDebugChannelOutput`** — internal diagnostics output | Service tool visibility |
| **Specific channel payload fields** — KPIT names channels but not their data structures | Required for implementing the ROS2 message types |

### Needs Clarification

- **Section 6.1.3** describes `LpsSaJobMgrReqstChannel` as bidirectional (WeighApp → JobMgr for zero requests). WeighApp does have `LpsSaJobMgrReqstChannelOutput` in its `.rb` config, meaning it can publish to this channel. However, this is distinct from the UI path. In the KPIT document the characterization as "WeighApp sends zero requests to JobMgr" is misleading — zero requests originate from the operator (UI/button), not from WeighApp. WeighApp only receives zero commands, it does not initiate them.

### Verdict

KPIT documentation is approximately **75% accurate** as an architectural overview. It correctly communicates the structural concept and execution model, and is a good starting point for new engineers. The main gaps are: NVM architecture, secondary outputs (Horn, Debug), the `SimpleCalData` feature, cleanup behaviour, and field-level channel payload definitions. The MD file and `jobmgr.txt` diagram in this directory provide the missing detail.
