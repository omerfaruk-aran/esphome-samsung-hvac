# AC profile mapping evidence

All mappings below are reverse-engineered **INFERRED** source observations, not official Samsung documentation or verified device capabilities. Baseline: [6b1966bea8db9344d23763454f9cebed015d15c5](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/tree/6b1966bea8db9344d23763454f9cebed015d15c5). No real packet capture or identified hardware was used. Tests use synthetic fixtures under tests/fixtures/ac_profiles and the P2/P3 codec fixtures. Sentinel meanings are UNKNOWN for every mapping unless explicitly stated; the implementation does not identify an undocumented temperature sentinel.

## NASA AC

Direction is device → controller/bus; only Notification/Response packets are eligible. Codes, types and source handling come from [protocol_nasa.h:77–108](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/components/samsung_ac/protocol_nasa.h#L77) and [protocol_nasa.cpp:777–913](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/components/samsung_ac/protocol_nasa.cpp#L777). Filter/defrost codes come from [the schema:107](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/components/samsung_ac/__init__.py#L107). Each profile matches one explicit indoor source; outdoor telemetry requires a separate explicit outdoor source.

| Code | Field | Wire type / signedness | Unit / scale | Valid interpretation / restrictions |
|---|---|---|---|---|
| 0x4201 | target temperature | Variable / uint16 | °C / 0.1 | unsigned source interpretation; not a write range |
| 0x4205 / 0x4206 | evaporator in / out | Variable / int16 | °C / 0.1 | signed two's complement |
| 0x8204 | outdoor temperature | Variable / int16 | °C / 0.1 | explicit outdoor source only |
| 0x8235 | error code | Variable / uint16 | code / 1 | explicit outdoor source only; no error-text lookup |
| 0x4038 | humidity | Enum / uint8 | % / 1 | 0–100; other values invalidate; source names this percent but only logs it |
| 0x4000 | power | Enum / uint8 | boolean / 1 | only 0/1 accepted |
| 0x4001 | mode | Enum / uint8 | enum / 1 | 0 Auto, 1 Cool, 2 Dry, 3 Fan, 4 Heat |
| 0x4006 | selected fan | Enum / uint8 | enum / 1 | 0 Auto, 1 Low, 2 Medium, 3 High, 4 Turbo; explicit capability mask |
| 0x4060 | preset | Enum / uint8 | enum / 1 | 0 None, 1 Sleep, 2 Quiet, 3 Fast, 6 Long Reach, 7 Eco, 9 WindFree; explicit capability mask |
| 0x4011 / 0x407E | vertical / horizontal swing | Enum / uint8 | boolean / 1 | 0/1; each requires its configured capability |
| 0x4027 / 0x402E | filter warning / defrost | Enum / uint8 | boolean / 1 | 0/1; diagnostic observations only |
| 0x4046 / 0x4111 | silence / automatic cleaning | Enum / uint8 | boolean / 1 | 0/1; diagnostic observations only |

Boolean handling deliberately narrows legacy nonzero conversion to 0/1; other values invalidate the field. Invalid or unconfigured fan/preset values become Unknown, never a guessed default. 0x4007 actual fan is not substituted for selected fan. Preset names do not establish support on a particular device. NASA 0x4203/0x4204 room meanings remain UNKNOWN and disabled; EHS codes do not enter the AC profile. Energy/current/voltage mappings remain unavailable pending end-to-end scale review.

## Non-NASA AC

Direction is indoor → controller/bus, Cmd20 only, matching explicit 8-bit indoor source. [protocol_non_nasa.cpp](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus/blob/6b1966bea8db9344d23763454f9cebed015d15c5/components/samsung_ac/protocol_non_nasa.cpp) is the pinned source; exact P3 codec rules are documented in [non-nasa-engine.md](non-nasa-engine.md). No NASA identifiers are involved.

| Payload | Field | Wire type / signedness | Unit / scale | Interpretation |
|---|---|---|---|---|
| byte 0 / 1 / 2 / 7 | target / room / pipe in / pipe out | uint8 encoded temperature | °C / 1 or converted °F | bit 7 clear: raw−55 °C (signed result); set: (raw&127−32)×5/9 °C |
| byte 4 bit 7 | power | one bit, unsigned | boolean / 1 | bit set means on |
| byte 4 bits 0–5 | mode | unsigned enum | enum / 1 | 1 Heat, 2 Cool, 4 Dry, 8 Fan, 0x21 Auto Heat, 0x22 Auto; others Unknown |
| byte 3 bits 0–2 | fan | unsigned enum | enum / 1 | 0 Auto, 2 Low, 4 Medium, 5 High, 6 Fresh, 7 Turbo; configured mask required |
| byte 3 bits 3–7 | swing | unsigned enum | enum / 1 | 26 Vertical, 27 Horizontal, 28 Both, 31 Off; legacy raw 0 normalized Off; others Unknown |

Presets, humidity, outdoor telemetry, filter/defrost/silence/cleaning entities are unavailable for this profile. Cmd54 ACK, B0 intent and C6 poll never publish current AC state. Non-NASA Auto Heat remains explicit in the mode text; the internal climate projection has no exact equivalent and retains its last complete representable state. No active action is inferred from an operating mode.
