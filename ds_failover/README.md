# Project: ds_failover

## Goal

Test **Fast-DDS Discovery Server high availability**: nodes connect to two servers (primary + backup). When the primary server is killed, communication continues through the backup.

This is critical for production systems where the discovery server itself is a single point of failure.

**Topology:**
```
talker ──┬── Primary (port 11811) ──┬── listener
          └── Backup  (port 11812) ──┘
```

Nodes register with both servers simultaneously. If the primary disappears, Fast-DDS automatically uses the backup to maintain participant discovery.

**No generator changes needed** — the C++ nodes are identical to the single pub/sub project. The difference is entirely in the FastDDS XML profile and how you start the servers.

---

## YAML Config Summary

| File | Key Values |
|---|---|
| `application.yaml` | nodes: talker, listener (same as single pub/sub) |
| `config/discovery.yaml` | two servers: id 0 port 11811, id 1 port 11812 |
| `fastdds/dual_server_client.xml` | SUPER_CLIENT connecting to both servers |

The `discovery.yaml` documents the dual-server intent:
```yaml
discovery:
  servers:
    - id: 0
      address: 127.0.0.1
      port: 11811
    - id: 1
      address: 127.0.0.1
      port: 11812
```

---

## Step 1 — Generate

```bash
python tool/generator.py --project ds_failover
```

---

## Step 2 — Build

```bash
docker build --no-cache --build-arg PROJECT=ds_failover -f tool/Dockerfile -t ros2_ds_failover .
```

---

## Step 3 — Test (4 Terminals)

### Terminal 1 — Container + Primary Discovery Server (ID 0, port 11811)

```bash
docker run -it --name failover_test ros2_ds_failover bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Backup Discovery Server (ID 1, port 11812)

```bash
docker exec -it failover_test bash
fastdds discovery -i 1 -p 11812
```

Expected (for each server):
```
### Server is running ###
  Server ID:  0   (or 1 for the backup)
  Server GUID prefix: 44.53.00.5f...  (byte 3 changes per ID)
  Server Addresses:   UDPv4:[0.0.0.0]:11811
```

### Terminal 3 — Talker

Connect to both servers using semicolon-separated list:
```bash
docker exec -it failover_test bash
export ROS_DISCOVERY_SERVER="127.0.0.1:11811;127.0.0.1:11812"
ros2 run generated_pkg talker
```

### Terminal 4 — Listener

```bash
docker exec -it failover_test bash
export ROS_DISCOVERY_SERVER="127.0.0.1:11811;127.0.0.1:11812"
ros2 run generated_pkg listener
```

---

## Failover Test Procedure

1. Confirm pub/sub works with both servers running (messages flowing in Terminal 4)

2. Kill the primary server: press `Ctrl+C` in Terminal 1

3. Observe: the listener may pause briefly (1–3 s) while Fast-DDS detects the loss and re-routes through the backup server

4. Communication resumes through the backup — listener continues receiving messages

5. Restart the primary: run `fastdds discovery -i 0 -p 11811` again in Terminal 1

6. Both servers are now active again — Fast-DDS re-registers with both

---

## Alternative: FastDDS XML Profile (instead of env var)

Copy `fastdds/dual_server_client.xml` into the container and use it:

```bash
docker cp ds_failover/fastdds/dual_server_client.xml failover_test:/dual_server.xml
docker exec -it failover_test bash
export FASTRTPS_DEFAULT_PROFILES_FILE=/dual_server.xml
ros2 run generated_pkg talker
```

The XML profile (`fastdds/dual_server_client.xml`) configures the SUPER_CLIENT with both server GUIDs:
- Server 0: prefix `44.53.00.5f.45.50.52.4f.53.49.4d.41`, port 11811
- Server 1: prefix `44.53.01.5f.45.50.52.4f.53.49.4d.41`, port 11812

Server GUID prefix rule: byte 3 (`NN`) encodes the server ID:
`44.53.NN.5f.45.50.52.4f.53.49.4d.41`

---

## Cleanup

```bash
docker rm -f failover_test
```
