"""Exercise real ESPHome loading, validation and code generation."""

from pathlib import Path
import subprocess
import sys

import pytest
import yaml

ROOT = Path(__file__).resolve().parents[1]


def config():
    return {
        "esphome": {"name": "samsung-foundation"},
        "esp32": {"board": "esp32dev", "framework": {"type": "esp-idf"}},
        "logger": {"baud_rate": 0},
        "external_components": [
            {"source": {"type": "local", "path": str(ROOT / "components")}}
        ],
        "uart": [
            {
                "id": "hvac_uart",
                "rx_pin": "GPIO16",
                "tx_pin": "GPIO17",
                "baud_rate": 9600,
                "parity": "EVEN",
            }
        ],
        "samsung_hvac": [{"id": "hvac", "uart_id": "hvac_uart", "protocol": "nasa"}],
    }


def run_esphome(tmp_path, data, *command):
    path = tmp_path / "test.yaml"
    path.write_text(yaml.safe_dump(data, sort_keys=False))
    result = subprocess.run(
        [sys.executable, "-m", "esphome", *command, str(path)],
        capture_output=True,
        text=True,
        timeout=120,
    )
    assert "Component not found" not in result.stdout + result.stderr
    assert "Failed to load component" not in result.stdout + result.stderr
    return result


@pytest.mark.parametrize("protocol", ["nasa", "non_nasa"])
@pytest.mark.parametrize("framework", ["esp-idf", "arduino"])
def test_explicit_protocol_loads_and_generates(tmp_path, protocol, framework):
    data = config()
    data["samsung_hvac"][0]["protocol"] = protocol
    data["esp32"]["framework"]["type"] = framework
    result = run_esphome(tmp_path, data, "compile", "--only-generate")
    assert result.returncode == 0, result.stdout + result.stderr
    sources = tmp_path / ".esphome/build/samsung-foundation/src"
    assert "set_protocol" in (sources / "main.cpp").read_text()
    assert (sources / "esphome/components/samsung_hvac/protocol_selection.cpp").exists()


def test_separate_buses_are_independent(tmp_path):
    data = config()
    data["uart"].append(
        {**data["uart"][0], "id": "second_uart", "rx_pin": "GPIO25", "tx_pin": "GPIO26"}
    )
    data["samsung_hvac"].append(
        {"id": "second", "uart_id": "second_uart", "protocol": "non_nasa"}
    )
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("protocol", [None, "auto", "invalid"])
def test_requires_known_explicit_protocol(tmp_path, protocol):
    data = config()
    if protocol is None:
        del data["samsung_hvac"][0]["protocol"]
    else:
        data["samsung_hvac"][0]["protocol"] = protocol
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "protocol" in result.stdout + result.stderr
    assert "Component not found" not in result.stdout + result.stderr


@pytest.mark.parametrize("option", ["enable_writes", "raw_write", "fsv_write"])
def test_rejects_write_options(tmp_path, option):
    data = config()
    data["samsung_hvac"][0][option] = True
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "invalid option" in result.stdout + result.stderr


@pytest.mark.parametrize(
    ("key", "value", "message"),
    [
        ("baud_rate", 19200, "baud rate 9600"),
        ("parity", "NONE", "parity EVEN"),
        ("data_bits", 7, "8 data bits"),
        ("stop_bits", 2, "1 stop bits"),
        ("rx_pin", None, "rx_pin"),
        ("tx_pin", None, "tx_pin"),
    ],
)
def test_uart_framing_is_validated(tmp_path, key, value, message):
    data = config()
    if value is None:
        del data["uart"][0][key]
    else:
        data["uart"][0][key] = value
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert message in result.stdout + result.stderr


def test_duplicate_transport_owner_is_rejected(tmp_path):
    data = config()
    data["samsung_hvac"].append(
        {"id": "duplicate", "uart_id": "hvac_uart", "protocol": "non_nasa"}
    )
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "exclusive" in result.stdout + result.stderr


def test_generic_uart_device_cannot_compete(tmp_path):
    data = config()
    data["button"] = [
        {
            "platform": "uart",
            "name": "Competing writer",
            "uart_id": "hvac_uart",
            "data": [0x32],
        }
    ]
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "exclusive" in result.stdout + result.stderr


def test_uart_write_automation_cannot_bypass_owner(tmp_path):
    data = config()
    data["esphome"]["on_boot"] = [
        {"then": [{"uart.write": {"id": "hvac_uart", "data": [0x32]}}]}
    ]
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "uart.write" in result.stdout + result.stderr


def test_dummy_receiver_cannot_consume_owned_rx(tmp_path):
    data = config()
    data["uart"][0]["debug"] = {"dummy_receiver": True}
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "dummy_receiver must be disabled" in result.stdout + result.stderr


def test_legacy_owner_cannot_share_uart(tmp_path):
    # Load a minimal contract fixture rather than copying the legacy component.
    fixture = tmp_path / "components/samsung_ac"
    fixture.mkdir(parents=True)
    (fixture / "__init__.py").write_text(
        "import esphome.config_validation as cv\n"
        "from esphome.components import uart\n"
        'DEPENDENCIES = ["uart"]\n'
        "CONFIG_SCHEMA = cv.Schema({}).extend(uart.UART_DEVICE_SCHEMA)\n"
    )
    data = config()
    data["external_components"].append(
        {"source": {"type": "local", "path": str(tmp_path / "components")}}
    )
    data["samsung_ac"] = {"uart_id": "hvac_uart"}
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "exclusive" in result.stdout + result.stderr


@pytest.mark.parametrize(
    "payload",
    [
        {"uart.write": "fixture"},
        {"uart_id": "hvac_uart"},
    ],
)
def test_unrelated_json_keys_are_not_uart_configuration(tmp_path, payload):
    data = config()
    data["wifi"] = {"ssid": "build-only-fixture"}
    data["http_request"] = {"verify_ssl": False}
    data["esphome"]["on_boot"] = [
        {
            "then": [
                {
                    "http_request.post": {
                        "url": "http://example.invalid",
                        "json": payload,
                    }
                }
            ]
        }
    ]
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode == 0, result.stdout + result.stderr
