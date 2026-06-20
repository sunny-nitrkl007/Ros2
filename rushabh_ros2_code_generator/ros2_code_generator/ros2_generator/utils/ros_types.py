"""
ROS type parsing helpers.

Version History:
- v1.0.0: Added message/service package inference and C++ type conversion.
- v1.1.0: Added custom interface generation helpers for .msg and .srv files.
"""

from dataclasses import dataclass
from .errors import ValidationError


@dataclass(frozen=True)

# Class: RosType
# Purpose: See module docstring for this class's role in the generation pipeline.
class RosType:
    package: str
    kind: str
    name: str

    @property
    def cpp_type(self) -> str:
        return f"{self.package}::{self.kind}::{self.name}"

    @property
    def include_path(self) -> str:
        import re
        snake = re.sub(r"(?<!^)(?=[A-Z])", "_", self.name).lower()
        return f"{self.package}/{self.kind}/{snake}.hpp"



# Function: parse_ros_type
# Purpose: See module docstring and inline code for detailed behavior.
def parse_ros_type(value: str, expected_kind: str | None = None) -> RosType:
    parts = value.split("/") if value else []
    if len(parts) != 3 or parts[1] not in {"msg", "srv"}:
        raise ValidationError(f"Invalid ROS type '{value}'. Expected pkg/msg/Type or pkg/srv/Type.")
    if expected_kind and parts[1] != expected_kind:
        raise ValidationError(f"Invalid ROS type '{value}'. Expected kind '{expected_kind}'.")
    return RosType(parts[0], parts[1], parts[2])



# Function: param_cpp_type
# Purpose: See module docstring and inline code for detailed behavior.
def param_cpp_type(param_type: str) -> str:
    mapping = {"bool": "bool", "int": "int64_t", "double": "double", "string": "std::string"}
    if param_type not in mapping:
        raise ValidationError(f"Unsupported parameter type '{param_type}'.")
    return mapping[param_type]



# Function: param_default_cpp
# Purpose: See module docstring and inline code for detailed behavior.
def param_default_cpp(value, param_type: str) -> str:
    if param_type == "bool":
        return "true" if bool(value) else "false"
    if param_type == "int":
        return str(int(value))
    if param_type == "double":
        return str(float(value))
    if param_type == "string":
        escaped = str(value).replace('\\', '\\\\').replace('"', '\\"')
        return f'"{escaped}"'
    raise ValidationError(f"Unsupported parameter type '{param_type}'.")



# Function: render_interface_field
# Purpose: See module docstring and inline code for detailed behavior.
def render_interface_field(field: dict) -> str:
    """
    Render one .msg/.srv field line.

    Version History:
    - v1.1.0: Added interface field rendering.
    """
    return f"{field['type']} {field['name']}"
