#!/usr/bin/env bash
set -euo pipefail
for ex in examples/*; do
  if [ -d "$ex" ]; then
    echo "Generating $ex"
    python3 -m ros2_generator.main generate "$ex" output
  fi
done
