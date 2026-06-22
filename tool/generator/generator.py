import os, sys, re, argparse
from jinja2 import Environment, FileSystemLoader, StrictUndefined
import loader as config

TOOL_DIR       = os.path.dirname(os.path.abspath(__file__))  # tool/generator/
PARENT_DIR     = os.path.dirname(TOOL_DIR)                    # tool/
ROOT_DIR       = os.path.dirname(PARENT_DIR)                  # project root
TOOL_TEMPLATES = os.path.join(PARENT_DIR, "templates")

parser = argparse.ArgumentParser(description="ROS2 V3 code generator")
parser.add_argument("--project", required=True, help="Project folder name")
args = parser.parse_args()

PROJECT_PATH = os.path.join(ROOT_DIR, args.project)

if not os.path.isdir(PROJECT_PATH):
    print(f"ERROR: project not found: {PROJECT_PATH}")
    sys.exit(1)

env = Environment(
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

# ── File path helper ──────────────────────────────────────────────────────────

def out(rel_path):
    return os.path.join(PROJECT_PATH, rel_path)

# ── Node generation ───────────────────────────────────────────────────────────

def generate_node(node):
    hpp_path = out(f"generated_pkg/include/{node['name']}.hpp")
    cpp_path = out(f"generated_pkg/src/{node['name']}.cpp")

    existing_impls = extract_impl_blocks(cpp_path)

    hpp = env.get_template("node.hpp.jinja").render(node=node)
    with open(hpp_path, 'w', encoding='utf-8') as f:
        f.write(hpp)

    cpp = env.get_template("node.cpp.jinja").render(node=node)
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

# ── Package file generation ───────────────────────────────────────────────────

def generate_package(nodes):
    deps  = config.collect_deps(nodes)
    cmake = env.get_template("CMakeLists.txt.jinja").render(nodes=nodes, deps=deps)
    with open(out("generated_pkg/CMakeLists.txt"), 'w', encoding='utf-8') as f:
        f.write(cmake)

    pkg = env.get_template("package.xml.jinja").render(project=args.project, deps=deps)
    with open(out("generated_pkg/package.xml"), 'w', encoding='utf-8') as f:
        f.write(pkg)

    print(f"  OK CMakeLists.txt + package.xml  (deps: {', '.join(deps)})")

# ── Dockerfile generation ─────────────────────────────────────────────────────

def generate_dockerfile(application, discovery):
    servers   = discovery.get("servers", [{}])
    first_srv = servers[0] if servers else {}

    rendered = env.get_template("Dockerfile.jinja").render(
        project=args.project,
        extra_packages=application.get("extra_packages", []),
        apt_packages=application.get("apt_packages", []),
        domain_id=application.get("domain_id", 10),
        discovery_mode="server" if discovery else "",
        discovery_ip=first_srv.get("ip", "127.0.0.1"),
        discovery_port=first_srv.get("port", 11811),
    )
    dockerfile_path = os.path.join(PARENT_DIR, "Dockerfile")
    with open(dockerfile_path, "w", encoding="utf-8") as f:
        f.write(rendered)
    mode = discovery.get("mode", "CLIENT")
    print(f"   -> {dockerfile_path}  (mode: {mode}, DS: {first_srv.get('ip')}:{first_srv.get('port')})")

# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    print(f"\n==> Generating: {args.project}\n")
    os.makedirs(out("generated_pkg/src"),     exist_ok=True)
    os.makedirs(out("generated_pkg/include"), exist_ok=True)

    cfg = config.load(PROJECT_PATH)
    print()

    for node in cfg['nodes']:
        generate_node(node)
    generate_package(cfg['nodes'])
    generate_dockerfile(cfg['application'], cfg['discovery'])

    print(f"\n==> Done. Output: {out('generated_pkg/')}\n")

if __name__ == "__main__":
    main()
