from pathlib import Path


def test_required_files_exist():
    root = Path(__file__).resolve().parents[1]
    required = ["CMakeLists.txt", "package.xml", "config/params.yaml", "launch/bringup.launch.py"]
    required += ["src/node_a.cpp", "include/rsg_v1_three_node_cross_pubsub_matrix/node_a.hpp"]
    required += ["src/node_b.cpp", "include/rsg_v1_three_node_cross_pubsub_matrix/node_b.hpp"]
    required += ["src/node_c.cpp", "include/rsg_v1_three_node_cross_pubsub_matrix/node_c.hpp"]
    for rel in required:
        assert (root / rel).exists(), f"Missing generated file: {rel}"
