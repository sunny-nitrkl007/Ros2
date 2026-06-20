# RSG V1 grammar loader.
#
# Responsibilities:
# - Load application.yaml plus optional folder-based grammar files.
# - Keep optional files optional while allowing examples to provide all files.
# - Load ros2/execution.yaml for executor selection.
# - Treat generation.enabled=false as a successful no-output grammar.

from pathlib import Path
import yaml
from ros2_generator.utils.errors import ValidationError


def read_yaml(path: Path, required: bool = True) -> dict:
    # Read one YAML file and return a dictionary.
    if not path.exists():
        if required:
            raise ValidationError(f"Required YAML file not found: {path}")
        return {}
    with path.open("r", encoding="utf-8") as handle:
        data = yaml.safe_load(handle) or {}
    if not isinstance(data, dict):
        raise ValidationError(f"YAML file must contain a mapping: {path}")
    return data


def load_grammar(grammar_dir: str | Path) -> dict:
    # Load all supported grammar files from an application/example folder.
    root = Path(grammar_dir)
    if not root.exists():
        raise ValidationError(f"Grammar directory does not exist: {root}")

    application = read_yaml(root / "application.yaml")
    generation = application.get("generation", {})

    if generation.get("enabled", True) is False:
        return {
            "root": root,
            "application": application,
            "runtime": {},
            "topology": {},
            "parameters": {},
            "nodes": {"nodes": []},
            "topics": {"topics": []},
            "services": {"services": []},
            "qos": {"qos_profiles": []},
            "interfaces": {"interfaces": {"messages": [], "services": []}},
            "logging": {},
            "execution": {},
        }

    return {
        "root": root,
        "application": application,
        "runtime": read_yaml(root / "general" / "runtime.yaml", required=False),
        "topology": read_yaml(root / "topology" / "topology.yaml", required=False),
        "parameters": read_yaml(root / "parameters" / "parameters.yaml", required=False),
        "nodes": read_yaml(root / "ros2" / "nodes.yaml", required=False),
        "topics": read_yaml(root / "ros2" / "topics.yaml", required=False),
        "services": read_yaml(root / "ros2" / "services.yaml", required=False),
        "qos": read_yaml(root / "ros2" / "qos_profiles.yaml", required=False),
        "interfaces": read_yaml(root / "ros2" / "interfaces.yaml", required=False),
        "logging": read_yaml(root / "ros2" / "logging.yaml", required=False),
        "execution": read_yaml(root / "ros2" / "execution.yaml", required=False),
    }
