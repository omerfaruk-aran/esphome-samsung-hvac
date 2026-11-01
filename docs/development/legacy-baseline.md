# Legacy baseline and provenance

- Inspection date: 2026-11-01.
- New repository initial main: `aa6d1c885e979bfe1dc4a99994b9ec1fa82ad1fa`.
- Legacy pinned commit: `6b1966bea8db9344d23763454f9cebed015d15c5`.
- Newly observed legacy main from `git ls-remote`: `6b1966bea8db9344d23763454f9cebed015d15c5` (unchanged).
- Method: separate clone, detached checkout at the pinned SHA; read-only source inspection. No branches, commits, tags or remote mutations were made in legacy.
- Working-tree check: `git -C legacy-readonly status --porcelain` produced no output after inspection.
- Hardware: NOT TESTED. No firmware was uploaded and no bus traffic was sent.

## License review

The [pinned legacy LICENSE](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/LICENSE) contains the full MIT terms, copyright 2024 Ömer Faruk ARAN, and explicit original-development attribution to Lanwin. The new repository already has MIT LICENSE and NOTICE. Preserve both attribution and applicable notices if any implementation is later reused.

This phase adds original configuration/skeleton/test code and factual source inventories, with pinned references. No legacy protocol source or third-party implementation was copied. Other projects listed in the workspace plan were not imported. Missing/incomplete/incompatible license findings in that plan remain restrictions on future reuse, not a grant of permission. Community reverse engineering is not official Samsung documentation.

## Baseline validation limitations

Legacy test scripts write `test.exe` into their working directory, so those scripts were not run. Instead, clang++ compiled the original sources directly with outputs and runtime working directory outside the checkout. NASA and Non-NASA queue executables compiled and exited 0. The general Non-NASA executable failed compilation; see the execution ledger for the exact command and error. No source changes were made to make it pass. Existing CI runs firmware builds with host tests commented out. Captured-looking test strings lack identified model/capture provenance; do not label them hardware-confirmed.

Review detailed [feature](../compatibility/legacy-feature-inventory.md), [configuration](../compatibility/legacy-yaml-reference.md), [entity](../compatibility/legacy-entity-matrix.md) and [parity](../architecture/legacy-parity.md) inventories before implementing protocols.
