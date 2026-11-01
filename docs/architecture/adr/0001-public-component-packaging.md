# ADR-0001: Public component packaging

- Status: Accepted for P0 after independent read-only review; implementation gates remain separate.
- Date: 2026-11-01.
- Related phases: P0 and subsequent implementation gates.

## Context

The independent new integration must satisfy the workspace charter and preserve the read-only legacy baseline. Prefer small, testable modules and no unsupported compatibility or protocol claims.

## Decision

Use one public samsung_hvac root with internal C++ modules and later protocol/profile modules. Dependencies start with uart; multi-instance configuration is explicit.

## Alternatives considered

Multiple public components require dependency and ownership wiring before there is a stable profile API; defer that complexity. A monolithic protocol/profile file would recreate legacy coupling.

## Consequences

The initial nested protocols/selection.cpp prototype failed four codegen tests: ESPHome 2026.10.1 only copies direct component resources. Use flat private files (protocol_selection.cpp/.h) with separate responsibilities and namespaces. The root dump_config references a function implemented in that separate translation unit, so both firmware links must include it. Do not patch ESPHome loader internals. Logical protocol/profile boundaries need not be public components or nested source directories. See esphome/loader.py ComponentManifest.resources in the pinned installation.

## Evidence and validation

See [legacy inventory](../../compatibility/legacy-feature-inventory.md), [ESPHome external components](https://esphome.io/components/external_components/), [UART integration](https://esphome.io/components/uart/), and the P0 execution ledger. The official ESPHome documentation establishes framework behavior; community legacy source establishes only reverse-engineered research evidence. Build/test results are recorded separately after execution. Hardware: NOT TESTED.
