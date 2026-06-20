# Project: discovery_server_multi_robot

## Goal

Test **ROS2 namespace isolation** through a single Fast-DDS Discovery Server.

Two simulated robots share the same discovery server but publish on isolated namespaced topics (`/robot1/chatter` and `/robot2/chatter`). Each robot's listener only receives messages from its own publisher.

This is essential for multi-robot deployments where all robots share a single discovery infrastructure but need separate communication channels.

**Topology:**
```
robot1_talker  → /robot1/chatter → robot1_listener
                      ↕ (via single discovery server)
robot2_talker  → /robot2/chatter → robot2_listener
```

**Generator change made:** `namespace` field added to node YAML. Templates now pass the namespace as the second argument to `Node("name", "namespace")`, which prefixes all topics automatically.

---

## YAML Config Summary

| File | Key Values |
|---|---|
| `application.yaml` | nodes: robot1_talker, robot1_listener, robot2_talker, robot2_listener |
| `nodes/robot1_talker.yaml` | type: publisher, namespace: /robot1, topic: chatter |
| `nodes/robot1_listener.yaml` | type: subscriber, namespace: /robot1, topic: chatter |
| `nodes/robot2_talker.yaml` | type: publisher, namespace: /robot2, topic: chatter |
| `nodes/robot2_listener.yaml` | type: subscriber, namespace: /robot2, topic: chatter |

The `namespace` field in node YAML:
```yaml
node:
  name: robot1_talker
  type: publisher
  namespace: /robot1      # <-- new optional field
  topic: chatter
  qos: default_qos
  parameters: robot1_talker
```

This generates: `Node("robot1_talker", "/robot1")` → the node lives at `/robot1/robot1_talker` and publishes on `/robot1/chatter`.

---

## Step 1 — Generate

```bash
python tool/generator.py --project discovery_server_multi_robot
```

---

## Step 2 — Build

```bash
docker build --no-cache --build-arg PROJECT=discovery_server_multi_robot -f tool/Dockerfile -t ros2_discovery_server_multi_robot .
```

---

## Step 3 — Test (5 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --rm --name multi_robot_test ros2_discovery_server_multi_robot bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Robot 1 Publisher

```bash
docker exec -it multi_robot_test bash
ros2 run generated_pkg robot1_talker
```

### Terminal 3 — Robot 1 Subscriber

```bash
docker exec -it multi_robot_test bash
ros2 run generated_pkg robot1_listener
```

### Terminal 4 — Robot 2 Publisher

```bash
docker exec -it multi_robot_test bash
ros2 run generated_pkg robot2_talker
```

### Terminal 5 — Robot 2 Subscriber

```bash
docker exec -it multi_robot_test bash
ros2 run generated_pkg robot2_listener
```

---

## What to Observe

- `robot1_listener` only receives from `robot1_talker` (on `/robot1/chatter`)
- `robot2_listener` only receives from `robot2_talker` (on `/robot2/chatter`)
- No cross-talk between robots despite sharing the same discovery server

Verify the isolated namespaced topics exist:
```bash
docker exec -it multi_robot_test bash
ros2 topic list
# /robot1/chatter
# /robot2/chatter
```

Echo only robot1:
```bash
ros2 topic echo /robot1/chatter
# Messages only from robot1_talker
```

---

## Discovery Server Lifecycle

The Discovery Server (DS) is only required during the **handshake/discovery phase** — it brokers the initial exchange of endpoints and addresses between nodes. Once each robot's talker and listener have found each other through the DS, Fast-DDS establishes **direct peer-to-peer DDS connections** between them.

```
Discovery phase (DS required):   robot1_talker  <──DS──>  robot1_listener
                                  robot2_talker  <──DS──>  robot2_listener
                                           ↓ once discovered ↓
Data phase (DS not involved):    robot1_talker  ──────────>  robot1_listener
                                  robot2_talker  ──────────>  robot2_listener
                                              (direct DDS unicast, namespaced)
```

Key implications:
- The DS is **out of the communication path** entirely after discovery completes
- Killing the DS does **not** break already-established connections between nodes
- New nodes trying to join **after** the DS is killed cannot discover anyone
- Only the initial handshake requires the DS to be reachable
- Namespace isolation (`/robot1`, `/robot2`) is enforced at the DDS topic level and is independent of the DS — it persists in direct connections

---

## Cleanup

```bash
docker rm -f multi_robot_test
```
