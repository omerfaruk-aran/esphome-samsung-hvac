# Typed passive message observations

The optional NASA-only root `messages` list exposes typed scalar observations through standard ESPHome sensors/binary sensors. It does not poll, send a Read request, register a controller or enable any write. At most32 observers share the root's single RX path alongside at most 8 product profiles. Non-NASA custom extraction is cataloged for research but is not an executable generic entity interface yet; its AC profile remains independent.

## Catalog selection

Ordinary product fields belong in AC/EHS profiles. For a source-labelled passive FSV or additional observation, select a catalog ID and exact source address:

```yaml
# Fragment under an existing receive-only NASA samsung_hvac root.
# Synthetic source address, not wiring or deployment guidance.
messages:
  - id: heating_upper_observation
    source_address: 0x200002
    catalog: ehs_fsv_heating_upper
    sensor:
      name: Heating Upper Limit Observation
  - id: power_observation
    source_address: 0x200000
    catalog: nasa_power
    binary_sensor:
      name: Power Observation
```

The [machine-readable catalog](../../components/samsung_hvac/message_catalog.json) is validated at component import and in tests. Select only passive NASA scalar entries. Unknown/disabled catalog names, wrong source roles, mismatched entity types/units, Non-NASA buses and write/poll options fail YAML validation. Source roles use NASA 24-bit indoor 0x20xxxx or outdoor 0x10xxxx addresses, not another product's implicit source. Destination role is explicitly any: passive Notification/Response observations may address another bus participant. ACK/NACK/Read/Write/Request do not publish current values.

Passive FSV observations can stay unknown indefinitely when no other controller requests/broadcasts them. There is no startup polling, automatic expiry, optimistic state or restored observer state. A firmware build does not prove that a particular device broadcasts an entry.

## Explicit custom descriptors

A custom scalar requires code, matching wire type, signedness, unit, finite nonzero scale, offset, source role and source URL. Its evidence stays UNKNOWN. Optional raw sentinels, valid_values (up to 16), and raw_range filter raw values before conversion. Sentinels are user-specified filters, not confirmed device behavior. No input can promote a custom descriptor to catalog confirmation.

```yaml
messages:
  - id: raw_4204_observation
    source_address: 0x200000
    custom:
      code: 0x4204
      value_type: variable
      signed: false
      unit: ""
      scale: 1
      offset: 0
      source_role: indoor
      source_url: https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/261
    sensor:
      name: Raw 0x4204 Observation
```

This example deliberately publishes an uninterpreted unsigned word, not a room/zone temperature. Disputed 0x4203/4204/42D6 named semantics stay disabled; custom observations of those IDs must be raw unsigned, unitless, scale 1/offset 0. They cannot populate product profile state or control a zone. Other custom semantics are explicit user assertions with UNKNOWN support, not verified Samsung definitions.

Enum/Variable/LongVariable are 1/2/4 bytes big-endian, validated against the code's type bits. Structures are not numeric scalar entities. Raw values are retained as uint32 and signed conversion uses explicit 8/16/32-bit two's complement. A double-precision conversion checks finite results, raw sentinels/ranges and allowed values. ESPHome sensor publication is float32: integers above 2^24 may lose individual counts. Values outside finite float range publish NaN. Invalid binary values invalidate state. Units must match the descriptor; standard filters may intentionally change values, so configure them carefully. No physical temperature sentinel or hardware health guarantee is invented.

## Write boundary

There is no public write option, raw send action, FSV setter or production Guard instance. The isolated [offline write policy](write-policy.md) tests future admission/readback rules without a frame encoder or UART. All catalog entries are passive or disabled. Even explicit custom configuration cannot enable transmission. Hardware and real Home Assistant publication/registry/restore remain NOT TESTED.
