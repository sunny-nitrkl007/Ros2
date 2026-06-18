# Project: discovery_server_single_pub_sub

## Goal

Verify that a basic ROS2 publisher and subscriber can find each other **only through a Fast-DDS Discovery Server** — not through standard UDP multicast.
This is the baseline project. All other projects build on what is proven here.

**Topology:** 1 publisher (`talker`) → Discovery Server → 1 subscriber (`listener`)

---

## YAML Config Summary

| File | Key Values |
|---|---|
| `application.yaml` | nodes: talker, listener |
| `nodes/talker.yaml` | type: publisher, topic: chatter, rate: 2 Hz |
| `nodes/listener.yaml` | type: subscriber, topic: chatter, best_effort QoS |
| `topics.yaml` | chatter → std_msgs/msg/String |
| `parameters.yaml` | talker.publish_rate=2.0, listener.log_prefix="Received:" |

---

## Step 1 — Generate C++ Code

From `discoveryTesting/`:
```bash
python tool/generator.py --project discovery_server_single_pub_sub
```

Generated files:
- `generated_pkg/src/talker.cpp`
- `generated_pkg/src/listener.cpp`
- `generated_pkg/include/talker.hpp`
- `generated_pkg/include/listener.hpp`
- `generated_pkg/CMakeLists.txt`
- `generated_pkg/package.xml`

---

## Step 2 — Build Docker Image

From `discoveryTesting/` (always from here, not from inside `tool/`):
```bash
docker build --no-cache -f tool/Dockerfile -t ros2_ds_single .
```

First build takes 3–5 minutes. Subsequent builds use cache.

---

## Step 3 — Test (3 Terminals)

### Terminal 1 — Start container + Discovery Server

```bash
docker run -it --name ros2_test ros2_ds_single bash
```

Inside the container:
```bash
fastdds discovery -i 0 -p 11811
```

Expected:
```
### Server is running ###
  Server GUID prefix: 44.53.00.5f.45.50.52.4f.53.49.4d.41
  Server Addresses:   UDPv4:[0.0.0.0]:11811
```

### Terminal 2 — Run Talker

```bash
docker exec -it ros2_test bash
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
ros2 run generated_pkg talker
```

Expected:
```
[INFO] [talker]: Publishing: 'Hello from talker! count=0'
[INFO] [talker]: Publishing: 'Hello from talker! count=1'
```

### Terminal 3 — Run Listener

```bash
docker exec -it ros2_test bash
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
ros2 run generated_pkg listener
```

Expected:
```
[INFO] [listener]: Received: Hello from talker! count=0
[INFO] [listener]: Received: Hello from talker! count=1
```

---

## Proof the Discovery Server Is Being Used

### Test 1 — Kill the server, communication stops
Press `Ctrl+C` in Terminal 1. The listener stops receiving within a few seconds.
Normal multicast DDS would continue — this proves dependency on the server.

### Test 2 — Node visibility requires the env var

```bash
docker exec -it ros2_test bash

# Without env var — nodes are invisible
ros2 node list
# (empty)

# With env var — nodes appear
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
ros2 node list
# /talker  /listener

ros2 topic echo /chatter
```

---

## Cleanup

```bash
docker rm -f ros2_test
```
