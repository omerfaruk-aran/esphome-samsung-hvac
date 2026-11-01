# Shared transport behavior and limits

The public samsung_hvac component owns one native ESP32 UART. It currently receives and validates framing only. There is no YAML option for polling, registration, HVAC controls, FSV, raw writes or enabling TX. Optional flow_control_pin belongs under samsung_hvac and is initialized to receive direction; do not also configure UART-managed flow control. Build fixture GPIOs are not wiring instructions.

## RX

Each loop consumes at most 64 UART bytes. NASA and Non-NASA integrity framers own separate per-instance state; they share bounded byte-storage mechanics, not protocol control state. NASA validates the 0x32/0x34 envelope, declared size and CRC-16 polynomial 0x1021 initialized to zero; Non-NASA validates its 14-byte envelope and XOR checksum. These rules derive from the pinned community legacy baseline and are not official Samsung specifications.

The frame buffer is capped at 1502 bytes (an engineering limit derived from legacy). Incomplete frames expire after 50 ms without another byte. Noise and invalid candidates resynchronize on a subsequent start byte. A plausible but incomplete long frame may defer recovery until timeout; bounded storage is maintained. Payload semantics, message type/length checks and command handling belong to P2/P3. A valid outer frame does not establish a valid typed message.

Diagnostics count UART bytes, validated frames, noise/timeout-discarded bytes, invalid candidates, incomplete-frame timeouts, overflow, TX-during-RX discard candidates, sent frames, rejected/expired queue entries and TX faults. Invalid-candidate start bytes are counted as invalid candidates, not in the noise/timeout discard total. No raw payloads or private captures are logged. Diagnostics are internal counters and configuration logs, not Home Assistant entities yet. The driver itself may discard bytes on hardware RX overrun without a per-byte event in ESPHome; such losses cannot be inferred from these counters.

## Internal TX scheduler

Host simulation exercises the internal scheduler; production does not enable it. A future protocol consumer must independently satisfy the write policy before any active traffic can be exposed.

- Four queued frames including the in-flight frame; empty/oversized/full/disabled submissions are rejected.
- A valid bus frame must have been observed; incomplete RX, exhausted RX budget or less than 100 ms of silence defers TX.
- Per-loop writes are limited to both available driver space and 64 bytes; frames never interleave.
- Queued frames expire after 5000 ms. An active send/drain/guard has a 5000 ms deadline; failure clears the queue, releases direction and remains faulted until restart. No automatic transport retry.
- Direction stays asserted until driver-confirmed completion plus a 3 ms nonblocking guard, then the next frame waits another silence interval.
- RX arriving during TX or direction guard is discarded and counted as echo candidates; the software cannot classify collisions from that count.
- Disabling TX during an active send faults closed and may truncate that frame. Shutdown releases direction. Neither behavior establishes a device-level rollback guarantee.

The ESP32 adapter uses ESPHome available_for_write and ESP-IDF uart_wait_tx_done(port, 0), not a blocking flush. Both Arduino and ESP-IDF use this native UART driver in the pinned ESPHome version. The installed ESP-IDF 5.5.6 implementation was inspected; the status poll includes its driver's short 2 µs synchronization delay. See [Espressif UART API](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32/api-reference/peripherals/uart.html). Physical line completion and turnaround timing remain NOT TESTED on equipment.

Memory is fixed after setup: one 1502-byte RX buffer plus a four-slot maximum-frame queue and small bookkeeping. Bus instances are independent. The schema rejects declared UART ID competitors, raw uart.write actions, dummy RX consumers and same-pin UART aliases. It cannot detect two physically interconnected buses described with different pins or sandbox arbitrary user lambdas/C++.

## Verification

Run python -m pytest -q. The host test compiles the real C++ core with ASan/UBSan and simulated IO; fixture origins and limitations are in tests/fixtures/framing/README.md. Firmware fixtures compile the adapter and direction GPIO on ESP32 Arduino and ESP-IDF. None of these checks uploads firmware or validates an identified HVAC model.
