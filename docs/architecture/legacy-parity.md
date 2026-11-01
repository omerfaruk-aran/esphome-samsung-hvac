# Legacy parity tracking

No feature has achieved complete legacy PARITY. P0–P8 provide software groundwork and simulation evidence, with production remaining passive. The complete schema and entity rows are tracked in [the YAML reference](../compatibility/legacy-yaml-reference.md) and [entity matrix](../compatibility/legacy-entity-matrix.md). All runtime equivalence starts UNKNOWN.

| Behavior group | Phase owner | Current result | Required evidence |
|---|---|---|---|
| UART ownership, direction, recovery | P1 | PARTIAL; bounded transport tests, different safety/timing defaults | bounded-stream and scheduler tests, framework builds |
| NASA frame/typed-set/request lifecycle | P2 | PARTIAL; strict typed codec/read lifecycle, no production active reads | provenance-labelled golden and malformed fixtures |
| Non-NASA registration, B0, ACK, coalescing, sleep | P3 | PARTIAL; independent lifecycle tests, stricter ACK/readback policy | delayed/duplicate/non-echo/rapid-command regressions |
| AC state/control/capabilities and multi-split | P4 | PARTIAL; isolated passive observations, internal climate, no public controls | each legacy entity, state/command fixtures and YAML |
| EHS DHW/outlet/Water Law/zones | P5 | PARTIAL; inferred passive DHW/outlet/Water Law/state, distinct zone enables only | device-scoped mapping sources, zone isolation |
| Custom entities, descriptors, FSV | P6 | PARTIAL; NASA typed passive observers/FSV, strict catalog and offline policy; live writes unavailable | schema and fail-closed write-policy tests |
| MQTT/debug migration, IDs, restored state, filters | P7 | PARTIAL; complete baseline dispositions and compiled examples, no facade/identity or full migration guarantee | explicit disposition for every configuration and observable behavior |
| Platforms/CI/docs | P8 | PARTIAL; actual quality/firmware matrix, source/license/dependency checks and user/developer/release docs; physical maturity unverified | required checks and documented scope |
| Device compatibility and release maturity | P9 | NOT TESTED | identified hardware evidence |

Unsupported behavior must be explicitly justified; a missing implementation remains UNKNOWN. Users may remain on the legacy project. No compatibility facade or stable migration is available in P0.
