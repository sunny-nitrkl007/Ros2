# Mining Load Cycle — ROS2 V3 Demo

## Overview

This project simulates a **continuous mining load cycle** between a hydraulic excavator (like a CAT 395) and a haul truck (like a CAT 777 or 785). The excavator digs, swings, and dumps material into the truck. The truck drives to a dump zone, empties, and returns. A fleet supervisor orchestrates the whole cycle and logs productivity metrics.

The simulation demonstrates:
- A 4-state excavator arm state machine (IDLE → DIGGING → RAISING → DUMPING)
- Truck navigation between GPS waypoints with sub-2 m arrival tolerance
- Proximity-based safety interlock: excavator does not move until the truck is in position
- Cross-ECU service calls (arm signals dispatcher; dispatcher commands truck; dispatcher calls fleet emergency stop)
- Cycle counting and tonne-kilometre productivity logging
- Fleet health telemetry aggregated from multiple machines

Everything is YAML-driven. Config files declare all topics, services, QoS policies, parameters, and the discovery server. The generator (`tool/generator/generator.py`) produces all C++ boilerplate — you only write business logic inside impl blocks.

---

## System Architecture

### Logical ECU Grouping

The 8 nodes are logically grouped into 3 ECUs (containers in a future multi-container deployment):

```
┌─────────────────────────────────────────────────────────────────────────┐
│  EXCAVATOR ECU                                                          │
│                                                                         │
│  ┌──────────────┐   dig_command (Vector3)   ┌────────────────────┐     │
│  │  arm_planner │ ─────────────────────────► │ bucket_controller  │     │
│  │  (5 Hz FSM)  │                            │  (event-driven)    │     │
│  └──────┬───────┘                            └────────┬───────────┘     │
│         │ calls /mining/fleet/request_load            │ serves          │
│         │ when bucket raised                          │ /mining/excavator│
│         │                                             │ /check_payload   │
│         ▲ proximity_alert (Bool)                      │                 │
│  ┌──────────────────┐  truck_pose  ┌──────────────┐  │ bucket_state    │
│  │proximity_detector│ ◄─────────── │truck_navigator│  │ (Vector3)       │
│  │    (10 Hz check) │              │  (10 Hz nav)  │  │                 │
│  └──────────────────┘              └──────▲────────┘  │                 │
│                                           │           │                 │
├───────────────────────────────────────────┼───────────┼─────────────────┤
│  TRUCK ECU                                │           │                 │
│                              waypoint_command         │                 │
│  (truck_navigator publishes truck_pose,   │           │                 │
│   payload_monitor receives bucket_state) ─┘           ▼                 │
│                                           ┌────────────────────┐        │
│                                           │  payload_monitor   │        │
│                                           │  (event-driven)    │        │
│                                           └────────┬───────────┘        │
│                                                    │ payload_status      │
│                                                    │ (Vector3)           │
├────────────────────────────────────────────────────┼────────────────────┤
│  FLEET ECU                                         │                    │
│                                                    ▼                    │
│  ┌───────────────┐  waypoint_command ┌──────────────────────────────┐  │
│  │load_dispatcher│ ─────────────────► (truck_navigator subscribes)   │  │
│  │  (2 Hz FSM)   │ ◄──────────────── payload_status, proximity_alert │  │
│  └───────┬───────┘  load_assignment  └──────────────────────────────┘  │
│          │ serves /mining/fleet/request_load                            │
│          │                                                              │
│  ┌───────────────┐   health_telemetry  ┌────────────────────────────┐  │
│  │ fleet_monitor │ ──────────────────► │       site_logger          │  │
│  │ (1 Hz status) │   serves emergency  │  (0.5 Hz cycle reporting)  │  │
│  └───────────────┘        stop         └────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────┘
```

All nodes run on **Domain ID 40** and connect to a FastDDS Discovery Server at `127.0.0.1:11811` in SUPER_CLIENT mode.

---

## Load Cycle — Full Workflow

One complete load cycle follows this sequence:

```
PHASE 1 — POSITIONING
  load_dispatcher (POSITIONING state)
  │   publishes waypoint_command → dig_zone (5, 0)
  │
  truck_navigator receives waypoint, drives to (5, 0) at 8 m/s
  │
  proximity_detector computes Euclidean distance from truck to excavator origin (0, 0)
  │   When dist ≤ 12 m → publishes proximity_alert = true
  │
  load_dispatcher transitions POSITIONING → WAITING

PHASE 2 — WAITING (truck parked at dig zone)
  arm_planner receives proximity_alert = true
  │   Transitions arm_state_ IDLE → DIGGING
  │
  dig_cycle (5 Hz FSM):
    DIGGING: lowers bucket 30 ticks (~6 s), angle sweeps −90° → −30°
    RAISING: lifts bucket (angle returns to −85°), calls /mining/fleet/request_load
  │
  load_dispatcher on_request_load service callback → sets load_requested_ = true
  │   Transitions WAITING → LOADING

PHASE 3 — LOADING (excavator dumps into truck)
  arm_planner transitions RAISING → DUMPING
  │   Swings over truck bed (angle → −20°), holds 20 ticks (~4 s)
  │
  bucket_controller on_dig_command:
    Smoothly tracks arm angle
    While is_digging_ = true: accumulates fill_rate (30 kg/s)
    Publishes bucket_state (x=angle, y=load_pct, z=depth) on every callback
  │
  payload_monitor on_bucket_state:
    Detects dump event (load_pct was ≥ 0.80, now < 0.10 after swing)
    Adds ~25,500 kg (last_load_pct × 30,000 kg bucket) to total_payload_kg_
    Publishes payload_status (x=total_kg, y=target_kg, z=load_pct)
  │
  load_dispatcher on_payload_status:
    When payload_status.z ≥ 1.0 (truck full) → payload_full_ = true
    Transitions LOADING → DISPATCHING

PHASE 4 — DISPATCHING (truck departs to dump zone)
  load_dispatcher:
    Publishes waypoint_command → dump_zone (150, 0)
    Publishes load_assignment "CYCLE_COMPLETE count=N" when truck leaves dig zone
  │
  truck_navigator drives to (150, 0), arrives, publishes updated truck_pose
  │
  proximity_alert drops to false (truck > 12 m away)
  payload_monitor: truck is emptied at dump → payload resets
  │
  site_logger on_payload_status detects reset (last_pct ≥ 0.95, now < 0.10)
  │   Increments cycle_count_, total_tonnes_ += target_payload_kg / 1000
  │
  load_dispatcher: !truck_at_zone_ && payload_full_ → cycles_complete_++
  │   Resets payload_full_, transitions DISPATCHING → POSITIONING
  │
  [cycle repeats]
```

---

## Config File Deep-Dive

### `config/application.yaml`

```yaml
application:
  name: mining_load_cycle
  domain_id: 40
  discovery_profile: mining_super_client
  nodes:
    - arm_planner
    - bucket_controller
    - proximity_detector
    - truck_navigator
    - payload_monitor
    - load_dispatcher
    - fleet_monitor
    - site_logger
```

- `domain_id: 40` — isolates this project from domain 20 (ADAS) and domain 30 (haul truck)
- `discovery_profile: mining_super_client` — all 8 nodes use the same DS endpoint
- `nodes` ordering matches logical execution flow (excavator ECU first, fleet last)

---

### `config/data.yaml`

```yaml
data:
  messages:
    truck_pose:       geometry_msgs/msg/PoseStamped  → /mining/truck/truck_pose
    dig_command:      geometry_msgs/msg/Vector3       → /mining/excavator/dig_command
    bucket_state:     geometry_msgs/msg/Vector3       → /mining/excavator/bucket_state
    proximity_alert:  std_msgs/msg/Bool               → /mining/excavator/proximity_alert
    payload_status:   geometry_msgs/msg/Vector3       → /mining/truck/payload_status
    waypoint_command: geometry_msgs/msg/PoseStamped   → /mining/fleet/waypoint_command
    load_assignment:  std_msgs/msg/String              → /mining/fleet/load_assignment
    health_telemetry: diagnostic_msgs/msg/DiagnosticArray → /mining/fleet/health_telemetry
    cycle_report:     std_msgs/msg/String              → /mining/fleet/cycle_report

  services:
    check_payload:  std_srvs/srv/Trigger → /mining/excavator/check_payload
    request_load:   std_srvs/srv/Trigger → /mining/fleet/request_load
    emergency_stop: std_srvs/srv/Trigger → /mining/fleet/emergency_stop
```

**Topic namespace design:**

| Namespace | ECU | Data |
|---|---|---|
| `/mining/excavator/` | Excavator ECU | dig commands, bucket state, proximity alert |
| `/mining/truck/` | Truck ECU | truck position, payload status |
| `/mining/fleet/` | Fleet ECU | waypoints, load assignments, telemetry, reports |

**How Vector3 is used as a custom struct:**

ROS2's `geometry_msgs/msg/Vector3` carries three doubles (x, y, z). This project repurposes them as lightweight structs:

| Topic | x | y | z |
|---|---|---|---|
| `dig_command` | Target arm angle (deg) | Dig depth (m) | Swing speed (rad/s) |
| `bucket_state` | Current arm angle (deg) | Load percentage (0–1) | Current depth (m) |
| `payload_status` | Current truck payload (kg) | Target payload (kg) | Load fraction (0–1) |

This avoids defining custom `.msg` files while keeping the simulation self-contained. A production system would define proper typed messages.

**Service design:**

| Service | Server | Client | Purpose |
|---|---|---|---|
| `/mining/excavator/check_payload` | `bucket_controller` | `payload_monitor` | Payload monitor asks: "is the bucket full?" Bucket controller responds and resets load on confirmation |
| `/mining/fleet/request_load` | `load_dispatcher` | `arm_planner` | Arm signals dispatcher: "I have a bucket ready to dump — approve?" Allows dispatcher to gate the loading phase |
| `/mining/fleet/emergency_stop` | `fleet_monitor` | `load_dispatcher` | Dispatcher calls fleet-wide halt; fleet_monitor sets emergency_active_ and returns ack |

---

### `config/qos_profiles.yaml`

```yaml
qos_profiles:
  reliable_qos:
    history: keep_last
    depth: 10
    reliability: reliable
    deadline_ms: 200

  cycle_qos:
    history: keep_last
    depth: 5
    reliability: reliable
    deadline_ms: 500

  fleet_qos:
    history: keep_last
    depth: 10
    reliability: reliable
    deadline_ms: 1000
```

**Why no `best_effort` or lifespan in mining?**

The haul truck project used `best_effort` for GPS (20 Hz — fresh readings always available). Mining operates differently:
- Arm angle changes are slow (mechanical motion), so a missed `bucket_state` matters — `reliable` is correct
- Cycle events (waypoints, load assignments) happen once every few minutes — a dropped waypoint command means the truck sits idle
- All profiles are `reliable` with graduated deadlines: 200 ms for real-time control paths, 500 ms for cycle data, 1000 ms for fleet aggregation

| Profile | Used for | Deadline rationale |
|---|---|---|
| `reliable_qos` | Truck pose, dig commands, proximity alert, waypoints | 200 ms matches ~5–10 Hz publication; ensures no missed position |
| `cycle_qos` | Bucket state, payload status | 500 ms matches 5 Hz arm cycle and event-driven callbacks |
| `fleet_qos` | Load assignment, health telemetry, cycle report | 1000 ms matches 1–2 Hz fleet aggregation; low-frequency data |

---

### `config/discovery_profiles.yaml`

```yaml
discovery_profiles:
  mining_super_client:
    mode: SUPER_CLIENT
    servers:
      - ip: 127.0.0.1
        port: 11811
```

Same SUPER_CLIENT architecture as the other V3 demos. All 8 nodes register with the DS at startup. The DS maintains the full participant roster so nodes don't need LAN multicast. This is critical in mining environments where Wi-Fi is managed and multicast is typically blocked.

---

### `config/parameters.yaml`

| Node | Parameter | Value | Meaning |
|---|---|---|---|
| `arm_planner` | `dig_rate` | 5.0 Hz | State machine evaluation rate |
| | `dig_depth` | 2.5 m | How deep the bucket penetrates the material |
| | `swing_speed` | 0.3 rad/s | Arm angular velocity |
| `bucket_controller` | `max_load_kg` | 250.0 kg | Maximum bucket capacity (demo scale; real 395: ~26,000 kg) |
| | `fill_rate` | 30.0 kg/s | Fill accumulation rate while digging |
| | `dump_threshold` | 0.85 | Bucket considered full at 85 % capacity |
| `proximity_detector` | `proximity_radius` | 12.0 m | Truck must be within 12 m of excavator to trigger alert |
| | `excavator_x/y` | 0.0, 0.0 | Excavator's fixed world position |
| `truck_navigator` | `max_speed` | 8.0 m/s | ~29 km/h (appropriate for a 785 in a tight loading area) |
| | `position_tolerance` | 2.0 m | Within 2 m of waypoint = arrived |
| `payload_monitor` | `target_payload_kg` | 200.0 kg | Target truck capacity (demo scale) |
| `load_dispatcher` | `dig_zone_x/y` | 5.0, 0.0 | Where the truck parks to be loaded |
| | `dump_zone_x/y` | 150.0, 0.0 | Where the truck dumps its load (150 m away) |
| `fleet_monitor` | `monitor_rate` | 1.0 Hz | Health telemetry publish rate |
| `site_logger` | `log_rate` | 0.5 Hz | Cycle report publish rate (every 2 s) |

---

### `config/contracts/<node>.yaml`

Each contract file specifies one node's behavioural interface:

**arm_planner** has two callback groups:
- `arm_group` (mutually exclusive): protects the state machine variables from concurrent writes
- `service_group` (reentrant): the async `request_load` response callback must not wait for `arm_group` to complete

**load_dispatcher** similarly has two groups:
- `dispatch_group` (mutually exclusive): protects `cycle_state_`, `truck_at_zone_`, `payload_full_`
- `service_group` (reentrant): the `on_request_load` service server must respond immediately even when `dispatch_group` is running the `cycle_tick`

All other nodes are single-group or single-threaded — they have no concurrent callback interaction.

---

## Node Reference

### `arm_planner` — Excavator Arm (Excavator ECU)

**Real-world role:** Plans and executes the excavator arm movement sequence, like the swing/dig/lift/dump cycle of a CAT 395 with Grade with Assist technology.

**Contract:** multi-threaded (2), subscribes proximity_alert, publishes dig_command, client of request_load, timer 5 Hz

**State machine (`dig_cycle` at 5 Hz):**

```
State 0 — IDLE
  Holds arm at −90° (resting position)
  Waits for truck_ready_ = true (set by on_proximity_alert)
  On truck arrival → IDLE → DIGGING

State 1 — DIGGING
  dig_tick_ increments each cycle
  Angle sweeps: −90° + 60° × min(1, tick/25) → reaches −30° at tick 25
  Sends dig_depth_ (2.5 m) and swing_speed_ to bucket_controller
  After 30 ticks (~6 s) → DIGGING → RAISING

State 2 — RAISING
  Decrements current_angle_ by 3°/tick (lifts bucket)
  When angle ≤ −85° (near vertical):
    Calls /mining/fleet/request_load service asynchronously
    → RAISING → DUMPING

State 3 — DUMPING
  Holds arm at −20° (swung over truck bed)
  dig_depth_ = 0 (no digging)
  After 20 ticks (~4 s):
    Resets truck_ready_ = false, current_angle_ = −90°
    → DUMPING → IDLE
```

---

### `bucket_controller` — Bucket Mechanics (Excavator ECU)

**Real-world role:** Controls bucket angle, tracks material accumulation, and responds to payload verification requests — analogous to the CAT Grade with Assist bucket angle control.

**Contract:** single-threaded, subscribes dig_command, publishes bucket_state, server of check_payload

**Behaviour (`on_dig_command`):**
- Smoothly tracks `target_angle_` using 15 % exponential filter per callback: `current_angle_ += (target − current) × 0.15`
- While `is_digging_` (dig_depth > 0.1): accumulates `fill_rate × 0.2 kg` per callback (~5 Hz → 30 kg/s)
- Stops accumulating when `current_load_kg_` reaches `max_load_kg_`
- Publishes `bucket_state` on every callback: `(current_angle, load_pct, depth)`

**Behaviour (`on_check_payload`):**
- Responds `success=true` when `current_load_kg_ >= dump_threshold × max_load_kg_`
- On full bucket confirmation: **resets `current_load_kg_ = 0`** (simulates dump)
- Message includes current vs max kg

---

### `proximity_detector` — Truck Arrival Sensor (Excavator ECU)

**Real-world role:** Detects when the haul truck has parked in the loading bay, equivalent to the presence sensor or GPS-geofence check on a real excavator-truck loading interlock.

**Contract:** single-threaded, subscribes truck_pose, publishes proximity_alert, timer 10 Hz

**Behaviour:**
- `on_truck_pose`: stores truck world position (x, y)
- `proximity_check` (10 Hz): computes Euclidean distance from truck to `excavator_x_/excavator_y_` (0, 0)
- Publishes `std_msgs/Bool`: `true` when `dist ≤ proximity_radius_` (12 m)
- Edge-triggered logging: logs only when in_range_ changes, not every cycle

---

### `truck_navigator` — Truck Drive System (Truck ECU)

**Real-world role:** Point-to-point navigation between waypoints, like a simplified version of the CAT MineStar Command for Hauling autonomous steering controller.

**Contract:** single-threaded, subscribes waypoint_command, publishes truck_pose, timer 10 Hz

**Behaviour:**
- `on_waypoint_command`: sets new `target_x_/y_`, clears `at_target_`
- `navigation_tick` (10 Hz):
  - If no target or at target: publishes current pose unchanged (truck stationary)
  - Computes vector to target, normalises, steps by `min(max_speed/10, dist)` metres
  - When `dist ≤ position_tolerance_` (2 m): sets `at_target_ = true`, logs "Arrived"
  - Always publishes `truck_pose` — even stationary — so proximity_detector has current data

Initial truck position is (100, 50) — parked away from the excavator zone at startup.

---

### `payload_monitor` — Payload Accounting (Truck ECU)

**Real-world role:** Tracks how much material has been loaded into the truck, equivalent to the CAT Payload Weighing System (PWS) on a 777/785 series.

**Contract:** single-threaded, subscribes bucket_state, publishes payload_status, client of check_payload

**Behaviour (`on_bucket_state`):**
- Tracks `last_load_pct_` (previous bucket fill percentage)
- Dump detection: when `last_load_pct_ ≥ 0.8` and current `load_pct < 0.1` — the bucket was nearly full and just emptied
- On dump: adds `last_load_pct × 30,000 kg` to `total_payload_kg_` (30,000 kg ≈ one full 395 bucket)
- When `total_payload_kg_ ≥ target_payload_kg_`: sets `payload_full_ = true`, calls `/mining/excavator/check_payload` to confirm and reset bucket
- Publishes `payload_status` Vector3: `(total_kg, target_kg, load_fraction)`

---

### `load_dispatcher` — Cycle Orchestrator (Fleet ECU)

**Real-world role:** The intelligence layer that sequences the load cycle — equivalent to the MineStar Command dispatch system that coordinates all machines on site.

**Contract:** multi-threaded (2), subscribes truck_pose + payload_status + proximity_alert, publishes waypoint_command + load_assignment, server of request_load, client of emergency_stop, timer 2 Hz

**State machine (`cycle_tick` at 2 Hz):**

```
State 0 — POSITIONING
  Continuously publishes dig_zone waypoint (5, 0)
  Transition: truck_at_zone_ = true → WAITING

State 1 — WAITING
  Waits for arm_planner to raise bucket and call request_load
  on_request_load handler sets load_requested_ = true
  Transition: load_requested_ → LOADING

State 2 — LOADING
  Passively waits while excavator dumps into truck
  Monitors payload_status via on_payload_status callback
  Transition: payload_full_ = true → DISPATCHING

State 3 — DISPATCHING
  Continuously publishes dump_zone waypoint (150, 0)
  When truck leaves dig zone (!truck_at_zone_) AND payload confirmed full:
    cycles_complete_++
    Publishes load_assignment "CYCLE_COMPLETE count=N"
    Resets payload_full_, cycle_state_ → POSITIONING
```

**Why the dispatcher rather than the arm planner drives the cycle?** The dispatcher has global visibility: it knows truck position, payload level, and proximity state simultaneously. The arm planner only knows if a truck is nearby. Centralising the state machine in the dispatcher keeps the arm_planner and truck_navigator as pure actuator controllers.

---

### `fleet_monitor` — Fleet Health Supervisor (Fleet ECU)

**Real-world role:** Aggregates health data from multiple machines for the dispatcher and site operator, like the MineStar Fleet telemetry dashboard.

**Contract:** multi-threaded (2), subscribes bucket_state + truck_pose + payload_status, publishes health_telemetry, server of emergency_stop, timer 1 Hz

**Behaviour:**
- `on_bucket_state`: stores `last_bucket_load_` (fill percentage)
- `on_truck_pose`: stores `last_truck_x_/y_`
- `on_payload_status`: stores `last_payload_pct_`
- `on_emergency_stop`: sets `emergency_active_ = true`, responds with ack
- `monitor_tick` (1 Hz): publishes a `DiagnosticArray` with three entries:
  1. `excavator.bucket` — OK if > 10 % load and no emergency, WARN otherwise
  2. `truck.position` — truck world coordinates and distance from origin
  3. `truck.payload` — WARN when payload fraction ≥ 1.0 (truck full and not yet dispatched)

---

### `site_logger` — Productivity Logger (Fleet ECU)

**Real-world role:** Records cycle count and total tonnes hauled — equivalent to the CAT MineStar productivity report that fleet managers review at shift end.

**Contract:** single-threaded, subscribes payload_status + health_telemetry, publishes cycle_report, timer 0.5 Hz

**Behaviour:**
- `on_payload_status`: detects cycle completion (payload fraction resets from ≥ 0.95 to < 0.10). Increments `cycle_count_`, adds `target_payload_kg / 1000.0` tonnes to `total_tonnes_`
- `on_health_telemetry`: counts active ERROR-level statuses as `fault_count_`
- `log_tick` (0.5 Hz, every 2 s): publishes `std_msgs/String` cycle_report and logs it:
  ```
  [SITE] cycles=3 hauled=600.0t payload=45% faults=0
  ```

---

## Data Flow — Full Cycle Trace

### Startup (t = 0)

- All 8 nodes register with DS at `127.0.0.1:11811`
- `truck_navigator` starts at (100, 50), publishes pose every 100 ms
- `load_dispatcher` starts in state POSITIONING, publishes waypoint (5, 0)
- `arm_planner` starts in state IDLE, publishes resting angle −90°

### T = 0–17 s — Truck drives to dig zone

- `truck_navigator` receives waypoint (5, 0), moves 0.8 m/tick (8 m/s at 10 Hz)
- Distance from (100, 50) to (5, 0) ≈ 105 m → arrives in ~13 s
- `proximity_detector` publishes `false` until truck is within 12 m of (0, 0)
- `proximity_alert = true` fires when truck reaches ≈ (12, 0)

### T = 17–23 s — Excavation cycle

- `arm_planner` receives `proximity_alert = true`, transitions IDLE → DIGGING
- Over 30 ticks (6 s): arm angle sweeps −90° → −30°, `bucket_controller` accumulates ~180 kg
- `arm_planner` transitions DIGGING → RAISING (lifts bucket), calls `/mining/fleet/request_load`
- `load_dispatcher` receives request, transitions WAITING → LOADING

### T = 23–27 s — Dumping

- `arm_planner` transitions RAISING → DUMPING, swings to −20° over truck bed
- `bucket_controller` publishes `bucket_state.y` drops from 0.72 → 0.0 (dump event)
- `payload_monitor` detects the drop, adds ≈21,600 kg to truck total
- After 4 s, arm_planner resets to IDLE

### T = 27+ s — Repeat until truck full

- More excavation cycles repeat (each ~10 s) until `total_payload_kg ≥ target_payload_kg`
- `payload_monitor` sets `payload_full_ = true` and publishes payload_status.z = 1.0

### Dispatch — Truck drives to dump zone

- `load_dispatcher` transitions LOADING → DISPATCHING, publishes waypoint (150, 0)
- `truck_navigator` drives 155 m to dump zone → arrives in ~19 s
- `proximity_alert` drops to false (truck left)
- `load_dispatcher`: !truck_at_zone_ → `cycles_complete_++`, publishes load_assignment "CYCLE_COMPLETE count=1"
- `site_logger` detects payload reset → `cycle_count_++`, `total_tonnes_ += 0.2`

### Cycle restarts — Dispatcher sends truck back to dig zone

---

## How the YAML Files Relate to Each Other

```
application.yaml
    │
    ├── discovery_profile: mining_super_client ──► discovery_profiles.yaml
    │                                                (Dockerfile ENV vars)
    │
    └── nodes: [arm_planner, bucket_controller, ...]
              │
              └── contracts/<node>.yaml
                        │
                        ├── parameter_ref: ─────────► parameters.yaml
                        │                               (declare_parameter calls)
                        │
                        ├── interactions[].qos_profile: ► qos_profiles.yaml
                        │                                  (rclcpp::QoS settings)
                        │
                        └── interactions[].dataref: ──► data.yaml
                                                         ├── type → #include + template args
                                                         └── topic_path / service_path
```

The generator resolves all references, produces:
- `generated_pkg/include/<node>.hpp` — class declaration, callback signatures, members
- `generated_pkg/src/<node>.cpp` — constructor (parameters, publishers, subscribers, timers), impl block stubs
- `generated_pkg/CMakeLists.txt` — ament targets, dependency resolution
- `generated_pkg/package.xml` — ROS2 package manifest
- `tool/docker/Dockerfile` — Discovery Server environment config

---

## Service Interaction Map

```
arm_planner ──────────────────────────────► /mining/fleet/request_load ──► load_dispatcher
                   (arm raised, ready to dump)         (approve → LOADING state)

payload_monitor ──────────────────────────► /mining/excavator/check_payload ──► bucket_controller
                   (is truck full?)                     (yes/no + reset on confirm)

load_dispatcher ──────────────────────────► /mining/fleet/emergency_stop ──► fleet_monitor
                   (call halt if anomaly)               (emergency_active_ = true)
```

All services use `std_srvs/srv/Trigger` (empty request, `{success, message}` response). In a production system these would be typed services carrying structured data (load ID, machine ID, operator authorisation).

---

## Build and Run

### Generate C++ from YAML

Run from the project directory:

```bash
python3 tool/generator/generator.py --project mining_load_cycle
```

Expected output:
```
OK arm_planner          [1pub  1sub  0srv_server  1srv_client  1timer]
OK bucket_controller    [1pub  1sub  1srv_server  0srv_client  0timer]
OK proximity_detector   [1pub  1sub  0srv_server  0srv_client  1timer]
OK truck_navigator      [1pub  1sub  0srv_server  0srv_client  1timer]
OK payload_monitor      [1pub  1sub  0srv_server  1srv_client  0timer]
OK load_dispatcher      [2pub  3sub  1srv_server  1srv_client  1timer]
OK fleet_monitor        [1pub  3sub  1srv_server  0srv_client  1timer]
OK site_logger          [1pub  2sub  0srv_server  0srv_client  1timer]
OK CMakeLists.txt + package.xml  (deps: diagnostic_msgs, geometry_msgs, rclcpp, std_msgs, std_srvs)
```

### Build Docker Image

```bash
docker build --no-cache -f tool/docker/Dockerfile -t ros2_v3_mining .
```

### Run

```bash
# Start FastDDS Discovery Server
fastdds discovery -i 0 -p 11811

# Run all 8 nodes
docker run --rm --network host ros2_v3_mining
```

### Observe Topics and Services

```bash
# Watch the cycle orchestrator state
ros2 topic echo /mining/fleet/load_assignment

# Watch truck movement
ros2 topic echo /mining/truck/truck_pose

# Watch payload build-up
ros2 topic echo /mining/truck/payload_status

# Watch site productivity log
ros2 topic echo /mining/fleet/cycle_report

# Watch fleet health
ros2 topic echo /mining/fleet/health_telemetry

# Check bucket status
ros2 service call /mining/excavator/check_payload std_srvs/srv/Trigger

# Trigger fleet emergency stop
ros2 service call /mining/fleet/emergency_stop std_srvs/srv/Trigger
```

---

## Expected Console Output (runtime)

```
[arm_planner]       Truck arrived at dig zone — starting excavation
[arm_planner]       ARM: IDLE → DIGGING
[bucket_controller] (receiving dig_command continuously, publishing bucket_state)
[arm_planner]       ARM: DIGGING → RAISING
[arm_planner]       Dispatcher ack: Load cycle approved by dispatcher
[arm_planner]       ARM: RAISING → DUMPING
[payload_monitor]   Dump: +21600kg → truck total=21600kg / 200000kg (10%)
[arm_planner]       ARM: DUMPING → IDLE (cycle complete)
[load_dispatcher]   DISPATCH: LOADING → DISPATCHING (truck full)
[load_dispatcher]   DISPATCH: Cycle 1 complete → repositioning truck
[site_logger]       Cycle 1 complete — total hauled: 0.2 t
[site_logger]       [SITE] cycles=1 hauled=0.2000t payload=0% faults=0
[fleet_monitor]     Fleet: bucket=0%  truck=(150.0,0.0)  payload=0%
[proximity_detector] Truck left dig zone (155.0m)
[truck_navigator]   Arrived at (5.0, 0.0)
[proximity_detector] Truck entered dig zone (8.4m from excavator)
[arm_planner]       Truck arrived at dig zone — starting excavation
[arm_planner]       ARM: IDLE → DIGGING
```
