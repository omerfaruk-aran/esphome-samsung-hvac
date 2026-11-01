"""Exercise CI/security policy checks without network or publication."""

import importlib.util
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]


def checker():
    spec = importlib.util.spec_from_file_location(
        "repository_checks", ROOT / "tools/check_repository.py"
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def workflow():
    return """name: Validation
on: [pull_request, push]
permissions:
  contents: read
jobs:
  tests:
    runs-on: ubuntu-24.04
    steps:
      - uses: actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1
        with:
          persist-credentials: false
      - run: python -m pytest -q
"""


def test_readonly_workflow_is_accepted():
    checker().validate_workflow(workflow())


@pytest.mark.parametrize(
    "case", ["tag", "write", "privileged_event", "credentials", "job_write"]
)
def test_unsafe_workflow_rejected(case):
    text = workflow()
    if case == "tag":
        text = text.replace("3d3c42e5aac5ba805825da76410c181273ba90b1", "v7.0.1")
    elif case == "write":
        text = text.replace("contents: read", "contents: write")
    elif case == "privileged_event":
        text = text.replace("pull_request,", "pull_request_target,")
    elif case == "credentials":
        text = text.replace("persist-credentials: false", "persist-credentials: true")
    elif case == "job_write":
        text = text.replace(
            "runs-on: ubuntu-24.04", "permissions: write-all\n    runs-on: ubuntu-24.04"
        )
    with pytest.raises(ValueError):
        checker().validate_workflow(text)


def test_floating_dependencies_rejected():
    with pytest.raises(ValueError):
        checker().validate_requirements("esphome>=2026.10.1\n")


def test_missing_local_document_link_rejected(tmp_path):
    path = tmp_path / "README.md"
    path.write_text("[missing](missing.md)")
    with pytest.raises(ValueError):
        checker().validate_links(tmp_path, [path])


def test_plaintext_example_secret_rejected(tmp_path):
    path = tmp_path / "sample.yaml"
    path.write_text("wifi:\n  password: fixture-only\n")
    with pytest.raises(ValueError, match="!secret"):
        checker().validate_example_secrets(path)


def test_example_helper_cannot_upload():
    import subprocess
    import sys

    result = subprocess.run(
        [sys.executable, str(ROOT / "tools/check_example.py"), "nasa-ac", "--upload"],
        capture_output=True,
        text=True,
    )
    assert result.returncode != 0
    assert "unrecognized arguments" in result.stderr
