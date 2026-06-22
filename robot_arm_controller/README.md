# Robot Arm Controller — ROS2 V3 Demo

## Overview

This is the **introductory V3 demo** — a two-node robot arm simulation that exercises every feature the V3 generator provides: multi-role nodes, typed parameters, named QoS profiles, callback groups, a service server, a service client, and async cross-node calls.

- `arm_controller` — publishes sinusoidal joint states at 100 Hz, receives mode commands via service, calls the collision detector every 2 seconds
- `collision_detector` — monitors joint positions via subscriber, publishes alerts when a joint nears its limit, serves the `check_collision` query

Both nodes run on **Domain ID 10** with FastDDS SUPER_CLIENT discovery.

---

## System Architecture

```
┌──────────────────────────────────────────────────────────────────┐
│              Domain ID 10  ·  SUPER_CLIENT @ 127.0.0.1:11811    │
│                                                                  │
│  ┌─────────────────────────┐    joint_states (100 Hz)           │
│  │     arm_controller      │ ──────────────────────────────────► │
│  │  (4 threads)            │                                     │
│  │                         │    arm_status (1 Hz)                │
│  │  timer: control_loop    │ ──────────────────────────────────► │
│  │         100 Hz          │                                     │
│  │                         │                                     │
│  │  sub: joint_commands    │ ◄────── (external commander)        │
│  │                         │                                     │
│  │  server: set_mode       │ ◄────── ros2 service call           │
│  │    data=true  → active  │                                     │
│  │    data=false → idle    │                                     │
│  │                         │                                     │
│  │  client:                │                                     │
│  │    check_collision      │──────────────────────┐             │
│  │    (every 2 s)          │                      ▼             │
│  └─────────────────────────┘    ┌───────────────────────────┐   │
│                                  │   collision_detector      │   │
│                                  │   (2 threads)             │   │
│                                  │                           │   │
│  joint_states ──────────────────►│  sub: joint_states       │   │
│                                  │                           │   │
│                                  │  timer: safety_check 50Hz│   │
│                                  │    monitors positions     │   │
│                                  │    publishes alert        │   │
│                                  │                           │   │
│                                  │  server: check_collision  │◄──┘
│                                  │    responds: Clear / RISK │   │
│                                  │                           │   │
│                                  │  pub: collision_alert     │──►│
│                                  └───────────────────────────┘   │
└──────────────────────────────────────────────────────────────────┘
```

---

## Config File Deep-Dive

### `config/application.yaml`

```yaml
application:
  name: robot_arm_system
  version: "3.0"
  domain_id: 10
  discovery_profile: robot_arm_ds
  apt_packages:
    - ros-jazzy-trajectory-msgs
    - ros-jazzy-sensor-msgs
  nodes:
    - arm_controller
    - collision_detector
```

- `domain_id: 10` — separate from ADAS (20), haul truck (30), mining (40); all four can run simultaneously without interference
- `apt_packages` — these are installed in the Docker image by the generated Dockerfile. `trajectory_msgs` and `sensor_msgs` are not in the base ROS image and must be listed here
- `nodes` — must match the names of files in `config/contracts/`

---

### `config/data.yaml`

```yaml
data:
  messages:
    joint_commands:
      type: trajectory_msgs/msg/JointTrajectory
      topic_path: /robot1/joint_commands

    joint_states:
      type: sensor_msgs/msg/JointState
      topic_path: /robot1/joint_states

    arm_status:
      type: std_msgs/msg/String
      topic_path: /robot1/arm_status

    collision_alert:
      type: std_msgs/msg/Bool
      topic_path: /robot1/collision_alert

  services:
    set_mode:
      type: std_srvs/srv/SetBool
      service_path: /robot1/set_mode

    check_collision:
      type: std_srvs/srv/Trigger
      service_path: /robot1/check_collision
```

**What it does:**

All six channels are defined here. `arm_controller` publishes `joint_states` and `collision_detector` subscribes to it — both reference the key `joint_states` in their contracts, so they get the same type (`sensor_msgs/msg/JointState`) and the same topic path (`/robot1/joint_states`) automatically. You cannot have a type mismatch between producer and consumer.

**Topic namespace:** all channels live under `/robot1/` — if you rename the robot, you change it in one place here.

---

### `config/qos_profiles.yaml`

```yaml
qos_profiles:
  sensor_qos:
    reliability: best_effort
    durability: volatile
    history: keep_last
    depth: 5
    deadline_ms: 20

  reliable_qos:
    reliability: reliable
    durability: volatile
    history: keep_last
    depth: 10
    deadline_ms: 200
    lifespan_ms: 1000

  latch_qos:
    reliability: reliable
    durability: transient_local
    history: keep_last
    depth: 1
```

| Profile | Used for | Rationale |
|---|---|---|
| `sensor_qos` | `joint_states` | 100 Hz stream — `best_effort` drops stale readings rather than queueing. `deadline_ms: 20` catches a frozen publisher within one 50 Hz safety_check cycle |
| `reliable_qos` | `arm_status`, `collision_alert`, `joint_commands` | Commands and alerts must not be dropped. `lifespan_ms: 1000` makes old `arm_status` strings expire after 1 s |
| `latch_qos` | Available but not used in this demo | `transient_local` durability: late subscribers get the last published value immediately on connection |

---

### `config/discovery_profiles.yaml`

```yaml
discovery_profiles:
  robot_arm_ds:
    mode: SUPER_CLIENT
    servers:
      - ip: 127.0.0.1
        port: 11811
```

In SUPER_CLIENT mode every node connects to the FastDDS Discovery Server (DS) at startup and queries it for the full participant list. The DS address is baked into the generated Dockerfile as `ROS_DISCOVERY_SERVER=127.0.0.1:11811`. This is why `ros2 topic list` shows nothing from outside the container — the nodes skip LAN multicast and talk directly to the DS.

---

### `config/parameters.yaml`

```yaml
parameters:
  arm_controller:
    joint_names:
      value: [shoulder, elbow, wrist]
      type: string_array
    max_velocity:
      value: 1.0
      type: double
    control_rate:
      value: 100.0
      type: double
    mode:
      value: "idle"
      type: string

  collision_detector:
    safety_margin:
      value: 0.05
      type: double
    check_rate:
      value: 50.0
      type: double
    alert_on_proximity:
      value: true
      type: bool
```

Each parameter group is named after the node and referenced by `parameter_ref:` in the contract. The generator emits the correct typed `declare_parameter<T>()` call:

- `string_array` → `declare_parameter<std::vector<std::string>>()`
- `double` → `declare_parameter<double>()`
- `bool` → `declare_parameter<bool>()`

`safety_margin: 0.05` means a joint position > 0.95 (95 % of the ±1.0 range) triggers a collision alert.

---

### `config/contracts/arm_controller.yaml`

```yaml
contract:
  node:
    name: arm_controller
    executor: { type: multi_threaded, threads: 4 }
    parameter_ref: arm_controller

  callback_groups:
    - name: control_group
      type: mutually_exclusive
    - name: service_group
      type: reentrant

  timers:
    - name: control_loop
      rate_hz: 100.0
      callback_group: control_group

  interactions:
    - type: pub-sub
      role: producer
      dataref: joint_states
      qos_profile: sensor_qos

    - type: pub-sub
      role: producer
      dataref: arm_status
      qos_profile: reliable_qos

    - type: pub-sub
      role: consumer
      dataref: joint_commands
      qos_profile: reliable_qos
      callback_group: control_group

    - type: service
      role: server
      dataref: set_mode
      callback_group: service_group

    - type: service
      role: client
      dataref: check_collision
```

**Why two callback groups?**

- `control_group` (mutually exclusive): `control_loop` timer and `on_joint_commands` subscriber share state variables. Making them mutually exclusive means only one runs at a time — no mutex needed.
- `service_group` (reentrant): the `on_set_mode` service server must respond even while `control_loop` is running. If `set_mode` were in `control_group`, a long control tick would delay the mode-change response. Reentrant allows it to execute in parallel on one of the 4 executor threads.

---

### `config/contracts/collision_detector.yaml`

```yaml
contract:
  node:
    name: collision_detector
    executor: { type: multi_threaded, threads: 2 }
    parameter_ref: collision_detector

  callback_groups:
    - name: safety_group
      type: mutually_exclusive

  timers:
    - name: safety_check
      rate_hz: 50.0
      callback_group: safety_group

  interactions:
    - type: pub-sub
      role: consumer
      dataref: joint_states
      qos_profile: sensor_qos
      callback_group: safety_group

    - type: pub-sub
      role: producer
      dataref: collision_alert
      qos_profile: reliable_qos

    - type: service
      role: server
      dataref: check_collision
      callback_group: safety_group
```

Single callback group (`safety_group`, mutually exclusive) for all three callbacks. This means:
- `on_joint_states` and `safety_check` cannot race on `latest_positions_`
- `on_check_collision` service handler reads `collision_detected_` safely — the state won't change mid-read

---

## Node Reference

### `arm_controller`

**State variables (added to HPP):**
```cpp
std::vector<double> joint_positions_;   // current simulated positions
bool   mode_active_ = false;            // idle=false, active=true
int    tick_        = 0;                // timer tick counter
```

**`control_loop` (100 Hz):**
- Each joint oscillates: `joint_positions_[i] = sin(tick_ * 0.005 * (i + 1))`
- Builds a `sensor_msgs/JointState` with `joint_names_` and publishes to `/robot1/joint_states`
- Every 100 ticks (1 second): publishes `std_msgs/String` arm_status (`mode=idle|active tick=N`)
- Every 200 ticks (2 seconds): if `check_collision_client_->service_is_ready()`, sends async Trigger request and logs the response message

**`on_joint_commands`:**
- Receives `trajectory_msgs/JointTrajectory` (reserved for external commander override)

**`on_set_mode`:**
- `data=true` → `mode_active_ = true`, responds `"mode set to active"`
- `data=false` → `mode_active_ = false`, responds `"mode set to idle"`

---

### `collision_detector`

**State variables (added to HPP):**
```cpp
std::vector<double> latest_positions_;
bool   collision_detected_ = false;
```

**`on_joint_states`:**
- Copies incoming positions into `latest_positions_`

**`safety_check` (50 Hz):**
- Checks if any `|position| > (1.0 - safety_margin_)` (default: > 0.95)
- If collision: `collision_detected_ = true`, publishes `Bool(true)`, logs WARN
- If previously in collision and now clear: `collision_detected_ = false`, publishes `Bool(false)`, logs INFO "cleared"

**`on_check_collision`:**
- Service handler (synchronous response to `arm_controller`'s async call)
- `response->success = collision_detected_`
- `response->message = "COLLISION RISK DETECTED"` or `"Clear — all joints within limits"`

---

## Data Flow

```
arm_controller::control_loop (100 Hz)
    │
    ├── joint_positions_[i] = sin(tick × 0.005 × (i+1))
    │
    ├── publishes JointState → /robot1/joint_states
    │         ↓
    │   collision_detector::on_joint_states
    │         └── latest_positions_ = msg->positions
    │
    ├── every 100 ticks:
    │   publishes arm_status → /robot1/arm_status
    │
    └── every 200 ticks:
        async call → /robot1/check_collision
              ↓
        collision_detector::on_check_collision
              └── returns {success: collision_detected_, message: "..."}

collision_detector::safety_check (50 Hz)
    │
    ├── reads latest_positions_
    ├── if |pos| > 0.95 → collision_detected_ = true
    └── publishes Bool → /robot1/collision_alert
```

**Natural collision cycle:** joints oscillate as `sin(t)` — they reach ±1.0 every ~1.26 seconds per joint. At 50 Hz, `safety_check` will fire the alert for roughly 16 ticks (~320 ms) out of each sine period.

---

## Build and Run

### Generate C++

Run from the project directory:

```bash
python3 tool/generator/generator.py --project robot_arm_controller
```

Expected output:
```
OK arm_controller      [2pub  1sub  1srv_server  1srv_client  1timer]
OK collision_detector  [1pub  1sub  1srv_server  0srv_client  1timer]
OK CMakeLists.txt + package.xml  (deps: rclcpp, sensor_msgs, std_msgs, std_srvs, trajectory_msgs)
```

### Build Docker Image

```bash
docker build --no-cache -f tool/docker/Dockerfile -t ros2_v3_robot_arm .
```

### Run (3 terminals)

**Terminal 1 — Container + Discovery Server:**
```bash
docker run -it --rm --name robot_arm ros2_v3_robot_arm bash
fastdds discovery -i 0 -p 11811
```

**Terminal 2 — Arm Controller:**
```bash
docker exec -it robot_arm bash
ros2 run generated_pkg arm_controller
```

**Terminal 3 — Collision Detector:**
```bash
docker exec -it robot_arm bash
ros2 run generated_pkg collision_detector
```

### Toggle arm mode

```bash
docker exec -it robot_arm bash
ros2 service call /robot1/set_mode std_srvs/srv/SetBool "{data: true}"
ros2 service call /robot1/set_mode std_srvs/srv/SetBool "{data: false}"
```

---

## Expected Console Output

**arm_controller:**
```
[INFO] [arm_controller]: Parameters loaded.
[INFO] [arm_controller]: Publisher ready: /robot1/joint_states
[INFO] [arm_controller]: Status: mode=idle  tick=100
[WARN] [arm_controller]: Collision check: COLLISION RISK DETECTED
[INFO] [arm_controller]: Status: mode=idle  tick=200
[INFO] [arm_controller]: Collision check: Clear — all joints within limits
```

**collision_detector:**
```
[INFO] [collision_detector]: Parameters loaded. safety_margin=0.050
[WARN] [collision_detector]: COLLISION ALERT: joint approaching limit (safety_margin=0.050)
[INFO] [collision_detector]: check_collision query → COLLISION RISK DETECTED
[INFO] [collision_detector]: Collision alert cleared — joints back in safe range.
```

> **Note:** `ros2 topic list` and `ros2 service list` show nothing from outside the container in SUPER_CLIENT mode — discovery traffic goes through the DS, not LAN multicast. Use `ros2 node list` to verify node registration. Direct `ros2 service call` from inside the container works normally.
