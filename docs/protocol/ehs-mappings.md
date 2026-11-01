# EHS passive mapping evidence

Every enabled mapping is **INFERRED**, based on reverse-engineered community source, not official Samsung documentation or a confirmed device capture. Direction: hydro/indoor → controller/bus, Notification/Response only. Outdoor fields require a separately configured outdoor source. Sentinel meanings are UNKNOWN for every numeric field; physical ranges/health are not established by these conversions. No write range, polling or control is enabled.

Sources:

- **L:** MIT legacy [protocol_nasa.h:77](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/components/samsung_ac/protocol_nasa.h#L77), [core handlers:777](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/components/samsung_ac/protocol_nasa.cpp#L777), [DHW modes:684](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/components/samsung_ac/protocol_nasa.cpp#L684), [zone labels:125](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/components/samsung_ac/protocol_nasa.h#L125).
- **N:** MIT nasactl [const.py](https://github.com/paolostivanin/nasactl/blob/629c43fce1c6a0595b5070028e34cf4ad0f25386/components/nasactl/const.py) at pinned 629c43fce1c6a0595b5070028e34cf4ad0f25386. Full license verified: copyright 2026 Paolo Stivanin. No implementation source transplanted. Facts below were independently implemented/tested.
- **W:** [Water Law discussion 269](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/269), including [the 0x4248 lead](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/discussions/269#discussioncomment-15283581); this is a community report, not universal semantics or write authorization.

| Code | YAML observation | Wire type / signedness | Unit / scale | Source |
|---|---|---|---|---|
| 0x4237 | dhw_temperature | Variable / int16 | °C / 0.1 | L label; N lines37–47 conversion |
| 0x4235 | dhw_target_temperature | Variable / uint16 | °C / 0.1 | L handler791 |
| 0x4238 | water_outlet_temperature | Variable / int16 | °C / 0.1 | N48–58 |
| 0x4247 | water_outlet_target_temperature | Variable / uint16 | °C / 0.1 | L784 |
| 0x4236 | water_return_temperature | Variable / int16 | °C / 0.1 | N59–69 |
| 0x4248 | water_law_shift | Variable / int16 | °C / 0.1 | N247–256; W supporting lead |
| 0x427F | water_law_target_temperature | Variable / int16 | °C / 0.1 | N80–90 |
| 0x42E9 | water_flow | Variable / uint16 | L/min / 0.1 | N70–79 |
| 0x8204 | outdoor_temperature | Variable / int16 | °C / 0.1 | L884; N154–165; explicit outdoor source |
| 0x8235 | error_code | Variable / uint16 | code / 1 | L905; explicit outdoor source; no guessed text |
| 0x4000 | power_state | Enum / uint8 | boolean / 1 | L803; only 0/1 accepted |
| 0x4065 | dhw_power_state | Enum / uint8 | boolean / 1 | L821; only 0/1 accepted |
| 0x4066 | dhw_mode | Enum / uint8 | enum / 1 | L684; N553–559;0 Economy, 1 Standard, 2 Power, 3 Force |
| 0x4001 | operation_mode | Enum / uint8 | enum / 1 | L827; N561–566;0 Auto, 1 Cool, 2 Dry, 3 Fan, 4 Heat |
| 0x4089 | water_pump_state | Enum / uint8 | boolean / 1 | N661–666; only 0/1 accepted |
| 0x402E | defrosting | Enum / uint8 | boolean / 1 | L schema107; N655–660; only 0/1 accepted |
| 0x4119 | zones[number:1].power_state | Enum / uint8 | boolean / 1 | L125 and1121; only configured zone,0/1 |
| 0x411E | zones[number:2].power_state | Enum / uint8 | boolean / 1 | L126 and1124; only configured zone,0/1 |

Each scalar type is derived from its message code; Enum is 1 byte, Variable 2 bytes big-endian. Invalid bool/enums invalidate the field rather than guessing. Generic mode text is an observed protocol value, not a claim that every heat pump supports all listed operations, and no heating/cooling action is inferred.

Signedness decision: target 0x4235 follows L's unsigned read handler; N marks it signed. They agree for ordinary positive targets but disagree above 0x7FFF. This profile does not interpret that region as a negative DHW target or a known sentinel.0x4247 likewise follows L unsigned. Current/return/outlet and Water Law conversions follow N's explicit signed interpretation. These engineering choices remain INFERRED; capture-backed device policies must resolve out-of-range/sentinel semantics before controls or universal claims.

Water Law shift 0x4248 follows N's explicit shift label, while W calls it a Water Law value/target. The derived flow target 0x427F remains a separate observation. Neither is assigned to a zone, and a label is not device verification. Room codes0x4203/0x4204 and candidate Zone 2 target 0x42D6 remain UNKNOWN/disabled. Generic water outlet target 0x4247 never populates Zone 1/2 state. Zone temperatures/targets, valves, electrical energy scaling and FSV controls are not exposed by this profile.

Tests: tests/fixtures/ehs/{hydro_state,outdoor_state}.hex and metadata.json (synthetic, no device_models/capture_reference), tests/cpp/test_ehs_profile.cpp, and tests/test_config.py EHS cases. Host integration feeds simulated UART → production Transport/Framer/NASA Engine → EHS profile and asserts zero TX. Hardware and real entity publication: NOT TESTED.
