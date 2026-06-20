FROM ros:jazzy

RUN mkdir -p /ros2_ws/src

COPY discovery_server_single_pub_sub/generated_pkg/ /ros2_ws/src/generated_pkg/
COPY tool/env.sh /env.sh

WORKDIR /ros2_ws

RUN /bin/bash -c "\
    source /opt/ros/jazzy/setup.bash && \
    colcon build --packages-select generated_pkg"

ENV AMENT_PREFIX_PATH=/ros2_ws/install/generated_pkg:/opt/ros/jazzy
ENV PATH=/ros2_ws/install/generated_pkg/lib/generated_pkg:/opt/ros/jazzy/bin:$PATH

RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc && \
    echo "source /ros2_ws/install/setup.bash" >> /root/.bashrc && \
    echo "source /env.sh" >> /root/.bashrc

COPY tool/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]