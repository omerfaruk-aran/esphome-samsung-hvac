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


def test_optional_direction_pin_generates(tmp_path):
    data = config()
    data["samsung_hvac"][0]["flow_control_pin"] = "GPIO23"
    result = run_esphome(tmp_path, data, "compile", "--only-generate")
    assert result.returncode == 0, result.stdout + result.stderr
    main = tmp_path / ".esphome/build/samsung-foundation/src/main.cpp"
    assert "set_flow_control_pin" in main.read_text()


def test_non_esp32_runtime_is_rejected(tmp_path):
    data = config()
    del data["esp32"]
    data["esp8266"] = {"board": "d1_mini"}
    data["uart"][0]["rx_pin"] = "GPIO4"
    data["uart"][0]["tx_pin"] = "GPIO5"
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "ESP32" in result.stdout + result.stderr


def test_uart_aliases_cannot_claim_same_pins(tmp_path):
    data = config()
    for pin in ("rx_pin", "tx_pin"):
        data["uart"][0][pin] = {
            "number": data["uart"][0][pin],
            "allow_other_uses": True,
        }
    data["uart"].append({**data["uart"][0], "id": "alias_uart"})
    data["samsung_hvac"].append(
        {"id": "alias_hvac", "uart_id": "alias_uart", "protocol": "non_nasa"}
    )
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "pins" in result.stdout + result.stderr


@pytest.mark.parametrize(
    "conflict", ["own_rx", "own_tx", "other_rx", "other_direction"]
)
def test_direction_pin_is_exclusive(tmp_path, conflict):
    data = config()
    for pin in ("rx_pin", "tx_pin"):
        data["uart"][0][pin] = {
            "number": data["uart"][0][pin],
            "allow_other_uses": True,
        }
    number = "GPIO16" if conflict == "own_rx" else "GPIO17"
    if conflict.startswith("other"):
        data["uart"].append(
            {
                **data["uart"][0],
                "id": "second_uart",
                "rx_pin": {"number": "GPIO25", "allow_other_uses": True},
                "tx_pin": "GPIO26",
            }
        )
        data["samsung_hvac"].append(
            {"id": "second", "uart_id": "second_uart", "protocol": "non_nasa"}
        )
        number = "GPIO25" if conflict == "other_rx" else "GPIO23"
        if conflict == "other_direction":
            data["samsung_hvac"][1]["flow_control_pin"] = {
                "number": number,
                "allow_other_uses": True,
            }
    data["samsung_hvac"][0]["flow_control_pin"] = {
        "number": number,
        "allow_other_uses": True,
    }
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert "direction pin" in result.stdout + result.stderr


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


def ac_device(protocol="nasa", address=0x200000):
    return {
        "id": "living_room",
        "profile": "nasa_ac" if protocol == "nasa" else "non_nasa_ac",
        "address": address,
        "target_temperature": {"name": "Living Room Target"},
        "power_state": {"name": "Living Room Power"},
        "operation_mode": {"name": "Living Room Mode"},
        "fan_modes": ["auto", "low", "medium", "high"],
        "fan_mode": {"name": "Living Room Fan"},
        "climate": {"name": "Living Room Internal Climate", "internal": True},
    }


@pytest.mark.parametrize("protocol,address", [("nasa", 0x200000), ("non_nasa", 0)])
def test_ac_profiles_generate(tmp_path, protocol, address):
    data = config()
    root = data["samsung_hvac"][0]
    root["protocol"] = protocol
    root["devices"] = [ac_device(protocol, address)]
    result = run_esphome(tmp_path, data, "compile", "--only-generate")
    assert result.returncode == 0, result.stdout + result.stderr
    main = (tmp_path / ".esphome/build/samsung-foundation/src/main.cpp").read_text()
    assert "add_profile" in main
    assert "set_climate" in main


def test_ac_multisplit_codegen(tmp_path):
    data = config()
    first = ac_device()
    second = {
        "id": "bedroom",
        "profile": "nasa_ac",
        "address": 0x200001,
        "power_state": {"name": "Bedroom Power"},
    }
    data["samsung_hvac"][0]["devices"] = [first, second]
    result = run_esphome(tmp_path, data, "compile", "--only-generate")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    "case,expected",
    [
        ("mismatch", "profile must match"),
        ("duplicate", "Duplicate device address"),
        ("bad_address", "indoor address"),
        ("missing_id", "id"),
        ("public_climate", "internal"),
        ("nasa_room", "room_temperature"),
        ("legacy_preset", "presets"),
        ("fan_without_capability", "fan_modes"),
        ("nasa_fresh", "fresh"),
        ("swing_without_capability", "vertical_swing"),
        ("outdoor_without_address", "outdoor_address"),
        ("outdoor_indoor_address", "outdoor address"),
        ("too_many", "8"),
    ],
)
def test_ac_unsupported_configuration_rejected(tmp_path, case, expected):
    data = config()
    device = ac_device()
    devices = [device]
    if case == "mismatch":
        device.update(profile="non_nasa_ac", address=0)
    elif case == "duplicate":
        devices.append(
            {"id": "duplicate", "profile": "nasa_ac", "address": device["address"]}
        )
    elif case == "bad_address":
        device["address"] = 0x800000
    elif case == "missing_id":
        del device["id"]
    elif case == "public_climate":
        device["climate"]["internal"] = False
    elif case == "nasa_room":
        device["room_temperature"] = {"name": "Room"}
    elif case == "legacy_preset":
        data["samsung_hvac"][0]["protocol"] = "non_nasa"
        device.update(profile="non_nasa_ac", address=0, presets=["windfree"])
    elif case == "fan_without_capability":
        del device["fan_modes"]
    elif case == "nasa_fresh":
        device["fan_modes"] = ["fresh"]
    elif case == "swing_without_capability":
        device["vertical_swing_state"] = {"name": "Swing"}
    elif case == "outdoor_without_address":
        device["outdoor_temperature"] = {"name": "Outdoor"}
    elif case == "outdoor_indoor_address":
        device["outdoor_address"] = 0x200001
    elif case == "too_many":
        devices = [
            {"id": f"device_{i}", "profile": "nasa_ac", "address": 0x200000 + i}
            for i in range(9)
        ]
    data["samsung_hvac"][0]["devices"] = devices
    result = run_esphome(tmp_path, data, "config")
    assert result.returncode != 0
    assert expected in result.stdout + result.stderr
