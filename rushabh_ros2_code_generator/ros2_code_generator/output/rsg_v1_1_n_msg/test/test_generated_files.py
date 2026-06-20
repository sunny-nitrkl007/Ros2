from pathlib import Path


def test_required_files_exist():
    root = Path(__file__).resolve().parents[1]
    required = ["CMakeLists.txt", "package.xml", "config/params.yaml", "launch/bringup.launch.py"]
    required += ["src/sender.cpp", "include/rsg_v1_1_n_msg/sender.hpp"]
    required += ["src/receiver_1.cpp", "include/rsg_v1_1_n_msg/receiver_1.hpp"]
    required += ["src/receiver_2.cpp", "include/rsg_v1_1_n_msg/receiver_2.hpp"]
    required += ["src/receiver_3.cpp", "include/rsg_v1_1_n_msg/receiver_3.hpp"]
    required += ["msg/MachineStatus.msg"]
    for rel in required:
        assert (root / rel).exists(), f"Missing generated file: {rel}"
