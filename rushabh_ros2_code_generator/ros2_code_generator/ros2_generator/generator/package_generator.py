"""
RSG V1 module documentation.

Package generation orchestrator for RSG V1.

Responsibilities:
- Render package files, C++ node files, optional artifacts, and custom msg/srv files.
- Keep ROS 2 C++ text inside Jinja templates, not scattered Python strings.

Version History:
- v1.6.1: Added detailed maintainer documentation comments.
"""

from pathlib import Path
from ros2_generator.render.renderer import TemplateRenderer
from ros2_generator.utils.ros_types import parse_ros_type, render_interface_field



# Class: PackageGenerator
# Purpose: See module docstring for this class's role in the generation pipeline.
class PackageGenerator:
    def __init__(self, model, output_root: Path):
        self.model = model
        self.output_root = Path(output_root)
        self.package_dir = self.output_root / self.model.app.package_name
        self.template_root = Path(__file__).resolve().parents[1] / "templates"
        self.renderer = TemplateRenderer(self.template_root)
        self.created_files = []
        self.preserved_files = []

    def _ctx(self):
        return {"app": self.model.app, "generation": self.model.generation, "code_layout": self.model.code_layout,
                "interfaces": self.model.interfaces, "nodes": self.model.nodes, "topics": self.model.topics,
                "services": self.model.services, "qos_profiles": self.model.qos_profiles,
                "dependencies": self.model.dependencies, "parse_ros_type": parse_ros_type,
                "created_files": self.created_files, "preserved_files": self.preserved_files}

    def _write(self, template, rel, ctx, executable=False, preserve=False):
        out = self.package_dir / rel
        if preserve and out.exists() and self.model.generation.preserve_user_code:
            self.preserved_files.append(rel); return
        self.renderer.render_to_file(template, out, ctx, executable)
        self.created_files.append(rel)

    def _write_text(self, rel, text):
        out = self.package_dir / rel
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(text, encoding="utf-8")
        self.created_files.append(rel)

    def _generate_interfaces(self):
        for m in self.model.interfaces.messages:
            body = "\n".join(render_interface_field(f) for f in m.fields) + "\n"
            self._write_text(f"msg/{m.name}.msg", body)
        for s in self.model.interfaces.services:
            req = "\n".join(render_interface_field(f) for f in s.request)
            res = "\n".join(render_interface_field(f) for f in s.response)
            self._write_text(f"srv/{s.name}.srv", req + "\n---\n" + res + "\n")

    def generate(self) -> Path:
        self.package_dir.mkdir(parents=True, exist_ok=True)
        ctx = self._ctx()
        for d in ["src", f"include/{self.model.app.package_name}"]:
            (self.package_dir / d).mkdir(parents=True, exist_ok=True)
        if self.model.interfaces.enabled:
            self._generate_interfaces()
        self._write("package/CMakeLists.txt.j2", "CMakeLists.txt", ctx)
        self._write("package/package.xml.j2", "package.xml", ctx)
        for node in self.model.nodes:
            nctx = dict(ctx, node=node)
            if self.model.code_layout.mode == "inline_user_sections":
                self._write("cpp/inline_node.hpp.j2", f"include/{self.model.app.package_name}/{node.name}.hpp", nctx)
                self._write("cpp/inline_node.cpp.j2", f"src/{node.name}.cpp", nctx)
            else:
                self._write("cpp/node.hpp.j2", f"include/{self.model.app.package_name}/generated/{node.name}.hpp", nctx)
                self._write("cpp/node.cpp.j2", f"src/generated/{node.name}.cpp", nctx)
                self._write("cpp/node_main.cpp.j2", f"src/generated/{node.name}_main.cpp", nctx)
                self._write("cpp/user_logic.hpp.j2", f"include/{self.model.app.package_name}/user/{node.name}_logic.hpp", nctx, preserve=True)
                self._write("cpp/user_logic.cpp.j2", f"src/user/{node.name}_logic.cpp", nctx, preserve=True)
        if self.model.generation.config: self._write("config/params.yaml.j2", "config/params.yaml", ctx)
        if self.model.generation.launch: self._write("launch/bringup.launch.py.j2", "launch/bringup.launch.py", ctx)
        if self.model.generation.tests: self._write("test/test_generated_files.py.j2", "test/test_generated_files.py", ctx)
        if self.model.generation.scripts:
            self._write("scripts/build.sh.j2", "scripts/build.sh", ctx, executable=True)
            self._write("scripts/run.sh.j2", "scripts/run.sh", ctx, executable=True)
        if self.model.generation.readme: self._write("docs/README.md.j2", "README.md", ctx)
        if self.model.generation.generation_report:
            ctx = self._ctx(); self._write("docs/generation_report.yaml.j2", "generation_report.yaml", ctx)
        return self.package_dir
