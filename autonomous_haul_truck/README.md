# Autonomous Haul Truck — ROS2 V3 Demo

## Overview

This project simulates an **autonomous mining haul truck** driving a 500-metre haul route between a loading zone and a dump point. It demonstrates how a real Caterpillar 793 or 797 autonomous truck processes sensor data, plans a route around obstacles, executes velocity/steering commands, detects proximity hazards, and reports mechanical health — all as a set of ROS2 nodes communicating over a FastDDS Discovery Server.

The project is entirely YAML-driven: all nodes, topics, services, QoS policies, parameters, and the discovery configuration are declared in config files. The generator tool (`tool/generator.py`) reads those files and produces ready-to-compile C++ — no hand-writing of boilerplate.

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Domain ID 30  ·  SUPER_CLIENT @ 127.0.0.1:11811│
│                                                                     │
│  ┌─────────────────┐    vehicle_pose (20 Hz, best_effort)           │
│  │  gps_imu_fusion │ ──────────────────────────────► route_planner  │
│  └─────────────────┘                                                │
│                                                                     │
│  ┌─────────────────┐    obstacle_map (10 Hz, reliable)              │
│  │ lidar_processor │ ──────────┬──────────────────► route_planner   │
│  └─────────────────┘          └──────────────────► hazard_detector  │
│                                                                     │
│  ┌──────────────┐    planned_route (5 Hz, reliable)                 │
│  │ route_planner│ ──────────────────────────────► vehicle_controller│
│  └──────────────┘                                                   │
│                                                                     │
│  ┌──────────────────┐    drive_command (20 Hz, reliable)            │
│  │vehicle_controller│ ──────────────────────────► hazard_detector   │
│  └──────────────────┘                                               │
│           ▲  calls /truck/safety/request_stop (service)             │
│           │                                                         │
│  ┌────────────────┐    hazard_alert (50 Hz, safety_qos)             │
│  │hazard_detector │ ─────────────────────────► vehicle_controller   │
│  │  (srv server)  │ ← ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ serves stop    │
│  └────────────────┘                                                 │
│                                                                     │
│  ┌────────────────────┐  health_report (2 Hz, reliable)             │
│  │truck_health_monitor│ ──────────────────────────► (consumers)     │
│  │   (srv server)     │ ← ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─  serves status  │
│  └────────────────────┘                                             │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Config File Deep-Dive

The entire system is declared in six YAML files inside `config/`. The generator reads them in a fixed sequence. Understanding each file is the key to understanding how the system works.

### `config/application.yaml`

```yaml
application:
  name: autonomous_haul_truck
  domain_id: 30
  discovery_profile: truck_super_client
  nodes:
    - gps_imu_fusion
    - lidar_processor
    - route_planner
    - vehicle_controller
    - hazard_detector
    - truck_health_monitor
```

**What it does:**

- `name` becomes the ROS2 package name (`autonomous_haul_truck`) in the generated `package.xml` and `CMakeLists.txt`.
- `domain_id: 30` isolates all nodes in this project from every other DDS domain running on the same machine. A node on domain 30 cannot see or talk to a node on domain 20 (ADAS project) or domain 40 (mining project).
- `discovery_profile: truck_super_client` tells the generator which entry in `discovery_profiles.yaml` to apply when building the Dockerfile. That profile configures every node to connect to a central FastDDS Discovery Server rather than doing multicast peer-discovery.
- `nodes` is an ordered list of node names. Each name must have a matching `contracts/<name>.yaml` file. The generator processes them in this order.

---

### `config/data.yaml`

```yaml
data:
  messages:
    vehicle_pose:
      type: geometry_msgs/msg/PoseStamped
      topic_path: /truck/localization/vehicle_pose
    obstacle_map:
      type: nav_msgs/msg/OccupancyGrid
      topic_path: /truck/perception/obstacle_map
    planned_route:
      type: nav_msgs/msg/Path
      topic_path: /truck/planning/planned_route
    drive_command:
      type: geometry_msgs/msg/Twist
      topic_path: /truck/control/drive_command
    hazard_alert:
      type: diagnostic_msgs/msg/DiagnosticArray
      topic_path: /truck/safety/hazard_alert
    health_report:
      type: diagnostic_msgs/msg/DiagnosticArray
      topic_path: /truck/health/health_report

  services:
    request_stop:
      type: std_srvs/srv/Trigger
      service_path: /truck/safety/request_stop
    system_status:
      type: std_srvs/srv/Trigger
      service_path: /truck/health/system_status
```

**What it does:**

`data.yaml` is the **shared type registry** for the whole project. Every topic and every service is declared here exactly once.

- A `message` entry defines a pub-sub channel: `type` is the ROS2 message type, `topic_path` is the DDS topic string.
- A `service` entry defines an RPC endpoint: `type` is the ROS2 service type, `service_path` is the service name.
- Contract files reference entries here using `dataref:` — they never repeat the type or topic path. This means if you need to rename `/truck/localization/vehicle_pose`, you change it in one place and it propagates everywhere.

**Topic namespace design:**

| Namespace | Purpose |
|---|---|
| `/truck/localization/` | Sensor-fused pose outputs |
| `/truck/perception/` | LiDAR processed grid |
| `/truck/planning/` | Waypoint path from planner |
| `/truck/control/` | Actuator commands |
| `/truck/safety/` | Hazard alerts and stop service |
| `/truck/health/` | Mechanical health telemetry |

---

### `config/qos_profiles.yaml`

```yaml
qos_profiles:
  realtime_qos:
    history: keep_last
    depth: 5
    reliability: best_effort
    deadline_ms: 50

  reliable_qos:
    history: keep_last
    depth: 10
    reliability: reliable
    deadline_ms: 200

  safety_qos:
    history: keep_last
    depth: 10
    reliability: reliable
    deadline_ms: 20
    lifespan_ms: 100
```

**What it does:**

Named QoS profiles let you apply a consistent policy to a topic without repeating all the fields in every contract. Contract files reference them by name (`qos_profile: safety_qos`).

**Why three tiers?**

| Profile | Used for | Rationale |
|---|---|---|
| `realtime_qos` | `vehicle_pose` (GPS/IMU) | Pose is published at 20 Hz. A stale GPS fix is useless — `best_effort` drops it rather than queuing. `depth: 5` prevents a slow subscriber from consuming old readings. |
| `reliable_qos` | Obstacle map, route, health | These are larger messages or lower frequency. `reliable` retransmits on loss. 200 ms deadline matches the 5–10 Hz publication rate. |
| `safety_qos` | `hazard_alert` | Hazard alerts must be both `reliable` (no silent drops) and fresh (`lifespan_ms: 100` makes stale alerts self-expire after 100 ms). `deadline_ms: 20` catches a frozen `hazard_detector` within one 50 Hz cycle. |

---

### `config/discovery_profiles.yaml`

```yaml
discovery_profiles:
  truck_super_client:
    mode: SUPER_CLIENT
    servers:
      - ip: 127.0.0.1
        port: 11811
```

**What it does:**

In SUPER_CLIENT mode, every node queries the FastDDS Discovery Server (DS) for the full participant list rather than multicasting on the LAN. This means:

- Discovery works across Docker networks, Kubernetes pods, or VLANs where multicast is blocked.
- All discovery traffic goes through one server at `127.0.0.1:11811`, making the network topology deterministic and observable.
- DDS domain ID (30) is enforced at runtime — nodes on other domains never appear in the roster.

The Dockerfile generated by the tool sets the FastDDS environment variable `ROS_DISCOVERY_SERVER=127.0.0.1:11811` and the participant mode to `SUPER_CLIENT`.

---

### `config/parameters.yaml`

Declares all tunable parameters for every node. Each node has a named group (`gps_imu_fusion_params`, etc.) that is referenced by `parameter_ref:` in the node's contract.

**Key parameters and their meaning:**

| Node | Parameter | Value | Meaning |
|---|---|---|---|
| `gps_imu_fusion` | `fusion_rate` | 20.0 Hz | Pose publish rate |
| | `position_noise` | 0.3 m | Gaussian noise amplitude on simulated GPS fix |
| | `heading_drift` | 0.005 rad/tick | Heading random walk per publish cycle |
| `lidar_processor` | `scan_rate` | 10.0 Hz | OccupancyGrid publish rate |
| | `max_range` | 100.0 m | Furthest distance the grid covers |
| | `obstacle_density` | 0.15 | Controls how many rocks appear in simulation |
| `route_planner` | `destination_x` | 500.0 m | Haul route endpoint along X-axis |
| | `waypoint_spacing` | 20.0 m | Distance between consecutive waypoints |
| `vehicle_controller` | `max_speed` | 12.0 m/s | ~43 km/h top speed (real 793F: 67 km/h empty) |
| | `max_steering` | 25.0 deg | Maximum steering angle |
| | `brake_decel` | 2.5 m/s² | Emergency braking deceleration |
| `hazard_detector` | `critical_distance` | 8.0 m | Obstacle this close → escalate to ERROR after 5 ticks |
| | `warning_distance` | 20.0 m | Obstacle closer than this → WARN |
| | `check_rate` | 50.0 Hz | Hazard evaluation frequency |
| `truck_health_monitor` | `engine_temp_max` | 95.0 °C | Above this → WARN (CAT C175 limit ~100 °C) |
| | `hydraulic_pressure_min` | 150.0 bar | Below this → WARN |

---

### `config/contracts/<node>.yaml`

A contract file is the **behavioural specification** for one node. It declares which callback groups it uses, which data channels it connects to, and what timers it runs — but never the implementation logic. The generator reads this and wires up the C++ class automatically.

**Structure of a contract:**

```
contract:
  node:
    name: <name>
    executor: { type: single_threaded | multi_threaded, threads: N }
    parameter_ref: <param-group-name>

  callback_groups:
    - name: <group>
      type: mutually_exclusive | reentrant

  interactions:
    - type: pub-sub | service
      role: producer | consumer | server | client
      dataref: <key in data.yaml>
      qos_profile: <key in qos_profiles.yaml>   # pub-sub only
      callback_group: <group>                    # optional, locks callback to a group

  timers:
    - name: <method-name>
      rate_hz: <frequency>
      callback_group: <group>
```

---

## Node Reference

### `gps_imu_fusion` — Localization

**Real-world role:** Combines GPS position fix and IMU heading into a single fused pose estimate, similar to how a CAT MineStar system blends RTK-GPS with an IMU to give centimetre-accurate position.

**Contract summary:**
- Executor: multi-threaded (2 threads)
- Callback group: `fusion_group` (reentrant)
- Publishes: `vehicle_pose` at 20 Hz using `realtime_qos`
- No subscribers
- Timer: `fusion_tick` at 20 Hz

**Parameters:** `fusion_rate`, `position_noise`, `heading_drift`

**Simulation behaviour (`fusion_tick`):**
- Advances `pos_x_` by 0.40 m per tick → ~8 m/s haul speed
- Adds sinusoidal noise (`position_noise_` amplitude) to X and Y
- Applies a small heading drift (`heading_drift_`) using sin-based random walk
- Publishes a `geometry_msgs/PoseStamped` with heading encoded as quaternion (rotation around Z)
- Logs position every 100 ticks (every 5 seconds)

**Why reentrant callback group?** GPS fusion is stateless between two successive callbacks (each tick is independent), so multiple callbacks can overlap safely.

---

### `lidar_processor` — Perception

**Real-world role:** Processes raw LiDAR point cloud into an occupancy grid, similar to how a 793 processes its eight surround-view LiDAR units into a bird's-eye obstacle map for the autonomous system.

**Contract summary:**
- Executor: single-threaded
- Callback group: `scan_group` (mutually exclusive)
- Publishes: `obstacle_map` at 10 Hz using `reliable_qos`
- No subscribers
- Timer: `scan_tick` at 10 Hz

**Parameters:** `scan_rate`, `max_range`, `obstacle_density`

**Simulation behaviour (`scan_tick`):**
- Creates a 200×200 OccupancyGrid at 1 m/cell resolution → 200 m × 200 m field of view
- Origin is at the front of the truck, centred laterally
- Places three "rock formations" using sinusoidal offsets:
  - Rock X positions: columns 20, 80, 140 in the grid (±8 m wave on time)
  - Rock Y positions: centre ± 25 m sinusoidal wave (each formation moves independently)
  - Each rock is a 9×9 cell block with occupancy value 85
- `reliable_qos` ensures the route planner and hazard detector always receive every grid — a dropped map could cause a phantom-clear-path error

---

### `route_planner` — Path Planning

**Real-world role:** Maintains the haul route from current position to destination, replanning in real time when the LiDAR grid shows an obstacle in the corridor.

**Contract summary:**
- Executor: multi-threaded (2 threads)
- Callback group: `planning_group` (mutually exclusive)
- Subscribes: `vehicle_pose` (realtime_qos), `obstacle_map` (reliable_qos)
- Publishes: `planned_route` at 5 Hz using `reliable_qos`
- Timer: `planning_tick` at 5 Hz

**Parameters:** `planning_rate`, `destination_x` (500 m), `destination_y` (0 m), `waypoint_spacing` (20 m)

**Simulation behaviour:**

*`on_vehicle_pose`* — Stores current X/Y position in `current_x_`, `current_y_`. Sets `has_pose_ = true` to unblock the planning timer.

*`on_obstacle_map`* — Scans the grid column-by-column from column 5 onward. For each column it checks a ±7 cell band around the truck's forward axis (grid centre row). The first occupied cell (value > 50) sets `nearest_obs_m_` = that column index (1 column = 1 metre because resolution is 1.0).

*`planning_tick`* — Generates 5 lookahead waypoints spaced `waypoint_spacing_` metres apart from the current position toward the destination:
- If `nearest_obs_m_ < 40` (obstacle within 40 m): applies a 9 m lateral offset to the first waypoint that decays linearly to 0 m by the fifth waypoint → smooth S-curve detour
- Each waypoint is a `geometry_msgs/PoseStamped` appended to a `nav_msgs/Path`
- When `remaining < 5 m`: publishes an empty path and logs "Destination reached"

**Why mutually exclusive callback group?** The `on_obstacle_map` callback writes to `nearest_obs_m_` and `planning_tick` reads it. Making them mutually exclusive prevents the data race without needing a mutex.

---

### `vehicle_controller` — Motion Control

**Real-world role:** Converts the planned route and hazard level into actuator commands (linear speed + steering angle), equivalent to the CAT autonomous truck's drive-by-wire controller.

**Contract summary:**
- Executor: multi-threaded (2 threads)
- Callback groups: `control_group` (mutually exclusive), `service_group` (reentrant)
- Subscribes: `planned_route` (reliable_qos, control_group), `hazard_alert` (safety_qos, control_group)
- Publishes: `drive_command` at 20 Hz using `reliable_qos`
- Service client: `request_stop` (calls hazard_detector)
- Timer: `control_tick` at 20 Hz

**Parameters:** `max_speed` (12 m/s), `max_steering` (25°), `control_rate` (20 Hz), `brake_decel` (2.5 m/s²)

**Simulation behaviour:**

*`on_planned_route`* — Stores the first waypoint's X and Y into `next_wp_x_`, `next_wp_y_`. Sets `has_route_ = true`.

*`on_hazard_alert`* — Iterates through all `DiagnosticStatus` entries in the array. Assigns `hazard_level_` = 0 (OK), 1 (WARN), or 2 (ERROR). On first ERROR detection:
- Sets `emergency_ = true`
- Asynchronously calls `/truck/safety/request_stop`; clears `emergency_` in the service callback when acknowledged

*`control_tick`* — Every 50 ms:
- If `emergency_` or no route: decelerates at `brake_decel_` m/s² until stopped, publishes zero velocity
- Otherwise: `target_speed = max_speed_` (halved to 40 % on WARN)
- Applies exponential smoothing: `current_speed_ += (target - current) × 0.08`
- Steering angle = `atan2(next_wp_y_, next_wp_x_)`, clamped to ±`max_steering_` in radians
- Publishes `geometry_msgs/Twist` with `linear.x = current_speed_`, `angular.z = steering`

**Two callback groups, why?**
- `control_group` is mutually exclusive: `on_planned_route`, `on_hazard_alert`, and `control_tick` share state variables (`next_wp_x_`, `hazard_level_`, `current_speed_`). Making them mutually exclusive is the simplest thread-safety approach.
- `service_group` is reentrant: the `request_stop` response callback must not wait for `control_group` to finish — otherwise a slow `control_tick` would deadlock the emergency path.

---

### `hazard_detector` — Safety

**Real-world role:** Real-time proximity check between the truck and detected obstacles, equivalent to the CAT autonomous truck's collision avoidance proximity system (CAS). Also acts as the authority for emergency stops.

**Contract summary:**
- Executor: multi-threaded (2 threads)
- Callback groups: `monitor_group` (mutually exclusive), `service_group` (reentrant)
- Subscribes: `obstacle_map` (reliable_qos, monitor_group), `drive_command` (reliable_qos, monitor_group)
- Publishes: `hazard_alert` at 50 Hz using `safety_qos`
- Service server: `request_stop` (service_group)
- Timer: `hazard_check` at 50 Hz

**Parameters:** `critical_distance` (8 m), `warning_distance` (20 m), `check_rate` (50 Hz)

**Simulation behaviour:**

*`on_obstacle_map`* — Same forward-column scan as route_planner but with ±8 cell band. Stores the nearest occupied cell distance in metres as `nearest_dist_m_`.

*`on_drive_command`* — Stores `commanded_speed_` for future use (speed-dependent braking distance could extend the warning zone in a full implementation).

*`on_request_stop`* — Service server handler. Sets `emergency_active_ = true`, returns `success=true`. Logs `EMERGENCY STOP received`.

*`hazard_check` (50 Hz)* — Three-tier classification:
1. If `emergency_active_`: publish ERROR, message "Emergency stop active"
2. If `nearest_dist_m_ < critical_distance_` (8 m): increment `consecutive_warn_`. After 5 consecutive ticks (100 ms), publish ERROR; before that, publish WARN
3. If `nearest_dist_m_ < warning_distance_` (20 m): reset `consecutive_warn_`, publish WARN
4. Otherwise: reset `consecutive_warn_`, publish OK

The 5-consecutive-tick filter prevents a single spurious LiDAR reading from triggering a full emergency stop. This is the same debounce approach used in real safety-rated proximity systems.

**`safety_qos` with `lifespan_ms: 100`** — If `hazard_check` stops running (node crash, CPU overload), the last WARN/OK alert expires after 100 ms. The `vehicle_controller` will see no new hazard_alert, and its `safety_qos` subscriber deadline of 20 ms will fire — making a silent failure detectable.

---

### `truck_health_monitor` — Mechanical Health

**Real-world role:** Continuously monitors engine temperature, hydraulic pressure, and payload — similar to how CAT's VIMS (Vital Information Management System) polls CAN bus sensors and reports to the Cat Command control centre.

**Contract summary:**
- Executor: single-threaded
- Callback group: `health_group` (mutually exclusive)
- Publishes: `health_report` at 2 Hz using `reliable_qos`
- Service server: `system_status` (health_group)
- Timer: `health_tick` at 2 Hz

**Parameters:** `engine_temp_max` (95°C), `hydraulic_pressure_min` (150 bar), `report_rate` (2 Hz)

**Simulation behaviour:**

*`health_tick` (2 Hz)*:
- Engine temperature: `75 + 15·sin(tick·0.05) + 3·sin(tick·0.23)` °C — thermal cycling with a superimposed ripple
- Hydraulic pressure: `172 − 18·|sin(tick·0.07)|` bar — drops during heavy work, recovers during coasting
- For each metric, publishes a `DiagnosticStatus` entry: `OK` if within limits, `WARN` if exceeding threshold
- All entries collected into one `DiagnosticArray` and published on `/truck/health/health_report`
- Logs engine temp and hydraulic pressure every 4 ticks (every 2 seconds)

*`on_system_status`* — Service handler called by an external monitor or operator console. Responds with `success=true` if both engine temp ≤ max and hydraulic pressure ≥ min. On degraded state, message includes the out-of-range values.

---

## Data Flow — Step by Step

1. **`gps_imu_fusion`** publishes a noisy `PoseStamped` at 20 Hz → `/truck/localization/vehicle_pose`

2. **`route_planner`** receives the pose, stores current X/Y. In parallel it has received an `OccupancyGrid` from `lidar_processor` and knows the nearest obstacle column distance.

3. At 5 Hz, **`route_planner`** runs `planning_tick`: computes 5 lookahead waypoints. If an obstacle is within 40 m it adds a 9 m lateral detour. Publishes `nav_msgs/Path` → `/truck/planning/planned_route`.

4. **`vehicle_controller`** receives the path, stores the first waypoint. At 20 Hz it runs `control_tick`: steers toward the waypoint using atan2, applies exponential speed smoothing, publishes `geometry_msgs/Twist` → `/truck/control/drive_command`.

5. **`hazard_detector`** independently monitors the `obstacle_map` at 50 Hz. When an obstacle is within 8 m for 5 consecutive cycles it publishes an ERROR `DiagnosticArray` → `/truck/safety/hazard_alert`.

6. **`vehicle_controller`** receives the ERROR hazard alert, sets `emergency_ = true`, publishes a zero-velocity `Twist`, and calls `/truck/safety/request_stop` asynchronously.

7. **`hazard_detector`** handles the `request_stop` service call, sets `emergency_active_ = true`, acknowledges. Future `hazard_check` ticks continue to report ERROR until the obstacle clears.

8. **`truck_health_monitor`** runs independently at 2 Hz, publishing engine/hydraulic status. Any subscriber (operator HMI, fleet management system) can read `/truck/health/health_report` or call `/truck/health/system_status`.

---

## How the YAML Files Relate to Each Other

```
application.yaml
    │
    ├── discovery_profile: truck_super_client ──► discovery_profiles.yaml
    │                                                 └── Dockerfile ENV vars
    │
    └── nodes: [gps_imu_fusion, ...]
              │
              └── contracts/<node>.yaml (one per node)
                        │
                        ├── parameter_ref: ──────────────► parameters.yaml
                        │                                    └── declare_parameter() calls
                        │
                        ├── interactions[].qos_profile: ─► qos_profiles.yaml
                        │                                    └── rclcpp::QoS settings
                        │
                        └── interactions[].dataref: ─────► data.yaml
                                                            ├── type → #include + template args
                                                            └── topic_path/service_path → string in create_publisher/subscriber/service
```

The generator (`tool/generator.py`) walks this dependency graph, resolves all references, and feeds everything into Jinja2 templates to produce C++ source files.

---

## Build and Run

### Generate C++ from YAML

```bash
cd Version3.0
python tool/generator.py --project autonomous_haul_truck
```

Expected output:
```
OK gps_imu_fusion   [1pub  0sub  0srv_server  0srv_client  1timer]
OK lidar_processor  [1pub  0sub  0srv_server  0srv_client  1timer]
OK route_planner    [1pub  2sub  0srv_server  0srv_client  1timer]
OK vehicle_controller [1pub 2sub  0srv_server  1srv_client  1timer]
OK hazard_detector  [1pub  2sub  1srv_server  0srv_client  1timer]
OK truck_health_monitor [1pub 0sub  1srv_server  0srv_client  1timer]
OK CMakeLists.txt + package.xml  (deps: diagnostic_msgs, geometry_msgs, nav_msgs, rclcpp, std_srvs)
```

### Build Docker Image

```bash
docker build --no-cache -f tool/Dockerfile -t ros2_v3_haul_truck .
```

### Run

```bash
# Start FastDDS Discovery Server first
fastdds discovery -i 0 -p 11811

# Run all nodes
docker run --rm --network host ros2_v3_haul_truck
```

### Observe Topics

```bash
ros2 topic echo /truck/localization/vehicle_pose
ros2 topic echo /truck/safety/hazard_alert
ros2 topic echo /truck/health/health_report
ros2 service call /truck/health/system_status std_srvs/srv/Trigger
```

---

## Expected Console Output (runtime)

```
[gps_imu_fusion] GPS/IMU: pos=(40.0, 0.06)  heading=0.0012 rad
[route_planner]  Obstacle at 22m — detouring 9.0m lateral, 460.0m to destination
[hazard_detector] Obstacle approaching (19.3m)
[hazard_detector] CRITICAL obstacle at 7.9m
[vehicle_controller] EMERGENCY: hazard at critical level — stopping
[vehicle_controller] Stop ack: Emergency stop acknowledged by hazard_detector
[truck_health_monitor] Health: engine=82C  hydraulic=165bar
[site_logger]    GPS/IMU: pos=(200.0, 0.14)  heading=0.0031 rad
```
