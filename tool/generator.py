import os
import re
import sys
import argparse
import yaml
from jinja2 import Environment, FileSystemLoader, StrictUndefined

TOOL_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR  = os.path.dirname(TOOL_DIR)

parser = argparse.ArgumentParser(description="ROS2 code generator")
parser.add_argument("--project", required=True, help="Project folder name (sibling of tool/)")
args = parser.parse_args()

PROJECT_PATH      = os.path.join(ROOT_DIR, args.project)
TOOL_TEMPLATES    = os.path.join(TOOL_DIR, "templates")
PROJECT_TEMPLATES = os.path.join(PROJECT_PATH, "templates")

if not os.path.isdir(PROJECT_PATH):
    print(f"ERROR: project folder not found: {PROJECT_PATH}")
    sys.exit(1)

if not os.path.isdir(TOOL_TEMPLATES):
    print(f"ERROR: tool/templates/ not found: {TOOL_TEMPLATES}")
    sys.exit(1)

# Project templates override tool templates when a file with the same name exists
template_dirs = []
if os.path.isdir(PROJECT_TEMPLATES):
    template_dirs.append(PROJECT_TEMPLATES)
template_dirs.append(TOOL_TEMPLATES)

env = Environment(
    loader=FileSystemLoader(template_dirs),
    undefined=StrictUndefined,
)


def load_yaml(rel_path):
    full_path = os.path.join(PROJECT_PATH, rel_path)
    print(f"Loading: {full_path}")
    with open(full_path, encoding="utf-8") as f:
        return yaml.safe_load(f)


def out(rel_path):
    return os.path.join(PROJECT_PATH, rel_path)


application  = load_yaml("config/application.yaml")["application"]
topics       = load_yaml("config/topics.yaml").get("topics") or {}
qos_profiles = load_yaml("config/qos_profiles.yaml").get("qos_profiles") or {}
parameters   = load_yaml("config/parameters.yaml").get("parameters") or {}

print("\n===== CONFIG LOAD SUMMARY =====")
print(f"Project           : {args.project}")
print(f"Application name  : {application.get('name', 'N/A')}")
print(f"Domain ID         : {application.get('domain_id', 'N/A')}")
print(f"Nodes             : {application.get('nodes', [])}")
print(f"Topics            : {list(topics.keys())}")
print(f"QoS profiles      : {list(qos_profiles.keys())}")
print(f"Parameter groups  : {list(parameters.keys())}")
print("================================\n")


# ── Impl block preservation ───────────────────────────────────────────────────
# Markers wrap developer-written sections so re-running the generator keeps them.
# Format:  //-- begin impl [name] ---...
#              <your code>
#          //-- end impl [name] ---...

IMPL_RE     = re.compile(
    r'([ \t]*)//-- begin impl \[([^\]]+)\] -+\n(.*?)\1//-- end impl \[\2\] -+',
    re.DOTALL
)
IMPL_DASHES = '-' * 40

def extract_impl_blocks(path):
    if not os.path.isfile(path):
        return {}
    with open(path, encoding='utf-8') as f:
        content = f.read()
    # group(1)=indent, group(2)=name, group(3)=content
    return {m.group(2): m.group(3) for m in IMPL_RE.finditer(content)}

def inject_impl_blocks(rendered, preserved):
    if not preserved:
        return rendered
    def replacer(m):
        indent = m.group(1)
        name   = m.group(2)
        if name in preserved:
            return (
                f'{indent}//-- begin impl [{name}] {IMPL_DASHES}\n' +
                preserved[name] +
                f'{indent}//-- end impl [{name}] {IMPL_DASHES}'
            )
        return m.group(0)
    return IMPL_RE.sub(replacer, rendered)


# ── Config validator ──────────────────────────────────────────────────────────

_TOPIC_TYPES  = ("publisher", "subscriber", "lifecycle_publisher", "lifecycle_subscriber")
_SRV_TYPES    = ("service_server", "service_client")
_ACTION_TYPES = ("action_server", "action_client")
_LC_TYPES     = ("lifecycle_publisher", "lifecycle_subscriber")

def validate_config():
    errors = []

    for node_name in application.get('nodes', []):
        node_file = f"config/nodes/{node_name}.yaml"
        full_path = os.path.join(PROJECT_PATH, node_file)

        if not os.path.isfile(full_path):
            errors.append(f"Node '{node_name}': file missing at {node_file}")
            continue

        with open(full_path, encoding='utf-8') as f:
            node_yaml = yaml.safe_load(f).get('node', {})

        ntype = node_yaml.get('type', '')
        all_types = _TOPIC_TYPES + _SRV_TYPES + _ACTION_TYPES
        if ntype not in all_types:
            errors.append(
                f"Node '{node_name}': unknown type '{ntype}' "
                f"(valid: {all_types})"
            )
            continue

        if ntype in _TOPIC_TYPES:
            topic = node_yaml.get('topic', '')
            if not topic:
                errors.append(f"Node '{node_name}': missing 'topic' field")
            elif topic not in topics:
                errors.append(
                    f"Node '{node_name}': topic '{topic}' not found in topics.yaml "
                    f"(defined: {list(topics.keys())})"
                )

            qos_name = node_yaml.get('qos', '')
            if qos_name and qos_name not in qos_profiles:
                errors.append(
                    f"Node '{node_name}': qos '{qos_name}' not found in qos_profiles.yaml "
                    f"(defined: {list(qos_profiles.keys())})"
                )

        elif ntype in _SRV_TYPES:
            if not node_yaml.get('service'):
                errors.append(f"Node '{node_name}': missing 'service' field")
            if not node_yaml.get('srv_type'):
                errors.append(f"Node '{node_name}': missing 'srv_type' field")

        elif ntype in _ACTION_TYPES:
            if not node_yaml.get('action'):
                errors.append(f"Node '{node_name}': missing 'action' field")
            if not node_yaml.get('action_type'):
                errors.append(f"Node '{node_name}': missing 'action_type' field")

        param_group = node_yaml.get('parameters', '')
        if param_group and param_group not in parameters:
            errors.append(
                f"Node '{node_name}': parameter group '{param_group}' not found in parameters.yaml "
                f"(defined: {list(parameters.keys())})"
            )

    if errors:
        print(f"\nERROR: {len(errors)} config error(s):")
        for e in errors:
            print(f"  ✗ {e}")
        sys.exit(1)

    print("  Config OK")


# ── Type conversion helpers ───────────────────────────────────────────────────

def camel_to_snake(name):
    return re.sub(r'(?<!^)(?=[A-Z])', '_', name).lower()

def msg_to_cpp(msg):
    return msg.replace("/", "::")

def msg_to_include(msg):
    parts = msg.split("/")
    parts[-1] = camel_to_snake(parts[-1])
    return "/".join(parts) + ".hpp"

def srv_to_cpp(srv):
    return srv.replace("/", "::")

def srv_to_include(srv):
    parts = srv.split("/")
    parts[-1] = camel_to_snake(parts[-1])
    return "/".join(parts) + ".hpp"

def action_to_cpp(action):
    return action.replace("/", "::")

def action_to_include(action):
    parts = action.split("/")
    parts[-1] = camel_to_snake(parts[-1])
    return "/".join(parts) + ".hpp"


# ── Dependency collection ─────────────────────────────────────────────────────

def collect_deps(nodes):
    deps = {"rclcpp"}
    for node in nodes:
        ntype = node["type"]
        if node.get("message_type"):
            deps.add(node["message_type"].split("/")[0])
        if node.get("srv_type_raw"):
            deps.add(node["srv_type_raw"].split("/")[0])
        if node.get("action_type_raw"):
            deps.add(node["action_type_raw"].split("/")[0])
        if ntype in _ACTION_TYPES:
            deps.add("rclcpp_action")
        if ntype in _LC_TYPES:
            deps.add("rclcpp_lifecycle")
            deps.add("lifecycle_msgs")
    return sorted(deps)


# ── Node resolution ───────────────────────────────────────────────────────────

def resolve_node(node_name):
    node_yaml = load_yaml(f"config/nodes/{node_name}.yaml")["node"]
    node_type = node_yaml["type"]

    result = {
        "name":        node_name,
        "type":        node_type,
        "namespace":   node_yaml.get("namespace", ""),
        "param_group": node_yaml.get("parameters", ""),
        "parameters":  parameters.get(node_yaml.get("parameters", ""), {}),
    }

    if node_type in _TOPIC_TYPES:
        topic_name = node_yaml["topic"]
        topic_info = topics[topic_name]
        result.update({
            "topic":        topic_name,
            "message_type": topic_info["type"],
            "qos":          qos_profiles.get(node_yaml.get("qos", ""), {}),
            "qos_name":     node_yaml.get("qos", ""),
        })
    elif node_type in _SRV_TYPES:
        result.update({
            "service":      node_yaml["service"],
            "srv_type_raw": node_yaml["srv_type"],
        })
    elif node_type in _ACTION_TYPES:
        result.update({
            "action":          node_yaml["action"],
            "action_type_raw": node_yaml["action_type"],
        })

    return result


# ── Code generation ───────────────────────────────────────────────────────────

def generate_node(node):
    node_type   = node["type"]
    msg_type    = msg_include    = ""
    srv_type    = srv_include    = ""
    action_type = action_include = ""

    if node_type in _TOPIC_TYPES:
        msg_type    = msg_to_cpp(node["message_type"])
        msg_include = msg_to_include(node["message_type"])
        print(f"\n>> Node: {node['name']}  type={node_type}  topic={node.get('topic', '')}  ns='{node.get('namespace', '')}'")
    elif node_type in _SRV_TYPES:
        srv_type    = srv_to_cpp(node["srv_type_raw"])
        srv_include = srv_to_include(node["srv_type_raw"])
        print(f"\n>> Node: {node['name']}  type={node_type}  service={node.get('service', '')}  ns='{node.get('namespace', '')}'")
    elif node_type in _ACTION_TYPES:
        action_type    = action_to_cpp(node["action_type_raw"])
        action_include = action_to_include(node["action_type_raw"])
        print(f"\n>> Node: {node['name']}  type={node_type}  action={node.get('action', '')}  ns='{node.get('namespace', '')}'")

    ctx = dict(
        node=node,
        msg_type=msg_type,       msg_include=msg_include,
        srv_type=srv_type,       srv_include=srv_include,
        action_type=action_type, action_include=action_include,
    )

    cpp_path = out(f"generated_pkg/src/{node['name']}.cpp")
    hpp_path = out(f"generated_pkg/include/{node['name']}.hpp")

    existing_impls = extract_impl_blocks(cpp_path)

    rendered_cpp = env.get_template(f"{node_type}.cpp.jinja").render(**ctx)
    rendered_cpp = inject_impl_blocks(rendered_cpp, existing_impls)
    with open(cpp_path, "w", encoding="utf-8") as f:
        f.write(rendered_cpp)

    impl_note = f"  ({len(existing_impls)} impl blocks preserved)" if existing_impls else ""
    print(f"   -> {cpp_path}{impl_note}")

    rendered_hpp = env.get_template("node.hpp.jinja").render(**ctx)
    with open(hpp_path, "w", encoding="utf-8") as f:
        f.write(rendered_hpp)
    print(f"   -> {hpp_path}")


def generate_package(nodes):
    print("\n>> Package files")
    deps = collect_deps(nodes)

    cmake = env.get_template("CMakeLists.txt.jinja").render(nodes=nodes, deps=deps)
    with open(out("generated_pkg/CMakeLists.txt"), "w", encoding="utf-8") as f:
        f.write(cmake)

    package = env.get_template("package.xml.jinja").render(deps=deps)
    with open(out("generated_pkg/package.xml"), "w", encoding="utf-8") as f:
        f.write(package)

    print(f"   -> CMakeLists.txt  (deps: {', '.join(deps)})")
    print("   -> package.xml")


def generate_dockerfile():
    extra_packages = application.get("extra_packages", [])
    apt_packages   = application.get("apt_packages", [])
    template = env.get_template("Dockerfile.jinja")
    rendered = template.render(
        project=args.project,
        extra_packages=extra_packages,
        apt_packages=apt_packages,
    )
    dockerfile_path = os.path.join(TOOL_DIR, "Dockerfile")
    with open(dockerfile_path, "w", encoding="utf-8") as f:
        f.write(rendered)
    print(f"   -> {dockerfile_path}  (extra_packages: {extra_packages or 'none'}, apt_packages: {apt_packages or 'none'})")


def main():
    print(f"\n==> Generating project: {args.project}\n")

    validate_config()
    print()

    os.makedirs(out("generated_pkg/src"),     exist_ok=True)
    os.makedirs(out("generated_pkg/include"), exist_ok=True)

    resolved_nodes = []
    for node_name in application["nodes"]:
        node = resolve_node(node_name)
        resolved_nodes.append(node)
        generate_node(node)

    generate_package(resolved_nodes)

    print("\n>> Dockerfile")
    generate_dockerfile()

    print("\n===== GENERATION COMPLETE =====")
    print(f"Nodes  : {application['nodes']}")
    print(f"Output : {out('generated_pkg/')}")
    print("================================\n")


if __name__ == "__main__":
    main()
