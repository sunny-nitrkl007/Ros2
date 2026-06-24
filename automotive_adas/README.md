# Automotive ADAS — ROS2 V3 Demo

## Overview

A three-application ADAS (Advanced Driver Assistance System) demo built with the ROS2 grammar. Three independent ROS2 packages run across three ECUs (simulated as Docker containers), communicating over FastDDS discovery server.

| Application | Nodes | Role |
|---|---|---|
| `perception_system` | `perception1_node`, `perception2_node` | Detects obstacles, provides lane info |
| `planning_system` | `planning1_node`, `planning2_node` | Plans trajectory from perception data |
| `safety_system` | `safety_node` | Monitors for hazards, triggers emergency brake |

---

## System Architecture

```
┌────────────────────────────────────────────────────────────────────────┐
│  FastDDS Discovery Server — 192.168.10.100:11811  (one shared DS)     │
└──────────────────┬─────────────────────┬──────────────────────────────┘
                   │                     │
┌──────────────────▼──┐   ┌─────────────▼──────────┐   ┌───────────────────────┐
│  Perception ECU     │   │  Planning ECU           │   │  Safety ECU           │
│                     │   │                         │   │                       │
│  perception1_node   │   │  planning1_node         │   │  safety_node          │
│  ├─ pub obstacle_data──►│  ├─ sub obstacle_data   │   │  ├─ sub obstacle_data │
│  ├─ sub emergency_brake◄│  ├─ sub emergency_brake◄│   │  ├─ sub trajectory    │
│  └─ srv get_lane_info◄─►│  └─ pub trajectory ─────────►│  ├─ pub emergency_brake
│                     │   │                         │   │  └─ srv check_risk    │
│  perception2_node   │   │  planning2_node         │   │                       │
│  └─ client          │   │  ├─ client get_lane_info│   │                       │
│    request_path_upd◄────│  ├─ client check_risk──►│   │                       │
│                     │   │  └─ srv request_path_upd│   │                       │
└─────────────────────┘   └─────────────────────────┘   └───────────────────────┘
```

---

## Grammar Files

```
automotive_adas/
├── general/
│   ├── apps/
│   │   ├── applications.yaml          # 3 applications, each references a discovery profile
│   │   └── segments/
│   │       ├── perception_segments.yaml   # interactions + node→interaction mapping
│   │       ├── planning_segments.yaml
│   │       └── safety_segments.yaml
│   ├── Interfaces_data/
│   │   ├── topics_data.yaml           # topic name, path, and custom message fields
│   │   └── services_data.yaml         # service name, path, request/response fields
│   └── profiles/
│       ├── qos_profiles.yaml          # named QoS profiles referenced in segments
│       ├── parameter_profiles.yaml    # named parameter sets referenced in node defs
│       └── discovery_profiles.yaml    # FastDDS DS config referenced in applications
├── ecus/                              # ECU hardware config (empty — Docker replaces this)
└── topology/
    └── system_topology.yaml           # ECU-to-ECU connections (empty — Docker replaces this)
```

### How the files connect

```
applications.yaml
  └── name: perception_system
      discovery_ref: standard_ecu ──────► discovery_profiles.yaml
      nodes: [perception1_node, ...]

perception_segments.yaml
  └── segments:
        interactions:
          - publisher:
              topic_name: obstacle_data ─► topics_data.yaml (gets topic_path + field structure)
              qos_profile: pub_default_qos ► qos_profiles.yaml
      node:
        - name: perception1_node
          parameter_ref: perception_system_parameters ► parameter_profiles.yaml
          interactions_ref: [pub_obstacle_data, ...]
```

---

## Generated Output

Running the generator produces:

```
automotive_adas/generated/
├── adas_interfaces/            # shared custom message/service package
│   ├── msg/
│   │   ├── ObstacleData.msg
│   │   ├── Trajectory.msg
│   │   └── EmergencyBrake.msg
│   ├── srv/
│   │   ├── GetLaneInfo.srv
│   │   ├── CheckRisk.srv
│   │   └── RequestPathUpdate.srv
│   ├── CMakeLists.txt
│   └── package.xml
├── perception_system/          # one colcon package per ECU application
│   ├── src/
│   │   ├── perception1_node.cpp
│   │   └── perception2_node.cpp
│   ├── include/
│   │   ├── perception1_node.hpp
│   │   └── perception2_node.hpp
│   ├── CMakeLists.txt
│   ├── package.xml
│   └── env.sh                  # ROS_DOMAIN_ID + ROS_DISCOVERY_SERVER for this ECU
├── planning_system/
│   └── ...
└── safety_system/
    └── ...
```

`adas_interfaces` must be built first — all three application packages depend on it.

---

## Step 1 — Generate C++

Run from the project directory:

```bash
python3 tool/generator/generator.py --project automotive_adas
```

Expected output:
```
==> Generating: automotive_adas

  Interfaces (adas_interfaces)
    OK msg/ObstacleData.msg
    OK msg/Trajectory.msg
    OK msg/EmergencyBrake.msg
    OK srv/GetLaneInfo.srv
    OK srv/CheckRisk.srv
    OK srv/RequestPathUpdate.srv
    OK CMakeLists.txt + package.xml

  Application: perception_system
    OK perception1_node  [1pub  1sub  1srv_server  0srv_client  0timer]
    OK perception2_node  [0pub  0sub  0srv_server  1srv_client  0timer]
    OK CMakeLists.txt + package.xml  (deps: adas_interfaces, rclcpp)
    OK env.sh  (DS: 192.168.10.100:11811, domain: 0)
  ...
```

---

## Step 2 — Build

All four packages share one colcon workspace. Build `adas_interfaces` first so the generated message headers are available when the application packages compile.

```bash
cd automotive_adas/generated

# Build interfaces first
colcon build --packages-select adas_interfaces
source install/setup.bash

# Build all three application packages
colcon build --packages-select perception_system planning_system safety_system
source install/setup.bash
```

---

## Step 3 — Run (Docker — 4 terminals)

Each ECU application runs in its own container on a shared Docker bridge network.

**Terminal 1 — FastDDS Discovery Server:**
```bash
docker run -it --rm --name ds_server --network adas_net \
  ros:jazzy \
  fastdds discovery -i 0 -p 11811
```

**Terminal 2 — Perception container:**
```bash
docker run -it --rm --name perception --network adas_net \
  ros2_adas_perception bash
source install/setup.bash
source env.sh
ros2 run perception_system perception1_node &
ros2 run perception_system perception2_node
```

**Terminal 3 — Planning container:**
```bash
docker run -it --rm --name planning --network adas_net \
  ros2_adas_planning bash
source install/setup.bash
source env.sh
ros2 run planning_system planning1_node &
ros2 run planning_system planning2_node
```

**Terminal 4 — Safety container:**
```bash
docker run -it --rm --name safety --network adas_net \
  ros2_adas_safety bash
source install/setup.bash
source env.sh
ros2 run safety_system safety_node
```

> **Note:** Update `env.sh` in each generated application folder — replace `192.168.10.100` with the actual DS container IP or service name on your Docker network.

---

## Custom Messages and Services

All messages and services are generated from `Interfaces_data/` — no external ROS2 packages needed.

### Messages

| Message | Topic path | Fields |
|---|---|---|
| `ObstacleData` | `/perception/obstacle_data` | `id: int32`, `distance: float32`, `relative_velocity: float32`, `object_type: string` |
| `Trajectory` | `/planning/trajectory` | `x_points: float32[]`, `y_points: float32[]`, `target_speed: float32`, `emergency_flag: bool` |
| `EmergencyBrake` | `/safety/emergency_brake` | `apply_brake: bool`, `brake_force: float32`, `reason: string` |

### Services

| Service | Service path | Request | Response |
|---|---|---|---|
| `GetLaneInfo` | `/perception/get_lane_info` | `request: bool` | `lane_center_offset`, `lane_width`, `lane_type` |
| `CheckRisk` | `/safety/check_risk` | `obstacle_distance`, `vehicle_speed` | `risk_detected`, `collision_probability`, `risk_level` |
| `RequestPathUpdate` | `/planning/request_path_update` | `emergency: bool`, `obstacle_distance` | `success`, `new_target_speed`, `status_msg` |

---

## Implement Node Logic

Each node's callbacks contain `begin impl / end impl` blocks. Add your logic inside — the generator preserves these blocks on every re-run.

```cpp
void Perception1Node::on_sub_emergency_state(const adas_interfaces::msg::EmergencyBrake::SharedPtr msg)
{
    //-- begin impl [on_sub_emergency_state] ----------------------------------------
    if (msg->apply_brake) {
        RCLCPP_WARN(this->get_logger(), "Emergency brake received: %s", msg->reason.c_str());
    }
    //-- end impl [on_sub_emergency_state] ------------------------------------------
}
```

Re-running the generator updates the generated scaffold without erasing your impl blocks.
