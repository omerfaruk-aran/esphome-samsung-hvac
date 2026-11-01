"""Validate or compile reference YAML with ephemeral, never-deployed test secrets."""

import argparse
import base64
from pathlib import Path
import secrets
import subprocess
import sys
import tempfile
import yaml

ROOT = Path(__file__).resolve().parents[1]
EXAMPLES = ("nasa-ac", "non-nasa-ac", "ehs", "multi-split", "capture")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("example", choices=EXAMPLES)
    parser.add_argument(
        "--compile", action="store_true", help="Build firmware without uploading"
    )
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="samsung-example-") as directory:
        work = Path(directory)
        path = work / (args.example + ".yaml")
        path.write_text(
            (ROOT / "examples" / path.name)
            .read_text()
            .replace("../components", str(ROOT / "components"))
        )
        values = {
            "wifi_ssid": "build-only-network",
            "wifi_password": secrets.token_hex(12),
            "api_encryption_key": base64.b64encode(secrets.token_bytes(32)).decode(),
        }
        (work / "secrets.yaml").write_text(yaml.safe_dump(values))
        command = [sys.executable, "-m", "esphome", "compile"]
        if not args.compile:
            command.append("--only-generate")
        command.append(str(path))
        result = subprocess.run(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=1200,
        )
        output = result.stdout
        for value in values.values():
            output = output.replace(value, "[redacted]")
        print(output, end="")
        return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
