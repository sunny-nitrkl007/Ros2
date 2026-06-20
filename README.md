# ROS2 Discovery Server — YAML Driven Testing Framework

A YAML-driven code generation framework for testing ROS2 Fast-DDS Discovery Server capabilities.
Define your system topology in YAML, run the generator, get compiled C++ ROS2 nodes inside a Docker container — all without writing a single line of C++ manually.

---

## What This Project Tests

The Fast-DDS Discovery Server is a centralised replacement for multicast-based DDS participant discovery.
Instead of nodes broadcasting their presence and finding each other via UDP multicast, every node registers with a known server. This is critical for:
- Cloud and container deployments where multicast is unavailable
- Large networks where multicast traffic is unwanted
- Multi-robot systems where namespaced isolation is required
- High-availability setups requiring server redundancy

Each project folder below tests a specific capability or topology through the discovery server.

---

## Project Map

| Folder | Topology | What It Tests |
|---|---|---|
| [discovery_server_single_pub_sub](discovery_server_single_pub_sub/README.md) | 1 pub → 1 sub | Baseline: basic pub/sub through discovery server |
| [discovery_server_single_pub_multi_sub](discovery_server_single_pub_multi_sub/README.md) | 1 pub → 3 subs | One publisher broadcasting to multiple subscribers (fan-out) |
| [discovery_server_multi_pub_single_sub](discovery_server_multi_pub_single_sub/README.md) | 3 pubs → 1 sub | Multiple publishers at different rates into one subscriber (fan-in) |
| [discovery_server_services](discovery_server_services/README.md) | server ↔ client | ROS2 services (request/response) through discovery server |
| [discovery_server_multi_robot](discovery_server_multi_robot/README.md) | /robot1 + /robot2 | Namespace isolation — two robots, same server, separate topics |
| [discovery_server_failover](discovery_server_failover/README.md) | primary + backup | Server redundancy — nodes survive primary server failure |
| [discovery_server_actions](discovery_server_actions/README.md) | server ↔ client | ROS2 actions (goal/feedback/result) through discovery server |
| [discovery_server_lifecycle](discovery_server_lifecycle/README.md) | pub + sub | Lifecycle node state machine (configure/activate/deactivate) |
| [discovery_server_custom_interfaces](discovery_server_custom_interfaces/README.md) | pub + sub + svc | User-defined `.msg` and `.srv` types via a local interfaces package |

---

## Repository Structure

```
Version2.0/
├── tool/                                    # Shared infrastructure — used by all projects
│   ├── generator.py                         # YAML + Jinja2 → C++ + Dockerfile generator
│   ├── Dockerfile                           # Generated per project by generator.py (do not edit)
│   ├── env.sh                               # ROS_DOMAIN_ID and ROS_DISCOVERY_SERVER
│   ├── entrypoint.sh                        # Container entrypoint (sources ROS + env)
│   └── templates/                           # Jinja2 templates
│       ├── publisher.cpp.jinja
│       ├── subscriber.cpp.jinja
│       ├── service_server.cpp.jinja
│       ├── service_client.cpp.jinja
│       ├── action_server.cpp.jinja
│       ├── action_client.cpp.jinja
│       ├── lifecycle_publisher.cpp.jinja
│       ├── lifecycle_subscriber.cpp.jinja
│       ├── node.hpp.jinja
│       ├── CMakeLists.txt.jinja
│       ├── package.xml.jinja
│       └── Dockerfile.jinja                 # Dockerfile template (supports extra/apt packages)
│
├── discovery_server_single_pub_sub/
├── discovery_server_single_pub_multi_sub/
├── discovery_server_multi_pub_single_sub/
├── discovery_server_services/
├── discovery_server_multi_robot/
├── discovery_server_failover/
├── discovery_server_actions/
├── discovery_server_lifecycle/
└── discovery_server_custom_interfaces/
```

Each project has the same internal structure:

```
<project>/
├── config/
│   ├── application.yaml       # Node list, domain_id, extra_packages, apt_packages
│   ├── nodes/<name>.yaml      # Per-node: type, topic/service/action, QoS, params
│   ├── topics.yaml            # Topic name → message type mapping
│   ├── qos_profiles.yaml      # Named QoS profiles
│   ├── parameters.yaml        # Runtime parameter values per node
│   └── discovery.yaml         # Documentation only — not read by generator
├── generated_pkg/             # Generator output — do not edit by hand
└── README.md                  # Project-specific test guide
```

Projects that need custom behaviour can add:

```
<project>/
├── templates/                 # Optional: overrides any shared template (same filename wins)
└── custom_interfaces_pkg/     # Optional: defines .msg / .srv / .action types
```

---

## How the Generator Works

Run the generator first, then build the Docker image. The generator writes `tool/Dockerfile` for the current project before each build.

```
python tool/generator.py --project <name>
docker build --no-cache -f tool/Dockerfile -t <image> .
```

### YAML → Code Flow

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           config/application.yaml                               │
│                                                                                 │
│   name: my_system          ──► (project metadata)                               │
│   domain_id: 10            ──► (documentation, not used in generated code)      │
│   extra_packages:          ──► Dockerfile: COPY + colcon build (all packages)   │
│     - custom_interfaces_pkg                                                     │
│   apt_packages:            ──► Dockerfile: RUN apt-get install ...              │
│     - ros-jazzy-example-interfaces                                              │
│   nodes:                   ──► drives the generation loop below                 │
│     - node_a                                                                    │
│     - node_b                                                                    │
└──────────────────────────┬──────────────────────────────────────────────────────┘
                           │  for each node name
                           ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                      config/nodes/<name>.yaml                                   │
│                                                                                 │
│   type: publisher          ──► publisher.cpp.jinja    ──► src/<name>.cpp        │
│   type: subscriber         ──► subscriber.cpp.jinja   ──► src/<name>.cpp        │
│   type: service_server     ──► service_server.cpp.jinja                         │
│   type: service_client     ──► service_client.cpp.jinja                         │
│   type: action_server      ──► action_server.cpp.jinja                          │
│   type: action_client      ──► action_client.cpp.jinja                          │
│   type: lifecycle_publisher ─► lifecycle_publisher.cpp.jinja                    │
│   type: lifecycle_subscriber ► lifecycle_subscriber.cpp.jinja                   │
│                                                        ──► include/<name>.hpp   │
│                                                            (node.hpp.jinja)     │
└────┬────────────────┬────────────────┬────────────────────────────────────────--┘
     │                │                │
     │ topic: x       │ qos: y         │ parameters: z
     ▼                ▼                ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────────────────────────┐
│ topics.yaml  │ │qos_profiles  │ │ parameters.yaml                  │
│              │ │   .yaml      │ │                                  │
│ sensor_data: │ │ sensor_qos:  │ │ sensor_publisher:                │
│   type:      │ │  reliability:│ │   publish_rate: 1.0              │
│   custom_    │ │    reliable  │ │   sensor_id: temp_sensor_01      │
│   interfaces │ │  depth: 10   │ │                                  │
│   _pkg/msg/  │ │              │ │ ──► declare_parameter() calls    │
│   Sensor     │ │ ──► QoS      │ │     in generated .cpp            │
│              │ │     settings │ └──────────────────────────────────┘
│ ──► C++ type │ │     in .cpp  │
│     include  │ └──────────────┘
│     path     │
└──────────────┘
     │
     │  Type conversions applied by generator.py
     ▼
  custom_interfaces_pkg/msg/Sensor
    ├── msg_to_cpp()      ──► custom_interfaces_pkg::msg::Sensor   (in .cpp)
    └── msg_to_include()  ──► custom_interfaces_pkg/msg/sensor.hpp (#include)
                               └── camel_to_snake: Sensor → sensor

  custom_interfaces_pkg/srv/SensorQuery
    ├── srv_to_cpp()      ──► custom_interfaces_pkg::srv::SensorQuery
    └── srv_to_include()  ──► custom_interfaces_pkg/srv/sensor_query.hpp
                               └── camel_to_snake: SensorQuery → sensor_query

     │
     ▼
  ──► generated_pkg/CMakeLists.txt    (CMakeLists.txt.jinja)
  ──► generated_pkg/package.xml       (package.xml.jinja)
  ──► tool/Dockerfile                 (Dockerfile.jinja)
```

---

## Generator Commands

Run from `Version2.0/`:

```bash
python tool/generator.py --project discovery_server_single_pub_sub
python tool/generator.py --project discovery_server_single_pub_multi_sub
python tool/generator.py --project discovery_server_multi_pub_single_sub
python tool/generator.py --project discovery_server_services
python tool/generator.py --project discovery_server_multi_robot
python tool/generator.py --project discovery_server_failover
python tool/generator.py --project discovery_server_actions
python tool/generator.py --project discovery_server_lifecycle
python tool/generator.py --project discovery_server_custom_interfaces
```

---

## Docker — Build and Run

Always run from `Version2.0/`. Always run the generator first — it writes `tool/Dockerfile` for the selected project.

### Build

```
# Step 1 — generate code + Dockerfile for the project
python tool/generator.py --project <project_name>

# Step 2 — build the image
docker build --no-cache -f tool/Dockerfile -t <image_name> .
```

| Project | Image name |
|---|---|
| discovery_server_single_pub_sub | ros2_ds_single |
| discovery_server_single_pub_multi_sub | ros2_discovery_server_single_pub_multi_sub |
| discovery_server_multi_pub_single_sub | ros2_discovery_server_multi_pub_single_sub |
| discovery_server_services | ros2_discovery_server_services |
| discovery_server_multi_robot | ros2_discovery_server_multi_robot |
| discovery_server_failover | ros2_discovery_server_failover |
| discovery_server_actions | ros2_ds_actions |
| discovery_server_lifecycle | ros2_ds_lifecycle |
| discovery_server_custom_interfaces | ros2_ds_custom_interfaces |

### Run

```bash
docker run -it --rm --name <container_name> <image_name> bash
```

Then in separate terminals:
```bash
docker exec -it <container_name> bash
```

---

## Prerequisites

| Requirement | Version | Purpose |
|---|---|---|
| Python | 3.8+ | Running the generator |
| pyyaml | any | Generator reads YAML |
| jinja2 | any | Generator renders templates |
| Docker Desktop | 20.10+ | Building and running ROS2 container |

```bash
pip install pyyaml jinja2
```

---

## Supported Node Types

| YAML `type` | Template Used | What It Generates |
|---|---|---|
| `publisher` | `publisher.cpp.jinja` | Timer-driven publisher, configurable rate and message type |
| `subscriber` | `subscriber.cpp.jinja` | Subscription with configurable log prefix |
| `service_server` | `service_server.cpp.jinja` | Service server, responds to any `srv_type` |
| `service_client` | `service_client.cpp.jinja` | Periodic service caller, configurable interval |
| `action_server` | `action_server.cpp.jinja` | Action server with goal/feedback/result and step delay |
| `action_client` | `action_client.cpp.jinja` | Periodic action goal sender with feedback logging |
| `lifecycle_publisher` | `lifecycle_publisher.cpp.jinja` | Publisher with 5-state lifecycle state machine |
| `lifecycle_subscriber` | `lifecycle_subscriber.cpp.jinja` | Subscriber with lifecycle-gated message processing |

---

## Adding a New Project

1. Create `my_project/config/` with `application.yaml`, `nodes/*.yaml`, `topics.yaml`, `qos_profiles.yaml`, `parameters.yaml`
2. Run: `python tool/generator.py --project my_project`
3. Build: `docker build --no-cache -f tool/Dockerfile -t my_image .`
4. Add `my_project/README.md` with test steps

No changes to `tool/` needed for standard node types.

**To override a template for one project:** place a file with the same name inside `my_project/templates/`. Generator checks project templates first and falls back to `tool/templates/`.

**To add custom `.msg` / `.srv` / `.action` types:** create `my_project/custom_interfaces_pkg/` with its own `CMakeLists.txt` and `package.xml`, add `extra_packages: [custom_interfaces_pkg]` to `application.yaml`, and reference the type as `custom_interfaces_pkg/msg/MyType` in YAML. The generator handles everything else.

**To install extra apt packages:** add `apt_packages: [ros-jazzy-<pkg>]` to `application.yaml`.

---

## Discovery Server vs Normal DDS — What Actually Differs

The generated nodes are plain ROS2 nodes — no mention of the discovery server anywhere in the C++ code. The discovery mode is a **runtime environment decision** controlled entirely by the `ROS_DISCOVERY_SERVER` env var.

### Scenario A — env var set, DS not running

Fast-DDS switches to SERVER discovery mode and keeps retrying. Nodes never find each other → **no communication.**

### Scenario B — env var not set

Fast-DDS falls back to **SIMPLE discovery** (UDP multicast). Inside the same Docker container this works → **nodes discover each other without a DS.**

### How the two modes compare

```
Normal DDS (SIMPLE):
  talker   ──multicast "I exist"──►  (broadcast on LAN)
  listener ◄──multicast "I exist"──  (broadcast on LAN)
  Both hear each other → connect directly for data

Discovery Server:
  talker   ──register──►  DS  (unicast, known address)
  listener ──register──►  DS  (unicast, known address)
  DS tells talker about listener and vice versa
  Both then connect directly for data
```

Data transfer is **direct unicast** in both cases — the DS only changes how nodes *find* each other.

### When to use DS over multicast

| Situation | SIMPLE (multicast) | Discovery Server |
|---|---|---|
| Same machine / container | Works | Works |
| Docker across different hosts | Fails (multicast blocked) | Works |
| Cloud VMs | Fails (no multicast routing) | Works |
| Large network (100+ nodes) | Floods network | Scales fine |
| Cross-subnet | Fails | Works (plain unicast) |

---

## Real-World Context — Automotive / ECU Networks

### Why ECUs can't use normal DDS

```
Normal DDS (SIMPLE):   broadcast "I exist" on multicast group
ECU network reality:   CAN bus, automotive Ethernet — multicast BLOCKED
Result:                nodes never find each other
```

### How ECUs map to this project's concepts

| This project | ECU world equivalent |
|---|---|
| Docker container | One ECU (one compute domain) |
| talker node | ECU publishing sensor data (e.g. radar, lidar) |
| listener node | ECU consuming that data (e.g. fusion ECU) |
| Discovery Server | Central Domain Controller |
| `ROS_DISCOVERY_SERVER` env var | ECU bootloader config pointing to DS IP |
| Namespace (`/robot1`) | Vehicle zone (`/front_axle`, `/powertrain`) |
| Failover project | Redundant Domain Controllers (safety-critical) |

### Where micro-ROS comes in

```
MCU (micro-ROS) ──XRCE-DDS──► micro-ROS Agent (Linux) ──FastDDS──► Discovery Server
                  (serial/UDP)  (full DDS translator)
```

### Future scope

```
V3.0 (Docker Compose):
  container_ds           → Discovery Server (central domain controller)
  container_sensor_ecu   → radar/lidar publisher (plain CLIENT)
  container_fusion_ecu   → data aggregator (SUPER_CLIENT)
  container_actuator_ecu → brake/motor controller

V4.0 (Real hardware):
  Raspberry Pi / Jetson  → runs DS + fusion ECU
  STM32 (micro-ROS)      → sensor ECU via micro-ROS agent
  ESP32 (micro-ROS)      → actuator ECU
```

---

## Troubleshooting

| Problem | Cause | Fix |
|---|---|---|
| `Package 'generated_pkg' not found` | `AMENT_PREFIX_PATH` not set | Dockerfile sets it via `ENV` — rebuild with `--no-cache` |
| `fastdds: command not found` | ROS2 not sourced | Run `source /opt/ros/jazzy/setup.bash` first |
| Listener receives nothing | DS not running or `ROS_DISCOVERY_SERVER` missing | Ensure server is up; env var is set via `env.sh` in every exec session |
| Wrong executables in image | Generator not re-run before build | Always run `python tool/generator.py --project <name>` before `docker build` |
| `ros2 topic list` shows nothing | FastDDS DS limitation | DS propagates participant presence only — use `ros2 node list` instead |
| Docker build `COPY` error | `docker build` run from wrong directory | Must run from `Version2.0/` root, not from inside the project folder |
