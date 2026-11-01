# ADR-0005: Explicit protocol selection

- Status: Accepted for P0 after independent read-only review; implementation gates remain separate.
- Date: 2026-11-01.
- Related phases: P0 and subsequent implementation gates.

## Context

The independent new integration must satisfy the workspace charter and preserve the read-only legacy baseline. Prefer small, testable modules and no unsupported compatibility or protocol claims.

## Decision

Require protocol: nasa or non_nasa. No auto selection or active probe in P0. Any future auto detection starts passive and requires its own fixtures and review.

## Alternatives considered

Legacy global auto-lock behavior complicates recovery and multiple-bus isolation. Active detection could transmit to unknown equipment.

## Consequences

Users make an explicit per-bus choice. Invalid, missing and auto choices must fail validation. This is a new API; legacy is unchanged and no migration is promised.

## Evidence and validation

See [legacy inventory](../../compatibility/legacy-feature-inventory.md), [ESPHome external components](https://esphome.io/components/external_components/), [UART integration](https://esphome.io/components/uart/), and the P0 execution ledger. The official ESPHome documentation establishes framework behavior; community legacy source establishes only reverse-engineered research evidence. Build/test results are recorded separately after execution. Hardware: NOT TESTED.
