import os
import sys
import argparse
import yaml
from jinja2 import Environment, FileSystemLoader

# tool/ lives inside discoveryTesting/
# project folder is a sibling of tool/
TOOL_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR  = os.path.dirname(TOOL_DIR)

parser = argparse.ArgumentParser(description="ROS2 code generator")
parser.add_argument("--project", required=True, help="Project folder name (sibling of tool/)")
args = parser.parse_args()

PROJECT_PATH  = os.path.join(ROOT_DIR, args.project)
TEMPLATE_PATH = os.path.join(PROJECT_PATH, "templates")

if not os.path.isdir(PROJECT_PATH):
    print(f"ERROR: project folder not found: {PROJECT_PATH}")
    sys.exit(1)

env = Environment(loader=FileSystemLoader(TEMPLATE_PATH))


def load_yaml(rel_path):
    full_path = os.path.join(PROJECT_PATH, rel_path)
    print(f"Loading: {full_path}")
    with open(full_path) as f:
        return yaml.safe_load(f)


def out(rel_path):
    return os.path.join(PROJECT_PATH, rel_path)


application  = load_yaml("config/application.yaml")["application"]
topics       = load_yaml("config/topics.yaml")["topics"]
qos_profiles = load_yaml("config/qos_profiles.yaml")["qos_profiles"]
parameters   = load_yaml("config/parameters.yaml")["parameters"]

print("\n===== CONFIG LOAD SUMMARY =====")
print(f"Project           : {args.project}")
print(f"Application name  : {application.get('name', 'N/A')}")
print(f"Domain ID         : {application.get('domain_id', 'N/A')}")
print(f"Nodes             : {application.get('nodes', [])}")
print(f"Topics            : {list(topics.keys())}")
print(f"QoS profiles      : {list(qos_profiles.keys())}")
print(f"Parameter groups  : {list(parameters.keys())}")
print("================================\n")


def resolve_node(node_name):
    node_yaml  = load_yaml(f"config/nodes/{node_name}.yaml")["node"]
    topic_name = node_yaml["topic"]
    topic_info = topics[topic_name]
    qos        = qos_profiles[node_yaml["qos"]]
    params     = parameters[node_yaml["parameters"]]
    return {
        "name":         node_name,
        "type":         node_yaml["type"],
        "topic":        topic_name,
        "message_type": topic_info["type"],
        "qos":          qos,
        "parameters":   params,
        "qos_name":     node_yaml["qos"],
        "param_group":  node_yaml["parameters"],
    }


def msg_to_cpp(msg):
    return msg.replace("/", "::")


def msg_to_include(msg):
    parts = msg.split("/")
    parts[-1] = parts[-1].lower()
    return "/".join(parts) + ".hpp"


def generate_node(node):
    print(f"\n>> Node: {node['name']}  type={node['type']}  topic={node['topic']}")

    msg_type    = msg_to_cpp(node["message_type"])
    msg_include = msg_to_include(node["message_type"])

    rendered_cpp = env.get_template(f"{node['type']}.cpp.jinja").render(
        node=node, msg_type=msg_type, msg_include=msg_include
    )
    cpp_path = out(f"generated_pkg/src/{node['name']}.cpp")
    with open(cpp_path, "w", encoding="utf-8") as f:
        f.write(rendered_cpp)
    print(f"   -> {cpp_path}")

    rendered_hpp = env.get_template("node.hpp.jinja").render(
        node=node, msg_type=msg_type, msg_include=msg_include
    )
    hpp_path = out(f"generated_pkg/include/{node['name']}.hpp")
    with open(hpp_path, "w", encoding="utf-8") as f:
        f.write(rendered_hpp)
    print(f"   -> {hpp_path}")


def generate_package(nodes):
    print("\n>> Package files")

    cmake = env.get_template("CMakeLists.txt.jinja").render(nodes=nodes)
    with open(out("generated_pkg/CMakeLists.txt"), "w", encoding="utf-8") as f:
        f.write(cmake)

    package = env.get_template("package.xml.jinja").render()
    with open(out("generated_pkg/package.xml"), "w", encoding="utf-8") as f:
        f.write(package)

    print("   -> CMakeLists.txt")
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

    print("\n===== GENERATION COMPLETE =====")
    print(f"Nodes  : {application['nodes']}")
    print(f"Output : {out('generated_pkg/')}")
    print("================================\n")


if __name__ == "__main__":
    main()
