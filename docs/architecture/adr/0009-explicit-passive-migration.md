# ADR-0009: Explicit optional migration without a compatibility facade

- Status: Accepted for P7 after independent read-only review of the migration scope.
- Date: 2026-11-01.

## Decision

Document every pinned legacy schema/entity declaration with its old default/source and current PARTIAL/UNSUPPORTED disposition. Provide codegen-tested software examples and preserve the user's option to keep the legacy integration. Do not retarget old component references, modify the legacy repository or automatically rewrite Home Assistant registries.

Do not ship a compatibility facade. Old writable keys would imply behavior that passive sensors and internal-only climate projections cannot provide. Protocol inference, capability inheritance, filter order, entity types and identity/restore also differ. Reject old unsupported keys and require explicit protocol/profile/address/capability decisions.

## Consequences

The supported observation subset can be evaluated without silently migrating commands or hardware. Full-control parity, physical recovery and Home Assistant identity/restore remain unverified. A compile-time id is not a promised registry identifier. Future migration improvements need evidence and tests before broadening this contract; no physical upload or control action is authorized by the guide.

See the complete migration inventory, entity matrix, optional migration guide and execution ledger. Hardware NOT TESTED.
