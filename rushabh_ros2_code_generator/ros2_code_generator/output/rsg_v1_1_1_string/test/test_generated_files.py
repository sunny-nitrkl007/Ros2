from pathlib import Path


def test_required_files_exist():
    root = Path(__file__).resolve().parents[1]
    required = ["CMakeLists.txt", "package.xml", "config/params.yaml", "launch/bringup.launch.py"]
    required += ["src/sender.cpp", "include/rsg_v1_1_1_string/sender.hpp"]
    required += ["src/receiver.cpp", "include/rsg_v1_1_1_string/receiver.hpp"]
    for rel in required:
        assert (root / rel).exists(), f"Missing generated file: {rel}"
