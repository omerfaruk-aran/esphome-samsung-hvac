# 10 — Execution Ledger

Codex must update this file after every meaningful task. Do not mark future phases complete in advance.

## Baseline
- New repository: https://github.com/omerfaruk-aran/esphome-samsung-hvac
- Legacy repository (read-only): https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus
- Legacy baseline verified 2026-11-01: `6b1966bea8db9344d23763454f9cebed015d15c5`
- Workspace package prepared: 2026-11-01
- Overall status: IN PROGRESS — P7 migration

## Status vocabulary
`NOT STARTED` / `IN PROGRESS` / `PASS` / `FAIL` / `BLOCKED` / `NOT TESTED`

## Phase summary

| Phase | Status | Commit / PR | Tests / Evidence | Notes |
|---|---|---|---|---|
| P0 Foundation | PASS | 2fc4012 / PR #1 | 24 tests, Ruff, both local firmware builds and all three CI jobs PASS | Six ADRs reviewed; hardware NOT TESTED |
| P1 Transport | PASS | 71155da / PR #3 | 32 tests, both local builds and all three CI jobs PASS | DE ownership fixed; pytest9.0.3; hardware NOT TESTED |
| P2 NASA | PASS | a66a87b / PR #4 | 33 tests, both local builds, all three CI jobs PASS | Review clean; hardware NOT TESTED; active reads unavailable |
| P3 Non-NASA | PASS | 7069274 / PR #5 | 34 tests, both local builds, all three CI jobs PASS | Review races fixed; hardware NOT TESTED; passive production |
| P4 AC profiles | PASS | fad669e / PR #6 | 51tests, both local frameworks and all three CI jobs PASS | Read-only entities/internal climate; control evidence unresolved |
| P5 EHS | PASS | 9fdce0a / PR #7 | 63tests, both frameworks and all three CI jobs PASS | Evidence-backed passive fields; disputed zones stay disabled |
| P6 Catalog / FSV | PASS | d87914a / PR #8 | 98tests, both local frameworks and CI41301108612 three jobs PASS | Passive typed entities; active writes unavailable |
| P7 Migration | IN PROGRESS | phase/p7-migration | pending | Explicit baseline dispositions and compiled migration examples |
| P8 CI / Docs | NOT STARTED | — | — | — |
| P9 Hardware / Release | NOT STARTED | — | — | — |

## Decision log
| Date | Decision | Rationale | ADR |
|---|---|---|---|
| 2026-11-01 | New independent repository; legacy repository read-only | Protect existing users | ADR0001/0006 accepted in P0 |

## Work log format
Add newest entries first. Each entry must include date/phase, summary/files, commit/PR, exact test commands/results, hardware tested or NOT TESTED, risks and next action.


## 2026-11-01 / P7 — explicit legacy migration dispositions

- Files: complete eight-column legacy-yaml-reference.md and machine-readable legacy-migration.json for65pinned schema declarations; all27entity dispositions, feature/parity tracker; migration-guide.md; four examples (NASA AC, Non-NASA AC, EHS, multi-split); tests/test_migration.py and five legacy-key rejection cases; ADR0009, README/changelog.
- Tests first: missing inventory/examples observed RED; real examples then generated through ESPHome with actual !secret references and ephemeral non-deployed secrets outside the repository. Test failure output masks values. No real credential or secrets.yaml committed. Explicit API encryption/Wi-Fi references, fixed IDs, typed state-only entities and capability/address changes documented.
- Independent review: no Critical/Important; readonly AST verification matched all65old keys/requirements/defaults/source lines to pinned legacy HEAD.10migration-related tests PASS in2.66s. Two Minor issues closed: stale UNKNOWN prose updated to PARTIAL/UNSUPPORTED dispositions; test now compares all Markdown/JSON key/source/requirement/default/equivalent/status/note fields, not only counts/notes. A preset helper name row was clarified as unsupported display alias, distinct from standard entity names. No component behavior changed.
- Final `../.venv/bin/python -m pytest -q` →108PASS in35.42s; `../.venv/bin/ruff check components tests`, `../.venv/bin/ruff format --check components tests`, `git diff --check` →PASS. Six sanitizer C++ executables,80config tests,17catalog tests and5migration inventory/real-codegen tests. Log artifacts/p7/pytest-reviewed.log.
- Required existing firmware matrix: `../.venv/bin/esphome compile tests/yaml/esp-idf.yaml` and Arduino counterpart →PASS, exit0. Logs artifacts/p7/build-{idf,arduino}.log. Additional relevant full migration builds with actual Wi-Fi/encrypted API modules: `../.venv/bin/python ../artifacts/p7/build_migration.py nasa-ac` and non-nasa-ac counterpart →PASS, exit0, using temporary non-deployed secrets outside repo; logs migration-{idf,arduino}.log. No upload/network connection to a device.
- Documented Git option independently tested: encrypted-API EHS example using remote refd87914a833316e3b5bb63cb2cc7189a8e4a8350c cloned/fetched via ESPHome and `compile --only-generate` →PASS. Log artifacts/p7/git-reference.log. New repo reverified PUBLIC, isForkfalse, defaultmain. Legacy SHA6b1966b/worktree unchanged.
- Guide explicitly covers protocol/address conversion, changed writable entity types, defaults/inheritance/preset aliases, typed/raw/filter order, sensor-only offset versus climate, missing/invalid/last-observed state, inherited update_interval removal, native API secrets, registry/restore limits and private backup/authorized rollback boundary. Official ESPHome2026.10.1 API/YAML/sensor docs checked for encryption/secrets/filter behavior. No auto-converter/facade, physical action or forced migration. No full PARITY asserted.
- Authorization/validation/logging/audit/localization/migration: unsupported old controls/debug/keepalive/poll settings fail by key; native entity transport replaces no legacy debug audit stream; no raw logs/customer data/credentials published, public content English. Hardware, actual HA identity/restore/network behavior and physical rollback NOT TESTED. Production writes remain unavailable.
- Local P7 software gate PASS, CI/push/PR/squash pending before P8. Branch phase/p7-migration. ADR0009 records the reviewed no-facade decision; no externally consequential change to the legacy API or repository.


## 2026-11-01 / P6 — typed catalog, passive observers and offline write policy

- Modules: catalog.py + version1message_catalog.json (44entries), typed_message.h/.cpp, message_entity.h/.cpp, message_config.py, write_policy.h/.cpp; separate bounded32message observers alongside8profiles in the existing sole root owner. No UART/protocol state ownership moved. Both firmware fixtures instantiate numeric/binary/FSV/custom/precision observers; Arduino also has a separate NASA UART alongside Non-NASA.
- Catalog: strict standard-library schema/semantic/evidence validation, protocol-specific widths/layouts, scale/offset/signedness/raw sentinel/range/value bounds, sources/capture/test/model fields. Shipped access values only passive/disabled and evidence only inferred/unknown. NASA AC/EHS/selected passive FSV and descriptive Non-NASA Cmd20 entries; no model/capture claim or third-party implementation transplant.
- Public interface: NASA Notification/Response scalar observers only, exact source and roles, explicit catalog or custom metadata, no poll/write option. Named4203/4204/42D6 semantics remain disabled; custom raw unsigned/unitless/scale1offset0 observation does not assign room/zone state. Custom evidence stays UNKNOWN. Non-NASA generic observers deliberately unavailable; its product profile and descriptive catalog stay isolated.
- TDD: missing catalog/core/observer modules and absent YAML messages schema observed RED before implementation. Malformed JSON typing/non-scalar values reproduced RED and now reject with ValueError. Guard copy/move prevention regression RED→GREEN; no duplicate ticket/rate history through copy. Initial full suite96PASS, both frameworks compiled; later review exposed additional correctness cases.
- Independent review: no Critical/live TX bypass; two Important findings and one Minor. SDK FloatLiteral emitted float-suffixed scale/offset despite double contract (scale1e-100 underflow, offset4294967295 rounding). Real codegen regression RED→GREEN with numeric-only validated double RawExpression; both firmware precision probes compile. Fixed actual decoded physical range/representability: scale1000,min=max0.0001 could admit raw0 outside allowlist. Both outside-range and unequal decoded-requested cases RED→GREEN; re-decoded value now rechecks strict min/max, step and roundoff-relative equivalence, without enlarging physical bounds. Strict decoded0.3boundary assertion also passes. Minor mask1/shift7 schema failure reproduced RED→GREEN via normalized contiguous mask, lower-bit/boolean/temperature extraction and extracted-domain checks. Reviewer ran22catalog/host tests PASS before regression fixes; no final firmware rerun by reviewer.
- Offline Guard: immutable opt-in/verified-family/unicast/device/code/range/step policy, at most8rules, one intent, minimum1000ms rate measured from preparation/actualTX, explicit preparation/readback deadlines, no retries, sticky uncertain/send faults, matching ticket and exact raw source/destination/code readback; ACK nonterminal. Positive policies exist only in synthetic tests. No root Guard, approved family, bus-frame encoder, public opt-in or actual transport bridge is shipped. OBSERVED_MATCH is state convergence, not command causality. Physical writes remain BLOCKED pending capture-backed policies/authorized pilot.
- Final `../.venv/bin/python -m pytest -q` →98PASS in17.24s (six ASan/UBSan C++ programs,75real YAML/codegen cases,17catalog tests). Log artifacts/p6/pytest-final.log. An earlier final attempt had97PASS/1test-setup FAIL because the new offset assertion did not set its offset input; corrected the fixture and reran. No product regression hidden.
- `../.venv/bin/ruff check components tests`, `../.venv/bin/ruff format --check components tests`, `git diff --check` →PASS. `../.venv/bin/esphome compile tests/yaml/esp-idf.yaml` and Arduino counterpart →PASS, exit0 after product fixes, including scale1e-100/offset4294967295double probes. Logs artifacts/p6/firmware-{idf,arduino}-reviewed.log. Later changes only tests/docs/ledger. Python3.13.7, ESPHome2026.10.1, pytest9.0.3. No upload/live hardware action; legacy SHA/worktree unchanged.
- ADR0008 accepted for passive/offline scope; advanced/catalog/write-policy notes, parity/entity matrices and changelog updated. User authorization/validation/logging/audit/localization/migration: no transmission enablement, strict typed/source/unit/unknown-key validation, no raw payload/credential logging, English public content and explicit PARTIAL legacy custom behavior. Sensor float32 precision/sentinel uncertainty and passive FSV availability documented. Hardware/actual write readback/real HA publication/registry/restore NOT TESTED.
- Local P6 software gate passes after reproducing review fixes; live-control readiness is not accepted. Branch phase/p6-typed-catalog; maintainer-auth push/PR/CI/squash gate pending before P7. No routine approval or protection bypass.


## 2026-11-01 / P5 — EHS passive profile and local acceptance

- Modules: ehs_profile.h/.cpp pure bounded state, ehs_entities.h/.cpp, ehs_config.py; stateless PacketConsumer interface and small AC/root routing/schema adaptation within the existing shared eight-device bound. No protocol state machine sharing or new UART path. Firmware fixture instantiates EHS numeric/binary/text alongside NASA AC; Non-NASA Arduino coverage retained.
- Evidence: pinned MIT legacy6b1966b and MIT nasactl629c43fce1c6a0595b5070028e34cf4ad0f25386, full copyright2026PaoloStivanin terms verified. Public Water Law269, remote-sensor261, EHS294 and selected82model reports inspected. No unlicensed implementation source imported and no raw customer logs published. Model reports stay separate from compatibility proof.
- Tests first: missing EHS source caused host RED; EHS YAML failed before profile schema existed. Independent implementation then passed source/category/scaling/negative-shift/enum/outdoor/zone-isolation cases. Synthetic hydro/outdoor fixtures use independently generated CRCs. Integration feeds simulated UART through production Transport/Framer/NASA Engine into EHS state and asserts zero TX/direction enables.
- Mappings/evidence/profile notes and EHS compatibility matrix added; legacy entity/parity rows updated. Targets4235/4247 deliberately follow legacy unsigned reads; current/return/Water Law follow explicit nasactl signed interpretation. Source differences/sentinel uncertainty documented; no universal out-of-range semantics. Disputed4203/4204/42D6 disabled, generic outlet target never aliases a zone;4119/411E publish only configured source-labelled enable observations. No verified multi-zone support claimed.
- Independent review: no Critical/Important, one Minor missing golden assertions for power/pump/defrost/mode. Added explicit expected values; no product bug found. Reviewer ran63tests PASS in13.97s, independently checked both fixture lengths/CRC using binascii.crc_hqx, diff check PASS, legacy clean; did not rerun firmware. Reviewer judged local P5 inferred read-only software gate PASS.
- Final `../.venv/bin/python -m pytest -q` →63PASS in14.54s, five ASan/UBSan C++ programs and58 real ESPHome YAML/codegen cases. `../.venv/bin/ruff check components tests`, `../.venv/bin/ruff format --check components tests`, `git diff --check` →PASS. Log artifacts/p5/pytest-reviewed.log. Existing P4/Non-NASA tests retained.
- `../.venv/bin/esphome compile tests/yaml/esp-idf.yaml` and Arduino counterpart →PASS, exit0, after all product changes; later changes only documentation/assertions. Logs artifacts/p5/build-{idf,arduino}.log. No upload/network/hardware action. Python3.13.7, ESPHome2026.10.1, pytest9.0.3. Legacy SHA unchanged and working tree clean.
- Authorization/validation/logging/audit/localization/migration: no writable schema or TX handle, strict address/profile/zone/duplicate/unsupported-key validation, no new payload logging or secrets; English public content, per-entity PARTIAL migration disposition. Hardware/model compatibility/real HA publication/registry/restore NOT TESTED. Energy, valves, ambiguous zone targets, active reads and FSV writes remain unavailable.
- No new ADR needed: consumer dispatch refines ADR0003 without moving UART ownership; ADR0004/0006 govern inferred evidence and writes. Branch phase/p5-ehs-readonly; commit/PR/CI pending. Next prerequisite: verified maintainer identity/auth, PR checks, squash merge before P6.


## 2026-11-01 / P4 — passive AC profiles and final local verification

- Modules/files: ac_profile.h/.cpp (pure per-device state and disabled offline control assessment), ac_entities.h/.cpp (standard ESPHome entities/internal climate), ac_config.py, root schema/routing; AC golden fixtures/metadata, host/schema/codegen tests, both API-enabled firmware fixtures; ADR0007, mapping/profile documentation, legacy entity/parity matrices, README/changelog.
- TDD: initial host test failed without profile sources; real profile YAML initially failed before devices schema existed. Source-backed state/capability/address/category cases then passed. Synthetic golden frames decode through the real codecs into profiles; no captured data or compatibility is claimed. User confirmed no local captures and authorized continuing with public evidence.
- Arduino initially FAILED because bit(b) and DISABLED macros collided with new symbols. Host compile with the exact definitions reproduced RED; renamed field_mask and CONTROL_DISABLED → GREEN. Both full frameworks rerun after the fix.
- Independent review found one Important issue: Non-NASA invalid swing enum1 could be published as Off. Regression valid BOTH→raw enum1 observed RED, strict valid-enum whitelist → GREEN. Reviewer confirmed correction and no open Critical/Important/Minor findings; independent suite51PASS in10.72s and final targeted host4PASS in4.72s. Reviewer did not rerun firmware.
- Final stable-snapshot commands: `../.venv/bin/python -m pytest -q` →51PASS in11.10s; `../.venv/bin/ruff check components tests`, `../.venv/bin/ruff format --check components tests`, `git diff --check` →PASS. Tests include4C++ programs under ASan/UBSan and47 real ESPHome schema/codegen cases. Logs artifacts/p4/pytest-reviewed.log and regression logs.
- `../.venv/bin/esphome compile tests/yaml/esp-idf.yaml` and `../.venv/bin/esphome compile tests/yaml/arduino.yaml` →PASS, exit0, after review fixes; logs artifacts/p4/firmware-{idf,arduino}-reviewed.log. These compile native API entities with network:{} but no network credentials/actual connection; no deployment/upload. ESPHome2026.10.1, Python3.13.7, pytest9.0.3.
- Review ruling: P4's supported baseline rows, representative profiles, unsupported-capability rejection and synthetic state/disabled-command boundaries meet the passive software gate once CI passes. Full legacy climate/control parity remains PARTIAL; P3 command lifecycle fixtures are not P4 public-control proof. This limitation does not block P5 read-only work. No public controls are implemented/enabled, disputed NASA room codes remain UNKNOWN, hardware/HA registry/restore remain NOT TESTED.
- Authorization/validation/logging/audit/localization/migration: no live actions or TX enablement; explicit IDs, protocol/address/capability validation; no raw payloads or secrets logged; English-only public strings and warning on internal climate control attempts; every legacy entity disposition remains tracked without silent migration. Verified MIT provenance; no third-party implementation source transplanted; legacy working tree unchanged.
- Local identity/auth/remote verified before delivery: authenticated maintainer via local gh/keyring, expected independent origin/main7069274. No connector identity or protection bypass. Branch phase/p4-ac-profiles; commit/PR/CI pending. Hardware NOT TESTED. Next prerequisite: PR checks and squash merge before P5.


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

- Final P1 head 98d06073cce5a942154ef16ab786c662fb7c4a7f; CI run 41290705727. Initial fa66993 run canceled automatically by the configured PR concurrency group after the dependency fix; not counted as PASS. Final host/schema and ESP-IDF jobs passed; Arduino still running at this checkpoint.


## 2026-11-01 / P1 acceptance and P2 start

- PR #3 merged by squash as71155da4cb611d9b193e370d0b552923b69f22d0 after all three jobs in CI41290705727 SUCCESS. Source head98d0607 matched at merge; protections unchanged. P2 branch created from fetched/fast-forwarded latest main.
- P2 additional source research: nasactl629c43fce1c6a0595b5070028e34cf4ad0f25386 full MIT license verified. It creates READ packets with zero typed scalar payloads; LongVariable wire integers are big-endian; Structure consumes remaining payload. These are reverse-engineering source observations, not Samsung documentation or device confirmation. No third-party implementation source is imported.
- P1 deferred minor diagnostic accounting remains documented; no critical criterion was marked PASS on hardware evidence. Hardware remains NOT TESTED.


## 2026-11-01 / P2 — NASA typed packets and read lifecycle

- Files: nasa_packet.h/.cpp, nasa_read_queue.h/.cpp, nasa_engine.h/.cpp; passive root integration, NASA synthetic fixtures and metadata, sanitizer tests; protocol notes, README/changelog.
- Initial new-module host test failed before implementation. Typed parser, encoding, read queue, correlation/retry/quarantine/rollover tests then passed. An additional delayed-event regression reproduced an overdue response being accepted when tick had not run; fixed by enforcing deadlines in packet and TX-completion entry points.
- Preliminary full suite: `../.venv/bin/python -m pytest -q` →33 passed in5.63s, including separate sanitizer transport and NASA executables. Additional CRC-valid inner fuzz cases added afterward; final results below.
- Ruling: raw scalars stay unsigned; explicit signed helper does not assign product semantics. Structure must be last and is retained losslessly. Unknown/disputed mappings stay unassigned. Cost: unsupported header/structure variants are rejected until evidence supports them.
- Ruling: production remains passive; read lifecycle is host-tested but does not submit to UART or enable TX. Future active integration must bridge actual completion/cancellation and policy. Cost: no user-facing active polling is claimed in P2; this boundary must be assessed against the phase gate rather than silently called complete.
- Hardware NOT TESTED. No live device actions, captures or speculative writes. No third-party implementation source copied. Branch phase/p2-nasa; commit/PR pending.


### P2 review and final local verification

- Independent read-only review: no material findings; final snapshot33pytest PASS in5.80s, diff --check PASS, all17 fixture envelopes/CRCs/provenance independently checked with binascii. Signedmin/max8/16/32, maximum1484-byte opaque Structure within1502-byte frame,255 messages and all7 DataType categories reviewed.
- `../.venv/bin/esphome compile tests/yaml/esp-idf.yaml` and Arduino counterpart → PASS after the encoder alias fix. Logs: artifacts/p2/firmware-{idf,arduino}-final.log. The alias regression observed RED then GREEN; encoding a packet into its own storage uses a bounded scratch frame to preserve structure bytes.
- Host coverage: seven NASA assertion groups and10000CRC-valid inner fuzz cases, plus P1 transport groups/100000 random bytes, under ASan/UBSan. The final full suite is33pytest tests, not33 hardware cases.
- Review ruling: active polling YAML/scheduler integration is not a P2 acceptance requirement in plan/05; the implemented read lifecycle is host-tested and production remains passive. Cost: do not claim active-read latency, bus starvation or physical read success; later active integration must bridge actual TX completion/cancellation and explicit policy.
- Review limits: hardware compatibility/timing/captures NOT TESTED; synthetic fixtures do not establish universal semantics; disputed message meanings unassigned. Firmware builds were verified by implementer, not independently rerun by reviewer. No phase marked hardware PASS.
- No new ADR required; existing independent-engine/evidence/write-safety decisions apply. No live hardware actions or legacy modifications.

- P2 delivery: ee4345a (see git for full SHA), branch phase/p2-nasa, PR https://github.com/omerfaruk-aran/esphome-samsung-hvac/pull/4. Final local suite33PASS in6.11s; Ruff/diffPASS. CI41292696348 host/schema/codegen SUCCESS, firmware jobs pending. No merge before final CI gate.


## 2026-11-01 / P2 acceptance and P3 start

- P2 PR#4 merged by squash as a66a87b44f88a8557c12a9c641ce1d6f3df662cd after CI41292696348 all three jobs SUCCESS, matched sourcehead ee4345a1a0bd75172f0279baeaf942d403a21bad. P3 branch from latest main, protections unchanged.
- P1 pytest Dependabot alert#1 verified fixed after the patched dependency reached main. No dependabot PR or security settings modified.
- P3 pinned source inspection confirmed B0 full-state layout, D1 registration bytes and C6 polling/54 echo behavior; legacy queue regression scenarios reviewed. No legacy files modified.
- Ruling: because Non-NASA has no unambiguous per-request sequence in non-echo ACKs, ACK alone will not complete new control intents; require Cmd20 readback. Cost: deliberate legacy behavioral difference; ACK-only devices remain unverified/unsupported until capture-backed review.
- Hardware NOT TESTED. Production remains passive; software lifecycle timing is not a physical-control guarantee.


## 2026-11-01 / P3 — independent Non-NASA engine

- Files/modules: non_nasa_packet, non_nasa_registration, non_nasa_control_queue, non_nasa_engine, passive root routing/counters; six-group C++ regression program, synthetic fixtures/metadata; protocol notes, parity tracker, README/changelog.
- Initial host program failed before sources existed. The new independent executable links no NASA source and runs with ASan/UBSan. Preliminary full suite34PASS in6.76s, Ruff/diffPASS.
- Test-driven corrections: a rate-limited keepalive broadcast could survive into a stale reply; RED→GREEN drop/expiry guard added. Wake APIs and engine wiring were first observed RED, then added with fresh-indoor-status gating. Arduino full firmware initially failed because LOW/HIGH macros collided with fan enum members; a host compile with those exact defines reproduced RED, then LOW_SPEED/HIGH_SPEED names fixed it. ESP-IDF initial build passed; final both-framework builds rerunning after fixes.
- Source-backed Cmd20/Cmd54/CmdC6/B0/D1 behavior is tested without importing legacy source. Celsius negative pipe reads use signed raw-minus55; Fahrenheit reads convert explicitly, Fahrenheit control is rejected. CmdF3 power scaling stays unresolved/raw.
- Ruling: all acknowledgements are nonterminal until requested fields are observed in Cmd20. Exact echoes suppress uncertain retries; non-echo and old ACKs cannot verify/clear newer intents. Cost: intentional PARTIAL legacy behavior and no ACK-only-device compatibility claim.
- Ruling: internal8-device slots,10000ms shared D1 limiter, three attempts,100ms preparation windows,30ms keepalive delay, fresh status after wake,4500ms ACK/readback interval,5000ms preparation/15000ms intent bounds are source-informed engineering policies. Cost: controller ownership and physical response/silence timing remain unverified; no active production integration is exposed.
- Authorization/security/localization/migration: production TX remains disabled; no public raw/control settings or real hardware writes; per-device policy/range/cancellation bridge is a later prerequisite. No payload/credential logging. US English public content. No database migration; legacy compatibility differences tracked explicitly.
- Hardware NOT TESTED. Branch phase/p3-non-nasa; commit/PR pending. Next: final tests/builds, independent review, relevant CI/PR gate before P4.


### P3 independent review and completed fix pass

- Independent read-only reviewer ran34tests PASS in6.86s and reproduced two Important races in a separate /tmp C++ program. No production-default bypass found; scope/provenance/PARTIAL parity accepted with these fixes required.
- Fixed: a new Cmd20 could change unrequested full-state fields while an old B0 preparation still passed is_current. Regression RED on remote power-off → generation invalidation/rebuild GREEN. An already-observed goal also invalidates unnecessary preparation; cancellation retires the obsolete generation without failing a newer intent.
- Fixed: C6 discarded the outstanding D1 TX-completion ticket, measuring the shared limiter from preparation instead of actual completion. Repro RED at only6000ms after TX → C6 now preserves the issued ticket, completion timestamp sets the10000ms limiter → GREEN.
- Additional associated regression: an unknown/late registration outcome could be reenabled manually after its prepared deadline. RED→GREEN sticky fault now requires reconstruction/restart, preventing an uncertain send from reopening the limiter path.
- Final `../.venv/bin/python -m pytest -q` →34PASS in7.17s, including seven Non-NASA assertion groups and LOW/HIGH macro defines under ASan/UBSan; Ruff check/format and git diff --check PASS.
- Both `../.venv/bin/esphome compile tests/yaml/esp-idf.yaml` and Arduino counterpart PASS after review fixes. Logs artifacts/p3/firmware-{idf,arduino}-reviewed.log. Hardware NOT TESTED.
- Review limits/rulings: physical timing, controller-seat ownership, capture correctness and equipment behavior remain unverified; no active production lifecycle submission. These costs remain documented. The fix pass uses reproducing tests and the full suite; no additional review seat is needed for already-covered fixes.


## 2026-11-01 / P3 acceptance and P4 start

- PR#5 merged by squash70692742dc883d87f31b38deaf5bbfee746d503b after all three jobs in CI41295164475 SUCCESS; matchedc0cad7c5f0cdd693d5110eec2e9d20c419be24f8. P4 branch from latestmain; protections unchanged.
- P4 research: public discussion#82 first100 comments and discussion#78 all77 comments read/search-indexed for model/evidence leads; issue#384 and2 comments read. Model reports are not packet/behavior verification. A model cited by a different commenter than the capture author was not attached to that capture as verified provenance. No customer logs copied into repository.
- Pinned SDK inspection: ClimateTraits requires Off/target and has no read-only climate flag. Ruling: keep climate projection internal-only until real control support exists; expose read-only state entities first. Cost: public climate control remains unavailable and must be considered explicitly at P4 acceptance.
- Ruling: disputed NASA room mappings remain disabled/UNKNOWN despite model compatibility reports. Source-backed Non-NASA room readings can be scoped to its profile. No hardware result or compatibility is invented.
- Optional async request for local model/capture paths sent; public source research and safe P4 work continue while waiting. No physical-device write requested or performed.

- P4 delivery: commit642df8d5db4b9adfd6850ef915b68f04eef945ca, PR https://github.com/omerfaruk-aran/esphome-samsung-hvac/pull/6. CI41297319356 three jobs in progress; no merge before all succeed.

## 2026-11-01 / P4 acceptance and P5 start

- PR#6 merged by squashfad669ee73d892a9511d4f87e12934db30381541 after CI41297319356 all three jobs SUCCESS, sourcehead642df8d5db4b9adfd6850ef915b68f04eef945ca. P5 branch from latestmain; no protection bypass. P4 passive software gate PASS, full parity PARTIAL, hardware NOT TESTED.
- P5: inspect pinned MIT legacy/nasactl and public Water Law/EHS/remote-sensor reports. User confirmed no local captures; no evidence level upgrade or live write.

- P5 delivery: commit8a6b2d1bf0ee061315cc3f78fb66e708f8704e26, PR https://github.com/omerfaruk-aran/esphome-samsung-hvac/pull/7, CI41298588789 three jobs in progress. No merge before all checks succeed.

## 2026-11-01 / P5 acceptance and P6 start

- PR#7 merged by squash9fdce0a6c2d4e5f4d558e293fa7854b8a0e8b8b5 after CI41298588789 all three jobs SUCCESS, sourcehead8a6b2d1bf0ee061315cc3f78fb66e708f8704e26. P6 branch from latestmain; no protection bypass. P5 source-labelled read-only software PASS; hardware/model/real HA publication NOT TESTED.

- P6 delivery: commitda91512, PR https://github.com/omerfaruk-aran/esphome-samsung-hvac/pull/8. Local tests/builds/review fixes complete; CI gate pending before merge/P7.

## 2026-11-01 / P6 acceptance and P7 start

- PR#8 merged by squashd87914a833316e3b5bb63cb2cc7189a8e4a8350c after CI41301108612 all three jobs SUCCESS, sourceheadda91512f76b1898b0e44726a9d03cad12a5ab19a. P7 branch from latestmain; protections unchanged. Passive/offline P6 software PASS, live writes/actual readback/hardware remain BLOCKED/NOT TESTED.
