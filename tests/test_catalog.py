"""Validate catalog structure, wire semantics and evidence without ESPHome imports."""

import copy
import importlib.util
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "components/samsung_hvac"


def catalog_module():
    spec = importlib.util.spec_from_file_location(
        "samsung_catalog", PATH / "catalog.py"
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_catalog_loads_with_explicit_evidence():
    catalog = catalog_module().load_catalog()
    assert catalog["nasa_target"]["code"] == 0x4201
    assert catalog["ehs_water_law_shift"]["signedness"] == "signed"
    assert catalog["ehs_fsv_heating_upper"]["code"] == 0x424E
    assert catalog["non_nasa_room"]["layout"] == {"byte": 1, "mask": 255, "shift": 0}
    assert catalog["unresolved_4204"]["access"] == "disabled"
    assert not any(entry["access"] == "write" for entry in catalog.values())


@pytest.mark.parametrize(
    "case",
    [
        "missing",
        "extra",
        "duplicate",
        "width",
        "type",
        "nonfinite",
        "zero_scale",
        "sentinel",
        "signedness",
        "false_confirmation",
        "access",
        "source",
        "bitfield",
        "boolean_length",
        "non_scalar_values",
        "bitfield_shift",
    ],
)
def test_malformed_catalog_rejected(case):
    module = catalog_module()
    catalog = module.load_catalog()
    entry = copy.deepcopy(catalog["nasa_target"])
    document = {"version": 1, "messages": [entry]}
    if case == "missing":
        del entry["sentinel_status"]
    elif case == "extra":
        entry["secret_write_key"] = True
    elif case == "duplicate":
        document["messages"].append(copy.deepcopy(entry))
    elif case == "width":
        entry["payload_length"] = 4
    elif case == "type":
        entry["value_type"] = "enum"
    elif case == "nonfinite":
        entry["scale"] = float("nan")
    elif case == "zero_scale":
        entry["scale"] = 0
    elif case == "sentinel":
        entry["sentinels"] = [65536]
    elif case == "signedness":
        entry["signedness"] = "guess"
    elif case == "false_confirmation":
        entry["evidence_level"] = "confirmed"
    elif case == "access":
        entry["access"] = "write"
    elif case == "source":
        entry["source_url"] = []
    elif case == "boolean_length":
        entry = copy.deepcopy(catalog["nasa_power"])
        entry["payload_length"] = True
        document["messages"] = [entry]
    elif case == "non_scalar_values":
        entry["valid_values"] = [{}]
    elif case == "bitfield_shift":
        entry = copy.deepcopy(catalog["non_nasa_power"])
        entry["layout"]["mask"] = 1
        document["messages"] = [entry]
    elif case == "bitfield":
        entry = copy.deepcopy(catalog["non_nasa_room"])
        entry["layout"]["byte"] = 8
        document["messages"] = [entry]
    with pytest.raises(ValueError):
        module.validate_catalog(document)
