"""Reject readiness claims without the required explicit evidence record."""

import copy
import importlib.util
import json
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]


def validator():
    spec = importlib.util.spec_from_file_location(
        "pilot_readiness", ROOT / "tools/pilot_readiness.py"
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.validate_readiness


def record():
    return json.loads((ROOT / "docs/development/pilot-readiness.json").read_text())


def test_current_readiness_record_is_consistent():
    validator()(record())


@pytest.mark.parametrize(
    "case",
    [
        "hardware_without_evidence",
        "production_without_gates",
        "release_without_approval",
        "missing_profile",
        "observed_without_capture",
        "boolean_version",
        "overall_pass_without_gates",
        "release_with_blocked_gates",
        "missing_gate",
    ],
)
def test_unsupported_readiness_promotion_rejected(case):
    data = copy.deepcopy(record())
    if case == "overall_pass_without_gates":
        data["overall_status"] = "PASS"
    elif case == "release_with_blocked_gates":
        data["release_authorized"] = True
        data["release_approval_reference"] = "synthetic-test-approval-only"
    elif case == "missing_gate":
        del data["critical_gates"]["physical_rollback"]
    elif case == "observed_without_capture":
        data["profiles"][0]["hardware_evidence_level"] = "observed"
    elif case == "boolean_version":
        data["version"] = True
    elif case == "hardware_without_evidence":
        data["profiles"][0]["hardware_status"] = "PASS"
    elif case == "production_without_gates":
        data["production_ready"] = True
    elif case == "release_without_approval":
        data["release_authorized"] = True
    elif case == "missing_profile":
        data["profiles"].pop()
    with pytest.raises(ValueError):
        validator()(data)


def synthetic_accepted_record():
    """Test metadata only; never a report of hardware actions or compatibility."""
    data = record()
    data.update(
        overall_status="PASS",
        production_ready=True,
        legacy_parity_status="PARITY",
        blockers=[],
    )
    for profile in data["profiles"]:
        profile.update(
            hardware_status="PASS",
            hardware_evidence_level="confirmed",
            physical_actions_performed=True,
            device_models=["synthetic-test-only"],
        )
        for field in (
            "controller_model",
            "firmware_reference",
            "interface_model",
            "topology_reference",
            "authorization_reference",
            "capture_reference",
            "test_reference",
            "results_reference",
        ):
            profile[field] = "synthetic-test-only"
    data["critical_gates"] = dict.fromkeys(data["critical_gates"], "PASS")
    return data


def test_synthetic_complete_record_is_structurally_consistent():
    validator()(synthetic_accepted_record())


@pytest.mark.parametrize("status", ["FAIL", "BLOCKED"])
def test_production_flag_requires_overall_pass(status):
    data = synthetic_accepted_record()
    data["overall_status"] = status
    with pytest.raises(ValueError):
        validator()(data)
