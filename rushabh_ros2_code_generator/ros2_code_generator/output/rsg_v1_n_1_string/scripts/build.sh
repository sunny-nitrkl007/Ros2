#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
colcon build --packages-select rsg_v1_n_1_string
