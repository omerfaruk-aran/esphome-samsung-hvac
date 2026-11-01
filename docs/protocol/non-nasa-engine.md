# Non-NASA engine and conservative control lifecycle

The engine has its own packet/status, registration and control state. It imports no NASA headers and its host test links no NASA source. P1 owns the only physical UART path. Production only receives/decodes packets and updates counters; registration, keepalive, wake and control preparation are disabled and never submitted to UART.

## Wire evidence

The pinned legacy baseline describes a 14-byte frame with 0x32/0x34 envelope and XOR of bytes 1–11 at byte 12. Cmd20 fields decode target/room/pipe temperatures, power, mode, fan and swing. Temperature reads preserve the Fahrenheit flag and use an explicit conversion; Celsius raw-minus55 arithmetic uses a signed representation, including negative pipe temperatures. This is community reverse-engineered evidence, not official Samsung documentation.

Cmd54 retains its eight payload bytes, CmdC6 identifies a source-derived controller polling pattern, B0 formatting preserves the full observed control state, and D1 formatting uses the source-derived registration bytes. Other commands, including CmdF3, remain raw packets; no disputed current/power scaling is published or inferred from that fixture. Unknown command bytes are retained, not automatically interpreted as control/status.

## Registration, sleep and keepalive

These helpers run only in host simulation in P3. Internal enable flags default false and have no YAML surface.

- A recognized checksum-valid bus packet must be observed before registration preparation. Noise or unknown traffic cannot trigger repeated registration by itself.
- D1 registration, wake and keepalive share a 10000ms minimum attempt interval. Completion time also updates the limiter, including sends prepared earlier. A C6 confirmation never drops the accounting ticket for an outstanding D1 completion. Explicit booleans handle an attempt at millis=0.
- At most three attempts occur without a confirming C6 poll. A prepared send expires after 5000ms and disables further preparation when the outcome is unknown. Failed or uncertain TX remains faulted until reconstruction/restart; toggling enable cannot bypass that state.
- Opted-in keepalive is triggered by the inspected outdoor broadcast pattern, delayed 30ms and dropped when rate-limited or stale. It is not emitted later as an unsolicited old response.
- Pending unsent controls can request a wake after 1000ms. The same limiter applies. A wake invalidates the control base until the indoor unit supplies a fresh Cmd20; a C6 alone is insufficient.
- C6 observation only shows that controller address 0xD0 is being polled. It does not prove this integration owns that logical controller seat, or that two controllers can coexist. Hardware/controller ownership remains unverified.

## Coalesced control intentions

The control helper stores at most eight device slots and requires a recent observed, compatible full Cmd20 state. It accepts patches for power, mode, target, fan and swing. Mode implies power-on unless an explicit contradictory same-patch power-off is rejected. Updates merge across loop cycles and preserve still-pending user fields; unrequested fields follow fresh observed state. Unknown mode/fan/swing, Fahrenheit control, target outside 16–30°C, incompatible room data and stale/unobserved device bases fail closed. These are conservative engineering/source limits, not verified safe ranges for every model.

One frame can be prepared per valid C6 preparation window. Prepared generations and completion tokens distinguish commands changed before/after TX. New Cmd20 observations that change the prepared full-state bytes, or already satisfy the goal, invalidate the older preparation; the future sender must check is_current(token) immediately before dispatch and cancel obsolete preparations. TX completion, not mere queue acceptance, starts the 4500ms reply/readback deadline. There are at most three transmitted attempts, a 15000ms intent lifetime and a 5000ms prepared deadline. Repeated updates remain coalesced and cannot expand the fixed slot count.

ACK never completes an intent. An exact Cmd54 echo records receipt and suppresses uncertain retries, but waits for Cmd20 readback; non-echo, duplicate or old acknowledgements cannot clear a newer intent. Only fresh observed state matching the requested fields produces VERIFIED. Already-observed desired state can satisfy an intent without transmitting anything; this status means the state is observed, not proof that this integration caused it. If observed state moves away from both the prior observation and requested values after an echo, the intent can be marked SUPERSEDED and the external state becomes the new base. A readback timeout is distinct from a transport/ACK timeout.

This intentionally differs from the legacy immediate ACK-clear and permissive non-echo ACK behavior. Devices that do not publish adequate Cmd20 readback are not claimed compatible. An off unit may normalize fan/swing and prevent verifying those fields; no false control success is reported. Later remote changes must not be overwritten by a new command built from an expired desired state.

## Integration boundary and remaining risks

The host lifecycle can encode registration/B0 bytes, but the production root never enables preparation or the P1 transmitter. No live bus writes were performed. The active completion/cancellation bridge, logical-controller ownership, device allowlists, verified ranges and physical response windows must be established before exposing controls. In particular, the 100ms preparation windows and P1 silence policy are engineering limits, not measured Non-NASA bus timing guarantees.

Fixtures and metadata under tests/fixtures/non_nasa are synthetic/source-derived INFERRED evidence, not captures. Tests exercise queue coalescing before/after prepare/TX/ACK, stale and non-echo acknowledgements, readback, remote override, per-device isolation, retries, sleep/wake, rate limits, buffer/capacity, rollover and Arduino LOW/HIGH macro compatibility under ASan/UBSan. Both firmware frameworks compile the passive adapter. Hardware: NOT TESTED. No universal compatibility or legacy parity is asserted.
