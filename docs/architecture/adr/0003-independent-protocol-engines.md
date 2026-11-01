# ADR-0003: Independent protocol engines

- Status: Accepted for P0 after independent read-only review; implementation gates remain separate.
- Date: 2026-11-01.
- Related phases: P0 and subsequent implementation gates.

## Context

The independent new integration must satisfy the workspace charter and preserve the read-only legacy baseline. Prefer small, testable modules and no unsupported compatibility or protocol claims.

## Decision

NASA and Non-NASA codecs, parsers, correlation/registration state and queues will be instance-owned and separate. The transport will serialize completed frames without interpreting message semantics.

## Alternatives considered

Global parser packets or shared mutable protocol state can leak state across devices and buses; mechanical legacy transplantation is rejected.

## Consequences

P0 selection enum is configuration only. No parser, ACK handling, registration or codec is implemented yet.

## Evidence and validation

See [legacy inventory](../../compatibility/legacy-feature-inventory.md), [ESPHome external components](https://esphome.io/components/external_components/), [UART integration](https://esphome.io/components/uart/), and the P0 execution ledger. The official ESPHome documentation establishes framework behavior; community legacy source establishes only reverse-engineered research evidence. Build/test results are recorded separately after execution. Hardware: NOT TESTED.
