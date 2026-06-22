"""
loader.py — load, validate, and resolve all YAML config into Python dicts.
No Jinja2, no file writing. Pure data in, structured dicts out.
"""

import os, sys, yaml

# ── Node type groups ──────────────────────────────────────────────────────────

TOPIC_TYPES  = ("publisher", "subscriber", "lifecycle_publisher", "lifecycle_subscriber")
SRV_TYPES    = ("service_server", "service_client")
ACTION_TYPES = ("action_server", "action_client")
LC_TYPES     = ("lifecycle_publisher", "lifecycle_subscriber")
ALL_TYPES    = TOPIC_TYPES + SRV_TYPES + ACTION_TYPES

# ── YAML reader ───────────────────────────────────────────────────────────────

def _read(project_path, rel, required=True):
    full = os.path.join(project_path, rel)
    if not os.path.isfile(full):
        if required:
            print(f"ERROR: required file missing: {full}")
            sys.exit(1)
        return None
    print(f"  loading {full}")
    with open(full, encoding='utf-8') as f:
        return yaml.safe_load(f)

# ── Dependency collection ─────────────────────────────────────────────────────

def collect_deps(nodes):
    deps = {'rclcpp'}
    for node in nodes:
        if node.get('message_type'):
            deps.add(node['message_type'].split('/')[0])
        if node.get('srv_type_raw'):
            deps.add(node['srv_type_raw'].split('/')[0])
        if node.get('action_type_raw'):
            deps.add(node['action_type_raw'].split('/')[0])
        if node['type'] in ACTION_TYPES:
            deps.add('rclcpp_action')
        if node['type'] in LC_TYPES:
            deps.add('rclcpp_lifecycle')
            deps.add('lifecycle_msgs')
    return sorted(deps)

# ── Validator ─────────────────────────────────────────────────────────────────

def _validate(project_path, application, topics, qos_profiles, parameters):
    errors = []

    for node_name in application.get('nodes', []):
        node_file = f"config/nodes/{node_name}.yaml"
        full_path = os.path.join(project_path, node_file)

        if not os.path.isfile(full_path):
            errors.append(f"Node '{node_name}': file missing at {node_file}")
            continue

        with open(full_path, encoding='utf-8') as f:
            node_yaml = yaml.safe_load(f).get('node', {})

        ntype = node_yaml.get('type', '')
        if ntype not in ALL_TYPES:
            errors.append(
                f"Node '{node_name}': unknown type '{ntype}' (valid: {ALL_TYPES})"
            )
            continue

        if ntype in TOPIC_TYPES:
            topic = node_yaml.get('topic', '')
            if not topic:
                errors.append(f"Node '{node_name}': missing 'topic' field")
            elif topic not in topics:
                errors.append(
                    f"Node '{node_name}': topic '{topic}' not in topics.yaml "
                    f"(defined: {list(topics.keys())})"
                )
            qos_name = node_yaml.get('qos', '')
            if qos_name and qos_name not in qos_profiles:
                errors.append(
                    f"Node '{node_name}': qos '{qos_name}' not in qos_profiles.yaml "
                    f"(defined: {list(qos_profiles.keys())})"
                )

        elif ntype in SRV_TYPES:
            if not node_yaml.get('service'):
                errors.append(f"Node '{node_name}': missing 'service' field")
            if not node_yaml.get('srv_type'):
                errors.append(f"Node '{node_name}': missing 'srv_type' field")

        elif ntype in ACTION_TYPES:
            if not node_yaml.get('action'):
                errors.append(f"Node '{node_name}': missing 'action' field")
            if not node_yaml.get('action_type'):
                errors.append(f"Node '{node_name}': missing 'action_type' field")

        param_group = node_yaml.get('parameters', '')
        if param_group and param_group not in parameters:
            errors.append(
                f"Node '{node_name}': parameter group '{param_group}' not in parameters.yaml "
                f"(defined: {list(parameters.keys())})"
            )

    if errors:
        print(f"\nERROR: {len(errors)} config error(s):")
        for e in errors:
            print(f"  ✗ {e}")
        sys.exit(1)

    print("  Config OK")

# ── Node resolver ─────────────────────────────────────────────────────────────

def _resolve_node(project_path, node_name, topics, qos_profiles, parameters):
    node_yaml = _read(project_path, f"config/nodes/{node_name}.yaml")["node"]
    node_type = node_yaml["type"]

    result = {
        "name":        node_name,
        "type":        node_type,
        "namespace":   node_yaml.get("namespace", ""),
        "param_group": node_yaml.get("parameters", ""),
        "parameters":  parameters.get(node_yaml.get("parameters", ""), {}),
    }

    if node_type in TOPIC_TYPES:
        topic_name = node_yaml["topic"]
        result.update({
            "topic":        topic_name,
            "message_type": topics[topic_name]["type"],
            "qos":          qos_profiles.get(node_yaml.get("qos", ""), {}),
            "qos_name":     node_yaml.get("qos", ""),
        })
    elif node_type in SRV_TYPES:
        result.update({
            "service":      node_yaml["service"],
            "srv_type_raw": node_yaml["srv_type"],
        })
    elif node_type in ACTION_TYPES:
        result.update({
            "action":          node_yaml["action"],
            "action_type_raw": node_yaml["action_type"],
        })

    return result

# ── Public entry point ────────────────────────────────────────────────────────

def load(project_path):
    """
    Load all YAML configs, validate all references, resolve node files to dicts.
    Returns dict with keys: application, nodes.
    Calls sys.exit(1) on any config error.
    """
    application  = _read(project_path, "config/application.yaml")["application"]
    topics       = _read(project_path, "config/topics.yaml").get("topics") or {}
    qos_profiles = _read(project_path, "config/qos_profiles.yaml").get("qos_profiles") or {}
    parameters   = _read(project_path, "config/parameters.yaml").get("parameters") or {}

    print(f"\n  Project : {application.get('name', '?')}"
          f"  domain_id={application.get('domain_id', '?')}"
          f"  nodes={application.get('nodes', [])}")

    _validate(project_path, application, topics, qos_profiles, parameters)

    nodes = [
        _resolve_node(project_path, name, topics, qos_profiles, parameters)
        for name in application['nodes']
    ]

    return {
        'application': application,
        'nodes':       nodes,
    }
