# Read-only EHS profile

The development profile `ehs` observes NASA heat-pump state using explicit hydro and optional outdoor sources. It creates standard ESPHome numeric/binary/text sensors. It has no UART, scheduler, poller, climate control, water-heater control, writable number/select/switch or FSV write. All mappings are INFERRED; see [mapping evidence and limitations](../protocol/ehs-mappings.md) and [compatibility evidence](../compatibility/ehs-matrix.md).

## Configuration

Use a NASA root and an explicit stable device id. The root's eight-device limit covers AC and EHS together; profiles cannot duplicate an indoor source address on one bus. NASA AC and EHS may coexist as different devices on the same bus, using one transport owner. An EHS profile on Non-NASA or an invalid hydro/outdoor role fails validation.

```yaml
# Fragment under an existing receive-only samsung_hvac NASA root.
# Synthetic addresses only; not wiring or deployment guidance.
devices:
  - id: hydro
    profile: ehs
    address: 0x200002
    dhw_temperature:
      name: DHW Temperature
    dhw_target_temperature:
      name: DHW Target Temperature
    water_outlet_temperature:
      name: Water Outlet Temperature
    water_law_shift:
      name: Water Law Shift
    zones:
      - number: 1
        power_state:
          name: Zone 1 Enabled
      - number: 2
        power_state:
          name: Zone 2 Enabled
```

All entity keys are optional. Additional numeric keys: water_outlet_target_temperature, water_return_temperature, water_law_target_temperature, water_flow, outdoor_temperature, error_code. Outdoor temperature/error require outdoor_address in 0x10xxxx. Binary keys: power_state, dhw_power_state, water_pump_state, defrosting. Text keys: operation_mode, dhw_mode. The mapping table defines all codes/types/units/scales and their source limitations.

Zones is optional, at most two entries, each with number 1 or 2 and optional power_state. Duplicate zone numbers fail validation. Configuring a zone enables only its distinct source-labelled power observation; no temperatures/targets are inferred, no controller zone is enabled by configuration, and this does not establish verified multi-zone support. Unknown zone/current/target or control keys fail validation. Zone 2 data cannot populate Zone 1 or generic water outlet state.

## Publication and integration

Only matching source and Notification/Response observations update state; ACK/NACK/Read/Write/Request never publish current product state. Sensors publish only fields present in a packet, preserving missing fields as unknown or last-observed. Invalid bools invalidate; invalid text becomes Unknown. Numeric sentinel meanings are not established; source conversions can produce out-of-range values. There is no state expiry, optimistic update, restore or active polling; passive FSV values may never appear without another controller requesting them.

The adapter implements a stateless decoded-packet consumer interface used by the root; independent NASA/Non-NASA engines do not share protocol state. Standard ESPHome schemas support filters and entity registration. Home Assistant registry/restore and real API availability remain NOT TESTED; compilation is not hardware evidence. No HACS runtime dependency is required.

## Validation status

Synthetic golden observations cover available hydro/outdoor fields; host tests cover source/category isolation, signed negative Water Law shift, invalid enums and independent zone invalidation. A simulated UART integration uses real transport/framer/engine code and proves no TX. Both firmware fixtures compile EHS adapter code; ESP-IDF additionally instantiates all EHS entity kinds alongside AC profiles and native API. No firmware was uploaded and no device action was performed. Hardware, model-specific mappings, sentinel meanings, valve/energy/zone targets and controls remain unverified.
