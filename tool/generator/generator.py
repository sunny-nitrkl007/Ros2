import os, sys, re, argparse
from jinja2 import Environment, FileSystemLoader, StrictUndefined
import loader as config

TOOL_DIR       = os.path.dirname(os.path.abspath(__file__))  # tool/generator/
PARENT_DIR     = os.path.dirname(TOOL_DIR)                    # tool/
ROOT_DIR       = os.path.dirname(PARENT_DIR)                  # project root
TOOL_TEMPLATES = os.path.join(PARENT_DIR, "templates")

parser = argparse.ArgumentParser(description="ROS2 code generator")
parser.add_argument("--project", required=True, help="Project folder name under the repo root")
args = parser.parse_args()

PROJECT_PATH = os.path.join(ROOT_DIR, args.project)

if not os.path.isdir(PROJECT_PATH):
    print(f"ERROR: project not found: {PROJECT_PATH}")
    sys.exit(1)

jinja_env = Environment(
    loader=FileSystemLoader(TOOL_TEMPLATES),
    trim_blocks=True,
    lstrip_blocks=True,
    undefined=StrictUndefined,
)

# ── Impl block preservation ───────────────────────────────────────────────────

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

# ── Node generation ───────────────────────────────────────────────────────────

def generate_node(node, src_dir, inc_dir):
    hpp_path = os.path.join(inc_dir, f"{node['name']}.hpp")
    cpp_path = os.path.join(src_dir, f"{node['name']}.cpp")

    existing_impls = extract_impl_blocks(cpp_path)

    hpp = jinja_env.get_template("node.hpp.jinja").render(node=node)
    with open(hpp_path, 'w', encoding='utf-8') as f:
        f.write(hpp)

    cpp = jinja_env.get_template("node.cpp.jinja").render(node=node)
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
    print(f"    OK {node['name']}  [{roles}]{impl_note}")

# ── Package file generation ───────────────────────────────────────────────────

def generate_package(nodes, out_dir, project_name):
    deps  = config.collect_deps(nodes)
    cmake = jinja_env.get_template("CMakeLists.txt.jinja").render(
        nodes=nodes, deps=deps, project_name=project_name)
    with open(os.path.join(out_dir, "CMakeLists.txt"), 'w', encoding='utf-8') as f:
        f.write(cmake)

    pkg = jinja_env.get_template("package.xml.jinja").render(
        project=project_name, deps=deps, project_name=project_name)
    with open(os.path.join(out_dir, "package.xml"), 'w', encoding='utf-8') as f:
        f.write(pkg)

    print(f"    OK CMakeLists.txt + package.xml  (deps: {', '.join(deps)})")

# ── env.sh generation ─────────────────────────────────────────────────────────

def generate_env_sh(discovery, out_dir):
    servers   = discovery.get("discovery_servers", [{}])
    first_srv = servers[0] if servers else {}
    domain_id = discovery.get("domain_id", 0)
    ds_ip     = first_srv.get("ip", "127.0.0.1")
    ds_port   = first_srv.get("port", 11811)

    content = (
        "#!/bin/bash\n"
        f"export ROS_DOMAIN_ID={domain_id}\n"
        f"export ROS_DISCOVERY_SERVER={ds_ip}:{ds_port}\n"
    )
    with open(os.path.join(out_dir, "env.sh"), 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"    OK env.sh  (DS: {ds_ip}:{ds_port}, domain: {domain_id})")

# ── Main generation flow ──────────────────────────────────────────────────────

def main():
    print(f"\n==> Generating: {args.project}\n")

    cfg      = config.load_ros2_grammar(PROJECT_PATH)
    gen_root = os.path.join(PROJECT_PATH, "generated")

    # ── adas_interfaces package ───────────────────────────────────────────────
    iface_dir = os.path.join(gen_root, "adas_interfaces")
    os.makedirs(os.path.join(iface_dir, "msg"), exist_ok=True)
    os.makedirs(os.path.join(iface_dir, "srv"), exist_ok=True)

    print("\n  Interfaces (adas_interfaces)")

    msgs = []
    for topic in cfg['topics']:
        msg_name = config.to_class_name(topic['topic_name'])
        rendered = jinja_env.get_template("msg.jinja").render(fields=topic['structure'])
        with open(os.path.join(iface_dir, "msg", f"{msg_name}.msg"), 'w', encoding='utf-8') as f:
            f.write(rendered)
        print(f"    OK msg/{msg_name}.msg")
        msgs.append(msg_name)

    srvs = []
    for svc in cfg['services']:
        srv_name = config.to_class_name(svc['service_name'])
        rendered = jinja_env.get_template("srv.jinja").render(
            request_fields=svc['request_structure'],
            response_fields=svc['response_structure'],
        )
        with open(os.path.join(iface_dir, "srv", f"{srv_name}.srv"), 'w', encoding='utf-8') as f:
            f.write(rendered)
        print(f"    OK srv/{srv_name}.srv")
        srvs.append(srv_name)

    cmake_iface = jinja_env.get_template("interfaces_CMakeLists.txt.jinja").render(msgs=msgs, srvs=srvs)
    with open(os.path.join(iface_dir, "CMakeLists.txt"), 'w', encoding='utf-8') as f:
        f.write(cmake_iface)

    pkg_iface = jinja_env.get_template("interfaces_package.xml.jinja").render()
    with open(os.path.join(iface_dir, "package.xml"), 'w', encoding='utf-8') as f:
        f.write(pkg_iface)

    print(f"    OK CMakeLists.txt + package.xml")

    # ── Per-application packages ──────────────────────────────────────────────
    for app in cfg['applications']:
        app_dir = os.path.join(gen_root, app['name'])
        src_dir = os.path.join(app_dir, "src")
        inc_dir = os.path.join(app_dir, "include")
        os.makedirs(src_dir, exist_ok=True)
        os.makedirs(inc_dir, exist_ok=True)

        print(f"\n  Application: {app['name']}")

        for node in app['nodes']:
            generate_node(node, src_dir, inc_dir)

        generate_package(app['nodes'], app_dir, app['name'])
        generate_env_sh(app['discovery'], app_dir)

    print(f"\n==> Done. Output: {gen_root}/\n")

if __name__ == "__main__":
    main()
