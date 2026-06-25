#!/bin/bash
# Inject demo data: publishes all topics at 1 Hz, calls all services every 5s.
# Copy to any running container and run:
#   docker cp generated/demo/run_demo.sh automotive_adas-perception-1:/tmp/
#   docker exec -it automotive_adas-perception-1 bash /tmp/run_demo.sh

source /opt/ros/jazzy/setup.bash
source /ros2_ws/install/setup.bash

cleanup() { echo "Stopping demo..."; kill $(jobs -p) 2>/dev/null; wait; }
trap cleanup SIGINT SIGTERM

echo "==> ADAS demo: publishing topics at 1 Hz, calling services every 5s"
echo "    Ctrl+C to stop."

ros2 topic pub '/perception/obstacle_data' adas_interfaces/msg/ObstacleData '{id: 1, distance: 1.0, relative_velocity: 1.0, object_type: demo}' --rate 1 &
ros2 topic pub '/planning/trajectory' adas_interfaces/msg/Trajectory '{x_points: [], y_points: [], target_speed: 1.0, emergency_flag: false}' --rate 1 &
ros2 topic pub '/safety/emergency_brake' adas_interfaces/msg/EmergencyBrake '{apply_brake: false, brake_force: 1.0, reason: demo}' --rate 1 &

while true; do
    sleep 5
    ros2 service call '/perception/get_lane_info' adas_interfaces/srv/GetLaneInfo '{request: false}' 2>/dev/null || true
    ros2 service call '/safety/check_risk' adas_interfaces/srv/CheckRisk '{obstacle_distance: 1.0, vehicle_speed: 1.0}' 2>/dev/null || true
    ros2 service call '/planning/request_path_update' adas_interfaces/srv/RequestPathUpdate '{emergency: false, obstacle_distance: 1.0}' 2>/dev/null || true
done
