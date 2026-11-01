# EHS compatibility evidence

**No device is verified with this independent integration.** Hardware: NOT TESTED. Rows below are research leads or synthetic software coverage, not a support guarantee. Source facts are INFERRED; no OBSERVED/CONFIRMED capture is claimed. Firmware/controller/topology details not stated below remain UNKNOWN.

| Device or scope | Public evidence | Software coverage here | Hardware / compatibility |
|---|---|---|---|
| Generic NASA hydro observations | pinned MIT legacy and nasactl sources in the mapping table | DHW/outlet/Water Law/flow/pump/state; synthetic fixtures and ESPHome builds | NOT TESTED; model applicability UNKNOWN |
| MIM-E03EN control kit | [discussion294](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/294) author reports a separate component for this board | no source copied from the unlicensed reference; no board capture linked to our fixtures | report only; NOT TESTED here |
| AE090RNYDEG indoor + AE090RXEDEG outdoor | [model report](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/82#discussioncomment-8703172) | research lead only; no verified packet/model association used | NOT TESTED here |
| AE160JXYDEH/EU mono outdoor | [model report](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/82#discussioncomment-9244244) | research lead only | NOT TESTED here |
| AE120RXYDEG heat pump | [legacy NASA/heating/DHW report](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/82#discussioncomment-11514022) | does not verify this implementation or every field | NOT TESTED here |
| Zone 1/2 enable codes0x4119/0x411E | pinned legacy labels/handlers; source address explicit | independent scalar/mask/state tests; YAML duplicate/unsupported-key rejection | no verified multi-zone model/capture; NOT TESTED |
| Zone temperatures/targets,0x4203/4204/42D6 | conflicting/unresolved evidence in plan research; [remote-sensor discussion261](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/261) | deliberately disabled; tests prove no state mutation | UNKNOWN / unsupported |
| Real controls, FSV writes, valves and electrical telemetry | insufficient capture/range/safety evidence for this profile | unavailable; no write option or live TX | BLOCKED pending device evidence and guarded policy |

Before upgrading any row, record exact indoor/outdoor/controller model and firmware, interface/bus topology, sanitized before/after captures, independently decoded fields, automated fixture and authorized test outcome. Do not paste credentials or unredacted customer logs. Captures from a different commenter cannot be joined to a model report as if they came from one test. No user must migrate from the legacy integration.
