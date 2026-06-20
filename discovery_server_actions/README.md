# Project: discovery_server_actions

## Goal

Test **ROS2 Actions** through a Fast-DDS Discovery Server.

Actions are the most complex ROS2 communication primitive. Unlike a service (send request → get one response), an action supports:
- **Goal**: client sends a request with parameters
- **Feedback**: server streams intermediate progress back to the client while working
- **Result**: server sends a final answer when done
- **Cancellation**: client can cancel a running goal mid-execution

This project uses `example_interfaces/action/Fibonacci` — a built-in ROS2 action that computes Fibonacci numbers, sending each new number as feedback and returning the full sequence as the result.

**Topology:** `fibonacci_client` --[goal]--> Discovery Server --[route]--> `fibonacci_server` --[feedback/result]--> `fibonacci_client`

**What makes actions interesting for DS testing:**
An action internally creates **5 sub-topics** per action:
- `_action/send_goal` (service)
- `_action/cancel_goal` (service)
- `_action/get_result` (service)
- `_action/feedback` (topic)
- `_action/status` (topic)

The DS must register and route all 5. This is the heaviest per-interaction endpoint registration load of any ROS2 primitive.

---

## Action Flow Detail

```
Client                           Server
  |                                |
  |----[send_goal: order=10]------>|  goal accepted, execute() starts in a thread
  |                                |
  |<---[feedback: F(2)=1]----------|  every 500 ms, next Fibonacci number
  |<---[feedback: F(3)=2]----------|
  |<---[feedback: F(4)=3]----------|
  |<---[feedback: F(5)=5]----------|
  |<---[feedback: F(6)=8]----------|
  |<---[feedback: F(7)=13]---------|
  |<---[feedback: F(8)=21]---------|
  |<---[feedback: F(9)=34]---------|
  |                                |
  |<---[result: [0,1,1,2,3,5,8,13,21,34]]------|
  |                                |
  (5 second wait, then next goal)
```

---

## YAML Config

Action node schema (replaces `topic`/`service` fields):
```yaml
node:
  name: fibonacci_server
  type: action_server        # or action_client
  action: /fibonacci         # action name registered with DS
  action_type: example_interfaces/action/Fibonacci
```

The generator auto-adds `rclcpp_action` and `example_interfaces` to `CMakeLists.txt` and `package.xml`.

---

## Step 1 — Generate

```bash
python tool/generator.py --project discovery_server_actions
```

Generated files:
- `generated_pkg/src/fibonacci_server.cpp` — accepts goals, streams feedback, returns result
- `generated_pkg/src/fibonacci_client.cpp` — sends goal every 5 s, logs feedback + result
- `generated_pkg/include/fibonacci_server.hpp`
- `generated_pkg/include/fibonacci_client.hpp`
- `generated_pkg/CMakeLists.txt` — includes `rclcpp_action`, `example_interfaces`
- `generated_pkg/package.xml`

---

## Step 2 — Build

```bash
docker build --no-cache --build-arg PROJECT=discovery_server_actions -f tool/Dockerfile -t ros2_ds_actions .
```

---

## Step 3 — Test (3 Terminals)

### Terminal 1 — Container + Discovery Server

```bash
docker run -it --rm --name actions_test ros2_ds_actions bash
fastdds discovery -i 0 -p 11811
```

### Terminal 2 — Action Server

```bash
docker exec -it actions_test bash
ros2 run generated_pkg fibonacci_server
```

Expected (one block per client goal):
```
[INFO] [fibonacci_server]: Action server '/fibonacci' ready.
[INFO] [fibonacci_server]: Received goal request: order=10
[INFO] [fibonacci_server]: Executing: computing Fibonacci(10)...
[INFO] [fibonacci_server]: Feedback [2/9]: F(2)=1
[INFO] [fibonacci_server]: Feedback [3/9]: F(3)=2
[INFO] [fibonacci_server]: Feedback [4/9]: F(4)=3
[INFO] [fibonacci_server]: Feedback [5/9]: F(5)=5
[INFO] [fibonacci_server]: Feedback [6/9]: F(6)=8
[INFO] [fibonacci_server]: Feedback [7/9]: F(7)=13
[INFO] [fibonacci_server]: Feedback [8/9]: F(8)=21
[INFO] [fibonacci_server]: Feedback [9/9]: F(9)=34
[INFO] [fibonacci_server]: Goal succeeded. Sequence length=10, last value=34
```

### Terminal 3 — Action Client

```bash
docker exec -it actions_test bash
ros2 run generated_pkg fibonacci_client
```

Expected:
```
[INFO] [fibonacci_client]: Action client ready. Sending goals to '/fibonacci' every 5 s.
[INFO] [fibonacci_client]: Sending goal: order=10
[INFO] [fibonacci_client]: Goal accepted by server.
[INFO] [fibonacci_client]: Feedback: 3 values so far, latest F=1
[INFO] [fibonacci_client]: Feedback: 4 values so far, latest F=2
[INFO] [fibonacci_client]: Feedback: 5 values so far, latest F=3
[INFO] [fibonacci_client]: Feedback: 6 values so far, latest F=5
[INFO] [fibonacci_client]: Feedback: 7 values so far, latest F=8
[INFO] [fibonacci_client]: Feedback: 8 values so far, latest F=13
[INFO] [fibonacci_client]: Feedback: 9 values so far, latest F=21
[INFO] [fibonacci_client]: Feedback: 10 values so far, latest F=34
[INFO] [fibonacci_client]: Result: 10 Fibonacci numbers computed, last=34
```

---

## What to Observe

### 1 — The 5 internal sub-topics the DS must register

```bash
docker exec -it actions_test bash
ros2 topic list
ros2 service list
```

You will see:
```
/fibonacci/_action/feedback
/fibonacci/_action/status
/fibonacci/_action/cancel_goal
/fibonacci/_action/get_result
/fibonacci/_action/send_goal
```

All 5 exist before the first goal is even sent — the DS registered them during node startup.

### 2 — Goal cancellation mid-flight

In a 4th terminal, get the goal handle and cancel it while it's running:
```bash
docker exec -it actions_test bash
ros2 action send_goal /fibonacci example_interfaces/action/Fibonacci "{order: 20}" --feedback
# Then Ctrl+C in the client terminal while feedback is still arriving
```

The server logs `Goal cancelled at step N.` — the action framework handles cleanup through the DS.

### 3 — Kill the DS and observe

Start a goal, then kill the DS in Terminal 1 (`Ctrl+C`). Current in-flight action completes (the connection was already established). Next goal from the client after 5 s will fail with `wait_for_action_server` timeout — new goal negotiation requires DS.

---

## Cleanup

```bash
docker rm -f actions_test
```
