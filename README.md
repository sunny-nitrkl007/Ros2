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
| [ds_fan_out](ds_fan_out/README.md) | 1 pub → 3 subs | Fan-out: one publisher, many subscribers on same topic |
| [ds_fan_in](ds_fan_in/README.md) | 3 pubs → 1 sub | Fan-in: many publishers at different rates, one subscriber |
| [ds_services](ds_services/README.md) | server ↔ client | ROS2 services (request/response) through discovery server |
| [ds_multi_robot](ds_multi_robot/README.md) | /robot1 + /robot2 | Namespace isolation — two robots, same server, separate topics |
| [ds_failover](ds_failover/README.md) | primary + backup | Server redundancy — nodes survive primary server failure |

---

## Repository Structure

```
discoveryTesting/
├── tool/                                    # Reusable infrastructure — shared across all projects
│   ├── generator.py                         # YAML + Jinja → C++ code generator
│   ├── Dockerfile                           # Builds any project (uses ARG PROJECT)
│   ├── entrypoint.sh                        # Container entrypoint
│   ├── client.xml                           # Fast-DDS SUPER_CLIENT profile (single server)
│   ├── server.xml                           # Fast-DDS SERVER profile (optional)
│   └── templates/                           # Shared Jinja2 templates (all projects use these)
│       ├── publisher.cpp.jinja
│       ├── subscriber.cpp.jinja
│       ├── service_server.cpp.jinja
│       ├── service_client.cpp.jinja
│       ├── node.hpp.jinja
│       ├── CMakeLists.txt.jinja
│       └── package.xml.jinja
│
├── discovery_server_single_pub_sub/         # Project 1 — baseline pub/sub
├── ds_fan_out/                              # Project 2 — fan-out topology
├── ds_fan_in/                               # Project 3 — fan-in topology
├── ds_services/                             # Project 4 — ROS2 services
├── ds_multi_robot/                          # Project 5 — namespaced robots
└── ds_failover/                             # Project 6 — server failover
```

Each project folder has the same internal structure:
```
<project>/
├── config/
│   ├── application.yaml     # Node list and system name
│   ├── nodes/<name>.yaml    # Per-node definition (type, topic/service, QoS, params)
│   ├── topics.yaml          # Topic → message type mapping
│   ├── qos_profiles.yaml    # Named QoS profiles
│   ├── parameters.yaml      # Runtime parameters per node
│   └── discovery.yaml       # Discovery server address reference
├── generated_pkg/           # Output of the generator (do not edit by hand)
└── README.md                # Project-specific test guide
```

---

## How the Generator Works

```
YAML config files  →  generator.py  →  C++ source  →  colcon build  →  ros2 run
```

1. `generator.py` reads all YAML files from a project's `config/` folder
2. It resolves each node: type, topic/service, QoS, parameters, namespace
3. It renders the appropriate Jinja2 template from `tool/templates/`
4. Output C++ is written to `<project>/generated_pkg/src/` and `include/`
5. `CMakeLists.txt` and `package.xml` are generated with correct dependencies auto-detected

### Generator commands

From the `discoveryTesting/` root:

```bash
python tool/generator.py --project discovery_server_single_pub_sub
python tool/generator.py --project ds_fan_out
python tool/generator.py --project ds_fan_in
python tool/generator.py --project ds_services
python tool/generator.py --project ds_multi_robot
python tool/generator.py --project ds_failover
```

---

## Docker — Build and Run

The single Dockerfile in `tool/` accepts a `PROJECT` build argument so one Dockerfile serves all projects.

### Build

Always run from `discoveryTesting/` (not from inside `tool/`):

```bash
# Default — builds discovery_server_single_pub_sub
docker build --no-cache -f tool/Dockerfile -t ros2_ds_single .

# Build a specific project
docker build --no-cache --build-arg PROJECT=ds_fan_out   -f tool/Dockerfile -t ros2_ds_fan_out   .
docker build --no-cache --build-arg PROJECT=ds_fan_in    -f tool/Dockerfile -t ros2_ds_fan_in    .
docker build --no-cache --build-arg PROJECT=ds_services  -f tool/Dockerfile -t ros2_ds_services  .
docker build --no-cache --build-arg PROJECT=ds_multi_robot -f tool/Dockerfile -t ros2_ds_multi_robot .
docker build --no-cache --build-arg PROJECT=ds_failover  -f tool/Dockerfile -t ros2_ds_failover  .
```

### Run

```bash
docker run -it --name ros2_test <image_name> bash
```

Then in separate terminals:
```bash
docker exec -it ros2_test bash
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
| `publisher` | `publisher.cpp.jinja` | Timer-driven publisher, configurable rate |
| `subscriber` | `subscriber.cpp.jinja` | Subscription with configurable log prefix |
| `service_server` | `service_server.cpp.jinja` | Service that responds to Trigger requests |
| `service_client` | `service_client.cpp.jinja` | Periodic service caller (every 2 s) |

### Optional YAML fields

| Field | Applies To | Effect |
|---|---|---|
| `namespace` | any node | Prefixes node and topics: `Node("name", "/robot1")` |

---

## Adding a New Project

1. Create a folder: `my_project/config/`
2. Write `application.yaml`, `nodes/*.yaml`, `topics.yaml`, `qos_profiles.yaml`, `parameters.yaml`
3. Run: `python tool/generator.py --project my_project`
4. Build: `docker build --build-arg PROJECT=my_project -f tool/Dockerfile -t my_image .`
5. Add a `my_project/README.md` with test steps

No changes to `tool/` are needed for pub/sub or service projects.

---

## Troubleshooting

| Problem | Cause | Fix |
|---|---|---|
| `Package 'generated_pkg' not found` | `AMENT_PREFIX_PATH` not set | Dockerfile sets it via `ENV` — rebuild with `--no-cache` |
| `fastdds: command not found` | ROS2 not sourced | Run `source /opt/ros/jazzy/setup.bash` first |
| Listener receives nothing | Discovery server not running or env var missing | Ensure server is up; set `ROS_DISCOVERY_SERVER=127.0.0.1:11811` in every terminal |
| Docker build uses stale cache | Old layer cached | Run `docker build --no-cache ...` |
| `not found: not found` on COPY | `docker build` run from wrong directory | Must run from `discoveryTesting/` — use `-f tool/Dockerfile` with `.` as context |
