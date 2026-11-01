# ADR-0002: UART ownership and multiple buses

- Status: Accepted for P0 after independent read-only review; implementation gates remain separate.
- Date: 2026-11-01.
- Related phases: P0 and subsequent implementation gates.

## Context

The independent new integration must satisfy the workspace charter and preserve the read-only legacy baseline. Prefer small, testable modules and no unsupported compatibility or protocol claims.

## Decision

Each physical UART has one samsung_hvac owner. Profiles never inherit UARTDevice. Reject duplicate root owners and competing UART devices through ESPHome final validation. Separate UART buses may have separate roots.

## Alternatives considered

Independent per-profile UART readers split frames and violate scheduling; a global singleton prevents valid multi-bus operation.

## Consequences

P0 registers an inert UARTDevice. RX, bounded scheduler, direction control and timing belong to P1; no transport safety claim is made from registration alone.

## Evidence and validation

See [legacy inventory](../../compatibility/legacy-feature-inventory.md), [ESPHome external components](https://esphome.io/components/external_components/), [UART integration](https://esphome.io/components/uart/), and the P0 execution ledger. The official ESPHome documentation establishes framework behavior; community legacy source establishes only reverse-engineered research evidence. Build/test results are recorded separately after execution. Hardware: NOT TESTED.
