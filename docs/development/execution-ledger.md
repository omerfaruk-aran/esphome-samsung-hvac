# 10 — Execution Ledger

Codex must update this file after every meaningful task. Do not mark future phases complete in advance.

## Baseline
- New repository: https://github.com/omerfaruk-aran/esphome-samsung-hvac
- Legacy repository (read-only): https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus
- Legacy baseline verified 2026-11-01: `6b1966bea8db9344d23763454f9cebed015d15c5`
- Workspace package prepared: 2026-11-01
- Overall status: IN PROGRESS — P1 transport

## Status vocabulary
`NOT STARTED` / `IN PROGRESS` / `PASS` / `FAIL` / `BLOCKED` / `NOT TESTED`

## Phase summary

| Phase | Status | Commit / PR | Tests / Evidence | Notes |
|---|---|---|---|---|
| P0 Foundation | PASS | 2fc4012 / PR #1 | 24 tests, Ruff, both local firmware builds and all three CI jobs PASS | Six ADRs reviewed; hardware NOT TESTED |
| P1 Transport | IN PROGRESS | phase/p1-transport | pending | P0 gate accepted; production TX remains disabled |
| P2 NASA | NOT STARTED | — | — | — |
| P3 Non-NASA | NOT STARTED | — | — | — |
| P4 AC profiles | NOT STARTED | — | — | — |
| P5 EHS | NOT STARTED | — | — | — |
| P6 Catalog / FSV | NOT STARTED | — | — | — |
| P7 Migration | NOT STARTED | — | — | — |
| P8 CI / Docs | NOT STARTED | — | — | — |
| P9 Hardware / Release | NOT STARTED | — | — | — |

## Decision log
| Date | Decision | Rationale | ADR |
|---|---|---|---|
| 2026-11-01 | New independent repository; legacy repository read-only | Protect existing users | Pending P0 |

## Work log format
Add newest entries first. Each entry must include date/phase, summary/files, commit/PR, exact test commands/results, hardware tested or NOT TESTED, risks and next action.


## 2026-11-01 / P0 — baseline and packaging

- Files: all five required baseline inventory documents; 65 schema declarations, 27 device entity declarations, 64 NASA label entries; six proposed ADRs; inert root component; configuration tests and firmware fixtures; pinned CI workflow.
- Branch: phase/p0-foundation, based on aa6d1c885e979bfe1dc4a99994b9ec1fa82ad1fa. Commit/PR: not created yet.
- Baseline: legacy pinned/main both 6b1966bea8db9344d23763454f9cebed015d15c5; detached checkout, git status --porcelain empty. No legacy writes.
- Tools: local gh installed because it was missing; maintainer completed gh auth login; local account verified. No credentials were printed.
- Exact checks from repository/: ../.venv/bin/python -m pytest -q → 22 passed in 2.51s; ../.venv/bin/ruff check components tests → PASS. ESP-IDF and Arduino firmware builds PASS locally; CI/review pending.
- RED evidence: initial missing-component checks failed; nested source packaging spike produced four failures and 18 passes. After using flat private source modules, 22 tests passed. One initial negative test was too broad and passed on unrelated config rejection; it was strengthened to require the component's own diagnostic.
- Ruling: ESPHome 2026.10.1 does not copy arbitrary nested C++ component sources. Keep one public root and flat private modules; no loader patch. Cost: directory hierarchy deferred, independent logical engine state still required.
- Ruling: reuse the freshly cloned repository/ on its focused phase branch instead of another worktree, honoring workspace layout and avoiding redundant checkouts.
- Authorization: current explicit task authorizes focused commits, push, PR and squash merge after checks/review. No deploy, publish or live device action is authorized.
- Impacts: no application authentication or database migration; schema rejects unknown write settings and competing UART owners; logging contains protocol/config status only; public strings remain US English. Legacy migration remains unavailable.
- Hardware: NOT TESTED. No upload or bus writes. Risks: no parser, transport scheduler, control entities or device compatibility yet.
- Next: complete both firmware builds, independent diff/ADR review, update this ledger, then PR/CI gate before P1.


### P0 verification update

Commands below run from repository/, unless otherwise stated:

- `../.venv/bin/python -m pytest -q` → 22 passed (final local run 2.70s).
- `../.venv/bin/ruff check components tests` → PASS; `../.venv/bin/ruff format --check components tests` → PASS.
- `../.venv/bin/esphome config tests/yaml/esp-idf.yaml` and `../.venv/bin/esphome config tests/yaml/arduino.yaml` → PASS.
- `../.venv/bin/esphome compile --only-generate tests/yaml/esp-idf.yaml` and Arduino counterpart → PASS.
- `../.venv/bin/esphome compile tests/yaml/esp-idf.yaml` → PASS after packaging correction; first nested-source attempt failed missing protocols/selection.h.
- `../.venv/bin/esphome compile tests/yaml/arduino.yaml` → PASS.
- `../.venv/bin/python -m pip check` → no broken requirements.
- `git diff --check` → PASS.
- Environment: macOS arm64, Python 3.13, ESPHome 2026.10.1, ESP-IDF 5.5.6; board esp32dev. Firmware build success is not hardware validation.
- Baseline host commands, from workspace root: `clang++ -std=c++17 -include cstdint legacy-readonly/test/main_test_<name>.cpp legacy-readonly/components/samsung_ac/{protocol.cpp,protocol_nasa.cpp,protocol_non_nasa.cpp,util.cpp,debug_mqtt.cpp,log.cpp} -Ilegacy-readonly/test -o artifacts/p0/legacy-<name>` then `artifacts/p0/legacy-<name>` with cwd artifacts/p0. For nasa and non_nasa_queue: compile and run exit 0. For non_nasa: compile exit 1; not run. Baseline warnings were retained; no legacy fixes attempted. Baseline logs are local artifacts/p0/legacy-*.log.
- Local build/config/test logs: artifacts/p0/ outside the public repository. No captures or credentials were included.
- Hardware: NOT TESTED. P0 remains IN PROGRESS until independent review and PR/CI acceptance.


### P0 independent review and fixes

- Fresh read-only reviewer: no Critical finding; one Important configuration crash reproduced with unrelated HTTP JSON key uart.write. Fixed by requiring a mapping before reading action ID. Regression observed RED, then GREEN; genuine uart.write rejection retained. The attempted nested JSON dictionary variant was invalid for upstream HTTP JSON schema and was removed; two legal payload variants remain.
- Minor inventory correction: symbolic preset_entry name parameter is now shown as capabilities.presets.<preset>, with six actual keys and disabled default. No behavior test applies to this prose correction.
- Six ADRs reviewed and accepted for P0; source layout adjustment is justified by failed prototype and both successful firmware builds.
- Review rulings: arbitrary user lambdas/C++ are trusted configuration and outside declarative ownership enforcement; supported test target is ESP32 Arduino/ESP-IDF only; RX/TX/CRC/queue/profile implementation remains P1–P6; hardware/EHS/parity claims remain unverified; remote CI must pass separately before merge. Costs: these limits must remain explicit and are not runtime or hardware guarantees.
- Baseline general Non-NASA compile failure: test/main_test_non_nasa.cpp line 100 and following compare the Temperature struct with integers (invalid operands); retained without modification.

- Final review-fix suite: `../.venv/bin/python -m pytest -q` → 24 passed; Ruff check/format and diff --check PASS. Independent reviewer ran the pre-fix 22-test suite and reproduced the additional JSON-key failure separately.

- Git delivery: local identity/auth reverified; commit `3c112b9`, branch `phase/p0-foundation`, PR https://github.com/omerfaruk-aran/esphome-samsung-hvac/pull/1. GitHub Validation run 41289216799 queued/in progress. No merge attempted before CI success. Public ledger snapshot predates the PR URL; next phase snapshot will include delivery evidence.


### P0 initial remote CI failure and correction

- Validation run 41289216799: FAIL in all three jobs during actions/setup-python cache initialization. Exact error: no file matched **/requirements.txt or **/pyproject.toml. All test/firmware steps were SKIPPED, not passing.
- Root cause: the project intentionally uses requirements-dev.txt, while setup-python pip cache discovery defaults to other filenames. Fix: explicitly set cache-dependency-path: requirements-dev.txt in both setup-python steps. No protection or test was disabled; no implementation changed.
- Remote rerun required before P0 acceptance. Node 20 action runtime deprecation warning is nonfatal; Actions remain SHA-pinned.


## 2026-11-01 / P0 acceptance and P1 start

- P0 PR #1 merged by squash as 2fc40128ec1d9d84c7bcdff30260472a869c5469 after all three jobs in Validation run 41289298625 completed SUCCESS. No ruleset change, force push or direct main push.
- Prior phase branches retained; new phase/p1-transport created from fetched/fast-forwarded main at 2fc4012.
- P1 plan: plan/12_P1_IMPLEMENTATION.md. Public component layout remains flat private modules due the verified loader constraint.
- Hardware status remains NOT TESTED. P1 introduces no configurable active traffic; scheduler is exercised with simulated IO first.


## 2026-11-01 / P1 — transport implementation and local validation

- Changed modules: bus_types, frame_assembler, nasa_framer, non_nasa_framer, bus_transport, sole ESP32 adapter in samsung_hvac, schema ownership/native/direction checks; host test harness and provenance-labelled framing fixtures; transport documentation, README/NOTICE/changelog; corrected three P0 source-link offsets.
- ADRs: apply 0001–0006; no new public TX surface. CRC/XOR are framing validation only; typed semantics remain P2/P3.
- Exact commands from repository/: `../.venv/bin/python -m pytest -q` → 28 passed in 4.63s. This includes `tests/test_host.py`, compiling the real transport with -std=c++17 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer -g and executing eight assertion groups plus 100000 deterministic random bytes. Initial host test failed for absent source; corrupt-candidate recovery and shared-pin aliases each reproduced RED before fixes.
- `../.venv/bin/ruff check components tests`, `../.venv/bin/ruff format --check components tests`, `git diff --check` → PASS.
- `../.venv/bin/esphome compile tests/yaml/esp-idf.yaml` and `../.venv/bin/esphome compile tests/yaml/arduino.yaml` → PASS with direction GPIO exercised. Full schema/codegen suite also passes for both protocols/frameworks.
- Branch: phase/p1-transport from P0 squash2fc4012; commit/PR pending. Independent read-only review in progress. Remote CI not run yet.
- Ruling: runtime scope is native ESP32 UART on Arduino/ESP-IDF because nonblocking driver-capacity and completion reporting are verified in that adapter. Other platform/adapter configurations fail schema. Cost: no ESP8266/LibreTiny/USB UART support is claimed.
- Ruling: fixed 1502-byte frame buffer, FIFO4, RX/write budgets64, interbyte timeout50ms, silence100ms, queue/active deadline5000ms, DEguard3ms are conservative engineering limits derived from source/driver inspection and host tests. Cost: device-specific timing/throughput may require later measured adjustments; hardware is NOT TESTED.
- Impacts: authorization remains receive-only production; no raw data logging, stable entity IDs or migration surface; US English public strings. No database migration. No legacy changes or commands to live equipment.
- Hardware: NOT TESTED. Unavailable driver-overrun events and physical wiring/collision behavior cannot be inferred from counters. TX remains unavailable in YAML; host simulated IO is the only enabled scheduler.
- Next: complete independent review/fixes, focused PR and actual CI gate; only then P2.


### P1 independent review and fixes

- Independent reviewer: pre-fix 28-test suite PASS (4.29s), no Critical finding, one Important direction-GPIO ownership bypass. Four real ESPHome configurations (own RX, own TX, other UART RX, other root DE) reproduced RED despite allow_other_uses. Fix: native GPIO direction schema and explicit exclusion against every UART RX/TX pin and every other direction pin. Full regression suite rerun below.
- Final: minor (deferred): discarded_bytes does not count the start byte consumed when rejecting a length/integrity candidate. invalid_frames counts that candidate separately; the discarded-byte counter currently measures noise and timeout discards. Exact total RX loss accounting remains a later diagnostic improvement, including unavailable driver overrun events. It does not affect parser acceptance or any safety gate.
- Review rulings: P2–P6 own typed semantics/ACK/retries/profiles/write policy; physical timing, collisions and driver overrun losses remain NOT TESTED; physically interconnected different pins and user C++ are outside declarative ownership enforcement; unsupported platforms/adapters remain rejected; remote CI success is still a separate prerequisite. Cost: software/simulation results cannot establish equipment compatibility.

- P1 final fix validation: `../.venv/bin/python -m pytest -q` → 32 passed in 5.01s; Ruff check/format and git diff --check PASS. C++ adapter/core unchanged since both full firmware builds passed, except a callback lifetime comment. GPIO schema fixes are exercised by real codegen and four RED→GREEN alias regressions.


### P1 PR and dependency security correction

- P1 implementation commit fa66993, PR https://github.com/omerfaruk-aran/esphome-samsung-hvac/pull/3. Local Git identity/auth reverified before push. Remote Validation run 41290628879 started; no merge before checks complete.
- GitHub reported Dependabot alert #1 for the development dependency pytest: vulnerable tmpdir handling in versions below 9.0.3. Updated requirements-dev.txt from 8.4.2 to the first patched 9.0.3, installed locally and reran the suite. This is a dependency correction in the phase validation path; no runtime code or protections changed. Dependabot's separate PR #2 is not merged or modified by this task.

- Patched dependency validation: `../.venv/bin/python -m pytest -q` with pytest9.0.3 → 32 passed; `../.venv/bin/python -m pip check` → no broken requirements; git diff --check PASS.
