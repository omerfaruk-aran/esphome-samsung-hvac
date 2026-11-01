# Passive AC profiles

P4 adds device-scoped NASA AC and Non-NASA AC state observations through standard ESPHome sensors, binary sensors and text sensors. Production remains receive-only. No active polling, registration, control, raw write or FSV write is enabled. Hardware and Home Assistant registry/restore behavior are NOT TESTED. This is not a migration or a production-ready HVAC controller.

The root owns at most eight devices and the only UART path. Pure ac_profile code consumes decoded packets and owns per-device state; ac_entities adapts observations to ESPHome. Protocol engines remain independent. Profiles have no UART or scheduler handle. Offline assess_control validates candidate values and returns CONTROL_DISABLED/UNSUPPORTED/INVALID; it never returns an encodable command. P3 tests exercise control lifecycle fixtures separately without sending them.

## Configuration

Choose protocol explicitly. Under samsung_hvac, devices requires an explicit id, matching profile and numeric address. NASA AC indoor addresses are 0x20xxxx; optional outdoor_address is 0x10xxxx. Non-NASA indoor addresses are 0x00–0x7F. Duplicate indoor addresses on a bus, mismatched protocols and unsupported entities fail code generation. Root-only configurations still work.

```yaml
# Fragment for an existing, correctly configured receive-only NASA bus.
# Addresses are synthetic examples, not automatic device discovery.
devices:
  - id: living_room
    profile: nasa_ac
    address: 0x200000
    target_temperature:
      name: Living Room Target Temperature
    power_state:
      name: Living Room Power
    operation_mode:
      name: Living Room Mode
    fan_modes: [auto, low, medium, high]
    fan_mode:
      name: Living Room Fan
  - id: bedroom
    profile: nasa_ac
    address: 0x200001
    power_state:
      name: Bedroom Power
```

Optional fan_modes are auto/low/medium/high/turbo and Non-NASA-only fresh. vertical_swing and horizontal_swing default false; corresponding *_swing_state binary entities require true. NASA presets are an explicit list of none/sleep/quiet/fast/long_reach/eco/windfree, required for preset_state. These settings select which observed values to represent; they do not prove device support or enable writes. No capability is discovered or enabled from another indoor unit.

Numeric keys: target_temperature, indoor_eva_in_temperature, indoor_eva_out_temperature; Non-NASA-only room_temperature; NASA-only humidity, outdoor_temperature, error_code (the last two require outdoor_address). Binary keys: power_state, configured swing states, and NASA-only filter_warning/defrosting/silence_state/automatic_cleaning_state. Text keys: operation_mode, configured fan_mode, NASA preset_state. See [all wire mappings and evidence](../protocol/ac-profile-mappings.md).

Optional climate accepts internal:true only (default true). ESPHome 2026.10.1 has no read-only climate capability and requires editable target/Off traits. Public climate would advertise unavailable controls, so internal:false fails validation. Its control callback logs a US English warning and does not change state or transmit. No writable switch, number, select or reset button is available. Internal projection publishes only after observed power, target and (when on) representable mode exist; no default Off/action/current temperature is fabricated. It retains its last complete representable observation if later fields are invalid. Use separate state entities for invalidation-aware diagnostics.

## State, identity and limits

Only present fields publish on each accepted observation. Absent fields remain unknown on boot or retain the prior observation; there is no polling, automatic expiry or restored profile state. Invalid numbers publish NaN, invalid booleans invalidate the ESPHome binary state, and invalid text publishes Unknown. Temperature sentinel meanings are UNKNOWN; source conversions are not sensor-health validation. ESPHome entity filters and automations use standard schemas; user filters may deliberately transform values. A schema id is a compile-time reference, not a promised Home Assistant registry ID. Keep explicit entity IDs/names where reproducibility matters; registry migration, localization and restore parity remain unverified.

Build fixtures in tests/yaml exercise both frameworks, multiple indoor devices and native API entity compilation. network:{} satisfies API code generation without credentials or an actual Wi-Fi/Ethernet connection. They are not deployable network/wiring configurations and must not be uploaded to equipment. Firmware compilation does not prove real API/HA behavior.

## Verification and remaining gates

Host tests feed source-derived synthetic golden frames through real codecs into isolated profiles and test unsupported control assessments. YAML tests exercise valid profiles, multi-split and rejected combinations. Existing P3 command fixtures cover queue/ACK/readback behavior offline. Identified device captures, real controls, HA registry/restore, and physical multi-split behavior remain NOT TESTED or BLOCKED. No full legacy PARITY is claimed; the entity matrix lists each supported observation and each deferred control/telemetry behavior.
