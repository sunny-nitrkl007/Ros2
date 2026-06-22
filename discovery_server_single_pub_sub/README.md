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

Run from `Version2.0/`:
```bash
python3 tool/generator.py --project discovery_server_single_pub_sub
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
```
docker build --no-cache -f tool/Dockerfile -t ros2_ds_single .
```

First build takes 3–5 minutes. Subsequent builds use cache.

---

## Step 3 — Test (3 Terminals)

### Terminal 1 — Start container + Discovery Server

```bash
docker run -it --rm --name ros2_test ros2_ds_single bash
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
ros2 node list
# /talker  /listener

ros2 topic echo /chatter
```

---

## SUPER_CLIENT vs CLIENT — Why It Matters

When `ROS_DISCOVERY_SERVER` is set, Fast-DDS gives every ROS2 node one of two roles:

| Role | What it sees |
|---|---|
| `CLIENT` | Only discovers participants that match its own topics — a talker only finds listeners on the same topic, nothing else |
| `SUPER_CLIENT` | Gets the **full registry** from the server — every participant, every topic, every endpoint on the network |

Every node in this project is configured as a **SUPER_CLIENT** (set in `tool/client.xml` and activated automatically when `ROS_DISCOVERY_SERVER` is exported).

This is why Test 2 above works:

```bash
# Without env var — node is invisible (not registered with DS)
ros2 node list      # (empty)

# With env var — SUPER_CLIENT gets the full picture from DS
ros2 node list      # /talker  /listener
ros2 topic list     # /chatter
```

`ros2 node list`, `ros2 topic list`, and `ros2 topic echo` are themselves ROS2 nodes. As SUPER_CLIENTs they get the complete view of the network from the DS — which is why they show all nodes and topics correctly. A plain CLIENT would only see participants it has a topic match with, giving incomplete or empty results for these commands.

### How SUPER_CLIENT gets configured — automatically

You never write any SUPER_CLIENT configuration explicitly. Setting the env var is the entire configuration:

```bash
# Method 1 — automatic (what you always use in this project)

# Method 2 — manual XML (alternative, same result, not used in test steps)
export FASTRTPS_DEFAULT_PROFILES_FILE=/path/to/tool/client.xml
```

When ROS2 sees `ROS_DISCOVERY_SERVER`, its middleware layer (rmw_fastrtps) automatically generates a FastDDS participant config in memory — equivalent to writing `<discoveryProtocol>SUPER_CLIENT</discoveryProtocol>` in an XML profile. Your node starts already configured as a SUPER_CLIENT with no XML file involved.

```
You set:    ROS_DISCOVERY_SERVER=127.0.0.1:11811
                    ↓
ROS2 rmw layer detects the env var
                    ↓
Automatically configures Fast-DDS participant as SUPER_CLIENT
pointing to 127.0.0.1:11811
                    ↓
Your node starts — already a SUPER_CLIENT, no XML needed
```

`tool/client.xml` exists in this project as **documentation/reference** — to show what ROS2 is doing under the hood, and as a fallback for non-ROS2 Fast-DDS applications that can't use `ROS_DISCOVERY_SERVER`. It is never actually loaded in any of the test steps.

---

## Discovery Server Lifecycle

The Discovery Server (DS) is only required during the **handshake/discovery phase** — it brokers the initial exchange of endpoints and addresses between nodes. Once `talker` and `listener` have found each other through the DS, Fast-DDS establishes a **direct peer-to-peer DDS connection** between them.

```
Discovery phase (DS required):   talker  <──DS──>  listener
                                      ↓ once discovered ↓
Data phase (DS not involved):    talker  ──────────>  listener
                                         (direct DDS unicast)
```

Key implications:
- The DS is **out of the communication path** entirely after discovery completes
- Killing the DS does **not** break an already-established talker ↔ listener connection
- New nodes trying to join **after** the DS is killed cannot discover anyone
- Only the initial handshake requires the DS to be reachable

> **Note:** The "Kill the server, communication stops" test above demonstrates what happens when the DS is stopped *before* or *during* initial discovery. If killed *after* both nodes have fully discovered each other, the existing message stream continues uninterrupted.

---

## Cleanup

```bash
docker rm -f ros2_test
```
