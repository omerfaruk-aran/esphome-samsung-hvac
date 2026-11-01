# AC compatibility evidence

No physical AC model is verified with this independent integration. Hardware and real Home Assistant behavior: NOT TESTED. The implemented scope is source-derived passive software, not production compatibility.

| Scope | Software evidence | Unverified/unsupported boundary |
|---|---|---|
| NASA AC indoor/outdoor observations | typed synthetic fixtures, device/source/category/capability isolation, real YAML and two-framework builds | all model-specific behavior; named room meaning; public controls |
| Non-NASA AC Cmd20 observations | separate codec/lifecycle fixtures, source isolation, temperatures/modes/fan/swing tests | physical registration/controller ownership, active controls, ACK-only devices and electrical scaling |
| Multi-split observation isolation | multiple explicit device IDs/addresses, per-device state and shared UART owner tests | physical multi-split topology, availability/timing and actual actuation |
| Public NASA/non-NASA reports | [discussion 82](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/82), [discussion 78](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/78) are research leads | reports are not this project's captures or verified support |

NASA source classes 20/10 and Non-NASA indoor 00–7F are configured software bounds, not a guarantee that every address or operation exists on every unit. Fan/swing/preset lists are explicit selection gates for observations, not auto-discovery or write permission. Compatibility claims require identified model/controller/firmware/interface, reproducible sanitized captures and linked tests. No current row meets the confirmed hardware standard.

See [mapping evidence](../protocol/ac-profile-mappings.md), [entity dispositions](legacy-entity-matrix.md), [EHS matrix](ehs-matrix.md) and [capture requirements](../packet-capture.md). Legacy remains independently usable.
