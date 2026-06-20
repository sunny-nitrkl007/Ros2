# Project: discovery_server_lifecycle

## Goal

Test **ROS2 Lifecycle Nodes** through a Fast-DDS Discovery Server.

Every production-grade ROS2 node uses the lifecycle pattern. A lifecycle node is not just "running" or "stopped" — it has a formal state machine that controls when it creates publishers, when it starts communicating, and when it releases resources. This makes startup sequencing and fault recovery deterministic.

**Topology:** `lifecycle_talker` → Discovery Server → `lifecycle_listener`
Both nodes are lifecycle-managed. You drive their states manually using the `ros2 lifecycle` CLI.

**What makes this interesting for DS testing:**
A lifecycle node **registers with the DS as soon as it starts** (in `unconfigured` state) — before it creates any publishers or subscriptions. The DS sees the node even when it is not yet communicating. This is fundamentally different from a regular node where registration and communication start together.

---

## The Lifecycle State Machine

```
        [create node]
              |
        unconfigured
              |  configure()
              v
          inactive   <----------+
              |  activate()     |  deactivate()
              v                 |
           active  ------------>+
              |
              | cleanup() (back to unconfigured)
              | shutdown() (finalized, node exits)
```

| State | Publisher exists? | Timer running? | Messages sent? |
|---|---|---|---|
| unconfigured | No | No | No |
| inactive | Yes (created in configure) | No | No |
| active | Yes | Yes | Yes |
| unconfigured (after cleanup) | No | No | No |

**For the subscriber:**

| State | Subscription exists? | Messages processed? |
|---|---|---|
| unconfigured | No | No |
| inactive | Yes (callbacks fire, silently dropped) | No |
| active | Yes | Yes |
| unconfigured (after cleanup) | No | No |

---

## YAML Config

```yaml
node:
  name: lifecycle_talker
  type: lifecycle_publisher   # or lifecycle_subscriber
  topic: chatter
  qos: default_qos
  parameters: lifecycle_talker
```

The YAML schema is identical to regular `publisher`/`subscriber` — only the `type` field changes. The generator picks the lifecycle template automatically.

The generator auto-adds `rclcpp_lifecycle` and `lifecycle_msgs` to `CMakeLists.txt` and `package.xml`.

---

## Step 1 — Generate

```bash
python tool/generator.py --project discovery_server_lifecycle
```

---

## Step 2 — Build

```
docker build --no-cache -f tool/Dockerfile -t ros2_ds_lifecycle .
```

---

## Step 3 — Test (4 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --rm --name lifecycle_test ros2_ds_lifecycle bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Lifecycle Talker (publisher)

```bash
docker exec -it lifecycle_test bash
ros2 run generated_pkg lifecycle_talker
```

The node starts and prints:
```
[INFO] [lifecycle_talker]: Node created. Current state: unconfigured.
```

It is now registered with the DS but publishes nothing.

### Terminal 3 — Lifecycle Listener (subscriber)

```bash
docker exec -it lifecycle_test bash
ros2 run generated_pkg lifecycle_listener
```

Same — registered with DS, no subscriptions yet.

### Terminal 4 — Drive State Transitions

```bash
docker exec -it lifecycle_test bash
```

---

## Step 4 — State Transition Commands (run in Terminal 4)

> **Note:** `ros2 lifecycle set` does not work with FastDDS Discovery Server — the DS
> propagates participant presence but not service endpoint data. Use `ros2 service call`
> directly instead. Transition IDs: configure=1, cleanup=2, activate=3, deactivate=4

### Phase 1 — Configure both nodes

```bash
ros2 service call /lifecycle_talker/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 1}}"
ros2 service call /lifecycle_listener/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 1}}"
```

Talker output:
```
[INFO] [lifecycle_talker]: on_configure: publisher created on 'chatter' at 1.0 Hz. State: inactive.
```

Listener output:
```
[INFO] [lifecycle_listener]: on_configure: subscribed to 'chatter'. State: inactive (messages ignored until active).
```

### Phase 2 — Activate both nodes

```bash
ros2 service call /lifecycle_talker/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 3}}"
ros2 service call /lifecycle_listener/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 3}}"
```

Talker output:
```
[INFO] [lifecycle_talker]: on_activate: publishing started. State: active.
[INFO] [lifecycle_talker]: Publishing: 'Hello from lifecycle_talker! count=0'
```

Listener output:
```
[INFO] [lifecycle_listener]: on_activate: now processing messages. State: active.
[INFO] [lifecycle_listener]: [lifecycle] Received: Hello from lifecycle_talker! count=0
```

### Phase 3 — Deactivate the talker only

```bash
ros2 service call /lifecycle_talker/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 4}}"
```

### Phase 4 — Reactivate

```bash
ros2 service call /lifecycle_talker/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 3}}"
```

### Phase 5 — Full cleanup cycle

```bash
ros2 service call /lifecycle_talker/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 4}}"
ros2 service call /lifecycle_talker/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 2}}"
ros2 service call /lifecycle_talker/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 1}}"
ros2 service call /lifecycle_talker/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 3}}"
```

---

## Introspection Commands

```bash
ros2 service call /lifecycle_talker/get_state lifecycle_msgs/srv/GetState "{}"
ros2 service call /lifecycle_listener/get_state lifecycle_msgs/srv/GetState "{}"
ros2 node list
```

Check the DS sees both nodes regardless of lifecycle state:
```bash
ros2 node list
# /lifecycle_talker
# /lifecycle_listener
```

The DS registers nodes on creation — you see them here even in `unconfigured` state before any configure/activate.

---

## What to Observe

| Observation | What It Proves |
|---|---|
| Node visible in `ros2 node list` before `configure` | DS registers on node creation, not on topic creation |
| No topic visible before `configure` | Topics only exist after `on_configure()` runs |
| Listener receives nothing while `inactive` even though subscription exists | `active_` flag, not DS, controls message processing |
| Count continues from where it left off after deactivate → activate | Timer reset, publisher re-activated, but counter never reset |
| After `cleanup`, topic disappears from `ros2 topic list` | Publisher destroyed; DS de-registers the endpoint |

---

## Cleanup

```bash
docker rm -f lifecycle_test
```
