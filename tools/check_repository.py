"""Read-only repository/CI/metadata/format checks; never publish or contact equipment."""

from importlib.metadata import distribution
import json
from pathlib import Path
import re
import subprocess
import sys
import shutil
from urllib.parse import unquote, urlsplit
import yaml

ROOT = Path(__file__).resolve().parents[1]


def require(condition, message):
    if not condition:
        raise ValueError(message)


def readonly(permissions):
    require(
        isinstance(permissions, dict)
        and all(value in ("read", "none") for value in permissions.values()),
        "Workflow permissions must be read-only",
    )


def validate_workflow(text):
    data = yaml.load(text, Loader=yaml.BaseLoader)
    require(isinstance(data, dict), "Invalid workflow")
    readonly(data.get("permissions"))
    require(
        data["permissions"].get("contents") == "read", "Explicit contents:read required"
    )
    events = data.get("on", {})
    require(
        "pull_request_target" not in events and "workflow_run" not in events,
        "Privileged workflow event is unavailable",
    )
    for job in data.get("jobs", {}).values():
        if "permissions" in job:
            readonly(job["permissions"])
        for step in job.get("steps", []):
            if "uses" not in step:
                continue
            action = step["uses"]
            require(
                re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_./-]+@[a-f0-9]{40}", action),
                "Actions must use immutable commit SHAs",
            )
            if action.startswith("actions/checkout@"):
                require(
                    step.get("with", {}).get("persist-credentials") == "false",
                    "Checkout credentials must not persist",
                )
    require(
        "secrets." not in text,
        "Validation workflow must not consume repository secrets",
    )


def validate_requirements(text):
    result = {}
    for line in text.splitlines():
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        match = re.fullmatch(r"([A-Za-z0-9_.-]+)==([A-Za-z0-9_.+-]+)", line)
        require(match is not None, "Direct dependencies must use exact pins")
        name, version = match.groups()
        require(name not in result, "Duplicate dependency")
        result[name] = version
    require(bool(result), "Missing development dependencies")
    return result


def validate_links(root, paths):
    root = root.resolve()
    for path in paths:
        for destination in re.findall(r"\[[^\]]*\]\(([^)]+)\)", path.read_text()):
            destination = destination.strip().strip("<>")
            parsed = urlsplit(destination)
            if parsed.scheme in ("http", "https", "mailto") or not parsed.path:
                continue
            require(
                not parsed.scheme and not parsed.path.startswith("/"),
                f"Invalid local link in {path.name}",
            )
            target = (path.parent / unquote(parsed.path)).resolve()
            require(
                target.is_relative_to(root) and target.exists(),
                f"Missing local link in {path.name}: {parsed.path}",
            )


def validate_example_secrets(path):
    tree = yaml.compose(path.read_text())

    def lookup(node, keys):
        for key in keys:
            if not isinstance(node, yaml.MappingNode):
                return None
            node = next(
                (value for name, value in node.value if name.value == key), None
            )
            if node is None:
                return None
        return node

    for keys in [("wifi", "ssid"), ("wifi", "password"), ("api", "encryption", "key")]:
        node = lookup(tree, keys)
        if node is not None:
            require(
                node.tag == "!secret", f"{path.name}: {'.'.join(keys)} must use !secret"
            )


def validate_licenses(requirements):
    manifest = json.loads((ROOT / "tools/dependency_licenses.json").read_text())
    entries = {entry["name"]: entry for entry in manifest["dependencies"]}
    require(
        set(entries) == set(requirements),
        "Dependency/license inventory must cover every direct pin",
    )
    for name, version in requirements.items():
        entry = entries[name]
        require(entry["version"] == version, "Stale dependency license version")
        installed = distribution(name)
        require(installed.version == version, f"Installed version mismatch: {name}")
        files = [
            file
            for file in installed.files
            if ".dist-info/licenses/" in str(file)
            and Path(file).name == entry["license_file"]
        ]
        require(len(files) == 1, f"Missing installed license evidence: {name}")
        text = installed.locate_file(files[0]).read_text()
        markers = {
            "MIT": ["Permission is hereby granted", "THE SOFTWARE IS PROVIDED"],
            "Apache-2.0": [
                "Apache License",
                "Version 2.0",
                "END OF TERMS AND CONDITIONS",
            ],
        }
        require(
            entry["license"] in markers
            and all(marker in text for marker in markers[entry["license"]]),
            f"Unexpected license evidence: {name}",
        )
        require(
            urlsplit(entry["source_url"]).scheme == "https",
            "License source must use HTTPS",
        )


def main():
    from pilot_readiness import validate_readiness

    validate_readiness(
        json.loads((ROOT / "docs/development/pilot-readiness.json").read_text())
    )
    paths = (
        subprocess.check_output(
            ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
            cwd=ROOT,
        )
        .decode()
        .split("\0")
    )
    paths = [ROOT / path for path in dict.fromkeys(paths) if path]
    tracked = (
        subprocess.check_output(["git", "ls-files", "--cached", "-z"], cwd=ROOT)
        .decode()
        .split("\0")
    )
    require(
        not any(
            Path(path).name in ("secrets.yaml", ".env") for path in tracked if path
        ),
        "Tracked local secrets file",
    )
    for path in paths:
        if (
            path.suffix in (".yml", ".yaml")
            and path.parent == ROOT / ".github/workflows"
        ):
            validate_workflow(path.read_text())
        if path.suffix == ".yaml" and path.parent == ROOT / "examples":
            validate_example_secrets(path)
    validate_links(ROOT, [path for path in paths if path.suffix == ".md"])
    validate_licenses(
        validate_requirements((ROOT / "requirements-dev.txt").read_text())
    )
    cpp = [
        str(path)
        for path in paths
        if path.suffix in (".h", ".cpp")
        and (ROOT / "components" in path.parents or ROOT / "tests/cpp" in path.parents)
    ]
    sibling = Path(sys.executable).with_name("clang-format")
    formatter = str(sibling) if sibling.exists() else shutil.which("clang-format")
    require(
        formatter is not None, "Install the pinned clang-format development dependency"
    )
    version = subprocess.check_output([formatter, "--version"], text=True)
    require("23.1.3" in version, "clang-format version must match the pinned tool")
    subprocess.run([formatter, "--dry-run", "--Werror", *cpp], check=True)
    print(
        "Repository, immutable Actions, local links, direct dependency/license evidence and C++ format checks passed."
    )


if __name__ == "__main__":
    main()
