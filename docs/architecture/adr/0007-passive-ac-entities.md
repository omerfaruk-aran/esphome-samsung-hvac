# ADR-0007: Passive AC entities and internal climate projection

- Status: Accepted for the P4 passive software scope after independent review; hardware/control gates remain separate.
- Date: 2026-11-01.

## Context

The available evidence is pinned reverse-engineered source and synthetic fixtures. No identified device capture or authorized hardware write is available. ADR-0006 forbids enabling unverified writes. ESPHome 2026.10.1 ClimateTraits requires Off and target temperature and provides no read-only climate flag.

## Decision

Expose passive state through standard sensors, binary sensors and text sensors, with explicit device IDs/addresses/capabilities. Keep optional climate projections internal only and reject external publication until guarded, verified control support exists. Never advertise writable entities that silently do nothing. Keep disputed NASA room codes disabled. Separate profile conversion from ESPHome publication and leave UART ownership in the root.

## Consequences

P4 software can test state isolation, capability validation and disabled command boundaries; it cannot claim full legacy climate/control parity, device support or Home Assistant registry/restore equivalence. Common read-only observations are useful without a HACS runtime dependency. Internal climate retains the last complete representable observation; separate diagnostic entities expose invalidation. Active control requires capture-backed device policies and a later scheduler completion/cancellation bridge, not merely a YAML switch.

## Validation

See [profile notes](../../development/ac-profiles.md), [mapping evidence](../../protocol/ac-profile-mappings.md), the entity/parity matrices and execution ledger. SDK headers inspected locally: climate_traits.h (mandatory Off/target and feature flags) and climate.h (control/traits/publish API). Hardware NOT TESTED.
