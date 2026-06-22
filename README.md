# ROS2 Discovery Server — Version 3.0

Version 3.0 is a **YAML-driven ROS2 code generator**. You declare every node's behaviour in config files — topics, services, QoS policies, parameters, callback groups, and the FastDDS Discovery Server configuration. The generator reads those files and produces ready-to-compile C++ and a Dockerfile. You only write business logic.

---

## What Changed from V2

| Feature | V2 | V3 |
|---|---|---|
| Node roles | One role per node file | One contract per node — all roles (pub + sub + server + client + timers) in one file |
| Interface registry | `topics.yaml` + inline `srv_type` | `data.yaml` — unified messages + services registry |
| Node config files | `nodes/<name>.yaml` with inline `publishers:` / `subscribers:` lists | `contracts/<name>.yaml` with flat `interactions:` list using `dataref:` |
| Discovery config | Static `discovery.yaml` or `docker/env.sh` | `discovery_profiles.yaml` — named profiles, referenced by `application.yaml` |
| Parameters | Bare `key: value` | `key: {value: x, type: double}` → typed `declare_parameter<T>()` |
| Callback groups | Not supported | `mutually_exclusive` / `reentrant` per callback, declared in contract |
| Executor | Always `rclcpp::spin` | `single_threaded` or `multi_threaded` with configurable thread count |
| Dockerfile | Manual per project | Generated from `Dockerfile.jinja` via `discovery_profiles.yaml` → ENV vars |
| Generator | Single `generator.py` monolith | Split: `loader.py` (YAML loading + validation) + `generator.py` (Jinja2 rendering) |

---

## Repository Structure

```
Version3.0/
├── tool/
│   ├── generator/
│   │   ├── generator.py       # Jinja2 rendering only — imports loader
│   │   └── loader.py          # YAML loading, validation, type resolution (no Jinja2)
│   ├── templates/
│   │   ├── node.cpp.jinja     # C++ node body with impl block preservation
│   │   ├── node.hpp.jinja     # Node class header
│   │   ├── CMakeLists.txt.jinja
│   │   ├── package.xml.jinja
│   │   └── Dockerfile.jinja   # FastDDS SUPER_CLIENT environment config
│   └── docker/
│       ├── entrypoint.sh      # Container entrypoint
│       └── Dockerfile         # Generated per project (overwritten on each run)
│
├── robot_arm_controller/      # Demo: multi-role robot arm (domain_id 10)
├── automotive_adas_stack/     # Demo: CAT ADAS pipeline (domain_id 20)
├── autonomous_haul_truck/     # Demo: CAT autonomous haul truck (domain_id 30)
└── mining_load_cycle/         # Demo: CAT excavator-truck load cycle (domain_id 40)
```

Each project has the same layout:

```
<project>/
├── config/
│   ├── application.yaml        # Package name, domain_id, discovery_profile, node list
│   ├── data.yaml               # Shared type registry — all messages and services
│   ├── qos_profiles.yaml       # Named QoS profiles used in contracts
│   ├── parameters.yaml         # Typed parameters per node group
│   ├── discovery_profiles.yaml # FastDDS DS address and mode
│   └── contracts/
│       └── <node_name>.yaml    # One per node: interactions, callback groups, timers
└── generated_pkg/
    ├── include/<node>.hpp
    ├── src/<node>.cpp
    ├── CMakeLists.txt
    └── package.xml
```

---

## YAML → Code Flow

```
application.yaml
    │
    ├── domain_id ──────────────────────────────► ENV ROS_DOMAIN_ID in Dockerfile
    │
    ├── discovery_profile: <name> ──────────────► discovery_profiles.yaml
    │                                               └── ip + port → ENV ROS_DISCOVERY_SERVER
    │                                               └── mode: SUPER_CLIENT → participant XML
    │
    └── nodes: [node_a, node_b, ...]
              │
              └── contracts/<node>.yaml
                        │
                        ├── parameter_ref: <group> ──► parameters.yaml
                        │                               └── typed declare_parameter<T>()
                        │
                        ├── interactions[].qos_profile: ──► qos_profiles.yaml
                        │                                    └── rclcpp::QoS depth/reliability/
                        │                                        deadline_ms/lifespan_ms
                        │
                        └── interactions[].dataref: ──────► data.yaml
                                                             ├── type → #include + C++ template args
                                                             └── topic_path / service_path → string
```

**Generator outputs (per run):**

```
generated_pkg/include/<node>.hpp   ← class declaration, subscriber/service signatures, parameters
generated_pkg/src/<node>.cpp       ← constructor wiring + impl block stubs (preserved on re-runs)
generated_pkg/CMakeLists.txt       ← ament targets, all deps auto-resolved from data.yaml types
generated_pkg/package.xml          ← ROS2 package manifest
tool/docker/Dockerfile                    ← FastDDS SUPER_CLIENT config baked in
```

---

## Config File Roles — Quick Reference

### `application.yaml`

Declares the package identity and node list. The generator uses `nodes:` as the ordered list of contract files to process.

```yaml
application:
  name: my_project       # → package name in CMakeLists + package.xml
  domain_id: 30          # → ROS_DOMAIN_ID in Dockerfile
  discovery_profile: my_ds_profile   # → key in discovery_profiles.yaml
  nodes:
    - node_a
    - node_b
```

### `data.yaml`

The **single source of truth** for all pub-sub channels and service endpoints. Contract files reference entries here by name (`dataref:`). A type mismatch between publisher and subscriber becomes impossible at generation time.

```yaml
data:
  messages:
    my_topic:
      type: sensor_msgs/msg/JointState  # resolves to C++ type + #include
      topic_path: /my/namespace/my_topic

  services:
    my_service:
      type: std_srvs/srv/Trigger
      service_path: /my/namespace/my_service
```

### `qos_profiles.yaml`

Named policies applied by reference in contracts. Supported fields: `reliability`, `history`, `depth`, `deadline_ms`, `lifespan_ms`, `durability`.

```yaml
qos_profiles:
  sensor_qos:
    reliability: best_effort
    depth: 5
    deadline_ms: 20

  safety_qos:
    reliability: reliable
    depth: 10
    deadline_ms: 20
    lifespan_ms: 100    # alert expires after 100 ms if node goes silent
```

### `discovery_profiles.yaml`

Maps a profile name to a FastDDS Discovery Server address. The generator inserts `ROS_DISCOVERY_SERVER` and the participant XML into the Dockerfile.

```yaml
discovery_profiles:
  my_ds_profile:
    mode: SUPER_CLIENT    # all nodes act as DS clients
    servers:
      - ip: 127.0.0.1
        port: 11811
```

### `parameters.yaml`

Typed parameter values per node. The generator emits `declare_parameter<T>(name, default)` for each entry. Supported types: `double`, `int`, `bool`, `string`, `string_array`.

```yaml
parameters:
  my_node_params:
    scan_rate:
      type: double
      value: 10.0
    joint_names:
      type: string_array
      value: [shoulder, elbow, wrist]
```

### `contracts/<node>.yaml`

Specifies one node's full behavioural contract. The key concepts:

- **`interactions:`** flat list of all pub-sub and service roles
- **`dataref:`** references a key in `data.yaml` — type and topic/service path are resolved from there
- **`callback_group:`** pins a callback to a named group (thread-safety)
- **`timers:`** declares timer methods with their rate and callback group

```yaml
contract:
  node:
    name: my_node
    executor: { type: multi_threaded, threads: 2 }
    parameter_ref: my_node_params

  callback_groups:
    - name: data_group
      type: mutually_exclusive
    - name: service_group
      type: reentrant           # service servers use reentrant so they never block

  interactions:
    - type: pub-sub
      role: producer            # publisher
      dataref: my_topic
      qos_profile: sensor_qos

    - type: pub-sub
      role: consumer            # subscriber
      dataref: my_topic
      qos_profile: sensor_qos
      callback_group: data_group

    - type: service
      role: server              # service server
      dataref: my_service
      callback_group: service_group

    - type: service
      role: client              # service client (async calls in business logic)
      dataref: my_service

  timers:
    - name: my_timer
      rate_hz: 10.0
      callback_group: data_group
```

---

## Impl Block Preservation

The generator protects your business logic across re-runs using fenced impl blocks:

```cpp
void MyNode::my_timer()
{
    //-- begin impl [my_timer] ----------------------------------------
    // Your code here — preserved on every generator re-run
    //-- end impl [my_timer] ------------------------------------------
}
```

When you re-run the generator (e.g. after adding a new topic to the contract), it extracts all existing impl blocks from the current `.cpp` file and injects them back into the freshly rendered file. The surrounding boilerplate (constructor, publisher setup, subscriber wiring) is regenerated cleanly; only the impl blocks survive as-is.

---

## Demo Projects

| Project | Domain | Nodes | Theme |
|---|---|---|---|
| [robot_arm_controller](robot_arm_controller/README.md) | 10 | 2 | Robot arm with cross-node service calls |
| [automotive_adas_stack](automotive_adas_stack/) | 20 | 6 | ADAS pipeline: sensor fusion → object detection → path planning → decision making |
| [autonomous_haul_truck](autonomous_haul_truck/README.md) | 30 | 6 | Autonomous 500 m haul run with obstacle avoidance and hazard detection |
| [mining_load_cycle](mining_load_cycle/README.md) | 40 | 8 | Excavator-truck load cycle with 4-state dispatcher FSM |

Each demo has its own `README.md` explaining the YAML files, node behaviour, data flow, and build/run instructions.

---

## Generating a Project

Run from the project directory:

```bash
python3 tool/generator/generator.py --project <project_name>
```

The generator prints a summary line per node:

```
OK gps_imu_fusion  [1pub  0sub  0srv_server  0srv_client  1timer]
OK route_planner   [1pub  2sub  0srv_server  0srv_client  1timer]
OK CMakeLists.txt + package.xml  (deps: geometry_msgs, nav_msgs, rclcpp, std_srvs)
```

Then build and run:

```bash
docker build --no-cache -f tool/docker/Dockerfile -t ros2_v3_<project> .
fastdds discovery -i 0 -p 11811   # terminal 1
docker run --rm --network host ros2_v3_<project>   # terminal 2
```

---

## Prerequisites

| Tool | Version | Purpose |
|---|---|---|
| Python | 3.8+ | Running the generator |
| pyyaml | any | YAML parsing |
| jinja2 | any | Template rendering |
| Docker Desktop | 20.10+ | Building and running containers |

```bash
pip install pyyaml jinja2
```
