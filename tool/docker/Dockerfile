FROM ros:jazzy
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-jazzy-trajectory-msgs \
    ros-jazzy-sensor-msgs \
    && rm -rf /var/lib/apt/lists/*
RUN mkdir -p /ros2_ws/src
COPY robot_arm_controller/generated_pkg/ /ros2_ws/src/generated_pkg/
WORKDIR /ros2_ws
RUN /bin/bash -c "\
    source /opt/ros/jazzy/setup.bash && \
    colcon build --packages-select generated_pkg"
ENV AMENT_PREFIX_PATH=/ros2_ws/install/generated_pkg:/opt/ros/jazzy
ENV PATH=/ros2_ws/install/generated_pkg/lib/generated_pkg:/opt/ros/jazzy/bin:$PATH
ENV ROS_DOMAIN_ID=10
ENV ROS_DISCOVERY_SERVER=127.0.0.1:11811
RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc && \
    echo "source /ros2_ws/install/setup.bash" >> /root/.bashrc
COPY tool/docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]