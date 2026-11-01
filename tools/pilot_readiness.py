"""Structural readiness/evidence checks; metadata cannot prove a physical test occurred."""

from pathlib import Path
import json
import re

PROFILES = {"nasa_ac", "non_nasa_ac", "ehs"}
GATES = {
    "identified_interface_and_topology",
    "nasa_ac_hardware",
    "non_nasa_ac_hardware",
    "ehs_hardware",
    "ehs_zone_isolation",
    "entity_api_availability",
    "reboot_fault_recovery",
    "physical_rollback",
    "home_assistant_identity_restore",
    "legacy_parity_acceptance",
    "verified_control_and_readback",
}
STATUSES = {"PASS", "FAIL", "BLOCKED", "NOT_TESTED", "PARTIAL"}
EVIDENCE_FIELDS = (
    "controller_model",
    "firmware_reference",
    "interface_model",
    "topology_reference",
    "authorization_reference",
    "capture_reference",
    "test_reference",
    "results_reference",
)


def require(condition, message):
    if not condition:
        raise ValueError(message)


def text(value):
    return isinstance(value, str) and bool(value.strip())


def validate_readiness(record):
    require(isinstance(record, dict), "Readiness record must be an object")
    require(
        type(record.get("version")) is int and record["version"] == 1,
        "Unsupported readiness version",
    )
    for field in (
        "software_commit",
        "software_tree",
        "ci_source_commit",
        "legacy_baseline",
    ):
        require(
            isinstance(record.get(field), str)
            and re.fullmatch(r"[a-f0-9]{40}", record[field]),
            "Missing immutable software/baseline reference",
        )
    require(
        record.get("overall_status") in STATUSES, "Invalid overall readiness status"
    )
    require(
        type(record.get("production_ready")) is bool
        and type(record.get("release_authorized")) is bool,
        "Explicit readiness/authorization flags required",
    )
    profiles = record.get("profiles")
    require(
        isinstance(profiles, list)
        and len(profiles) == 3
        and all(isinstance(profile, dict) for profile in profiles)
        and {profile.get("profile") for profile in profiles} == PROFILES,
        "Every requested profile needs an explicit hardware record",
    )
    for profile in profiles:
        require(profile.get("hardware_status") in STATUSES, "Invalid hardware status")
        require(
            profile.get("hardware_evidence_level")
            in ("unknown", "inferred", "observed", "confirmed"),
            "Invalid hardware evidence level",
        )
        require(
            type(profile.get("physical_actions_performed")) is bool,
            "Explicit physical action status required",
        )
        models = profile.get("device_models")
        require(
            isinstance(models, list) and all(text(model) for model in models),
            "Invalid device model evidence",
        )
        if profile["hardware_evidence_level"] in ("observed", "confirmed"):
            require(
                bool(models)
                and all(text(profile.get(field)) for field in EVIDENCE_FIELDS[:6]),
                "Observed hardware requires identified setup/authorization/capture references",
            )
        if profile["hardware_evidence_level"] == "confirmed":
            require(
                all(text(profile.get(field)) for field in EVIDENCE_FIELDS),
                "Confirmed hardware requires reproducible test/result references",
            )
        if profile["hardware_status"] == "PASS":
            require(
                bool(models)
                and profile["hardware_evidence_level"] == "confirmed"
                and profile["physical_actions_performed"],
                "Hardware PASS requires identified confirmed physical evidence",
            )
            require(
                all(text(profile.get(field)) for field in EVIDENCE_FIELDS),
                "Hardware PASS requires setup/authorization/capture/test/result references",
            )
    gates = record.get("critical_gates")
    require(
        isinstance(gates, dict)
        and set(gates) == GATES
        and all(status in STATUSES for status in gates.values()),
        "Every critical gate needs an explicit status",
    )
    for profile in profiles:
        if gates[profile["profile"] + "_hardware"] == "PASS":
            require(
                profile["hardware_status"] == "PASS",
                "Hardware gate cannot bypass its profile evidence",
            )
    accepted = (
        record.get("software_evidence_status") == "PASS"
        and all(profile["hardware_status"] == "PASS" for profile in profiles)
        and all(status == "PASS" for status in gates.values())
        and record.get("legacy_parity_status") == "PARITY"
        and not record.get("blockers")
    )
    if record["overall_status"] == "PASS" or record["production_ready"]:
        require(
            accepted,
            "Critical blocked/unverified criteria prevent readiness/production PASS",
        )
    if record["production_ready"]:
        require(
            record["overall_status"] == "PASS",
            "Production readiness requires overall PASS",
        )
    if record["release_authorized"]:
        require(
            text(record.get("release_approval_reference")),
            "Separate explicit release approval reference required",
        )
        require(
            accepted and record["overall_status"] == "PASS",
            "Release authorization cannot bypass critical readiness gates",
        )
    require(
        text(record.get("maturity")) and text(record.get("notes")),
        "Missing maturity/limitations",
    )
    return record


if __name__ == "__main__":
    root = Path(__file__).resolve().parents[1]
    record = validate_readiness(
        json.loads((root / "docs/development/pilot-readiness.json").read_text())
    )
    print(
        f"Readiness record consistent: {record['overall_status']}; hardware status is not established by software validation."
    )
