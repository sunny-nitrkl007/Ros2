from pathlib import Path


def test_required_files_exist():
    root = Path(__file__).resolve().parents[1]
    required = ["CMakeLists.txt", "package.xml", "config/params.yaml", "launch/bringup.launch.py"]
    required += ["src/server.cpp", "include/rsg_v1_service_server_client_std/server.hpp"]
    required += ["src/client.cpp", "include/rsg_v1_service_server_client_std/client.hpp"]
    for rel in required:
        assert (root / rel).exists(), f"Missing generated file: {rel}"
