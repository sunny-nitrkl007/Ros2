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

def load_yaml_optional(rel_path):
    full = os.path.join(PROJECT_PATH, rel_path)
    if not os.path.isfile(full):
        return None
    print(f"  loading {full}")
    with open(full, encoding="utf-8") as f:
        return yaml.safe_load(f)

def out(rel_path):
    return os.path.join(PROJECT_PATH, rel_path)

application      = load_yaml("config/application.yaml")["application"]
data             = load_yaml("config/data.yaml")["data"]
qos_profiles     = load_yaml("config/qos_profiles.yaml").get("qos_profiles") or {}
parameters       = load_yaml("config/parameters.yaml").get("parameters") or {}
disc_profiles    = load_yaml("config/discovery_profiles.yaml").get("discovery_profiles") or {}

# Resolve project-wide discovery profile from application.yaml
_disc_name  = application.get("discovery_profile", "")
discovery   = disc_profiles.get(_disc_name, {})

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
    'float64':      ('double',                   'as_double()'),
    'int':          ('int64_t',                  'as_int()'),
    'int32':        ('int64_t',                  'as_int()'),
    'bool':         ('bool',                     'as_bool()'),
    'string':       ('std::string',              'as_string()'),
    'string_array': ('std::vector<std::string>', 'as_string_array()'),
    'double_array': ('std::vector<double>',      'as_double_array()'),
    'int_array':    ('std::vector<int64_t>',     'as_integer_array()'),
}

def format_cpp_default(value, ptype):
    if ptype in ('string',):
        return f'"{value}"'
    if ptype == 'string_array':
        return '{' + ', '.join(f'"{v}"' for v in value) + '}'
    if ptype in ('double_array', 'int_array'):
        return '{' + ', '.join(str(v) for v in value) + '}'
    if ptype == 'bool':
        return 'true' if value else 'false'
    return str(value)

def resolve_params(ref_name):
    result = {}
    group = parameters.get(ref_name) or {}
    for pname, pdata in group.items():
        ptype = pdata['type']
        cpp_type, getter = PARAM_CPP.get(ptype, ('auto', 'get()'))
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

# ── Node resolver — reads contract file ──────────────────────────────────────

def resolve_node(node_name):
    raw     = load_yaml(f"config/contracts/{node_name}.yaml")["contract"]
    node_def = raw["node"]

    node = {
        'name':            node_def['name'],
        'class_name':      to_class_name(node_def['name']),
        'executor':        node_def.get('executor', {'type': 'single_threaded', 'threads': 1}),
        'publishers':      [],
        'subscribers':     [],
        'service_servers': [],
        'service_clients': [],
        'timers':          raw.get('timers', []),
        'parameters':      resolve_params(node_def.get('parameter_ref', '')),
        'all_includes':    set(),
    }

    node['callback_groups'] = [
        {**cg, 'cbg_type': CBG_TYPE.get(cg['type'], 'rclcpp::CallbackGroupType::MutuallyExclusive')}
        for cg in raw.get('callback_groups', [])
    ]

    for interaction in raw.get('interactions', []):
        itype   = interaction['type']    # 'pub-sub' or 'service'
        role    = interaction['role']    # 'producer', 'consumer', 'server', 'client'
        dataref = interaction['dataref']
        qos     = qos_profiles.get(interaction.get('qos_profile', 'reliable_qos'), {})
        cbg     = interaction.get('callback_group', '')

        if itype == 'pub-sub' and role == 'producer':
            entry  = data['messages'][dataref]
            cpp_t  = type_to_cpp(entry['type'])
            inc    = type_to_include(entry['type'])
            node['all_includes'].add(inc)
            node['publishers'].append({
                'interface_name': dataref,
                'topic':          entry['topic_path'],
                'cpp_type':       cpp_t,
                'qos':            qos,
                'member_name':    f"{dataref}_pub_",
            })

        elif itype == 'pub-sub' and role == 'consumer':
            entry  = data['messages'][dataref]
            cpp_t  = type_to_cpp(entry['type'])
            inc    = type_to_include(entry['type'])
            node['all_includes'].add(inc)
            node['subscribers'].append({
                'interface_name': dataref,
                'topic':          entry['topic_path'],
                'cpp_type':       cpp_t,
                'qos':            qos,
                'callback_group': cbg,
                'member_name':    f"{dataref}_sub_",
                'callback_name':  f"on_{dataref}",
            })

        elif itype == 'service' and role == 'server':
            entry  = data['services'][dataref]
            cpp_t  = type_to_cpp(entry['type'])
            inc    = type_to_include(entry['type'])
            node['all_includes'].add(inc)
            node['service_servers'].append({
                'interface_name': dataref,
                'service_name':   entry['service_path'],
                'cpp_type':       cpp_t,
                'callback_group': cbg,
                'member_name':    f"{dataref}_srv_",
                'callback_name':  f"on_{dataref}",
            })

        elif itype == 'service' and role == 'client':
            entry  = data['services'][dataref]
            cpp_t  = type_to_cpp(entry['type'])
            inc    = type_to_include(entry['type'])
            node['all_includes'].add(inc)
            node['service_clients'].append({
                'interface_name': dataref,
                'service_name':   entry['service_path'],
                'cpp_type':       cpp_t,
                'member_name':    f"{dataref}_client_",
            })

    node['all_includes'] = sorted(node['all_includes'])
    return node

# ── Dockerfile generation ─────────────────────────────────────────────────────

def generate_dockerfile():
    extra_packages = application.get("extra_packages", [])
    apt_packages   = application.get("apt_packages", [])

    servers    = discovery.get("servers", [{}])
    first_srv  = servers[0] if servers else {}

    template = env.get_template("Dockerfile.jinja")
    rendered = template.render(
        project=args.project,
        extra_packages=extra_packages,
        apt_packages=apt_packages,
        domain_id=application.get("domain_id", 10),
        discovery_mode="server" if discovery else "",
        discovery_ip=first_srv.get("ip", "127.0.0.1"),
        discovery_port=first_srv.get("port", 11811),
    )
    dockerfile_path = os.path.join(TOOL_DIR, "Dockerfile")
    with open(dockerfile_path, "w", encoding="utf-8") as f:
        f.write(rendered)
    mode = discovery.get("mode", "CLIENT")
    print(f"   -> {dockerfile_path}  (mode: {mode}, DS: {first_srv.get('ip')}:{first_srv.get('port')})")

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
