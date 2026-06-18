# Project: ds_fan_out

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
python tool/generator.py --project ds_fan_out
```

---

## Step 2 — Build

```bash
docker build --no-cache --build-arg PROJECT=ds_fan_out -f tool/Dockerfile -t ros2_ds_fan_out .
```

---

## Step 3 — Test (5 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --name fan_out_test ros2_ds_fan_out bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Talker

```bash
docker exec -it fan_out_test bash
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
ros2 run generated_pkg talker
```

### Terminals 3, 4, 5 — Three Listeners

Repeat for each, changing the node name:
```bash
docker exec -it fan_out_test bash
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
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

## Cleanup

```bash
docker rm -f fan_out_test
```
