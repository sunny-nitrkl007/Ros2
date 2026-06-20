import re
_PACKAGE_RE = re.compile(r"^[a-z][a-z0-9_]*$")
_IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
_ROS_NAME_RE = re.compile(r"^/?[A-Za-z0-9_~/][A-Za-z0-9_/~{}]*$")

# Function: is_valid_package_name
# Purpose: See module docstring and inline code for detailed behavior.
def is_valid_package_name(value: str) -> bool: return bool(value and _PACKAGE_RE.match(value))

# Function: is_valid_identifier
# Purpose: See module docstring and inline code for detailed behavior.
def is_valid_identifier(value: str) -> bool: return bool(value and _IDENTIFIER_RE.match(value))

# Function: is_valid_ros_name
# Purpose: See module docstring and inline code for detailed behavior.
def is_valid_ros_name(value: str) -> bool: return bool(value and _ROS_NAME_RE.match(value))

# Function: snake_to_camel
# Purpose: See module docstring and inline code for detailed behavior.
def snake_to_camel(value: str) -> str: return "".join(part.capitalize() for part in value.split("_") if part)
