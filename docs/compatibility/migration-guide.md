# Migration from the legacy integration

This is an **optional software transition to a passive development integration**, not a drop-in replacement for legacy HVAC controls. Keep the legacy installation if you need its current control behavior. This independent repository never changes old `@main` references, the legacy project, or existing devices. Hardware, real Home Assistant registry/restore behavior and full legacy PARITY are NOT TESTED.

## Decide whether the supported subset fits

The new project observes device-scoped AC/EHS state with one UART owner, separate NASA/Non-NASA engines and typed passive NASA messages. Public climate, writable numbers/switches/selects, filter reset, registration/keepalive, active polling and live FSV/raw writes are unavailable. NASA room temperature and ambiguous zone targets remain disabled. EHS zone enable observations do not establish verified multi-zone compatibility. Named electrical/energy/outdoor-operation telemetry and Non-NASA generic custom extraction are not carried forward.

All 65 legacy schema declarations have explicit defaults, source links and PARTIAL/UNSUPPORTED dispositions in [the YAML reference](legacy-yaml-reference.md) and [machine-readable inventory](legacy-migration.json). All 27 entity declarations have dispositions in [the entity matrix](legacy-entity-matrix.md). The [behavior tracker](../architecture/legacy-parity.md) distinguishes implementation from runtime equivalence. Unsupported controls are not silently converted into working controls.

## Prepare a separate candidate configuration

Back up the existing YAML, the exact component reference/ESPHome version, a known recovery firmware image and relevant Home Assistant entity/automation references privately. Verify that an established recovery method exists before any later authorized hardware change. Do not publish credentials, addresses from private logs or registry exports.

Use a separate candidate YAML. Do not include both components on one UART, use raw uart.write actions or share UART/direction pins. Declarative competing UART consumers are rejected, but you must also establish the physical topology before any later pilot. No new firmware upload or energized bus action is part of the software examples.

The examples below use a local checkout (`../components`), synthetic addresses/pins and encrypted native API references. They are codegen-tested software references, not verified model/wiring configurations:

- [NASA AC](../../examples/nasa-ac.yaml)
- [Non-NASA AC](../../examples/non-nasa-ac.yaml)
- [EHS observations](../../examples/ehs.yaml)
- [NASA multi-split](../../examples/multi-split.yaml)

Supply your own ignored local secrets.yaml for wifi_ssid, wifi_password and api_encryption_key, using [ESPHome secret references](https://esphome.io/guides/yaml/#secrets-and-the-secretsyaml-file). API encryption expects a base64 representation of 32 key bytes; follow the [official ESPHome API encryption documentation](https://esphome.io/components/api/#configuration-variables). The test suite creates ephemeral non-deployed values in its temporary directory and never commits them. Native API exposes entities through ESPHome; HACS is not a runtime dependency. Real network/API integration remains NOT TESTED here.

For an external-component Git reference instead of a checkout, pin a reviewed commit. The following is the tested P6 implementation baseline; consult the execution ledger before selecting a later revision:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/omerfaruk-aran/esphome-samsung-hvac
      ref: d87914a833316e3b5bb63cb2cc7189a8e4a8350c
    components: [samsung_hvac]
```

## Convert protocol, address and entity intent explicitly

Legacy configuration inferred protocol from address-string length and accepted writable device keys:

```yaml
# Legacy fragment for comparison only; do not load with the new component.
samsung_ac:
  uart_id: hvac_uart
  devices:
    - address: "20.00.00"
      power:
        name: Living Room Power
      target_temperature:
        name: Living Room Target
```

The passive equivalent declares its bus/profile and changes entity types:

```yaml
samsung_hvac:
  id: hvac
  uart_id: hvac_uart
  protocol: nasa
  devices:
    - id: living_room
      profile: nasa_ac
      address: 0x200000
      power_state:
        id: living_room_power
        name: Living Room Power
      target_temperature:
        id: living_room_target
        name: Living Room Target
```

Power is now a binary sensor and target is a sensor, so old switch/number services and automation setters must not be retained. No public climate takes their place. Non-NASA uses protocol: non_nasa, profile: non_nasa_ac and numeric address such as 0x00. EHS uses NASA with profile: ehs and a hydro source. Each device requires an explicit ID, duplicate sources are rejected and at most 8 devices are allowed per root. Multiple roots require separate exclusive physical UART buses. NASA outdoor temperature/error requires a separate outdoor_address; do not reuse an indoor source by assumption.

Root capabilities and automatic inheritance are removed. Per-device fan_modes is an explicit list, default empty; turbo is opt-in as an observed value, not a write feature. Swing flags default false. NASA presets use fixed names/values; longreach becomes long_reach and none must be included explicitly if wanted. Custom preset names/values, legacy fan spelling aliases and map_auto_to_heat_cool are not migrated. Mode text preserves Auto; the optional internal AC climate projection maps it to Heat/Cool. This difference is explicit and the projection has no working controls.

## Audit filters, defaults and missing state

Typed profiles already convert supported raw temperatures to Celsius. Do not copy a legacy multiply 0.1 filter on top of an already converted temperature. Custom NASA reads should use catalog descriptors where available or explicit wire type/signed/scale/offset/raw filters; do not transplant arbitrary raw lambdas or infer physical units for unresolved electrical words. Standard ESPHome sensor filters run after this component's typed conversion and in their configured order. See [ESPHome sensor filters](https://esphome.io/components/sensor/#sensor-filters).

Legacy room_temperature_offset affected both the room sensor and climate. A new Non-NASA room sensor filters.offset changes that sensor only; it does not alter the internal climate projection. The Non-NASA example's 0.5 offset demonstrates syntax, not a measured calibration. NASA named room fields remain unavailable. Generic unitless raw 0x4204 inspection is not room-temperature parity.

Missing fields begin unknown and retain the last observed value without automatic expiry. Invalid numeric values publish NaN, invalid booleans invalidate and invalid text becomes Unknown. Numeric sentinel meanings remain UNKNOWN where documented. Passive FSVs may not appear unless another controller requests them. Offline command/read lifecycle tests do not establish real device availability, actuation or readback.

The inherited old update_interval default 30s has no new counterpart: the root is a non-polling Component. uart_id is explicitly required. Root/entity IDs and names use standard ESPHome schemas but preserving a YAML id does not guarantee an unchanged Home Assistant registry ID, device association, restoration or entity type. Review dashboards, automations, availability, units and disabled/internal state separately. No automatic registry rewrite or restored-value migration is performed.

## Validate without touching equipment

From the repository with the pinned development environment and your private secrets file:

```sh
esphome config examples/nasa-ac.yaml
esphome compile --only-generate examples/nasa-ac.yaml
```

Use the matching example for other profiles. Do not substitute an upload/run command or assume a successful compile validates an interface/model. The repository also runs schema, parser, simulated-UART, policy and firmware tests; results are in the [execution ledger](../development/execution-ledger.md).

If a later authorized pilot fails, stop the candidate using the verified recovery procedure and return to the privately saved legacy configuration/firmware. Keep one bridge owner and do not combine both implementations as a fallback. Physical rollback and Home Assistant registry recovery remain NOT TESTED until performed on an identified setup. Remaining on the legacy project indefinitely is supported as a migration decision.

## Why there is no compatibility facade

Accepting old writable keys under new passive entities would conceal behavior changes and could break automations. The runtime API, protocol selection, capabilities and identity/restore contract are not equivalent, so no alias layer or automatic converter is shipped. Unknown legacy keys fail validation with their option name. Debug MQTT credentials/topics and raw/change-only logging knobs are intentionally removed; standard logger counters do not claim the same audit stream. No source reference, old default or community model report grants write authorization.

Actual physical rollback, native API availability and Home Assistant identity/restore remain NOT TESTED in the [P9 pilot audit](../development/hardware-pilot.md). Successful code generation does not establish these outcomes.
