# Demo: Three-Node Cross Pub/Sub Matrix

This example is intentionally simple and uses only `std_msgs/msg/String`.

## Routing

```text
A -> C: /matrix_demo/a_to_c/status
A -> C: /matrix_demo/a_to_c/command
A -> B: /matrix_demo/a_to_b/event
C -> A: /matrix_demo/c_to_a/feedback
B -> A: /matrix_demo/b_to_a/report
B -> B: /matrix_demo/b/internal_status
```

## Generate

```bash
cd rsg_v1_core
python3 -m ros2_generator.main generate examples/23_three_node_cross_pubsub_matrix output
```

## Build

```bash
cd output/rsg_v1_three_node_cross_pubsub_matrix
colcon build
source install/setup.bash
```

## Run

```bash
./scripts/run.sh
```

## Inspect

```bash
ros2 node list
ros2 topic list
ros2 topic info /matrix_demo/a_to_c/status
ros2 topic info /matrix_demo/b/internal_status
```

## Placeholder validation

```bash
cd rsg_v1_core
./validate_example_23_placeholders.sh
```
