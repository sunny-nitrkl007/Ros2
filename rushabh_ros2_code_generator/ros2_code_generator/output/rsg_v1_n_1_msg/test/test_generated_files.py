from pathlib import Path


def test_required_files_exist():
    root = Path(__file__).resolve().parents[1]
    required = ["CMakeLists.txt", "package.xml", "config/params.yaml", "launch/bringup.launch.py"]
    required += ["src/sender_1.cpp", "include/rsg_v1_n_1_msg/sender_1.hpp"]
    required += ["src/sender_2.cpp", "include/rsg_v1_n_1_msg/sender_2.hpp"]
    required += ["src/sender_3.cpp", "include/rsg_v1_n_1_msg/sender_3.hpp"]
    required += ["src/receiver.cpp", "include/rsg_v1_n_1_msg/receiver.hpp"]
    required += ["msg/MachineStatus.msg"]
    for rel in required:
        assert (root / rel).exists(), f"Missing generated file: {rel}"
