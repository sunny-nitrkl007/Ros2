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

## Step 2 — Build and Run (Docker)

Everything after code generation happens inside Docker. The `Dockerfile` copies the generated source into the image and runs `colcon build` during the image build. The `docker-compose.yml` spins up one container per ECU plus a shared FastDDS Discovery Server.

### Project structure for Docker

```
automotive_adas/
├── Dockerfile             # builds all 4 packages inside the image
├── docker-compose.yml     # 4 services: ds_server, perception, planning, safety
└── generated/             # source copied into the image at build time
    ├── adas_interfaces/
    ├── perception_system/
    ├── planning_system/
    └── safety_system/
```

### Build the image

Run from the project directory:

```bash
docker-compose build
```

This runs `colcon build` inside the container — `adas_interfaces` first (so message headers are ready), then all three application packages.

### Start all containers

```bash
docker-compose up
```

All four services start on a shared `adas_net` bridge network:

| Container | Role |
|---|---|
| `ds_server` | FastDDS Discovery Server on port 11811 |
| `perception` | `perception1_node` + `perception2_node` |
| `planning` | `planning1_node` + `planning2_node` |
| `safety` | `safety_node` |

`ROS_DISCOVERY_SERVER=ds_server:11811` is set via docker-compose — Docker DNS resolves `ds_server` to the DS container automatically, so no hardcoded IP is needed.

### Interactive shell inside a running container

```bash
docker exec -it automotive_adas-perception-1 bash
```

From inside the container:
```bash
ros2 topic list
ros2 service list
ros2 topic echo /perception/obstacle_data
```

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
