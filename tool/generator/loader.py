"""
loader.py — load, validate, and resolve all YAML config into Python dicts.
No Jinja2, no file writing. Pure data in, structured dicts out.
"""

import os, sys, re, yaml

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

_PARAM_CPP = {
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

def _fmt_default(value, ptype):
    if ptype == 'string':
        return f'"{value}"'
    if ptype == 'string_array':
        return '{' + ', '.join(f'"{v}"' for v in value) + '}'
    if ptype in ('double_array', 'int_array'):
        return '{' + ', '.join(str(v) for v in value) + '}'
    if ptype == 'bool':
        return 'true' if value else 'false'
    return str(value)

def _resolve_params(parameters, ref_name):
    result = {}
    for pname, pdata in (parameters.get(ref_name) or {}).items():
        ptype = pdata['type']
        cpp_type, getter = _PARAM_CPP.get(ptype, ('auto', 'get()'))
        result[pname] = {
            'cpp_type':     cpp_type,
            'default_expr': _fmt_default(pdata['value'], ptype),
            'getter':       getter,
        }
    return result

# ── Callback group mapping ────────────────────────────────────────────────────

_CBG_TYPE = {
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

# ── Validator ─────────────────────────────────────────────────────────────────

def _validate(project_path, application, data, qos_profiles, parameters, disc_profiles):
    errors = []

    disc_name = application.get('discovery_profile', '')
    if disc_name and disc_name not in disc_profiles:
        errors.append(
            f"application.discovery_profile '{disc_name}' not found in discovery_profiles.yaml "
            f"(defined: {list(disc_profiles.keys())})"
        )

    for node_name in application.get('nodes', []):
        contract_path = os.path.join(project_path, f"config/contracts/{node_name}.yaml")
        if not os.path.isfile(contract_path):
            errors.append(f"Node '{node_name}': contract missing at config/contracts/{node_name}.yaml")
            continue

        with open(contract_path, encoding='utf-8') as f:
            raw = yaml.safe_load(f)
        contract = raw.get('contract', {})
        node_def  = contract.get('node', {})

        pref = node_def.get('parameter_ref', '')
        if pref and pref not in parameters:
            errors.append(
                f"Node '{node_name}': parameter_ref '{pref}' not in parameters.yaml "
                f"(defined: {list(parameters.keys())})"
            )

        defined_cbgs = {cg['name'] for cg in contract.get('callback_groups', [])}

        for ix in contract.get('interactions', []):
            itype   = ix.get('type', '')
            role    = ix.get('role', '')
            dataref = ix.get('dataref', '')
            ix_id   = ix.get('name', f"{role}_{dataref}")

            bucket = 'messages' if itype == 'pub-sub' else 'services' if itype == 'service' else None
            if bucket and dataref not in data.get(bucket, {}):
                errors.append(
                    f"Node '{node_name}' / '{ix_id}': dataref '{dataref}' "
                    f"not found in data.{bucket} (defined: {list(data.get(bucket, {}).keys())})"
                )

            qp = ix.get('qos_profile', '')
            if qp and qp not in qos_profiles:
                errors.append(
                    f"Node '{node_name}' / '{ix_id}': qos_profile '{qp}' "
                    f"not in qos_profiles.yaml (defined: {list(qos_profiles.keys())})"
                )

            cbg = ix.get('callback_group', '')
            if cbg and cbg not in defined_cbgs:
                errors.append(
                    f"Node '{node_name}' / '{ix_id}': "
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

# ── Node resolver ─────────────────────────────────────────────────────────────

def _resolve_node(project_path, node_name, data, qos_profiles, parameters):
    contract_path = os.path.join(project_path, f"config/contracts/{node_name}.yaml")
    print(f"  loading {contract_path}")
    with open(contract_path, encoding='utf-8') as f:
        raw = yaml.safe_load(f)['contract']
    node_def = raw['node']

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
        'parameters':      _resolve_params(parameters, node_def.get('parameter_ref', '')),
        'all_includes':    set(),
    }

    node['callback_groups'] = [
        {**cg, 'cbg_type': _CBG_TYPE.get(cg['type'], 'rclcpp::CallbackGroupType::MutuallyExclusive')}
        for cg in raw.get('callback_groups', [])
    ]

    for ix in raw.get('interactions', []):
        itype   = ix['type']
        role    = ix['role']
        dataref = ix['dataref']
        qos     = qos_profiles.get(ix.get('qos_profile', 'reliable_qos'), {})
        cbg     = ix.get('callback_group', '')

        if itype == 'pub-sub' and role == 'producer':
            entry = data['messages'][dataref]
            cpp_t = type_to_cpp(entry['type'])
            node['all_includes'].add(type_to_include(entry['type']))
            node['publishers'].append({
                'interface_name': dataref,
                'topic':          entry['topic_path'],
                'cpp_type':       cpp_t,
                'qos':            qos,
                'member_name':    f"{dataref}_pub_",
            })
        elif itype == 'pub-sub' and role == 'consumer':
            entry = data['messages'][dataref]
            cpp_t = type_to_cpp(entry['type'])
            node['all_includes'].add(type_to_include(entry['type']))
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
            entry = data['services'][dataref]
            cpp_t = type_to_cpp(entry['type'])
            node['all_includes'].add(type_to_include(entry['type']))
            node['service_servers'].append({
                'interface_name': dataref,
                'service_name':   entry['service_path'],
                'cpp_type':       cpp_t,
                'callback_group': cbg,
                'member_name':    f"{dataref}_srv_",
                'callback_name':  f"on_{dataref}",
            })
        elif itype == 'service' and role == 'client':
            entry = data['services'][dataref]
            cpp_t = type_to_cpp(entry['type'])
            node['all_includes'].add(type_to_include(entry['type']))
            node['service_clients'].append({
                'interface_name': dataref,
                'service_name':   entry['service_path'],
                'cpp_type':       cpp_t,
                'member_name':    f"{dataref}_client_",
            })

    node['all_includes'] = sorted(node['all_includes'])
    return node

# ── Public entry point ────────────────────────────────────────────────────────

def load(project_path):
    """
    Load all YAML configs, validate all cross-references, resolve contracts to node dicts.
    Returns dict with keys: application, nodes, discovery.
    Calls sys.exit(1) on any config error.
    """
    application   = _read(project_path, "config/application.yaml")["application"]
    data          = _read(project_path, "config/data.yaml")["data"]
    qos_profiles  = _read(project_path, "config/qos_profiles.yaml").get("qos_profiles") or {}
    parameters    = _read(project_path, "config/parameters.yaml").get("parameters") or {}
    disc_profiles = _read(project_path, "config/discovery_profiles.yaml").get("discovery_profiles") or {}

    discovery = disc_profiles.get(application.get("discovery_profile", ""), {})

    _validate(project_path, application, data, qos_profiles, parameters, disc_profiles)

    nodes = [
        _resolve_node(project_path, name, data, qos_profiles, parameters)
        for name in application['nodes']
    ]

    return {
        'application': application,
        'nodes':       nodes,
        'discovery':   discovery,
    }
