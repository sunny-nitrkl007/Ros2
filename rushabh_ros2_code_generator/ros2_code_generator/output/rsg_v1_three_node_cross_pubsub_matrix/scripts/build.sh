#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
colcon build --packages-select rsg_v1_three_node_cross_pubsub_matrix
