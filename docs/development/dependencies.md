# Dependency and license checks

The new integration is independent and MIT-licensed; preserve [LICENSE](../../LICENSE) and [NOTICE](../../NOTICE), including Lanwin's original attribution. Pinned legacy/nasactl research sources and every enabled message entry retain provenance. No third-party implementation source was transplanted. Missing/incompatible reference licenses are not permission to copy source.

Direct development tools are exactly pinned in requirements-dev.txt and recorded in [the machine-readable tool inventory](../../tools/dependency_licenses.json). check_repository compares installed versions and full installed license-file markers with this reviewed inventory:

| Tool | Version | Reviewed license evidence | Purpose |
|---|---|---|---|
| ESPHome | 2026.10.1 | MIT installed distribution LICENSE and upstream source | real loading/codegen/firmware |
| pytest | 9.0.3 | MIT installed distribution LICENSE and upstream source | automated tests; patched P1 dependency |
| Ruff | 0.14.3 | MIT installed distribution LICENSE and upstream source | Python lint/format |
| clang-format wheel | 23.1.3 | full Apache-2.0 LICENSE.md/COPYING.md in the downloaded/installed wheel; [PyPI release](https://pypi.org/project/clang-format/23.1.3/) | deterministic C++ formatting; no formatter binary bundled here |

Official Actions were inspected at immutable commits: [checkout v7.0.1](https://github.com/actions/checkout/commit/3d3c42e5aac5ba805825da76410c181273ba90b1), [setup-python v7.1.0](https://github.com/actions/setup-python/commit/6d4fd22b20465ccc0f7354476588e8df7c9ca0e0). Both action.yml runtime entries use node24 and full root MIT terms were read. Checkout credential persistence remains false. No remote branch-protection/security setting was bypassed or weakened.

P8 read-only GitHub audit on 2026-11-01 reported no open Dependabot alerts; security updates, secret scanning and push protection enabled. Optional non-provider-pattern/validity features were reported disabled and were not changed. This is a dated platform report, not proof that every vulnerability or secret pattern is covered. Weekly pip/Action update configuration and pip check complement review; they do not replace a full security audit.

Tool/source license checks do not constitute a complete transitive SDK/toolchain/embedded-binary SBOM or legal clearance for a future release. Before distributing binaries, review the actual build's dependencies and applicable notices, including upstream LLVM/toolchain/ESPHome/ESP-IDF and any selected libraries. Do not assume that this direct-tool inventory licenses every optional ESPHome component. No firmware artifact or release is published by this workflow.

For new reference reuse, record repository/commit/file, full license/notice terms, provenance and copied material; prefer independent implementation from facts. Keep the legacy repository read-only. See [catalog evidence](../protocol/message-catalog.md), [security policy](../../SECURITY.md) and [release gates](release-process.md).
