FROM ros:jazzy

RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ros2_ws/src

COPY generated/adas_interfaces    adas_interfaces
COPY generated/perception_system  perception_system
COPY generated/planning_system    planning_system
COPY generated/safety_system      safety_system

WORKDIR /ros2_ws

RUN bash -c "\
    source /opt/ros/jazzy/setup.bash && \
    colcon build --packages-select adas_interfaces && \
    source install/setup.bash && \
    colcon build --packages-select perception_system planning_system safety_system"

RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc && \
    echo "source /ros2_ws/install/setup.bash" >> /root/.bashrc
