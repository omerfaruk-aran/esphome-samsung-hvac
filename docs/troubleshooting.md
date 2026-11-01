# Troubleshooting passive observations

Start with software validation and the [interface safety boundary](wiring-safety.md). Do not experiment with live writes, registration or polarity/voltage changes on unidentified equipment.

| Symptom | Software checks | Remaining evidence boundary |
|---|---|---|
| YAML rejects an option | Compare the explicit profile/capability/type/source contract and [migration table](compatibility/legacy-yaml-reference.md); old controls/debug/poll settings are not aliases | a schema correction does not validate hardware |
| Component/UART ownership error | Remove competing declarative consumers, raw UART actions, dummy receiver and shared UART/direction pins | physical topology cannot be inferred from YAML |
| No entity state | Confirm entity configuration, source address/profile and eligible observed packets; no active polling is performed | a device may not broadcast the requested value |
| Frames but no typed state | Review source/category/code/type against the [catalog](protocol/message-catalog.md); ACKs are not state, unresolved named mappings remain disabled | do not relabel unknown words as room/zone temperatures |
| Invalid/unexpected value | Check signedness, conversion, raw filters, sentinel uncertainty and duplicate scale filters | source conversions are INFERRED, not sensor-health validation |
| State appears old | Missing fields retain their last observation; no automatic expiry or restore is implemented | check privately captured data rather than fabricating availability |
| Climate cannot be made public | ESPHome's mandatory writable traits conflict with unavailable controls; only internal AC projection exists | full climate/control parity remains PARTIAL |
| FSV stays unknown | Passive FSV values may appear only when another controller requests them | no startup FSV polling or writer exists |
| Apparent log gaps | Use bounded capture chunks, inspect CRC/length and log continuity; reduce unrelated logging | API logs can drop/delay data and cannot prove wire timing |

The configuration dump includes bounded transport/engine counters at the time dump_config runs. These are not streaming diagnostic entities or a complete audit log. RX noise/integrity/typed errors, driver overrun exposure and diagnostic accounting limits are described in [transport notes](development/transport.md). Internal TX/lifecycle tests do not enable production TX.

For a report, include exact reviewed commit/ESPHome version, framework, identified model/interface/topology, redacted configuration, evidence level and reproduction. Keep raw/private logs and credentials out of public issues. See [capture guidance](packet-capture.md), [support](../SUPPORT.md) and [private security reporting](../SECURITY.md). Hardware reproduction remains NOT TESTED until actually performed.
