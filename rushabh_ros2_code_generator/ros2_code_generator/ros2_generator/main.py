"""
RSG V1 module documentation.

Command-line entry point for RSG V1.

Responsibilities:
- Run load -> normalize -> validate -> resolve dependencies -> generate.
- Keep CLI thin; feature behavior belongs in core modules and templates.

Version History:
- v1.6.1: Added detailed maintainer documentation comments.
"""

from pathlib import Path
import argparse
import sys
from ros2_generator.core.loader import load_grammar
from ros2_generator.core.normalizer import build_model
from ros2_generator.core.semantic_validator import validate_model
from ros2_generator.core.dependency_resolver import resolve_dependencies
from ros2_generator.generator.package_generator import PackageGenerator
from ros2_generator.utils.errors import GeneratorError


# Function: generate
# Purpose: See module docstring and inline code for detailed behavior.
def generate(grammar_dir: str, output_dir: str | None = None) -> Path | None:
    raw = load_grammar(grammar_dir)
    model = build_model(raw)
    if not model.generation.enabled:
        print("Generation disabled by grammar: generation.enabled=false. No files generated.")
        return None
    validate_model(model)
    resolve_dependencies(model)
    root = Path(output_dir or model.generation.output_directory)
    return PackageGenerator(model, root).generate()


# Function: main
# Purpose: See module docstring and inline code for detailed behavior.
def main(argv=None) -> int:
    parser = argparse.ArgumentParser(prog="ros2_generator")
    sub = parser.add_subparsers(dest="command", required=True)
    gen = sub.add_parser("generate", help="Generate package from grammar folder")
    gen.add_argument("grammar_dir")
    gen.add_argument("output_dir", nargs="?", default=None)
    args = parser.parse_args(argv)
    try:
        package_dir = generate(args.grammar_dir, args.output_dir)
        if package_dir is not None:
            print(f"Generated package: {package_dir}")
        return 0
    except GeneratorError as exc:
        print(f"[RSG ERROR] {exc}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())
