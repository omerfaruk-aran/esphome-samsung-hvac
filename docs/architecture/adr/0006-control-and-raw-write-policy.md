# ADR-0006: Control and raw write policy

- Status: Accepted for P0 after independent read-only review; implementation gates remain separate.
- Date: 2026-11-01.
- Related phases: P0 and subsequent implementation gates.

## Context

The independent new integration must satisfy the workspace charter and preserve the read-only legacy baseline. Prefer small, testable modules and no unsupported compatibility or protocol claims.

## Decision

P0 has no transmission, control entities, raw write option or FSV action. Later writes require explicit opt-in, a verified device family, per-device allowlist, validated finite ranges, rate/retry limits, timeout and readback.

## Alternatives considered

Unrestricted raw writes or permissive defaults cannot establish safety for unidentified equipment. Registration and polling are also active traffic and cannot be hidden in passive mode.

## Consequences

No write knobs are exposed in the foundation. Unknown options fail validation. No physical writes are authorized by software implementation or firmware compilation.

## Evidence and validation

See [legacy inventory](../../compatibility/legacy-feature-inventory.md), [ESPHome external components](https://esphome.io/components/external_components/), [UART integration](https://esphome.io/components/uart/), and the P0 execution ledger. The official ESPHome documentation establishes framework behavior; community legacy source establishes only reverse-engineered research evidence. Build/test results are recorded separately after execution. Hardware: NOT TESTED.
