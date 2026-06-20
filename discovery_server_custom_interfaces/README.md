# Discovery Server — Custom Interfaces

## Goal

Test that user-defined ROS2 message and service types work correctly through the Fast-DDS Discovery Server. All previous projects used built-in types (`std_msgs/String`, `std_srvs/Trigger`). This project defines its own `.msg` and `.srv` files and verifies that discovery, registration, and data exchange work the same way with custom types.

---

## What This Project Introduces

| Concept | Where |
|---|---|
| Custom `.msg` definition | `custom_interfaces_pkg/msg/Sensor.msg` |
| Custom `.srv` definition | `custom_interfaces_pkg/srv/SensorQuery.srv` |
| Two-package workspace | `custom_interfaces_pkg` + `generated_pkg` built together |
| Project-level template overrides | `templates/` — publisher and service templates fill custom fields |
| Project-specific Dockerfile | Can't use shared `tool/Dockerfile` — must copy both packages |

---

## Custom Interface Definitions

### `Sensor.msg`
```
string sensor_id
float64 temperature
float64 humidity
int32 sequence_number
```
Used on the `/sensor_data` topic (pub/sub pair).

### `SensorQuery.srv`
```
string sensor_id        # request: which sensor to query
---
float64 temperature     # response: last known reading
float64 humidity
bool sensor_found
string message
```
Used on the `/sensor_query` service (server/client pair).

---

## Nodes

| Node | Type | Interface | Role |
|---|---|---|---|
| `sensor_publisher` | publisher | `Sensor.msg` | Publishes simulated sensor readings (sin/cos wave) at 1 Hz |
| `sensor_subscriber` | subscriber | `Sensor.msg` | Receives and logs all sensor readings |
| `sensor_query_server` | service_server | `SensorQuery.srv` | Responds to on-demand sensor queries (hardcoded registry) |
| `sensor_query_client` | service_client | `SensorQuery.srv` | Queries the server every 3 s, logs result |

---

## YAML → Code Path

**topics.yaml** references `custom_interfaces_pkg/msg/Sensor`:
```yaml
topics:
  sensor_data:
    type: custom_interfaces_pkg/msg/Sensor
```

The generator splits `custom_interfaces_pkg/msg/Sensor` as:
- C++ type: `custom_interfaces_pkg::msg::Sensor`
- include: `custom_interfaces_pkg/msg/sensor.hpp`
- dep package: `custom_interfaces_pkg` (added automatically to `CMakeLists.txt` and `package.xml`)

No changes to `generator.py` were needed — the existing `msg_to_cpp` and `collect_deps` logic handles any package name.

**Project-level template overrides** (`templates/`) supply the custom field names (`msg.sensor_id`, `msg.temperature`, etc.) replacing the generic `msg.data` from the shared templates. The generator checks `discovery_server_custom_interfaces/templates/` first and falls back to `tool/templates/` for templates not overridden.

---

## Parameters (YAML-driven)

| Node | Parameter | Default | Effect |
|---|---|---|---|
| `sensor_publisher` | `sensor_id` | `temp_sensor_01` | Sensor ID stamped in every message |
| `sensor_publisher` | `publish_rate` | `1.0` | Hz — how fast readings are published |
| `sensor_query_client` | `target_sensor_id` | `temp_sensor_01` | Which sensor to query |
| `sensor_query_client` | `query_interval_s` | `3.0` | Seconds between queries |

Override at runtime without rebuilding:
```bash
ros2 param set /sensor_publisher sensor_id temp_sensor_02
ros2 param set /sensor_query_client target_sensor_id temp_sensor_02
```

---

## Discovery Server Lifecycle

The discovery server is only needed during the handshake phase. Once `sensor_publisher` and `sensor_subscriber` have registered with the DS and learned about each other, all `/sensor_data` messages flow directly over peer-to-peer DDS unicast. The DS can be stopped after discovery and messages will continue — it is not in the data path.

The same applies to the `/sensor_query` service: the DS registers server and client endpoints once; subsequent service calls are direct unicast between the two nodes.

---

## Generator Command

```bash
python tool/generator.py --project discovery_server_custom_interfaces
```

This regenerates `generated_pkg/` from the YAML config and project templates. The `custom_interfaces_pkg/` is never generated — it is authored manually and contains the canonical `.msg` / `.srv` definitions.

---

## Docker Build

This project uses its own Dockerfile (not the shared `tool/Dockerfile`) because it must copy and build two packages.

Run from `discoveryTesting/`:

```bash
docker build --no-cache \
  -f discovery_server_custom_interfaces/Dockerfile \
  -t ros2_ds_custom_interfaces .
```

colcon resolves the build order automatically via `package.xml` dependencies: `custom_interfaces_pkg` is built first, then `generated_pkg`.

---

## Step-by-Step Test

### Terminal 1 — Start the Discovery Server

```bash
docker run -it --name ds_custom ros2_ds_custom_interfaces bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Start the sensor publisher

```bash
docker exec -it ds_custom bash
ros2 run generated_pkg sensor_publisher
```

Expected output every second:
```
[temp_sensor_01] seq=0  temp=22.00 C  hum=55.00 %
[temp_sensor_01] seq=1  temp=23.99 C  hum=53.76 %
...
```

### Terminal 3 — Start the sensor subscriber

```bash
docker exec -it ds_custom bash
ros2 run generated_pkg sensor_subscriber
```

Expected — mirrors the publisher output:
```
[temp_sensor_01] seq=0  temp=22.00 C  hum=55.00 %
```

### Terminal 4 — Start the query server

```bash
docker exec -it ds_custom bash
ros2 run generated_pkg sensor_query_server
```

Expected:
```
Sensor query service '/sensor_query' ready.
```

### Terminal 5 — Start the query client

```bash
docker exec -it ds_custom bash
ros2 run generated_pkg sensor_query_client
```

Expected every 3 seconds:
```
Query [temp_sensor_01]: temp=23.50 C  hum=58.20 %
```

### What to Observe

- `sensor_subscriber` receives `custom_interfaces_pkg::msg::Sensor` messages with all four fields populated — confirming custom `.msg` types work through the DS
- `sensor_query_client` receives `custom_interfaces_pkg::srv::SensorQuery` responses — confirming custom `.srv` types work through the DS
- `ros2 topic echo /sensor_data` shows the full custom message structure
- `ros2 service type /sensor_query` shows `custom_interfaces_pkg/srv/SensorQuery`

### Inspect the custom types

```bash
docker exec -it ds_custom bash
ros2 interface show custom_interfaces_pkg/msg/Sensor
ros2 interface show custom_interfaces_pkg/srv/SensorQuery
```

---

## Troubleshooting

| Problem | Fix |
|---|---|
| `custom_interfaces_pkg not found` during build | `AMENT_PREFIX_PATH` must include the interfaces install path — Dockerfile sets this via `ENV` |
| `Package 'custom_interfaces_pkg' not found` at ros2 run | Container built from wrong image — rebuild and re-run using `ros2_ds_custom_interfaces` |
| Subscriber receives nothing | Ensure `ROS_DISCOVERY_SERVER` is set in every terminal before `ros2 run` |
| Docker build `COPY` error | Must run `docker build` from `discoveryTesting/` root, not from inside the project folder |
