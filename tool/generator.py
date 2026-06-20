import os
import sys
import argparse
import yaml
from jinja2 import Environment, FileSystemLoader

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

env = Environment(loader=FileSystemLoader(template_dirs))


def load_yaml(rel_path):
    full_path = os.path.join(PROJECT_PATH, rel_path)
    print(f"Loading: {full_path}")
    with open(full_path, encoding="utf-8") as f:
        return yaml.safe_load(f)


def out(rel_path):
    return os.path.join(PROJECT_PATH, rel_path)


application  = load_yaml("config/application.yaml")["application"]
discovery    = load_yaml("config/discovery.yaml").get("discovery") or {}
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


# ---------- type conversion helpers ----------

def msg_to_cpp(msg):
    return msg.replace("/", "::")

def msg_to_include(msg):
    parts = msg.split("/")
    parts[-1] = parts[-1].lower()
    return "/".join(parts) + ".hpp"

def srv_to_cpp(srv):
    return srv.replace("/", "::")

def srv_to_include(srv):
    parts = srv.split("/")
    parts[-1] = parts[-1].lower()
    return "/".join(parts) + ".hpp"

def action_to_cpp(action):
    return action.replace("/", "::")

def action_to_include(action):
    parts = action.split("/")
    parts[-1] = parts[-1].lower()
    return "/".join(parts) + ".hpp"


# ---------- dependency collection ----------

_TOPIC_TYPES  = ("publisher", "subscriber", "lifecycle_publisher", "lifecycle_subscriber")
_SRV_TYPES    = ("service_server", "service_client")
_ACTION_TYPES = ("action_server", "action_client")
_LC_TYPES     = ("lifecycle_publisher", "lifecycle_subscriber")


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


# ---------- node resolution ----------

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


# ---------- code generation ----------

def generate_node(node):
    node_type      = node["type"]
    msg_type       = msg_include    = ""
    srv_type       = srv_include    = ""
    action_type    = action_include = ""

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

    rendered_cpp = env.get_template(f"{node_type}.cpp.jinja").render(**ctx)
    cpp_path = out(f"generated_pkg/src/{node['name']}.cpp")
    with open(cpp_path, "w", encoding="utf-8") as f:
        f.write(rendered_cpp)
    print(f"   -> {cpp_path}")

    rendered_hpp = env.get_template("node.hpp.jinja").render(**ctx)
    hpp_path = out(f"generated_pkg/include/{node['name']}.hpp")
    with open(hpp_path, "w", encoding="utf-8") as f:
        f.write(rendered_hpp)
    print(f"   -> {hpp_path}")


def generate_env():
    domain_id = application.get("domain_id", 0)
    lines = [f"export ROS_DOMAIN_ID={domain_id}"]

    if discovery.get("mode") == "server":
        ip   = discovery.get("server_ip",   "127.0.0.1")
        port = discovery.get("server_port",  11811)
        lines.append(f"export ROS_DISCOVERY_SERVER={ip}:{port}")

    content = "\n".join(lines) + "\n"
    # written to tool/ so Dockerfile can reference one common path
    env_path = os.path.join(TOOL_DIR, "env.sh")
    with open(env_path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"   -> env.sh  (domain_id={domain_id}"
          + (f", discovery={ip}:{port}" if discovery.get('mode') == 'server' else "") + ")")


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


def main():
    print(f"\n==> Generating project: {args.project}\n")

    os.makedirs(out("generated_pkg/src"),     exist_ok=True)
    os.makedirs(out("generated_pkg/include"), exist_ok=True)

    resolved_nodes = []
    for node_name in application["nodes"]:
        node = resolve_node(node_name)
        resolved_nodes.append(node)
        generate_node(node)

    generate_package(resolved_nodes)
    generate_env()

    print("\n===== GENERATION COMPLETE =====")
    print(f"Nodes  : {application['nodes']}")
    print(f"Output : {out('generated_pkg/')}")
    print("================================\n")


if __name__ == "__main__":
    main()
