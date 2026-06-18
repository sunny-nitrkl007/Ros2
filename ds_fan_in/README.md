# Project: ds_fan_in

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
python tool/generator.py --project ds_fan_in
```

---

## Step 2 — Build

```bash
docker build --no-cache --build-arg PROJECT=ds_fan_in -f tool/Dockerfile -t ros2_ds_fan_in .
```

---

## Step 3 — Test (5 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --name fan_in_test ros2_ds_fan_in bash
fastdds discovery -i 0 -p 11811
```

### Terminals 2, 3, 4 — Three Publishers

```bash
docker exec -it fan_in_test bash
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
ros2 run generated_pkg talker_1   # runs at 2 Hz
```

Repeat for `talker_2` (1 Hz) and `talker_3` (0.5 Hz) in separate terminals.

### Terminal 5 — Listener

```bash
docker exec -it fan_in_test bash
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
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

## Cleanup

```bash
docker rm -f fan_in_test
```
