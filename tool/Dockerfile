ARG PROJECT=discovery_server_single_pub_sub

FROM ros:jazzy

# Re-declare ARG so it is usable after FROM
ARG PROJECT=discovery_server_single_pub_sub

# Create workspace
RUN mkdir -p /ros2_ws/src

# Copy the generated package and the project env file
COPY ${PROJECT}/generated_pkg/ /ros2_ws/src/generated_pkg/
COPY ${PROJECT}/env.sh /env.sh

# Build the package
WORKDIR /ros2_ws
RUN /bin/bash -c "\
    source /opt/ros/jazzy/setup.bash && \
    colcon build --packages-select generated_pkg"

# Set AMENT_PREFIX_PATH as a Docker ENV so it is present in every session
ENV AMENT_PREFIX_PATH=/ros2_ws/install/generated_pkg:/opt/ros/jazzy
ENV PATH=/ros2_ws/install/generated_pkg/lib/generated_pkg:/opt/ros/jazzy/bin:$PATH

# Source ROS base and the YAML-driven env (domain_id + discovery server) in every bash session
RUN echo "source /opt/ros/jazzy/setup.bash" >> /etc/bash.bashrc && \
    echo "source /env.sh" >> /etc/bash.bashrc

# Entrypoint sources ROS jazzy, then runs whatever command is passed
COPY tool/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
