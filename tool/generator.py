import os, sys, re, argparse, yaml
from jinja2 import Environment, FileSystemLoader, StrictUndefined

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
    undefined=StrictUndefined,
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

application   = load_yaml("config/application.yaml")["application"]
data          = load_yaml("config/data.yaml")["data"]
qos_profiles  = load_yaml("config/qos_profiles.yaml").get("qos_profiles") or {}
parameters    = load_yaml("config/parameters.yaml").get("parameters") or {}
disc_profiles = load_yaml("config/discovery_profiles.yaml").get("discovery_profiles") or {}

_disc_name = application.get("discovery_profile", "")
discovery  = disc_profiles.get(_disc_name, {})

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

def validate_config():
    errors = []

    disc_name = application.get('discovery_profile', '')
    if disc_name and disc_name not in disc_profiles:
        errors.append(
            f"application.discovery_profile '{disc_name}' not found in discovery_profiles.yaml "
            f"(defined: {list(disc_profiles.keys())})"
        )

    for node_name in application.get('nodes', []):
        contract_path = os.path.join(PROJECT_PATH, f"config/contracts/{node_name}.yaml")
        if not os.path.isfile(contract_path):
            errors.append(f"Node '{node_name}': contract file missing at config/contracts/{node_name}.yaml")
            continue

        with open(contract_path, encoding='utf-8') as f:
            raw = yaml.safe_load(f)
        contract = raw.get('contract', {})
        node_def  = contract.get('node', {})

        pref = node_def.get('parameter_ref', '')
        if pref and pref not in parameters:
            errors.append(
                f"Node '{node_name}': parameter_ref '{pref}' not found in parameters.yaml "
                f"(defined: {list(parameters.keys())})"
            )

        defined_cbgs = {cg['name'] for cg in contract.get('callback_groups', [])}

        for ix in contract.get('interactions', []):
            itype   = ix.get('type', '')
            role    = ix.get('role', '')
            dataref = ix.get('dataref', '')
            ix_id   = ix.get('name', f"{role}_{dataref}")

            if itype == 'pub-sub':
                if dataref not in data.get('messages', {}):
                    errors.append(
                        f"Node '{node_name}' / interaction '{ix_id}': "
                        f"dataref '{dataref}' not found in data.messages "
                        f"(defined: {list(data.get('messages', {}).keys())})"
                    )
            elif itype == 'service':
                if dataref not in data.get('services', {}):
                    errors.append(
                        f"Node '{node_name}' / interaction '{ix_id}': "
                        f"dataref '{dataref}' not found in data.services "
                        f"(defined: {list(data.get('services', {}).keys())})"
                    )

            qp = ix.get('qos_profile', '')
            if qp and qp not in qos_profiles:
                errors.append(
                    f"Node '{node_name}' / interaction '{ix_id}': "
                    f"qos_profile '{qp}' not found in qos_profiles.yaml "
                    f"(defined: {list(qos_profiles.keys())})"
                )

            cbg = ix.get('callback_group', '')
            if cbg and cbg not in defined_cbgs:
                errors.append(
                    f"Node '{node_name}' / interaction '{ix_id}': "
                    f"callback_group '{cbg}' not declared in callback_groups"
                )

        for timer in contract.get('timers', []):
            cbg = timer.get('callback_group', '')
            if cbg and cbg not in defined_cbgs:
                errors.append(
                    f"Node '{node_name}' / timer '{timer.get('name', '?')}': "
                    f"callback_group '{cbg}' not declared in callback_groups"
                )

    if errors:
        print(f"\nERROR: {len(errors)} config error(s):")
        for e in errors:
            print(f"  ✗ {e}")
        sys.exit(1)

    print("  Config OK")

# ── Type conversion helpers ───────────────────────────────────────────────────

def camel_to_snake(name):
    s = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s).lower()

def to_class_name(name):
    return ''.join(w.capitalize() for w in name.split('_'))

def type_to_cpp(ros_type):
    return ros_type.replace('/', '::')

def type_to_include(ros_type):
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
    raw      = load_yaml(f"config/contracts/{node_name}.yaml")["contract"]
    node_def = raw["node"]

    node = {
        'name':            node_def['name'],
        'class_name':      to_class_name(node_def['name']),
        'namespace':       node_def.get('namespace', ''),
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
        itype   = interaction['type']
        role    = interaction['role']
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

    servers   = discovery.get("servers", [{}])
    first_srv = servers[0] if servers else {}

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
    ctx      = {'node': node}
    hpp_path = out(f"generated_pkg/include/{node['name']}.hpp")
    cpp_path = out(f"generated_pkg/src/{node['name']}.cpp")

    existing_impls = extract_impl_blocks(cpp_path)

    hpp = env.get_template("node.hpp.jinja").render(**ctx)
    with open(hpp_path, 'w', encoding='utf-8') as f:
        f.write(hpp)

    cpp = env.get_template("node.cpp.jinja").render(**ctx)
    cpp = inject_impl_blocks(cpp, existing_impls)
    with open(cpp_path, 'w', encoding='utf-8') as f:
        f.write(cpp)

    roles = (
        f"{len(node['publishers'])}pub  "
        f"{len(node['subscribers'])}sub  "
        f"{len(node['service_servers'])}srv_server  "
        f"{len(node['service_clients'])}srv_client  "
        f"{len(node['timers'])}timer"
    )
    impl_note = f"  ({len(existing_impls)} impl blocks preserved)" if existing_impls else ""
    print(f"  OK {node['name']}  [{roles}]{impl_note}")

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

    validate_config()
    print()

    nodes = [resolve_node(n) for n in application['nodes']]
    print()
    for n in nodes:
        generate_node(n)
    generate_package(nodes)
    generate_dockerfile()

    print(f"\n==> Done. Output: {out('generated_pkg/')}\n")

if __name__ == "__main__":
    main()
