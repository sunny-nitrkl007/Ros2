# RSG V1 Core - Reduced Scope ROS 2 Smart Boilerplate Generator

This prototype is intentionally small and maintainable. It is inspired by a larger Jinja2-based ROS 2 generator, but only supports the agreed V1 scope:

- regular `rclcpp` nodes
- publishers
- subscribers
- service servers
- service clients
- parameters with validation
- logging
- subscriber watchdogs
- QoS profiles
- optional launch/config/tests/scripts/README/report generation

The folder structure is designed so an offline team can add features later without needing to rewrite the core architecture.

## Generate an example

```bash
python3 -m ros2_generator.main generate examples/08_full_scope_no_launch_tests output
```

Generated package appears under:

```text
output/<package_name>/
```

## Build generated ROS 2 package

From a ROS 2 workspace:

```bash
colcon build --packages-select <package_name>
```
