# Getting started with passive observations

Use this project only to evaluate the documented observation subset. Public HVAC controls, active polling/registration and FSV/raw writes are unavailable. Hardware and actual Home Assistant behavior remain NOT TESTED. Keep a working legacy installation if its controls are required; this independent project does not force a transition.

1. Read the [AC](compatibility/ac-matrix.md)/[EHS](compatibility/ehs-matrix.md) evidence and [interface safety](wiring-safety.md). A compiled ESP32 configuration is not a verified equipment interface.
2. Use ESPHome 2026.10.1. For development, use Python 3.13 in a virtual environment and install requirements-dev.txt from a reviewed checkout. End-user entity transport is standard ESPHome native API; HACS is not required.
3. Choose a [reference example](../examples/nasa-ac.yaml), [Non-NASA](../examples/non-nasa-ac.yaml), [EHS](../examples/ehs.yaml) or [multi-split](../examples/multi-split.yaml). The local source path expects this checkout. The [migration guide](compatibility/migration-guide.md) also documents a tested immutable Git source.
4. Supply your own ignored local secrets file and explicit IDs, addresses and supported capabilities. Examples contain secret references, synthetic pins/addresses and no ready-to-use hardware wiring. Never publish your Wi-Fi/API credentials. See [ESPHome secret references](https://esphome.io/guides/yaml/#secrets-and-the-secretsyaml-file) and [native API encryption](https://esphome.io/components/api/#configuration-variables).
5. Validate without connecting or uploading to equipment:

```sh
esphome config examples/nasa-ac.yaml
esphome compile --only-generate examples/nasa-ac.yaml
```

Developers can validate examples without supplying real credentials using `python tools/check_example.py nasa-ac`; add `--compile` for a full non-deployed test build. The helper uses temporary test values and never uploads.

A later hardware pilot needs identified indoor/outdoor/controller models and firmware, verified isolated interface/topology, capture privacy review and explicit authorization. No such pilot has been performed. Do not infer physical compatibility from public model reports or silently replace writable entity actions with passive sensors. See [migration limitations](compatibility/migration-guide.md), [troubleshooting](troubleshooting.md) and the [execution ledger](development/execution-ledger.md).
