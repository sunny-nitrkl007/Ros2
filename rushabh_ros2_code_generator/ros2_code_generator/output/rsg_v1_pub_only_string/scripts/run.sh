#!/usr/bin/env bash
set -eo pipefail

# ==============================================================================
# Generated run helper
# ------------------------------------------------------------------------------
# Important: do not use `set -u` before sourcing ROS/colcon setup files.
# Some setup scripts read optional variables such as COLCON_TRACE while unset.
# ==============================================================================

export RCUTILS_COLORIZED_OUTPUT=1

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PKG_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

if [ -f "${PKG_DIR}/install/setup.bash" ]; then
  source "${PKG_DIR}/install/setup.bash"
elif [ -f "${PKG_DIR}/../install/setup.bash" ]; then
  source "${PKG_DIR}/../install/setup.bash"
else
  echo "ERROR: Could not find install/setup.bash. Build and source first."
  echo "Try one of:"
  echo "  cd ${PKG_DIR} && colcon build && source install/setup.bash"
  echo "  cd ${PKG_DIR}/.. && colcon build --packages-select rsg_v1_pub_only_string && source install/setup.bash"
  exit 1
fi

ros2 launch rsg_v1_pub_only_string bringup.launch.py
