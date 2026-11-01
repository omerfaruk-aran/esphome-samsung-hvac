"""Strict versioned catalog schema and protocol/evidence checks (standard library)."""

import json
import math
from pathlib import Path
import re
from urllib.parse import urlsplit

FIELDS = set(
    "id protocol code value_type payload_length source_role destination_role direction access unit scale offset signedness sentinels sentinel_status valid_values conversion layout device_scope device_models evidence_level source_url capture_reference test_reference notes fsv raw_range".split()
)
NASA_TYPES = {"enum": 1, "variable": 2, "long_variable": 4, "structure": 0}


def _require(condition, message):
    if not condition:
        raise ValueError(message)


def _integer(value, maximum):
    return type(value) is int and 0 <= value <= maximum


def _finite(value):
    return type(value) in (int, float) and math.isfinite(value)


def _strings(value, nonempty=False):
    return (
        isinstance(value, list)
        and (not nonempty or bool(value))
        and all(isinstance(item, str) and bool(item.strip()) for item in value)
    )


def validate_catalog(document):
    _require(
        isinstance(document, dict) and set(document) == {"version", "messages"},
        "Invalid catalog envelope",
    )
    _require(
        type(document["version"]) is int and document["version"] == 1,
        "Unsupported catalog version",
    )
    _require(
        isinstance(document["messages"], list) and 0 < len(document["messages"]) <= 128,
        "Invalid catalog size",
    )
    result = {}
    for entry in document["messages"]:
        _require(
            isinstance(entry, dict) and set(entry) == FIELDS,
            "Missing or unexpected descriptor fields",
        )
        identifier = entry["id"]
        _require(
            isinstance(identifier, str)
            and re.fullmatch(r"[a-z][a-z0-9_]{0,63}", identifier),
            "Invalid descriptor ID",
        )
        _require(identifier not in result, "Duplicate descriptor ID")
        _require(entry["protocol"] in ("nasa", "non_nasa"), "Invalid protocol")
        _require(
            entry["access"] in ("passive", "disabled"), "Catalog writes are unavailable"
        )
        _require(
            entry["source_role"] in ("indoor", "outdoor")
            and entry["destination_role"] == "any"
            and entry["direction"] == "device_to_bus",
            "Invalid descriptor roles/direction",
        )
        _require(
            isinstance(entry["unit"], str) and len(entry["unit"]) <= 32, "Invalid unit"
        )
        _require(
            _finite(entry["scale"])
            and entry["scale"] != 0
            and _finite(entry["offset"]),
            "Invalid finite conversion",
        )
        _require(entry["signedness"] in ("signed", "unsigned"), "Invalid signedness")
        _require(type(entry["fsv"]) is bool, "Invalid FSV marker")
        _require(
            type(entry["payload_length"]) is int
            and entry["payload_length"] in (0, 1, 2, 4),
            "Invalid payload length",
        )
        if entry["protocol"] == "nasa":
            _require(_integer(entry["code"], 65535), "Invalid NASA code")
            expected = list(NASA_TYPES)[(entry["code"] >> 9) & 3]
            _require(
                entry["value_type"] == expected
                and entry["payload_length"] == NASA_TYPES[expected],
                "NASA wire type/length mismatch",
            )
            _require(
                entry["layout"] is None and entry["conversion"] == "linear",
                "Invalid NASA conversion/layout",
            )
            _require(
                expected != "structure" or entry["access"] == "disabled",
                "Structure numeric observations unavailable",
            )
        else:
            _require(
                _integer(entry["code"], 255)
                and entry["code"] == 0x20
                and entry["payload_length"] == 1,
                "Invalid Non-NASA Cmd20 descriptor",
            )
            _require(
                entry["value_type"] in ("temperature8", "bitfield")
                and entry["conversion"]
                == (
                    "non_nasa_temperature"
                    if entry["value_type"] == "temperature8"
                    else "bit_extract"
                ),
                "Invalid Non-NASA conversion",
            )
            layout = entry["layout"]
            _require(
                isinstance(layout, dict)
                and set(layout) == {"byte", "mask", "shift"}
                and _integer(layout["byte"], 7)
                and _integer(layout["mask"], 255)
                and layout["mask"] != 0
                and _integer(layout["shift"], 7),
                "Invalid Non-NASA layout",
            )
            shifted = layout["mask"] >> layout["shift"]
            _require(
                shifted != 0
                and (shifted & (shifted + 1)) == 0
                and (layout["mask"] & ((1 << layout["shift"]) - 1)) == 0,
                "Inconsistent Non-NASA mask/shift",
            )
            if entry["value_type"] == "temperature8":
                _require(
                    layout["mask"] == 255 and layout["shift"] == 0,
                    "Invalid encoded temperature extraction",
                )
            if entry["unit"] == "boolean":
                _require(shifted == 1, "Invalid boolean extraction")
            _require(
                entry["signedness"] == "unsigned"
                and entry["scale"] == 1
                and entry["offset"] == 0,
                "Invalid Non-NASA raw conversion",
            )
        maximum = (
            (1 << (8 * entry["payload_length"])) - 1 if entry["payload_length"] else 0
        )
        if entry["protocol"] == "non_nasa":
            maximum = entry["layout"]["mask"] >> entry["layout"]["shift"]
        _require(
            isinstance(entry["sentinels"], list)
            and len(entry["sentinels"]) <= 4
            and all(_integer(v, maximum) for v in entry["sentinels"]),
            "Invalid raw sentinels",
        )
        _require(
            entry["sentinel_status"] in ("unknown", "specified")
            and (entry["sentinel_status"] != "unknown" or not entry["sentinels"]),
            "Invalid sentinel evidence",
        )
        values = entry["valid_values"]
        _require(
            values is None
            or (
                isinstance(values, list)
                and 0 < len(values) <= 16
                and all(_integer(v, maximum) for v in values)
                and len(values) == len(set(values))
            ),
            "Invalid raw values",
        )
        raw_range = entry["raw_range"]
        _require(
            raw_range is None
            or (
                isinstance(raw_range, dict)
                and set(raw_range) == {"minimum", "maximum"}
                and all(_integer(v, maximum) for v in raw_range.values())
                and raw_range["minimum"] <= raw_range["maximum"]
            ),
            "Invalid raw range",
        )
        if entry["unit"] == "boolean":
            _require(
                entry["signedness"] == "unsigned"
                and entry["scale"] == 1
                and entry["offset"] == 0
                and values == [0, 1],
                "Invalid boolean conversion",
            )
        for key in ("device_scope", "test_reference"):
            _require(
                _strings(entry[key], True), "Missing descriptor scope/test reference"
            )
        _require(_strings(entry["device_models"]), "Invalid model evidence")
        _require(
            entry["evidence_level"] in ("confirmed", "observed", "inferred", "unknown"),
            "Invalid evidence level",
        )
        _require(
            entry["capture_reference"] is None
            or (
                isinstance(entry["capture_reference"], str)
                and bool(entry["capture_reference"].strip())
            ),
            "Invalid capture reference",
        )
        if entry["evidence_level"] in ("confirmed", "observed"):
            _require(
                bool(entry["device_models"]) and bool(entry["capture_reference"]),
                "Capture/model evidence required",
            )
        _require(
            entry["access"] != "passive" or entry["evidence_level"] != "unknown",
            "Unresolved named descriptor must stay disabled",
        )
        _require(_strings(entry["source_url"], True), "Missing source evidence")
        for url in entry["source_url"]:
            parsed = urlsplit(url)
            _require(
                parsed.scheme == "https"
                and parsed.hostname
                and not parsed.username
                and not parsed.password,
                "Invalid source URL",
            )
        _require(
            isinstance(entry["notes"], str) and bool(entry["notes"].strip()),
            "Missing evidence notes",
        )
        result[identifier] = entry
    return result


def load_catalog():
    def invalid_constant(value):
        raise ValueError("Nonfinite JSON constant")

    return validate_catalog(
        json.loads(
            Path(__file__).with_name("message_catalog.json").read_text(),
            parse_constant=invalid_constant,
        )
    )
