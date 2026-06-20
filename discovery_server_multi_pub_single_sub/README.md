# Project: discovery_server_multi_pub_single_sub

## Goal

Test **fan-in topology** through a Fast-DDS Discovery Server: multiple publishers on the same topic, all received by a single subscriber.

This verifies that the discovery server correctly registers multiple publisher endpoints and delivers all of them to one subscriber — a pattern common in data aggregation (multiple sensors → one processor).

**Topology:** 3 publishers (`talker_1`, `talker_2`, `talker_3`) → Discovery Server → 1 subscriber (`listener`)

Each publisher runs at a different rate (2 Hz, 1 Hz, 0.5 Hz) to make interleaving visible in the listener output.

**No generator changes needed** — fan-in is just more publisher nodes in `application.yaml`.

---

## YAML Config Summary

| File | Key Values |
|---|---|
| `application.yaml` | nodes: talker_1, talker_2, talker_3, listener |
| `nodes/talker_1.yaml` | type: publisher, topic: chatter |
| `nodes/talker_2.yaml` | type: publisher, topic: chatter |
| `nodes/talker_3.yaml` | type: publisher, topic: chatter |
| `nodes/listener.yaml` | type: subscriber, topic: chatter |
| `parameters.yaml` | talker_1: 2.0 Hz, talker_2: 1.0 Hz, talker_3: 0.5 Hz |

---

## Step 1 — Generate

```bash
python tool/generator.py --project discovery_server_multi_pub_single_sub
```

---

## Step 2 — Build

```bash
docker build --no-cache --build-arg PROJECT=discovery_server_multi_pub_single_sub -f tool/Dockerfile -t ros2_discovery_server_multi_pub_single_sub .
```

---

## Step 3 — Test (5 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --name multi_pub_single_sub_test ros2_discovery_server_multi_pub_single_sub bash
fastdds discovery -i 0 -p 11811
```

### Terminals 2, 3, 4 — Three Publishers

```bash
docker exec -it multi_pub_single_sub_test bash
ros2 run generated_pkg talker_1   # runs at 2 Hz
```

Repeat for `talker_2` (1 Hz) and `talker_3` (0.5 Hz) in separate terminals.

### Terminal 5 — Listener

```bash
docker exec -it multi_pub_single_sub_test bash
ros2 run generated_pkg listener
```

---

## What to Observe

- The single listener receives interleaved messages from all three publishers:
  ```
  [INFO] [listener]: Received: Hello from talker_1! count=10
  [INFO] [listener]: Received: Hello from talker_2! count=5
  [INFO] [listener]: Received: Hello from talker_1! count=11
  [INFO] [listener]: Received: Hello from talker_3! count=2
  ```
- talker_1 messages arrive most frequently, talker_3 least frequently
- Kill one publisher — listener continues receiving from the other two (demonstrates independent registration)

---

## Discovery Server Lifecycle

The Discovery Server (DS) is only required during the **handshake/discovery phase** — it brokers the initial exchange of endpoints and addresses between nodes. Once all three publishers and the listener have found each other through the DS, Fast-DDS establishes **direct peer-to-peer DDS connections** between them.

```
Discovery phase (DS required):   talker_1/2/3  <──DS──>  listener
                                        ↓ once discovered ↓
Data phase (DS not involved):    talker_1/2/3  ──────────>  listener
                                               (direct DDS unicast per publisher)
```

Key implications:
- The DS is **out of the communication path** entirely after discovery completes
- Killing the DS does **not** break already-established connections between nodes
- New nodes trying to join **after** the DS is killed cannot discover anyone
- Only the initial handshake requires the DS to be reachable

---

## Cleanup

```bash
docker rm -f multi_pub_single_sub_test
```
