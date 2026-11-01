# Hardware pilot and readiness audit

## Current decision

**Readiness: BLOCKED. Hardware: NOT TESTED for NASA AC, Non-NASA AC and EHS.**

No identified, electrically verified and authorized pilot setup or local capture has been supplied for this integration. No firmware upload, equipment command, physical connection, live capture or hardware test has been performed. The maintainer requested public-source research in the absence of local recordings. Public compatibility reports remain research leads, not pilot results. An identified-setup request is pending; no response or elapsed time authorizes physical actions.

The [machine-readable record](pilot-readiness.json) pins software evidence to P8 squash `f01a2e1d6d06f99688b4af93db07a10455744397`, whose Git tree `ecffc7b8aa34a9354d7a5de792f94f179cd3cb4a` matches tested source `7a05ad448ebabf328c3c14b0dad087a2ef8bc5cf`. [CI run 41307094537](https://github.com/omerfaruk-aran/esphome-samsung-hvac/actions/runs/41307094537) passed 119 tests and all six jobs. This immutable baseline precedes the P9 documentation/validator changes; final P9 checks are recorded in the [execution ledger](execution-ledger.md). Compilation and synthetic fixtures establish software behavior only.

| Gate | Actual status | Evidence still required |
|---|---|---|
| Interface and topology | NOT TESTED | Identified isolated interface, electrical specifications, qualified verification and private topology record |
| NASA AC | NOT TESTED | Identified indoor/outdoor/controller models, firmware, passive capture and reproducible field comparisons |
| Non-NASA AC | NOT TESTED | Identified models/controller, passive Cmd20 captures, controller ownership and sleeping/wake behavior |
| EHS and zone isolation | NOT TESTED | Identified hydro/outdoor/controller, independently observed DHW/zone state and model-scoped semantics |
| Native API/entity availability | NOT TESTED | Actual encrypted API connection, entity values/availability and reconnect observations |
| Reboot/fault recovery | NOT TESTED | Authorized isolated reset/recovery observations, no flooding or unexpected TX |
| Physical rollback | NOT TESTED | Operator-validated backup/recovery path and observed restoration |
| Home Assistant identity/restore | NOT TESTED | Private registry comparison, restart/restore results and duplicate checks |
| Legacy parity | PARTIAL | Acceptance of all supported behavior and documented unsupported controls/telemetry |
| Verified control/readback | BLOCKED | Capture-backed approved write family, production policy/transport bridge and separate physical authorization |

Current maturity is receive-only development. Public climate controls, active polling/registration, FSV/raw writes and verified multi-zone targets are unavailable. All shipped message mappings remain INFERRED or UNKNOWN. No production-ready or stable-release claim is supported. Merging software documentation cannot accept these hardware gates.

## Prerequisites for a future pilot

Prepare a private reviewable setup record before proposing any physical action:

1. Record indoor/outdoor/hydro/controller and isolated interface model, firmware/reference, exact candidate commit, selected protocol and qualified operator. Do not include customer names, addresses, network details or secrets in public records.
2. Obtain model-specific electrical/interface specifications and qualified verification of isolation, voltage, grounding, polarity, termination and topology. Follow [interface safety](../wiring-safety.md). The example GPIOs are placeholders. Never connect an ESP32 directly to an unknown bus.
3. Identify the existing controller and establish that it remains responsible for equipment control. One physical UART/RS485 bus has one integration transport owner; profiles never create another reader/writer.
4. Document separately authorized scope: observation, upload, controlled reset, recovery, or equipment operation. Repository work, a model name, public source research and permission to observe do not authorize the other actions. This document grants none of them.
5. Review a concrete backup and privately validated recovery path before an authorized candidate is installed. Do not replace a working installation or assume rollback has been tested. Preserve the legacy installation independently.
6. Define duration, expected ordinary equipment behavior, stop conditions, observer and private evidence location. Stop on unexpected TX, bus faults/flooding, equipment errors, uncertain electrical conditions or lost recovery access. Escalate to the qualified operator; do not experiment with polarity or speculative commands.

## Proposed observation procedures, not performed

After prerequisites and explicit physical authorization, an operator can use the [receive-only capture procedure](../packet-capture.md) with the immutable candidate. Verify the candidate still exposes no public writer and no active polling/registration bridge. UART debug is a tap on the existing RX owner, with bounded chunks; logs are best effort and do not establish wire timing or losslessness. An independently verified isolated measurement is needed to substantiate physical timing or absence of TX. Validate complete packet CRC/checksum before treating logs as a fixture.

- **NASA AC:** identify exact packet source/destination and category, compare passive state with the existing controller, keep units/signedness/sentinels explicit and retain before/after evidence only for separately authorized ordinary controller actions. Check two identified devices independently if multi-split isolation is claimed. Disputed `0x4203`/`0x4204` meanings stay disabled until model-scoped evidence resolves them.
- **Non-NASA AC:** collect passive Cmd20 state with identified Celsius/Fahrenheit setting and source address. Observe ordinary sleeping/waking and existing-controller traffic without enabling registration, keepalive or B0 control. Host ACK/retry tests are not real registration or equipment-control evidence. Do not assign CmdF3 electrical scaling from an unverified report.
- **EHS:** identify hydro/outdoor/controller sources, separately compare DHW, water outlet/return and Water Law observations, and distinguish Zone 1/2 enable messages from temperature targets. If two zones exist, record each zone independently with operator-approved ordinary controller operation; prove no cross-publication and record physical controller state. Synthetic zero-TX tests do not prove physical zone isolation. Keep `0x42D6` and disputed room/target mappings disabled. Single-zone observations cannot establish multi-zone support.

For actual API/migration testing, keep credentials private, use the encrypted native API and compare explicit entity IDs, units, invalid/unavailable values and restart behavior. Record observed Home Assistant registry/restore outcomes rather than assuming IDs or state are preserved. Recovery/reset/fault/physical rollback exercises require a separate reviewed and authorized scope; do not induce a live bus fault to satisfy a checklist. See [migration and rollback](../compatibility/migration-guide.md).

## Evidence promotion and acceptance

Record each profile separately in [pilot-readiness.json](pilot-readiness.json): exact models, controller, firmware, interface/topology, authorization, capture, reproducible test and result references; explicitly state whether this integration's physical actions were performed. A third-party capture may inform research, but cannot be relabeled as this pilot. Never combine a model reported by one person with another person's unidentified capture.

`observed` requires reliable identified capture/setup evidence; `confirmed` additionally requires repeatable device evidence and an automated fixture/test. Catalog promotion must retain raw type/width, direction, units, scale, signedness, sentinels and device scope. Re-check [catalog rules](../protocol/message-catalog.md). Remove or redact private data before publishing any minimal fixture; keep authorization and private capture records private with non-sensitive references.

Run `python tools/pilot_readiness.py` and `python tools/check_repository.py` after editing records. Validation rejects incomplete promotions and unsupported production claims, but cannot prove references are truthful or that a physical test happened. Independent evidence review and exact result commands remain mandatory. NOT TESTED, BLOCKED and PARTIAL are actual unresolved states, not passing measurements. Follow the [release gates](release-process.md); no release is authorized here.
