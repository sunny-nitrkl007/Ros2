"""
RSG V1 module documentation.

Dependency resolver for generated ROS 2 packages.

Responsibilities:
- Infer package.xml and CMake dependencies from actual grammar usage.
- Avoid dependencies for absent features.

Version History:
- v1.6.1: Added detailed maintainer documentation comments.
"""

from ros2_generator.core.models import ApplicationModel
from ros2_generator.utils.ros_types import parse_ros_type



# Function: resolve_dependencies
# Purpose: See module docstring and inline code for detailed behavior.
def resolve_dependencies(model: ApplicationModel) -> list[str]:
    deps = {"rclcpp"}

    if any(node.parameters for node in model.nodes):
        deps.add("rcl_interfaces")

    for topic in model.topics.values():
        pkg = parse_ros_type(topic.message_type, "msg").package
        if pkg != model.app.package_name:
            deps.add(pkg)

    for service in model.services.values():
        pkg = parse_ros_type(service.service_type, "srv").package
        if pkg != model.app.package_name:
            deps.add(pkg)

    if model.interfaces.enabled:
        deps.update({"rosidl_default_generators", "rosidl_default_runtime", "builtin_interfaces"})

    model.dependencies = sorted(deps)
    return model.dependencies
