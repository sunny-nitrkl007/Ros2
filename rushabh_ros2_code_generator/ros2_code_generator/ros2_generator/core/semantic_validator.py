"""
RSG V1 module documentation.

Semantic validator for RSG V1 normalized models.

Responsibilities:
- Validate names, references, QoS values, interface types, and endpoint links.
- Fail before rendering invalid ROS 2 code.

Version History:
- v1.6.1: Added detailed maintainer documentation comments.
"""

from ros2_generator.core.models import ApplicationModel
from ros2_generator.utils.errors import ValidationError
from ros2_generator.utils.naming import is_valid_package_name, is_valid_identifier, is_valid_ros_name
from ros2_generator.utils.ros_types import parse_ros_type


# Function: _unique
# Purpose: See module docstring and inline code for detailed behavior.
def _unique(values, label):
    seen = set()
    for v in values:
        if v in seen:
            raise ValidationError(f"Duplicate {label}: {v}")
        seen.add(v)


# Function: validate_model
# Purpose: See module docstring and inline code for detailed behavior.
def validate_model(model: ApplicationModel) -> None:
    if not is_valid_package_name(model.app.package_name):
        raise ValidationError(f"Invalid ROS 2 package name: {model.app.package_name}")
    if model.generation.language != "cpp": raise ValidationError("RSG V1 supports only language: cpp")
    if model.generation.build_system != "ament_cmake": raise ValidationError("RSG V1 supports only build_system: ament_cmake")
    if model.code_layout.mode not in {"inline_user_sections", "separated_user_logic"}: raise ValidationError("Invalid code_layout.mode")
    _unique([m.name for m in model.interfaces.messages], "interface message")
    _unique([s.name for s in model.interfaces.services], "interface service")
    for m in model.interfaces.messages:
        if not is_valid_identifier(m.name): raise ValidationError(f"Invalid message name: {m.name}")
        for f in m.fields:
            if not is_valid_identifier(f.get("name", "")): raise ValidationError(f"Invalid field in message {m.name}")
    for s in model.interfaces.services:
        if not is_valid_identifier(s.name): raise ValidationError(f"Invalid service name: {s.name}")
    for q in model.qos_profiles.values():
        if q.reliability not in {"reliable", "best_effort"}: raise ValidationError(f"Invalid QoS reliability in {q.name}")
        if q.durability not in {"volatile", "transient_local"}: raise ValidationError(f"Invalid QoS durability in {q.name}")
        if q.history not in {"keep_last", "keep_all"}: raise ValidationError(f"Invalid QoS history in {q.name}")
        if q.depth <= 0: raise ValidationError(f"QoS depth must be positive in {q.name}")
    for t in model.topics.values():
        if not is_valid_ros_name(t.topic_name): raise ValidationError(f"Invalid topic name: {t.topic_name}")
        parse_ros_type(t.message_type, "msg")
        if t.qos_profile not in model.qos_profiles: raise ValidationError(f"Unknown QoS {t.qos_profile}")
    for s in model.services.values():
        if not is_valid_ros_name(s.service_name): raise ValidationError(f"Invalid service name: {s.service_name}")
        parse_ros_type(s.service_type, "srv")
    _unique([n.name for n in model.nodes], "node")
    _unique([n.executable for n in model.nodes], "executable")
    for n in model.nodes:
        if n.type != "regular": raise ValidationError("RSG V1 only supports regular nodes")
        if not is_valid_identifier(n.name): raise ValidationError(f"Invalid node name: {n.name}")
        if not is_valid_identifier(n.executable): raise ValidationError(f"Invalid executable: {n.executable}")
        for p in n.publishers:
            if not p.topic: raise ValidationError(f"Publisher {p.name} unknown topic_ref {p.topic_ref}")
        for s in n.subscribers:
            if not s.topic: raise ValidationError(f"Subscriber {s.name} unknown topic_ref {s.topic_ref}")
        for s in n.services:
            if not s.service: raise ValidationError(f"Service {s.name} unknown service_ref {s.service_ref}")
        for c in n.clients:
            if not c.service: raise ValidationError(f"Client {c.name} unknown service_ref {c.service_ref}")


# =============================================================================
# RSG_V1_EXECUTOR_VALIDATOR_WRAPPER_SAFE
# -----------------------------------------------------------------------------
# Validates executor metadata added by normalizer wrapper.
# =============================================================================
_original_validate_model_before_executor_safe = validate_model


def validate_model(model: ApplicationModel) -> None:
    _original_validate_model_before_executor_safe(model)
    for node in model.nodes:
        executor = getattr(node, "execution_executor", "single_threaded")
        threads = int(getattr(node, "execution_threads", 1))
        if executor not in {"single_threaded", "multi_threaded"}:
            raise ValidationError(
                f"Invalid executor '{executor}' for node '{node.name}'. Supported values: single_threaded, multi_threaded."
            )
        if threads <= 0:
            raise ValidationError(f"number_of_threads must be positive for node '{node.name}'.")
        if executor == "single_threaded" and threads != 1:
            raise ValidationError(
                f"single_threaded executor for node '{node.name}' must use number_of_threads: 1."
            )


# =============================================================================
# RSG_V1_SERVICE_CONTROL_VALIDATOR_V114
# -----------------------------------------------------------------------------
# Validates explicit service-control grammar.
# =============================================================================
_original_validate_model_before_service_control_v114 = validate_model


def validate_model(model: ApplicationModel) -> None:
    _original_validate_model_before_service_control_v114(model)
    for node in model.nodes:
        parameter_names = {param.name for param in node.parameters}
        for service_spec in node.services:
            control = getattr(service_spec, "control", {}) or {}
            if not control:
                continue
            target = control.get("target_parameter")
            request_field = control.get("request_field")
            if not target:
                raise ValidationError(
                    f"Service '{service_spec.name}' in node '{node.name}' has control but no target_parameter."
                )
            if target not in parameter_names:
                raise ValidationError(
                    f"Service '{service_spec.name}' controls unknown parameter '{target}' in node '{node.name}'."
                )
            if not request_field:
                raise ValidationError(
                    f"Service '{service_spec.name}' in node '{node.name}' has control but no request_field."
                )
