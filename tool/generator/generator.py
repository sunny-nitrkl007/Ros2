import os, sys, re, argparse
from jinja2 import Environment, FileSystemLoader, StrictUndefined
import loader as config

TOOL_DIR       = os.path.dirname(os.path.abspath(__file__))  # tool/generator/
PARENT_DIR     = os.path.dirname(TOOL_DIR)                    # tool/
ROOT_DIR       = os.path.dirname(PARENT_DIR)                  # project root
TOOL_TEMPLATES = os.path.join(PARENT_DIR, "templates")

parser = argparse.ArgumentParser(description="ROS2 V2 code generator")
parser.add_argument("--project", required=True, help="Project folder name (sibling of tool/)")
args = parser.parse_args()

PROJECT_PATH      = os.path.join(ROOT_DIR, args.project)
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

# ── Type conversion helpers ───────────────────────────────────────────────────

def _camel_to_snake(name):
    return re.sub(r'(?<!^)(?=[A-Z])', '_', name).lower()

def _to_cpp(ros_type):
    return ros_type.replace('/', '::')

def _to_include(ros_type):
    parts = ros_type.split('/')
    parts[-1] = _camel_to_snake(parts[-1])
    return '/'.join(parts) + '.hpp'

# ── File path helper ──────────────────────────────────────────────────────────

def out(rel_path):
    return os.path.join(PROJECT_PATH, rel_path)

# ── Node generation ───────────────────────────────────────────────────────────

def generate_node(node):
    node_type   = node['type']
    msg_type    = msg_include    = ''
    srv_type    = srv_include    = ''
    action_type = action_include = ''

    if node_type in config.TOPIC_TYPES:
        msg_type    = _to_cpp(node['message_type'])
        msg_include = _to_include(node['message_type'])
        print(f"\n>> Node: {node['name']}  type={node_type}  topic={node.get('topic', '')}  ns='{node.get('namespace', '')}'")
    elif node_type in config.SRV_TYPES:
        srv_type    = _to_cpp(node['srv_type_raw'])
        srv_include = _to_include(node['srv_type_raw'])
        print(f"\n>> Node: {node['name']}  type={node_type}  service={node.get('service', '')}  ns='{node.get('namespace', '')}'")
    elif node_type in config.ACTION_TYPES:
        action_type    = _to_cpp(node['action_type_raw'])
        action_include = _to_include(node['action_type_raw'])
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
    with open(cpp_path, 'w', encoding='utf-8') as f:
        f.write(rendered_cpp)

    impl_note = f"  ({len(existing_impls)} impl blocks preserved)" if existing_impls else ""
    print(f"   -> {cpp_path}{impl_note}")

    rendered_hpp = env.get_template("node.hpp.jinja").render(**ctx)
    with open(hpp_path, 'w', encoding='utf-8') as f:
        f.write(rendered_hpp)
    print(f"   -> {hpp_path}")

# ── Package file generation ───────────────────────────────────────────────────

def generate_package(nodes):
    print("\n>> Package files")
    deps = config.collect_deps(nodes)

    cmake = env.get_template("CMakeLists.txt.jinja").render(nodes=nodes, deps=deps)
    with open(out("generated_pkg/CMakeLists.txt"), 'w', encoding='utf-8') as f:
        f.write(cmake)

    package = env.get_template("package.xml.jinja").render(deps=deps)
    with open(out("generated_pkg/package.xml"), 'w', encoding='utf-8') as f:
        f.write(package)

    print(f"   -> CMakeLists.txt  (deps: {', '.join(deps)})")
    print("   -> package.xml")

# ── Dockerfile generation ─────────────────────────────────────────────────────

def generate_dockerfile(application):
    rendered = env.get_template("Dockerfile.jinja").render(
        project=args.project,
        extra_packages=application.get("extra_packages", []),
        apt_packages=application.get("apt_packages", []),
    )
    dockerfile_path = os.path.join(PARENT_DIR, "docker", "Dockerfile")
    with open(dockerfile_path, 'w', encoding='utf-8') as f:
        f.write(rendered)
    print(f"   -> {dockerfile_path}")

# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    print(f"\n==> Generating project: {args.project}\n")
    os.makedirs(out("generated_pkg/src"),     exist_ok=True)
    os.makedirs(out("generated_pkg/include"), exist_ok=True)

    cfg = config.load(PROJECT_PATH)
    print()

    for node in cfg['nodes']:
        generate_node(node)

    generate_package(cfg['nodes'])

    print("\n>> Dockerfile")
    generate_dockerfile(cfg['application'])

    print(f"\n===== GENERATION COMPLETE =====")
    print(f"Nodes  : {[n['name'] for n in cfg['nodes']]}")
    print(f"Output : {out('generated_pkg/')}")
    print("================================\n")

if __name__ == "__main__":
    main()
