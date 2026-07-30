#!/bin/bash
# Entrypoint for the Stage 1 direct-DDS test harness image.
#
# No args: runs both harness nodes in this one container (simplest --
# same network namespace, so DDS discovery over loopback just works,
# no ROS_DOMAIN_ID/multicast config needed).
# "jobmgr" or "weighapp": runs only that node (for running the two
# sides in separate containers/hosts once discovery is configured).
set -e

source /opt/ros/jazzy/setup.bash
source /workspace/ros2_ws/install/setup.bash

MODE="${1:-both}"

case "$MODE" in
  jobmgr)
    exec ros2 run direct_dds_test_harness jobmgr_harness_node
    ;;
  weighapp)
    exec ros2 run direct_dds_test_harness weighapp_harness_node
    ;;
  both)
    # stdbuf -oL: stdout isn't a TTY under `docker run -d`, so it's fully
    # block-buffered by default -- force line buffering so `docker logs -f`
    # shows output promptly instead of only on buffer-fill/process-exit.
    (stdbuf -oL ros2 run direct_dds_test_harness weighapp_harness_node 2>&1 | sed 's/^/[weighapp] /') &
    WEIGH_PID=$!

    # Give weighapp's publishers/subscriptions a moment to come up before
    # jobmgr starts sending requests.
    sleep 1

    (stdbuf -oL ros2 run direct_dds_test_harness jobmgr_harness_node 2>&1 | sed 's/^/[jobmgr]  /') &
    JOB_PID=$!

    trap 'kill $WEIGH_PID $JOB_PID 2>/dev/null' EXIT INT TERM
    wait
    ;;
  *)
    echo "Unknown mode: $MODE (expected: both | jobmgr | weighapp)" >&2
    exit 1
    ;;
esac
