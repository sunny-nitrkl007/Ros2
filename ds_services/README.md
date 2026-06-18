# Project: ds_services

## Goal

Test **ROS2 services (request/response)** through a Fast-DDS Discovery Server.

Unlike pub/sub (fire and forget), a service call is synchronous: the client sends a request and waits for the server's response. This project verifies that the discovery server correctly registers service endpoints and routes request/response pairs between nodes.

**Topology:** `reset_client` → (request) → Discovery Server → (route) → `reset_server` → (response back)

**Service used:** `std_srvs/srv/Trigger` — empty request, response is `bool success` + `string message`.
This is a built-in ROS2 type so no custom interface generation is needed.

**Generator changes made:** Two new templates added to `tool/templates/`:
- `service_server.cpp.jinja` — handles incoming requests, sends response
- `service_client.cpp.jinja` — calls the service every 2 seconds, logs response

---

## YAML Config Summary

| File | Key Values |
|---|---|
| `application.yaml` | nodes: reset_server, reset_client |
| `nodes/reset_server.yaml` | type: service_server, service: /reset_system, srv_type: std_srvs/srv/Trigger |
| `nodes/reset_client.yaml` | type: service_client, service: /reset_system, srv_type: std_srvs/srv/Trigger |
| `topics.yaml` | empty (services don't use topics) |

Node YAML schema for services:
```yaml
node:
  name: reset_server
  type: service_server       # or service_client
  service: /reset_system     # service name (replaces 'topic')
  srv_type: std_srvs/srv/Trigger
```

---

## Step 1 — Generate

```bash
python tool/generator.py --project ds_services
```

The generator auto-detects `std_srvs` as a dependency and writes it into `CMakeLists.txt` and `package.xml`.

---

## Step 2 — Build

```bash
docker build --no-cache --build-arg PROJECT=ds_services -f tool/Dockerfile -t ros2_ds_services .
```

---

## Step 3 — Test (3 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --name services_test ros2_ds_services bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Service Server

```bash
docker exec -it services_test bash
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
ros2 run generated_pkg reset_server
```

Expected:
```
[INFO] [reset_server]: Service '/reset_system' ready.
[INFO] [reset_server]: Received service request.
[INFO] [reset_server]: Received service request.
```

### Terminal 3 — Service Client

```bash
docker exec -it services_test bash
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
ros2 run generated_pkg reset_client
```

Expected:
```
[INFO] [reset_client]: Client ready, calling '/reset_system' every 2 s.
[INFO] [reset_client]: Response: success=true  message='OK from reset_server'
[INFO] [reset_client]: Response: success=true  message='OK from reset_server'
```

---

## What to Observe

- Client sends a request every 2 seconds; server responds immediately
- If the server is not running, the client logs `Service not available, waiting...` — it keeps retrying
- Start the server after the client — the client discovers it through the discovery server and calls it automatically
- Kill the discovery server — client can no longer find the service (`wait_for_service` keeps timing out)

---

## Adding Business Logic

The server template generates a stub `handle_request()` function. To add real logic, edit the generated file directly (the generator can be re-run; treat generated code as a starting point):

```cpp
void ResetServer::handle_request(
    const std_srvs::srv::Trigger::Request::SharedPtr /*request*/,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    // TODO: your actual reset logic here
    reset_my_system();
    response->success = true;
    response->message = "Reset complete";
}
```

---

## Cleanup

```bash
docker rm -f services_test
```
