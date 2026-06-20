# Project: discovery_server_failover

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
python tool/generator.py --project discovery_server_failover
```

---

## Step 2 — Build

```
docker build --no-cache -f tool/Dockerfile -t ros2_discovery_server_failover .
```

---

## Step 3 — Test (4 Terminals)

### Terminal 1 — Container + Primary Discovery Server (ID 0, port 11811)

```bash
docker run -it --rm --name failover_test ros2_discovery_server_failover bash
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

## SUPER_CLIENT with Multiple Servers

Every node in this project is a **SUPER_CLIENT** — the same role used in all other projects. What is unique here is that the SUPER_CLIENT is pointed at **two servers at once** via the semicolon-separated env var:

```bash
export ROS_DISCOVERY_SERVER="127.0.0.1:11811;127.0.0.1:11812"
#                                   ↑ primary          ↑ backup
```

Fast-DDS registers with **both servers simultaneously** at startup and gets the full participant registry from each. The two registries are merged — the node sees one unified network picture.

```
SUPER_CLIENT (talker / listener)
    ├── registers with Primary (11811) → gets full registry from server 0
    └── registers with Backup  (11812) → gets full registry from server 1
                    ↓
         merged view: all participants known
```

When the primary dies:
- The SUPER_CLIENT detects the loss (heartbeat timeout, ~1–3 s)
- It continues using the backup's registry for any new participant discovery
- The existing direct peer-to-peer connection to the other node is unaffected

> In all other projects a single `127.0.0.1:11811` is used and there is no fallback — this project is the only one where the SUPER_CLIENT role is exercised with redundant servers. See [discovery_server_single_pub_sub/README.md](../discovery_server_single_pub_sub/README.md) for the base explanation of SUPER_CLIENT vs plain CLIENT.

### How SUPER_CLIENT gets configured — automatically

You never write SUPER_CLIENT configuration explicitly. The env var is the entire configuration:

```bash
# Method 1 — automatic (what you always use in this project)
export ROS_DISCOVERY_SERVER="127.0.0.1:11811;127.0.0.1:11812"

# Method 2 — manual XML (alternative, same result)
export FASTRTPS_DEFAULT_PROFILES_FILE=/path/to/fastdds/dual_server_client.xml
```

When ROS2 sees `ROS_DISCOVERY_SERVER`, its middleware layer (rmw_fastrtps) automatically generates a FastDDS participant config in memory for each server in the list. Your node starts already registered with both servers — no XML file involved.

```
You set:    ROS_DISCOVERY_SERVER="127.0.0.1:11811;127.0.0.1:11812"
                    ↓
ROS2 rmw layer detects the env var, parses both addresses
                    ↓
Automatically configures Fast-DDS participant as SUPER_CLIENT
pointing to both 11811 (primary) and 11812 (backup)
                    ↓
Your node starts — registered with both servers, no XML needed
```

`fastdds/dual_server_client.xml` exists in this project as **documentation/reference** — to show what ROS2 is doing under the hood for the dual-server case, and as a fallback for non-ROS2 Fast-DDS applications. It is provided as an alternative in the test steps but the env var method achieves the same result.

---

## Discovery Server Lifecycle

The Discovery Server (DS) is only required during the **handshake/discovery phase** — it brokers the initial exchange of endpoints and addresses between nodes. Once `talker` and `listener` have found each other through either server (primary or backup), Fast-DDS establishes a **direct peer-to-peer DDS connection** between them.

```
Discovery phase (DS required):   talker  <──primary/backup──>  listener
                                      ↓ once discovered ↓
Data phase (DS not involved):    talker  <─────────────────>  listener
                                              (direct DDS unicast)
```

Key implications:
- The DS is **out of the communication path** entirely after discovery completes
- Killing **both** servers does **not** break an already-established talker ↔ listener connection
- New nodes trying to join **after** all servers are killed cannot discover anyone
- The failover mechanism (primary → backup) is about maintaining the ability to discover **new** participants — not about keeping existing data channels alive

> **Distinction from failover:** The brief pause (1–3 s) observed when the primary is killed is Fast-DDS detecting the loss and re-registering with the backup for future discovery — the existing direct connection between talker and listener is unaffected once re-registration completes.

---

## Alternative: FastDDS XML Profile (instead of env var)

Copy `fastdds/dual_server_client.xml` into the container and use it:

```bash
docker cp discovery_server_failover/fastdds/dual_server_client.xml failover_test:/dual_server.xml
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
