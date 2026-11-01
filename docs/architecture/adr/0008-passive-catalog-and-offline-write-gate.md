# ADR-0008: Passive catalog observers and an offline write gate

- Status: Accepted for the P6 passive/offline software scope after independent review fixes; hardware/live writes remain blocked.
- Date:2026-11-01.

## Decision

Use a strictly validated versioned catalog with explicit evidence and protocol-specific layouts. Keep named unresolved semantics disabled. Permit explicit unitless unsigned raw observations of disputed words without assigning them to room/zone fields. Custom descriptors remain UNKNOWN and cannot manufacture catalog verification. Add bounded NASA scalar observers sharing the root's sole RX owner; do not enable active polling or writes.

An isolated noncopyable Guard validates future write intents against immutable opt-in/family/device/code/range/rate/deadline/readback policy. It has no live bridge or bus frame encoder. No approved family/policy or public enable option is shipped, and all catalog access remains passive/disabled. Real writes require the separate evidence and authorization gates in ADR0006.

## Consequences

P6 can establish catalog/schema/scalar/public passive entity and offline policy correctness. It cannot establish equipment actuation, actual readback, broadcast timing, FSV safety or write readiness. Raw observations are useful research data while disputed semantic mappings remain disabled. Non-NASA generic raw entities remain unavailable; their independent profile and descriptive Cmd20 catalog are retained. Future active integration must enforce one policy owner per target and bridge real TX/fresh observation events before any physical pilot.

See the catalog, advanced-message/write-policy notes, parity tracker and execution ledger for evidence and exact tests. Hardware NOT TESTED.
