import os, sys, re, argparse, yaml
from jinja2 import Environment, FileSystemLoader

TOOL_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR  = os.path.dirname(TOOL_DIR)

parser = argparse.ArgumentParser(description="ROS2 V3 code generator")
parser.add_argument("--project", required=True, help="Project folder name")
args = parser.parse_args()

PROJECT_PATH   = os.path.join(ROOT_DIR, args.project)
TOOL_TEMPLATES = os.path.join(TOOL_DIR, "templates")

if not os.path.isdir(PROJECT_PATH):
    print(f"ERROR: project not found: {PROJECT_PATH}")
    sys.exit(1)

env = Environment(
    loader=FileSystemLoader(TOOL_TEMPLATES),
    trim_blocks=True,
    lstrip_blocks=True,
)

# ── YAML loaders ──────────────────────────────────────────────────────────────

def load_yaml(rel_path):
    full = os.path.join(PROJECT_PATH, rel_path)
    print(f"  loading {full}")
    with open(full, encoding="utf-8") as f:
        return yaml.safe_load(f)

def out(rel_path):
    return os.path.join(PROJECT_PATH, rel_path)

application  = load_yaml("config/application.yaml")["application"]
interfaces   = load_yaml("config/interfaces.yaml")["interfaces"]
qos_profiles = load_yaml("config/qos_profiles.yaml").get("qos_profiles") or {}
parameters   = load_yaml("config/parameters.yaml").get("parameters") or {}

_disc_raw  = {}
_disc_file = os.path.join(PROJECT_PATH, "config/discovery.yaml")
if os.path.isfile(_disc_file):
    with open(_disc_file, encoding="utf-8") as _f:
        _disc_raw = yaml.safe_load(_f).get("discovery", {})
    print(f"  loading {_disc_file}")
discovery = _disc_raw

# ── Type conversion helpers ───────────────────────────────────────────────────

def camel_to_snake(name):
    s = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s).lower()

def to_class_name(name):
    return ''.join(w.capitalize() for w in name.split('_'))

def type_to_cpp(ros_type):
    # sensor_msgs/msg/JointState  ->  sensor_msgs::msg::JointState
    return ros_type.replace('/', '::')

def type_to_include(ros_type):
    # sensor_msgs/msg/JointState  ->  sensor_msgs/msg/joint_state.hpp
    parts = ros_type.split('/')
    parts[-1] = camel_to_snake(parts[-1])
    return '/'.join(parts) + '.hpp'

# ── Parameter helpers ─────────────────────────────────────────────────────────

PARAM_CPP = {
    'double':       ('double',                   'as_double()'),
    'int':          ('int64_t',                  'as_int()'),
    'bool':         ('bool',                     'as_bool()'),
    'string':       ('std::string',              'as_string()'),
    'string_array': ('std::vector<std::string>', 'as_string_array()'),
    'double_array': ('std::vector<double>',      'as_double_array()'),
    'int_array':    ('std::vector<int64_t>',     'as_integer_array()'),
}

def format_cpp_default(value, ptype):
    if ptype == 'string':
        return f'"{value}"'
    if ptype == 'string_array':
        return '{' + ', '.join(f'"{v}"' for v in value) + '}'
    if ptype in ('double_array', 'int_array'):
        return '{' + ', '.join(str(v) for v in value) + '}'
    if ptype == 'bool':
        return 'true' if value else 'false'
    return str(value)

def resolve_params(group_name):
    result = {}
    for pname, pdata in (parameters.get(group_name) or {}).items():
        ptype = pdata['type']
        cpp_type, getter = PARAM_CPP[ptype]
        result[pname] = {
            'cpp_type':     cpp_type,
            'default_expr': format_cpp_default(pdata['value'], ptype),
            'getter':       getter,
        }
    return result

# ── Callback group helpers ────────────────────────────────────────────────────

CBG_TYPE = {
    'mutually_exclusive': 'rclcpp::CallbackGroupType::MutuallyExclusive',
    'reentrant':          'rclcpp::CallbackGroupType::Reentrant',
}

# ── Dependency collection ─────────────────────────────────────────────────────

def collect_deps(nodes):
    deps = {'rclcpp'}
    for n in nodes:
        for entry in n['publishers'] + n['subscribers']:
            deps.add(entry['cpp_type'].split('::')[0])
        for entry in n['service_servers'] + n['service_clients']:
            deps.add(entry['cpp_type'].split('::')[0])
    return sorted(deps)

# ── Node resolver ─────────────────────────────────────────────────────────────

def resolve_node(node_name):
    raw = load_yaml(f"config/nodes/{node_name}.yaml")['node']

    node = {
        'name':            node_name,
        'class_name':      to_class_name(node_name),
        'namespace':       raw.get('namespace', ''),
        'executor':        raw.get('executor', {'type': 'single_threaded', 'threads': 1}),
        'publishers':      [],
        'subscribers':     [],
        'service_servers': [],
        'service_clients': [],
        'timers':          raw.get('timers', []),
        'parameters':      resolve_params(raw.get('parameters', '')),
        'all_includes':    set(),
    }

    node['callback_groups'] = [
        {**cg, 'cbg_type': CBG_TYPE.get(cg['type'], 'rclcpp::CallbackGroupType::MutuallyExclusive')}
        for cg in raw.get('callback_groups', [])
    ]

    for pub in raw.get('publishers', []):
        iface = interfaces['messages'][pub['interface']]
        qos   = qos_profiles.get(pub.get('qos', 'reliable_qos'), {})
        cpp_t = type_to_cpp(iface['type'])
        inc   = type_to_include(iface['type'])
        node['all_includes'].add(inc)
        node['publishers'].append({
            'interface_name': pub['interface'],
            'topic':          pub.get('topic', iface['default_topic']),
            'cpp_type':       cpp_t,
            'qos':            qos,
            'member_name':    f"{pub['interface']}_pub_",
        })

    for sub in raw.get('subscribers', []):
        iface = interfaces['messages'][sub['interface']]
        qos   = qos_profiles.get(sub.get('qos', 'reliable_qos'), {})
        cpp_t = type_to_cpp(iface['type'])
        inc   = type_to_include(iface['type'])
        node['all_includes'].add(inc)
        node['subscribers'].append({
            'interface_name': sub['interface'],
            'topic':          sub.get('topic', iface['default_topic']),
            'cpp_type':       cpp_t,
            'qos':            qos,
            'callback_group': sub.get('callback_group', ''),
            'member_name':    f"{sub['interface']}_sub_",
            'callback_name':  f"on_{sub['interface']}",
        })

    for srv in raw.get('service_servers', []):
        iface = interfaces['services'][srv['interface']]
        cpp_t = type_to_cpp(iface['type'])
        inc   = type_to_include(iface['type'])
        node['all_includes'].add(inc)
        node['service_servers'].append({
            'interface_name': srv['interface'],
            'service_name':   srv.get('service', iface['default_name']),
            'cpp_type':       cpp_t,
            'callback_group': srv.get('callback_group', ''),
            'member_name':    f"{srv['interface']}_srv_",
            'callback_name':  f"on_{srv['interface']}",
        })

    for cli in raw.get('service_clients', []):
        iface = interfaces['services'][cli['interface']]
        cpp_t = type_to_cpp(iface['type'])
        inc   = type_to_include(iface['type'])
        node['all_includes'].add(inc)
        node['service_clients'].append({
            'interface_name': cli['interface'],
            'service_name':   cli.get('service', iface['default_name']),
            'cpp_type':       cpp_t,
            'member_name':    f"{cli['interface']}_client_",
        })

    node['all_includes'] = sorted(node['all_includes'])
    return node

# ── Dockerfile generation ─────────────────────────────────────────────────────

def generate_dockerfile():
    extra_packages = application.get("extra_packages", [])
    apt_packages   = application.get("apt_packages", [])
    template = env.get_template("Dockerfile.jinja")
    rendered = template.render(
        project=args.project,
        extra_packages=extra_packages,
        apt_packages=apt_packages,
        domain_id=application.get("domain_id", 10),
        discovery_mode=discovery.get("mode", ""),
        discovery_ip=discovery.get("server_ip", "127.0.0.1"),
        discovery_port=discovery.get("server_port", 11811),
    )
    dockerfile_path = os.path.join(TOOL_DIR, "Dockerfile")
    with open(dockerfile_path, "w", encoding="utf-8") as f:
        f.write(rendered)
    print(f"   -> {dockerfile_path}  (apt_packages: {apt_packages or 'none'}, DS: {discovery.get('mode', 'none')})")

# ── Code generation ───────────────────────────────────────────────────────────

def generate_node(node):
    ctx = {'node': node}

    hpp = env.get_template("node.hpp.jinja").render(**ctx)
    with open(out(f"generated_pkg/include/{node['name']}.hpp"), 'w', encoding='utf-8') as f:
        f.write(hpp)

    cpp = env.get_template("node.cpp.jinja").render(**ctx)
    with open(out(f"generated_pkg/src/{node['name']}.cpp"), 'w', encoding='utf-8') as f:
        f.write(cpp)

    roles = (
        f"{len(node['publishers'])}pub  "
        f"{len(node['subscribers'])}sub  "
        f"{len(node['service_servers'])}srv_server  "
        f"{len(node['service_clients'])}srv_client  "
        f"{len(node['timers'])}timer"
    )
    print(f"  OK {node['name']}  [{roles}]")

def generate_package(nodes):
    deps = collect_deps(nodes)

    cmake = env.get_template("CMakeLists.txt.jinja").render(nodes=nodes, deps=deps)
    with open(out("generated_pkg/CMakeLists.txt"), 'w', encoding='utf-8') as f:
        f.write(cmake)

    pkg = env.get_template("package.xml.jinja").render(
        project=args.project, deps=deps)
    with open(out("generated_pkg/package.xml"), 'w', encoding='utf-8') as f:
        f.write(pkg)

    print(f"  OK CMakeLists.txt + package.xml  (deps: {', '.join(deps)})")

def main():
    print(f"\n==> Generating: {args.project}\n")
    os.makedirs(out("generated_pkg/src"),     exist_ok=True)
    os.makedirs(out("generated_pkg/include"), exist_ok=True)

    nodes = [resolve_node(n) for n in application['nodes']]
    print()
    for n in nodes:
        generate_node(n)
    generate_package(nodes)
    generate_dockerfile()

    print(f"\n==> Done. Output: {out('generated_pkg/')}\n")

if __name__ == "__main__":
    main()
