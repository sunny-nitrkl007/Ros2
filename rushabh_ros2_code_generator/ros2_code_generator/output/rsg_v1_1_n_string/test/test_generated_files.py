from pathlib import Path


def test_required_files_exist():
    root = Path(__file__).resolve().parents[1]
    required = ["CMakeLists.txt", "package.xml", "config/params.yaml", "launch/bringup.launch.py"]
    required += ["src/sender.cpp", "include/rsg_v1_1_n_string/sender.hpp"]
    required += ["src/receiver_1.cpp", "include/rsg_v1_1_n_string/receiver_1.hpp"]
    required += ["src/receiver_2.cpp", "include/rsg_v1_1_n_string/receiver_2.hpp"]
    required += ["src/receiver_3.cpp", "include/rsg_v1_1_n_string/receiver_3.hpp"]
    for rel in required:
        assert (root / rel).exists(), f"Missing generated file: {rel}"
