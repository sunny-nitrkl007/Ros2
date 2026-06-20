"""
RSG V1 module documentation.

Jinja2 renderer wrapper for RSG V1.

Responsibilities:
- Configure Jinja2 rendering.
- Render one template to one output file deterministically.

Version History:
- v1.6.1: Added detailed maintainer documentation comments.
"""

from pathlib import Path
from jinja2 import Environment, FileSystemLoader, StrictUndefined


# Class: TemplateRenderer
# Purpose: See module docstring for this class's role in the generation pipeline.
class TemplateRenderer:
    def __init__(self, template_root: Path):
        self.template_root = Path(template_root)
        self.env = Environment(loader=FileSystemLoader(str(self.template_root)), undefined=StrictUndefined, trim_blocks=True, lstrip_blocks=True, keep_trailing_newline=True)
    def render_to_file(self, template_name: str, output_path: Path, context: dict, executable: bool = False) -> None:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(self.env.get_template(template_name).render(**context), encoding="utf-8")
        if executable: output_path.chmod(0o755)
