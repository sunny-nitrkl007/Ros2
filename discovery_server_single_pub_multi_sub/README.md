# Project: discovery_server_single_pub_multi_sub

## Goal

Test **fan-out topology** through a Fast-DDS Discovery Server: one publisher broadcasting to multiple independent subscribers.

This verifies that the discovery server correctly propagates a single publisher's endpoint to many subscribers — a pattern common in sensor data distribution (one sensor → many consumers).

**Topology:** 1 publisher (`talker`) → Discovery Server → 3 subscribers (`listener_1`, `listener_2`, `listener_3`)

**No generator changes needed** — fan-out is just more nodes in `application.yaml`.

---

## YAML Config Summary

| File | Key Values |
|---|---|
| `application.yaml` | nodes: talker, listener_1, listener_2, listener_3 |
| `nodes/talker.yaml` | type: publisher, topic: chatter, rate: 2 Hz |
| `nodes/listener_1.yaml` | type: subscriber, topic: chatter |
| `nodes/listener_2.yaml` | type: subscriber, topic: chatter |
| `nodes/listener_3.yaml` | type: subscriber, topic: chatter |
| `parameters.yaml` | each listener has a distinct log prefix ([L1], [L2], [L3]) |

---

## Step 1 — Generate

```bash
python tool/generator.py --project discovery_server_single_pub_multi_sub
```

---

## Step 2 — Build

```
docker build --no-cache -f tool/Dockerfile -t ros2_discovery_server_single_pub_multi_sub .
```

---

## Step 3 — Test (5 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --rm --name single_pub_multi_sub_test ros2_discovery_server_single_pub_multi_sub bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Talker

```bash
docker exec -it single_pub_multi_sub_test bash
ros2 run generated_pkg talker
```

### Terminals 3, 4, 5 — Three Listeners

Repeat for each, changing the node name:
```bash
docker exec -it single_pub_multi_sub_test bash
ros2 run generated_pkg listener_1   # or listener_2 / listener_3
```

---

## What to Observe

- All three listeners receive every message from the single talker
- Each has a different log prefix so you can tell them apart:
  ```
  [INFO] [listener_1]: [L1] Received: Hello from talker! count=5
  [INFO] [listener_2]: [L2] Received: Hello from talker! count=5
  [INFO] [listener_3]: [L3] Received: Hello from talker! count=5
  ```
- Kill one listener — the other two continue unaffected
- Kill the discovery server — all three stop receiving (proves server dependency)

---

## Discovery Server Lifecycle

The Discovery Server (DS) is only required during the **handshake/discovery phase** — it brokers the initial exchange of endpoints and addresses between nodes. Once the talker and all listeners have found each other through the DS, Fast-DDS establishes **direct peer-to-peer DDS connections** between them.

```
Discovery phase (DS required):   talker  <──DS──>  listener_1 / listener_2 / listener_3
                                      ↓ once discovered ↓
Data phase (DS not involved):    talker  ──────────>  listener_1 / listener_2 / listener_3
                                              (direct DDS unicast per subscriber)
```

Key implications:
- The DS is **out of the communication path** entirely after discovery completes
- Killing the DS does **not** break already-established connections between nodes
- New nodes trying to join **after** the DS is killed cannot discover anyone
- Only the initial handshake requires the DS to be reachable

> **Note:** The "Kill the discovery server — all three stop receiving" observation above applies when the DS is stopped *before* or *during* the initial discovery handshake. If killed *after* discovery is complete, existing connections persist.

---

## Cleanup

```bash
docker rm -f single_pub_multi_sub_test
```
