"""
RSG V1 module documentation.

Normalizer for RSG V1 grammar.

Responsibilities:
- Convert raw YAML dictionaries into typed ApplicationModel dataclasses.
- Apply smart defaults for QoS, logging, node type, publish rate, and omitted sections.
- Attach resolved topic/service contracts to node endpoints.

Design note:
If a meaningful field is present in grammar, the normalized model should expose
enough data for templates to generate the corresponding ROS 2 code automatically.

Version History:
- v1.6.1: Added detailed maintainer documentation comments.
"""

from ros2_generator.core.models import *
from ros2_generator.utils.naming import snake_to_camel
from ros2_generator.utils.ros_types import param_cpp_type, param_default_cpp


# Function: _items
# Purpose: See module docstring and inline code for detailed behavior.
def _items(block: dict, key: str) -> list:
    block = block or {}
    value = block.get(key, [])
    if isinstance(value, dict):
        return value.get("items", []) or []
    return value or []


# Function: _section_items
# Purpose: See module docstring and inline code for detailed behavior.
def _section_items(node_yaml: dict, key: str) -> list:
    node_yaml = node_yaml or {}
    value = node_yaml.get(key, [])
    if isinstance(value, dict):
        return value.get("items", []) or []
    return value or []


# Function: build_model
# Purpose: See module docstring and inline code for detailed behavior.
def build_model(raw: dict) -> ApplicationModel:
    app_yaml = raw["application"]
    app_block = app_yaml.get("application", {})
    pkg_block = app_yaml.get("package", {})
    gen_block = app_yaml.get("generation", {})
    defaults = app_yaml.get("defaults", {})
    artifacts = gen_block.get("artifacts", {})
    regen = gen_block.get("regeneration", {})
    runtime = raw.get("runtime", {}).get("runtime", {})
    namespace = runtime.get("namespace", {}).get("value", defaults.get("namespace", ""))
    pkg_name = pkg_block.get("name", gen_block.get("output_package_name", app_block.get("name", "generated_app")))
    app = ApplicationInfo(
        name=app_block.get("name", pkg_name),
        description=app_block.get("description", "Generated ROS 2 package."),
        version=str(app_block.get("version", "0.1.0")),
        package_name=pkg_name,
        maintainer_name=pkg_block.get("maintainer_name", "Generated User"),
        maintainer_email=pkg_block.get("maintainer_email", "user@example.com"),
        license=pkg_block.get("license", "Apache-2.0"),
        namespace=namespace,
        log_level=defaults.get("log_level", "info"),
        use_sim_time=bool(defaults.get("use_sim_time", False)),
    )
    generation = GenerationOptions(
        output_package_name=gen_block.get("output_package_name", app.package_name),
        output_directory=gen_block.get("output_directory", "output"),
        enabled=bool(gen_block.get("enabled", True)),
        language=gen_block.get("language", "cpp"),
        build_system=gen_block.get("build_system", "ament_cmake"),
        ros_distro=gen_block.get("ros_distro", "humble"),
        core_package=bool(artifacts.get("core_package", True)),
        config=bool(artifacts.get("config", True)),
        launch=bool(artifacts.get("launch", True)),
        tests=bool(artifacts.get("tests", True)),
        scripts=bool(artifacts.get("scripts", True)),
        readme=bool(artifacts.get("readme", True)),
        generation_report=bool(artifacts.get("generation_report", True)),
        overwrite_generated=bool(regen.get("overwrite_generated", True)),
        preserve_user_code=bool(regen.get("preserve_user_code", True)),
        create_missing_user_stubs=bool(regen.get("create_missing_user_stubs", True)),
    )
    code_layout = CodeLayout(mode=gen_block.get("code_layout", {}).get("mode", "inline_user_sections"))
    iface_yaml = (raw.get("interfaces", {}) or {}).get("interfaces", {}) or {}
    interfaces = InterfaceModel(
        messages=[InterfaceMessage(name=m["name"], fields=m.get("fields", []) or []) for m in (iface_yaml.get("messages", []) or [])],
        services=[InterfaceService(name=s["name"], request=s.get("request", []) or [], response=s.get("response", []) or []) for s in (iface_yaml.get("services", []) or [])],
    )
    qos_items = _items(raw.get("qos", {}), "qos_profiles")
    if not qos_items:
        qos_items = [{"name": "reliable_default", "reliability": "reliable", "durability": "volatile", "history": "keep_last", "depth": 10}]
    qos_profiles = {q["name"]: QoSProfile(q["name"], q["reliability"], q["durability"], q["history"], int(q["depth"])) for q in qos_items}
    default_qos = next(iter(qos_profiles.keys()))
    topics = {}
    for t in _items(raw.get("topics", {}), "topics"):
        topics[t["name"]] = TopicContract(t["name"], t["topic_name"], t["message_type"], t.get("qos_profile", default_qos), t.get("description", ""))
    services = {s["name"]: ServiceContract(s["name"], s["service_name"], s["service_type"], s.get("description", "")) for s in _items(raw.get("services", {}), "services")}
    param_groups = ((raw.get("parameters", {}) or {}).get("parameters", {}) or {}).get("node_parameters", {}) or {}
    nodes_doc = (raw.get("nodes", {}) or {}).get("nodes", {}) or {}
    node_defaults = nodes_doc.get("defaults", {}) if isinstance(nodes_doc, dict) else {}
    default_logging = (raw.get("logging", {}) or {}).get("logging", {}) or {}
    endpoint_logging_default = bool(node_defaults.get("endpoint_logging", True))
    nodes = []
    for ny in _items(raw.get("nodes", {}), "nodes"):
        class_name = ny.get("generation", {}).get("class_name") or ny.get("class_name") or f"{snake_to_camel(ny['name'])}Node"
        param_group = ny.get("parameters", {}).get("parameter_group_ref") or ny.get("parameter_group")
        params = []
        if param_group:
            for p in param_groups.get(param_group, {}).get("items", []) or []:
                spec = ParameterSpec(p["name"], p["type"], p.get("default"), bool(p.get("dynamic", p.get("runtime_mutable", True))), p.get("description", ""), p.get("min"), p.get("max"), p.get("allowed_values", []) or [])
                spec.cpp_type = param_cpp_type(spec.type)
                spec.cpp_default = param_default_cpp(spec.default, spec.type)
                params.append(spec)
        node_logging_yaml = ny.get("logging", {})
        logging = LoggingSpec(
            enabled=bool(node_logging_yaml.get("enabled", default_logging.get("enabled", True))),
            level=node_logging_yaml.get("level", default_logging.get("default_level", app.log_level)),
            log_startup=bool(node_logging_yaml.get("log_startup", True)),
            log_shutdown=bool(node_logging_yaml.get("log_shutdown", True)),
            log_parameter_updates=bool(node_logging_yaml.get("log_parameter_updates", True)),
        )
        node = NodeSpec(ny["name"], ny.get("executable", ny["name"]), class_name, ny.get("type", node_defaults.get("type", "regular")), param_group, logging, params)
        for pub in _section_items(ny, "publishers"):
            if pub.get("enabled", True):
                node.publishers.append(PublisherSpec(pub["name"], pub["topic_ref"], pub.get("publish_mode", "timer"), pub.get("rate_hz", 1.0), pub.get("rate_parameter"), bool(pub.get("logging", endpoint_logging_default))))
        for sub in _section_items(ny, "subscribers"):
            if sub.get("enabled", True):
                wd = sub.get("watchdog", {}) or {}
                node.subscribers.append(SubscriberSpec(sub["name"], sub["topic_ref"], sub.get("callback", f"on_{sub['name']}"), bool(sub.get("logging", endpoint_logging_default)), WatchdogSpec(bool(wd.get("enabled", False)), int(wd.get("timeout_ms", 1000)), wd.get("severity", "warn"), bool(wd.get("idle_state", True)))))
        for srv in _section_items(ny, "services"):
            if srv.get("enabled", True):
                node.services.append(ServiceServerSpec(srv["name"], srv.get("service_ref", srv["name"]), srv.get("callback", f"on_{srv['name']}"), bool(srv.get("logging", endpoint_logging_default))))
        for cli in _section_items(ny, "clients"):
            if cli.get("enabled", True):
                node.clients.append(ServiceClientSpec(cli["name"], cli.get("service_ref", cli["name"]), cli.get("call_mode", "manual"), int(cli.get("wait_timeout_ms", 1000)), int(cli.get("request_timeout_ms", 2000)), cli.get("response_callback"), bool(cli.get("logging", endpoint_logging_default))))
        nodes.append(node)
    model = ApplicationModel(app, generation, code_layout, interfaces, qos_profiles, topics, services, nodes)
    for n in model.nodes:
        for p in n.publishers: p.topic = topics.get(p.topic_ref)
        for s in n.subscribers: s.topic = topics.get(s.topic_ref)
        for s in n.services: s.service = services.get(s.service_ref)
        for c in n.clients: c.service = services.get(c.service_ref)
    return model


# =============================================================================
# RSG_V1_EXECUTOR_WRAPPER_SAFE
# -----------------------------------------------------------------------------
# Adds executor metadata to each NodeSpec after the original model is built.
# This avoids touching existing grammar/model logic.
# =============================================================================
_original_build_model_before_executor_safe = build_model


def _rsg_v1_executor_items(block: dict, key: str) -> list:
    value = block.get(key, [])
    if isinstance(value, dict):
        return value.get("items", []) or []
    return value or []


def build_model(raw: dict) -> ApplicationModel:
    model = _original_build_model_before_executor_safe(raw)

    execution_root = raw.get("execution", {})
    execution_doc = execution_root.get("execution", {}) if isinstance(execution_root, dict) else {}
    if not isinstance(execution_doc, dict):
        execution_doc = {}

    default_execution = execution_doc.get("default", {}) or {}
    node_overrides = execution_doc.get("node_overrides", {}) or {}

    default_executor = default_execution.get("executor", "single_threaded")
    default_threads = int(default_execution.get("number_of_threads", 1))

    raw_node_items = _rsg_v1_executor_items(raw.get("nodes", {}), "nodes")
    raw_node_by_name = {
        item.get("name"): item
        for item in raw_node_items
        if isinstance(item, dict) and item.get("name")
    }

    for node in model.nodes:
        node_exec = {}
        if isinstance(node_overrides, dict):
            node_exec.update(node_overrides.get(node.name, {}) or {})
        node_yaml = raw_node_by_name.get(node.name, {})
        if isinstance(node_yaml, dict):
            node_exec.update(node_yaml.get("execution", {}) or {})

        node.execution_executor = node_exec.get("executor", default_executor)
        node.execution_threads = int(node_exec.get("number_of_threads", default_threads))

    return model


# =============================================================================
# RSG_V1_SERVICE_CONTROL_WRAPPER_V114
# -----------------------------------------------------------------------------
# Adds optional service-control metadata to ServiceServerSpec objects.
# This keeps dataclasses unchanged and lets templates check srv.control.
# =============================================================================
_original_build_model_before_service_control_v114 = build_model


def _rsg_v1_items_service_control_v114(block: dict, key: str) -> list:
    value = block.get(key, [])
    if isinstance(value, dict):
        return value.get("items", []) or []
    return value or []


def build_model(raw: dict) -> ApplicationModel:
    model = _original_build_model_before_service_control_v114(raw)
    raw_node_items = _rsg_v1_items_service_control_v114(raw.get("nodes", {}), "nodes")
    raw_node_by_name = {
        item.get("name"): item
        for item in raw_node_items
        if isinstance(item, dict) and item.get("name")
    }
    for node in model.nodes:
        raw_node = raw_node_by_name.get(node.name, {})
        raw_services = _rsg_v1_items_service_control_v114(raw_node, "services") if isinstance(raw_node, dict) else []
        raw_service_by_name = {
            item.get("name"): item
            for item in raw_services
            if isinstance(item, dict) and item.get("name")
        }
        for service_spec in node.services:
            raw_service = raw_service_by_name.get(service_spec.name, {})
            service_spec.control = raw_service.get("control", {}) if isinstance(raw_service, dict) else {}
    return model
