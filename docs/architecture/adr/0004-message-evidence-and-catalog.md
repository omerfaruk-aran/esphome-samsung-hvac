# ADR-0004: Message evidence and catalog

- Status: Accepted for P0 after independent read-only review; implementation gates remain separate.
- Date: 2026-11-01.
- Related phases: P0 and subsequent implementation gates.

## Context

The independent new integration must satisfy the workspace charter and preserve the read-only legacy baseline. Prefer small, testable modules and no unsupported compatibility or protocol claims.

## Decision

Descriptors must declare protocol, code, type, payload length, source/destination roles, direction, access, units, scale, signedness, sentinels, device scope, evidence level, source/capture/test references and notes. Use a machine-readable catalog with schema validation in P6.

## Alternatives considered

Bare numeric dictionaries hide provenance; treating source labels as confirmed device behavior is rejected.

## Consequences

Confirmed requires repeatable device capture plus automated fixture; observed requires a reliable capture; community code alone remains inferred. Conflicts remain unknown and disabled. The P0 legacy label inventory is not an executable catalog.

## Evidence and validation

See [legacy inventory](../../compatibility/legacy-feature-inventory.md), [ESPHome external components](https://esphome.io/components/external_components/), [UART integration](https://esphome.io/components/uart/), and the P0 execution ledger. The official ESPHome documentation establishes framework behavior; community legacy source establishes only reverse-engineered research evidence. Build/test results are recorded separately after execution. Hardware: NOT TESTED.
