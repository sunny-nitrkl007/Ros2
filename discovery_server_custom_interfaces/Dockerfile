FROM ros:jazzy

RUN mkdir -p /ros2_ws/src

# Copy the interfaces package first so generated_pkg can depend on it
COPY discovery_server_custom_interfaces/custom_interfaces_pkg/ /ros2_ws/src/custom_interfaces_pkg/
COPY discovery_server_custom_interfaces/generated_pkg/ /ros2_ws/src/generated_pkg/

WORKDIR /ros2_ws

# colcon resolves build order via package.xml dependencies automatically
RUN /bin/bash -c "\
    source /opt/ros/jazzy/setup.bash && \
    colcon build"

# Expose both install spaces so ros2 run can find both packages
ENV AMENT_PREFIX_PATH=/ros2_ws/install/generated_pkg:/ros2_ws/install/custom_interfaces_pkg:/opt/ros/jazzy
ENV PATH=/ros2_ws/install/generated_pkg/lib/generated_pkg:/opt/ros/jazzy/bin:$PATH
ENV ROS_DOMAIN_ID=10

RUN echo "source /opt/ros/jazzy/setup.bash" >> /etc/bash.bashrc

COPY tool/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
