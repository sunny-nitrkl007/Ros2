ARG PROJECT=discovery_server_single_pub_sub

FROM ros:jazzy

# Re-declare ARG so it is usable after FROM
ARG PROJECT=discovery_server_single_pub_sub

# Create workspace
RUN mkdir -p /ros2_ws/src

# Copy the generated package; env.sh is always in tool/ (generator writes it there)
COPY ${PROJECT}/generated_pkg/ /ros2_ws/src/generated_pkg/
COPY tool/env.sh /env.sh

# Build the package
WORKDIR /ros2_ws
RUN /bin/bash -c "\
    source /opt/ros/jazzy/setup.bash && \
    colcon build --packages-select generated_pkg"

# Source ROS base and env.sh (domain_id, discovery server, ament paths) in every bash session
RUN echo "source /opt/ros/jazzy/setup.bash" >> /etc/bash.bashrc && \
    echo "source /ros2_ws/install/setup.bash" >> /etc/bash.bashrc && \
    echo "source /env.sh" >> /etc/bash.bashrc

# Entrypoint sources ROS jazzy, then runs whatever command is passed
COPY tool/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
