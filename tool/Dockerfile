FROM ros:jazzy

# Create workspace
RUN mkdir -p /ros2_ws/src

# Copy the generated package into the workspace
COPY generated_pkg/ /ros2_ws/src/generated_pkg/

# Build the package
WORKDIR /ros2_ws
RUN /bin/bash -c "\
    source /opt/ros/jazzy/setup.bash && \
    colcon build --packages-select generated_pkg"

# Set AMENT_PREFIX_PATH as a Docker ENV so it is present in every session
# (docker run, docker exec, entrypoint) without relying on sourcing scripts
ENV AMENT_PREFIX_PATH=/ros2_ws/install/generated_pkg:/opt/ros/jazzy
ENV PATH=/ros2_ws/install/generated_pkg/lib/generated_pkg:/opt/ros/jazzy/bin:$PATH
ENV ROS_DOMAIN_ID=10

# Still source ROS base in bash sessions so ros2 CLI works
RUN echo "source /opt/ros/jazzy/setup.bash" >> /etc/bash.bashrc

# Entrypoint sources ROS jazzy, then runs whatever command is passed
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
