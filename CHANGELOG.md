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
