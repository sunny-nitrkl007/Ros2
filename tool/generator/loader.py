"""
loader.py — load, validate, and resolve ROS2 grammar YAML into Python dicts.
No Jinja2, no file writing. Pure data in, structured dicts out.
"""

import os, sys, yaml
from glob import glob


# ── YAML reader ───────────────────────────────────────────────────────────────

def _read(project_path, rel):
    full = os.path.join(project_path, rel)
    if not os.path.isfile(full):
        print(f"ERROR: required file missing: {full}")
        sys.exit(1)
    print(f"  loading {full}")
    with open(full, encoding='utf-8') as f:
        return yaml.safe_load(f)


# ── Helpers ───────────────────────────────────────────────────────────────────

def to_class_name(name):
    return ''.join(w.capitalize() for w in name.split('_'))

_PARAM_CPP = {
    'double':  ('double',    'as_double()'),
    'float64': ('double',    'as_double()'),
    'int':     ('int64_t',   'as_int()'),
    'int32':   ('int64_t',   'as_int()'),
    'bool':    ('bool',      'as_bool()'),
    'string':  ('std::string', 'as_string()'),
}

def _fmt_default(value, ptype):
    if ptype == 'bool':
        return 'true' if value else 'false'
    if ptype == 'string':
        return f'"{value}"'
    return str(value)

def collect_deps(nodes):
    deps = {'rclcpp'}
    for n in nodes:
        for entry in n['publishers'] + n['subscribers']:
            deps.add(entry['cpp_type'].split('::')[0])
        for entry in n['service_servers'] + n['service_clients']:
            deps.add(entry['cpp_type'].split('::')[0])
    return sorted(deps)


# ── QoS normaliser ────────────────────────────────────────────────────────────

def _norm_qos(q):
    return {
        'depth':       q.get('history_depth', 10),
        'reliability': 'reliable' if str(q.get('reliability', 'RELIABLE')).upper() == 'RELIABLE' else 'best_effort',
        'durability':  'transient_local' if str(q.get('durability', '')).upper() == 'TRANSIENT_LOCAL' else None,
        'deadline_ms': q.get('deadline_ms') or None,
        'lifespan_ms': q.get('lifespan_ms') or None,
    }


# ── Node resolver ─────────────────────────────────────────────────────────────

def _resolve_ros2_node(node_name, node_def, interaction_map, topic_map, service_map, qos_map, param_map):
    publishers      = []
    subscribers     = []
    service_servers = []
    service_clients = []

    for ref in node_def.get('interactions_ref', []):
        if ref not in interaction_map:
            print(f"ERROR: interactions_ref '{ref}' in node '{node_name}' not found in any segment")
            sys.exit(1)
        ix    = interaction_map[ref]
        itype = ix['_type']
        qos   = _norm_qos(qos_map.get(ix.get('qos_profile', ''), {}))

        if itype == 'publisher':
            t = topic_map[ix['topic_name']]
            publishers.append({
                'interface_name': ref,
                'topic':          t['topic_path'],
                'cpp_type':       f"adas_interfaces::msg::{to_class_name(ix['topic_name'])}",
                'member_name':    f"{ref}_pub_",
                'qos':            qos,
                'structure':      t.get('structure', []),
            })
        elif itype == 'subscriber':
            t = topic_map[ix['topic_name']]
            subscribers.append({
                'interface_name': ref,
                'topic':          t['topic_path'],
                'cpp_type':       f"adas_interfaces::msg::{to_class_name(ix['topic_name'])}",
                'member_name':    f"{ref}_sub_",
                'callback_name':  f"on_{ref}",
                'callback_group': '',
                'qos':            qos,
                'structure':      t.get('structure', []),
            })
        elif itype == 'service_server':
            s = service_map[ix['service_name']]
            service_servers.append({
                'interface_name':     ref,
                'service_name':       s['service_path'],
                'cpp_type':           f"adas_interfaces::srv::{to_class_name(ix['service_name'])}",
                'member_name':        f"{ref}_srv_",
                'callback_name':      f"on_{ref}",
                'callback_group':     '',
                'request_structure':  s.get('request_structure', []),
                'response_structure': s.get('response_structure', []),
            })
        elif itype == 'service_client':
            s = service_map[ix['service_name']]
            service_clients.append({
                'interface_name':    ref,
                'service_name':      s['service_path'],
                'cpp_type':          f"adas_interfaces::srv::{to_class_name(ix['service_name'])}",
                'member_name':       f"{ref}_client_",
                'request_structure': s.get('request_structure', []),
            })

    # Includes — one per unique topic/service name used
    all_includes = set()
    for ix_ref in node_def.get('interactions_ref', []):
        ix    = interaction_map[ix_ref]
        itype = ix['_type']
        if itype in ('publisher', 'subscriber'):
            all_includes.add(f"adas_interfaces/msg/{ix['topic_name']}.hpp")
        elif itype in ('service_server', 'service_client'):
            all_includes.add(f"adas_interfaces/srv/{ix['service_name']}.hpp")

    # Parameters
    params    = {}
    param_ref = node_def.get('parameter_ref', '')
    if param_ref:
        profile = param_map.get(param_ref)
        if not profile:
            print(f"ERROR: parameter_ref '{param_ref}' in node '{node_name}' not found in parameter_profiles.yaml")
            sys.exit(1)
        for p in profile.get('parameters', []):
            cpp_type, getter = _PARAM_CPP.get(p['type'], ('auto', 'get()'))
            params[p['name']] = {
                'cpp_type':     cpp_type,
                'default_expr': _fmt_default(p['default'], p['type']),
                'getter':       getter,
            }

    return {
        'name':            node_name,
        'class_name':      to_class_name(node_name),
        'namespace':       '',
        'executor':        {'type': 'single_threaded', 'threads': 1},
        'publishers':      publishers,
        'subscribers':     subscribers,
        'service_servers': service_servers,
        'service_clients': service_clients,
        'timers':          [],
        'parameters':      params,
        'callback_groups': [],
        'all_includes':    sorted(all_includes),
    }


# ── Public entry point ────────────────────────────────────────────────────────

def load_ros2_grammar(project_path):
    """
    Load ROS2 grammar project (general/ layout).
    Returns: { applications: [...], topics: [...], services: [...] }
    """
    def rd(rel):
        return _read(project_path, rel)

    apps_raw  = rd("general/apps/applications.yaml")
    topic_raw = rd("general/Interfaces_data/topics_data.yaml")
    srv_raw   = rd("general/Interfaces_data/services_data.yaml")
    qos_raw   = rd("general/profiles/qos_profiles.yaml")
    param_raw = rd("general/profiles/parameter_profiles.yaml")
    disc_raw  = rd("general/profiles/discovery_profiles.yaml")

    topic_map = {t['topic_name']: t for t in topic_raw['topic_data']}
    srv_map   = {s['service_name']: s for s in srv_raw['service_data']}
    qos_map   = {q['name']: q for q in qos_raw['qos_profiles']}
    param_map = {p['name']: p for p in param_raw['parameter_profiles']}
    disc_map  = {d['name']: d for d in disc_raw['discovery_profiles']}

    # Load all segment files
    interaction_map = {}
    node_def_map    = {}
    seg_dir = os.path.join(project_path, "general", "apps", "segments")
    for seg_file in sorted(glob(os.path.join(seg_dir, "*.yaml"))):
        print(f"  loading {seg_file}")
        with open(seg_file, encoding='utf-8') as f:
            data = yaml.safe_load(f)
        for seg in (data.get('segments') or []):
            for iblock in (seg.get('interactions') or []):
                for itype, ilist in iblock.items():
                    for ix in (ilist or []):
                        ix['_type'] = itype
                        interaction_map[ix['name']] = ix
        for node_def in (data.get('node') or []):
            node_def_map[node_def['name']] = node_def

    # Build per-application model
    applications = []
    for app in apps_raw['application']:
        disc  = disc_map.get(app.get('discovery_ref', ''), {})
        nodes = []
        for node_name in app.get('nodes', []):
            if node_name not in node_def_map:
                print(f"ERROR: node '{node_name}' listed in applications.yaml not found in any segment file")
                sys.exit(1)
            nodes.append(_resolve_ros2_node(
                node_name, node_def_map[node_name],
                interaction_map, topic_map, srv_map, qos_map, param_map,
            ))
        applications.append({
            'name':      app['name'],
            'discovery': disc,
            'nodes':     nodes,
        })

    print("  Config OK")
    return {
        'applications': applications,
        'topics':       list(topic_map.values()),
        'services':     list(srv_map.values()),
    }
