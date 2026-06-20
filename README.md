# ROS2 Discovery Server — Version 3.0

Version 3.0 extends the V2 YAML-driven generator with **multi-role nodes** — a single node YAML now declares all roles a node plays simultaneously (publisher + subscriber + service server + service client + timers), with explicit callback groups and typed parameters.

---

## What Changed from V2

| Feature | V2 | V3 |
|---|---|---|
| Node roles | One role per node file | Multiple roles per node file |
| Interface registry | `topics.yaml` + inline `srv_type` | `interfaces.yaml` — unified msg + srv |
| Parameters | Bare `key: value` | `key: {value: x, type: double}` → typed `declare_parameter<T>()` |
| Callback groups | Not supported | `mutually_exclusive` / `reentrant` per callback |
| Executor | Always `rclcpp::spin` | `single_threaded` or `multi_threaded` with thread count |
| Dockerfile | Manual per project | Generated from `Dockerfile.jinja` via generator |
| Discovery config | Static `env.sh` | `discovery.yaml` → `ENV ROS_DISCOVERY_SERVER` in Dockerfile |

---

## Repository Structure

```
Version3.0/
├── tool/
│   ├── generator.py              # YAML + Jinja2 → C++ + Dockerfile
│   ├── Dockerfile                # Generated per project (do not edit)
│   ├── entrypoint.sh             # Container entrypoint
│   └── templates/
│       ├── node.cpp.jinja        # Multi-role C++ node body
│       ├── node.hpp.jinja        # Node class header
│       ├── CMakeLists.txt.jinja
│       ├── package.xml.jinja
│       └── Dockerfile.jinja      # Dockerfile template
│
└── robot_arm_controller/
    ├── config/
    │   ├── application.yaml      # Node list, domain_id, apt_packages
    │   ├── discovery.yaml        # DS address → ENV ROS_DISCOVERY_SERVER
    │   ├── interfaces.yaml       # Unified msg + srv registry (the contracts)
    │   ├── qos_profiles.yaml     # Named QoS profiles
    │   ├── parameters.yaml       # Typed parameter values per node
    │   └── nodes/
    │       ├── arm_controller.yaml
    │       └── collision_detector.yaml
    └── generated_pkg/
        ├── src/
        │   ├── arm_controller.cpp
        │   └── collision_detector.cpp
        ├── include/
        │   ├── arm_controller.hpp
        │   └── collision_detector.hpp
        ├── CMakeLists.txt
        └── package.xml
```

---

## YAML → Code Flow

```
┌─────────────────────────────────────────────────────────────────────┐
│                      config/application.yaml                        │
│                                                                     │
│   domain_id: 10        ──► ENV ROS_DOMAIN_ID in Dockerfile         │
│   apt_packages:        ──► RUN apt-get install in Dockerfile        │
│   nodes: [arm_controller, collision_detector]                       │
└───────────────┬─────────────────────────────────────────────────────┘
                │ for each node name
                ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    config/nodes/<name>.yaml                         │
│                                                                     │
│   publishers:   [{interface: joint_states, qos: sensor_qos}]       │
│   subscribers:  [{interface: joint_commands, callback_group: ...}]  │
│   service_servers: [{interface: set_mode}]                          │
│   service_clients: [{interface: check_collision}]                   │
│   timers:       [{name: control_loop, rate_hz: 100.0}]             │
│   executor:     {type: multi_threaded, threads: 4}                 │
│   callback_groups: [{name: control_group, type: mutually_exclusive}]│
└────┬──────────────────┬──────────────────────────────────────────---┘
     │ interface lookup  │ qos / parameter lookup
     ▼                   ▼
┌──────────────────┐  ┌────────────────────────────────────────────────┐
│ interfaces.yaml  │  │ qos_profiles.yaml  │  parameters.yaml          │
│                  │  │                    │                           │
│ messages:        │  │ sensor_qos:        │  arm_controller:          │
│   joint_states:  │  │   reliability:     │    joint_names:           │
│     type: ...    │  │     best_effort    │      value: [...]         │
│     default_     │  │   depth: 5         │      type: string_array   │
│     topic: ...   │  │                    │    max_velocity:          │
│ services:        │  │ reliable_qos:      │      value: 1.0           │
│   set_mode:      │  │   reliability:     │      type: double         │
│     type: ...    │  │     reliable       │                           │
│     default_     │  │   depth: 10        │  ──► typed                │
│     name: ...    │  │                    │      declare_parameter<T> │
└──────────────────┘  └────────────────────────────────────────────────┘
     │
     ▼ generator resolves types:
  sensor_msgs/msg/JointState
    ├── type_to_cpp()      ──► sensor_msgs::msg::JointState
    └── type_to_include()  ──► sensor_msgs/msg/joint_state.hpp

  ──► generated_pkg/src/<name>.cpp     (node.cpp.jinja)
  ──► generated_pkg/include/<name>.hpp (node.hpp.jinja)
  ──► generated_pkg/CMakeLists.txt     (CMakeLists.txt.jinja)
  ──► generated_pkg/package.xml        (package.xml.jinja)
  ──► tool/Dockerfile                  (Dockerfile.jinja + discovery.yaml)
```

---

## Project: robot_arm_controller

A two-node robot arm simulation demonstrating V3 multi-role nodes and cross-node service calls.

### Topology

```
arm_controller                         collision_detector
─────────────────                      ──────────────────────
ROLE: publisher                        ROLE: subscriber
  /robot1/joint_states  ────────────►  /robot1/joint_states
  /robot1/arm_status

ROLE: subscriber                       ROLE: service_server
  /robot1/joint_commands               /robot1/check_collision
                                          └── responds: Clear / RISK
ROLE: service_server
  /robot1/set_mode                     ROLE: publisher
     data=true  → mode=active            /robot1/collision_alert
     data=false → mode=idle
                                       ROLE: timer (50 Hz)
ROLE: service_client                     safety_check()
  calls /robot1/check_collision            monitors joint positions
     every 2 seconds                       fires alert near limit

ROLE: timer (100 Hz)
  control_loop()
    publishes sinusoidal joint states
    publishes arm_status every 1 s
    calls check_collision every 2 s
```

### What you observe

- `arm_controller` publishes joint positions using `sin(t * 0.5 * joint_index)` — joints oscillate at different frequencies and naturally swing into the limit zone every few seconds
- `collision_detector` monitors those positions via subscriber, and when any joint exceeds `(1.0 - safety_margin)` it fires a `collision_alert` and logs a warning
- `arm_controller` calls `check_collision` every 2 s and logs the response — you see the two nodes interacting in real time

---

## Step 1 — Generate

Run from `Version3.0/`:

```bash
python tool/generator.py --project robot_arm_controller
```

Outputs: generated C++ files + `tool/Dockerfile`

---

## Step 2 — Build

Run from `Version3.0/`:

```bash
docker build --no-cache -f tool/Dockerfile -t ros2_v3_robot_arm .
```

---

## Step 3 — Test (3 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --rm --name robot_arm ros2_v3_robot_arm bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Arm Controller

```bash
docker exec -it robot_arm bash
ros2 run generated_pkg arm_controller
```

Expected (repeating every second):
```
[INFO] [arm_controller]: Status: mode=idle  tick=100
[INFO] [arm_controller]: Status: mode=idle  tick=200
[WARN] [arm_controller]: Collision check: COLLISION RISK DETECTED
```

### Terminal 3 — Collision Detector

```bash
docker exec -it robot_arm bash
ros2 run generated_pkg collision_detector
```

Expected:
```
[INFO] [collision_detector]: Parameters loaded. safety_margin=0.050
[WARN] [collision_detector]: COLLISION ALERT: joint approaching limit (safety_margin=0.050)
[INFO] [collision_detector]: check_collision query → COLLISION RISK DETECTED
[INFO] [collision_detector]: Collision alert cleared — joints back in safe range.
```

### Optional — Toggle arm mode

```bash
docker exec -it robot_arm bash
ros2 service call /robot1/set_mode std_srvs/srv/SetBool "{data: true}"
ros2 service call /robot1/set_mode std_srvs/srv/SetBool "{data: false}"
```

> **Note:** `ros2 service list` and `ros2 topic list` show nothing with FastDDS DS — use `ros2 node list` to confirm registration. Direct `ros2 service call` works.

---

## What Contracts Are and Why They Matter

In V2, topic types were declared inline in each node YAML. Two nodes could reference the same topic with mismatched types — the generator had no way to detect this.

In V3, `interfaces.yaml` is a **contract registry** — one entry per channel:

```yaml
interfaces:
  messages:
    joint_states:
      type: sensor_msgs/msg/JointState   # contract: type AND topic path
      default_topic: /robot1/joint_states

  services:
    check_collision:
      type: std_srvs/srv/Trigger         # contract: request + response shape
      default_name: /robot1/check_collision
```

**A contract entry tells you:**
- What data type flows on this channel — not just a string, the generator enforces it in C++
- What path the channel lives on — publisher and subscriber both get the same topic string
- For services: both request fields (above `---` in the `.srv` file) and response fields (below `---`) — the server callback signature and client request type are generated from the same entry

**Why this matters:** `arm_controller` publishes `joint_states` and `collision_detector` subscribes to `joint_states`. Both reference the contract by name. The generator resolves `sensor_msgs/msg/JointState` once, applies it to both, and generates matching C++ types and include paths. Type mismatch between producer and consumer becomes impossible at generation time.

---

## Prerequisites

| Requirement | Version | Purpose |
|---|---|---|
| Python | 3.8+ | Running the generator |
| pyyaml | any | YAML parsing |
| jinja2 | any | Template rendering |
| Docker Desktop | 20.10+ | Building and running containers |

```bash
pip install pyyaml jinja2
```
