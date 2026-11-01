# Manual release gates

There is no release, version tag, publishing workflow or approved production-ready artifact. Current maturity is receive-only development with INFERRED mappings and unverified hardware. Build success cannot substitute for model/interface/readback evidence.

Before any separately authorized release proposal, review a concrete immutable candidate and record:

- All required CI jobs, host/parser/catalog/schema/integration/firmware checks and independent review results.
- Exact scope, known unsupported controls/families, complete legacy dispositions and user-facing migration/rollback guidance.
- Identified hardware/interface/controller/firmware evidence for every claimed profile; unavailable coverage explicitly NOT TESTED and critical blockers visible.
- Device-specific type/unit/scale/signedness/sentinel/range facts and fixtures; no unverified write family or speculative live command.
- Actual API/entity availability, reboot/fault/recovery, Home Assistant identity/restore and physical rollback results where claimed.
- Dependency/security/license review of the actual candidate and any distributed binary/SBOM/notices; no secrets, private logs or test credentials.
- Changelog and maturity label matching evidence, maintainer authorization and a reviewed recovery plan.

Stable publication requires accepted P9 and legacy parity; neither is accepted now. An alpha/beta description must state missing hardware and unsupported families/features and must not imply production control support. Do not release a candidate with critical evidence/safety criteria blocked. Preparing this process does not authorize a tag, GitHub release, firmware upload or equipment action.

If an authorized pilot or later release shows a regression, stop rollout and use the privately validated recovery path. Do not silently fall back to a second UART owner or indefinite retries. Record the exact observed failure, affected model/commit, sanitized fixture and follow-up acceptance result. Current physical rollback and release readiness remain NOT TESTED/BLOCKED.

See [migration](../compatibility/migration-guide.md), [compatibility evidence](../compatibility/ehs-matrix.md), [capture procedure](../packet-capture.md), [testing](testing.md) and [execution ledger](execution-ledger.md).
