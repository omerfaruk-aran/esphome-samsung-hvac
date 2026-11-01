# Changelog

Notable changes will be recorded here. This project has not made a release.

## Unreleased

- Establish repository documentation and contribution infrastructure.

### P0 foundation (unreleased)

- Add a separately implemented, inert ESPHome component with explicit protocol selection and exclusive UART validation.
- Record the pinned legacy schema, 27 device entity declarations, protocol behavior and license provenance without importing protocol source.
- Add six reviewed ADRs and document the failed nested-source packaging prototype.
- Add schema/code-generation tests and Arduino/ESP-IDF firmware CI.
- No transport, HVAC control, supported device matrix or migration is available yet.


### P1 transport (unreleased)

- Add a receive-only, bounded UART path with independent NASA/Non-NASA integrity framing and diagnostics.
- Add an internal serialized TX scheduler with queue limits, discovery/silence gating, nonblocking drain polling, direction guard and sticky faults; TX remains unavailable in YAML.
- Test parser recovery, random streams, queue behavior, driver failures and clock rollover under address/undefined-behavior sanitizers.
- Require native ESP32 UART and reject competing aliases on the same pins.


### P2 NASA (unreleased)

- Add strict bounded typed NASA decoding, lossless opaque structures and consistent big-endian scalars, with explicit signed conversion.
- Distinguish packet categories and add a bounded read-only lifecycle with source/destination/sequence/message correlation, confirmed-TX timers, retries and sequence quarantine.
- Integrate passive NASA decoding and counters; production active reads and all control writes remain unavailable.
- Add evidence-labelled synthetic fixtures and sanitizer regression/fuzz tests. No device compatibility or disputed message meaning is asserted.


### P3 Non-NASA (unreleased)

- Add independent Non-NASA packet/status decoding and passive engine counters, without linking NASA state.
- Add disabled-by-default registration/keepalive/wake and coalesced control lifecycle helpers with shared rate limits, bounded retries, generation tokens and readback-based completion.
- Treat all ACKs as nonterminal; document deliberate legacy differences and unsupported Fahrenheit control/ambiguous telemetry scaling.
- Add synthetic fixtures, sanitizer regression tests, and Arduino macro compatibility coverage. No active registration, control YAML or hardware compatibility is available.

### P4 AC observations (unreleased)

- Add isolated, bounded NASA AC and Non-NASA AC profiles, explicit addresses/capabilities and standard ESPHome state entities.
- Keep public climate/control entities unavailable; optional climate projections are internal only. Disputed NASA room codes remain disabled.
- Add source-labelled state fixtures, profile/capability/codegen tests and API entity firmware compilation. Full legacy parity, device captures, hardware and Home Assistant registry/restore behavior remain unverified.

### P5 EHS observations (unreleased)

- Add a NASA-only, read-only EHS profile for source-labelled DHW, outlet, Water Law, flow and state observations, plus explicitly scoped outdoor telemetry.
- Keep Zone1/2 enable observations separate; ambiguous room/zone temperatures and targets remain disabled. No verified multi-zone device support is claimed.
- Add synthetic hydro/outdoor fixtures, zero-TX simulated-UART integration, YAML rejection tests, evidence/compatibility matrices and firmware entity coverage. No live device writes, active polling or FSV write option is added.

### P6 typed observations (unreleased)

- Add a versioned evidence-labelled catalog with strict protocol/wire/schema validation and NASA scalar catalog/custom observers.
- Add passive FSV observations, explicit raw-only inspection of disputed words, invalid/sentinel handling and documented float publication limits.
- Test an isolated immutable guarded-write lifecycle without a live bridge, public opt-in, retries or transmission. All shipped catalog entries remain passive/disabled; verified physical policies and active writes are unavailable.
- Add scalar/policy/catalog/YAML tests and firmware observer coverage on both frameworks. Hardware and real equipment readback remain NOT TESTED.

### P7 migration documentation (unreleased)

- Complete explicit dispositions for all 65 legacy schema declarations and 27 entity rows, preserving baseline source/default evidence without claiming full parity.
- Add optional passive migration, identity/filter/credential/rollback guidance and codegen-tested NASA AC, Non-NASA AC, EHS and multi-split examples.
- Keep legacy controls and unsupported keys explicit; no compatibility facade, automatic registry migration, hardware upload or forced transition is added.

### P8 CI and documentation (unreleased)

- Pin official Node 24 Actions and clang-format, enforce C++/Python formatting, repository policy/local-link/secret-reference and direct dependency/license evidence checks.
- Run quality tests and five firmware cases, including encrypted network examples and a bounded receive-only capture reference; no upload or publication job exists.
- Add user/developer/safety/troubleshooting/capture/compatibility/security/license/release documentation and weekly dependency update configuration.
- Keep hardware, live controls, full parity and release readiness unverified; no release or deployment is performed.
