"""Exercise documented migration inputs and completeness of legacy dispositions."""

import base64
import json
from pathlib import Path
import secrets
import re
import subprocess
import sys

import pytest
import yaml

ROOT = Path(__file__).resolve().parents[1]


def test_all_baseline_declarations_have_explicit_dispositions():
    data = json.loads((ROOT / "docs/compatibility/legacy-migration.json").read_text())
    assert data["baseline"] == "6b1966bea8db9344d23763454f9cebed015d15c5"
    rows = data["declarations"]
    assert len(rows) == 65
    assert (
        len({(row["old_scope"], row["old_key"], row["source_url"]) for row in rows})
        == 65
    )
    assert all(row["status"] in ("PARTIAL", "UNSUPPORTED") for row in rows)
    assert all(
        row["new_equivalent"]
        and row["notes"]
        and row["old_default"]
        and data["baseline"] in row["source_url"]
        for row in rows
    )
    table = (ROOT / "docs/compatibility/legacy-yaml-reference.md").read_text()
    assert (
        len(
            [
                line
                for line in table.splitlines()
                if line.startswith("| ") and "#L" in line
            ]
        )
        == 65
    )
    table_rows = [
        line for line in table.splitlines() if line.startswith("| ") and "#L" in line
    ]
    assert all(len(line.strip().split("|")[1:-1]) == 8 for line in table_rows)
    parsed = {}
    for line in table_rows:
        cells = [cell.strip() for cell in line.strip().split("|")[1:-1]]
        key = re.search(r"`([^`]+)`", cells[1]).group(1)
        source = re.search(r"\]\((https://[^)]+)\)", cells[4]).group(1)
        parsed[(cells[0], key, source)] = cells
    for row in rows:
        identity = (row["old_scope"], row["old_key"], row["source_url"])
        assert identity in parsed
        cells = parsed[identity]
        assert cells[2:] == [
            row["old_requirement"],
            row["old_default"],
            cells[4],
            row["new_equivalent"],
            row["status"],
            row["notes"],
        ]


@pytest.mark.parametrize("name", ["nasa-ac", "non-nasa-ac", "ehs", "multi-split"])
def test_migration_example_codegen(tmp_path, name):
    example = ROOT / "examples" / f"{name}.yaml"
    text = example.read_text().replace("../components", str(ROOT / "components"))
    path = tmp_path / f"{name}.yaml"
    path.write_text(text)
    # Non-deployed ephemeral test values; never committed or sent to a device.
    values = {
        "wifi_ssid": "build-only-network",
        "wifi_password": secrets.token_hex(12),
        "api_encryption_key": base64.b64encode(secrets.token_bytes(32)).decode(),
    }
    (tmp_path / "secrets.yaml").write_text(yaml.safe_dump(values))
    result = subprocess.run(
        [sys.executable, "-m", "esphome", "compile", "--only-generate", str(path)],
        capture_output=True,
        text=True,
        timeout=120,
    )
    output = result.stdout + result.stderr
    for value in values.values():
        output = output.replace(value, "[redacted]")
    assert result.returncode == 0, output
    assert "Component not found" not in output and "Traceback" not in output
