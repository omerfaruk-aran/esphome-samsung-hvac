# Samsung HVAC Modular for ESPHome

An independent, receive-only development integration for Samsung NASA AC, Non-NASA AC and NASA/EHS observations. Software tests and firmware builds pass for the documented fixtures. **No physical model or production control support is verified.** Public controls, active polling/registration and FSV/raw writes are unavailable.

| Scope | Implemented software | Evidence limits |
|---|---|---|
| NASA AC | isolated passive state, explicit fan/swing/preset observations, scoped outdoor telemetry | named room semantics unresolved; no public climate/control |
| Non-NASA AC | independent codec/lifecycles and passive Cmd20 state | active registration/control unconnected; electrical scaling unresolved |
| EHS | passive DHW/outlet/Water Law/flow/state and distinct configured zone enables | INFERRED mappings; no verified multi-zone model or zone targets |
| Advanced | strict typed catalog/custom NASA scalar and passive FSV observers | no active queries/writes; generic Non-NASA observers unavailable |
| Architecture | one UART owner, bounded RX/TX infrastructure, independent engines | physical timing, recovery and real Home Assistant behavior NOT TESTED |

Start with [getting started](docs/getting-started.md), [interface safety](docs/wiring-safety.md), [protocol selection](docs/protocol-selection.md) and the [documentation index](docs/README.md). Examples for [NASA AC](examples/nasa-ac.yaml), [Non-NASA AC](examples/non-nasa-ac.yaml), [EHS](examples/ehs.yaml) and [multi-split](examples/multi-split.yaml) are software references with synthetic pins/addresses and local secret references. They are not validated wiring or deployment configurations. Never connect an ESP32 directly to an unknown HVAC bus.

Home Assistant entities use standard ESPHome/native API; HACS is not required. Real API availability, registry/restore and hardware compatibility remain NOT TESTED. See the [AC](docs/compatibility/ac-matrix.md) and [EHS](docs/compatibility/ehs-matrix.md) evidence matrices.

The [legacy integration](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus) remains independently usable and unchanged. The [optional migration guide](docs/compatibility/migration-guide.md) accounts for all pinned schema/entity declarations and explicitly documents PARTIAL/UNSUPPORTED behavior. No full-control migration, facade or automatic identity rewrite is offered.

## Development checks

Use Python 3.13 and a virtual environment, then run:

```sh
python -m pip install -r requirements-dev.txt
python -m pip check
ruff check components tests tools
ruff format --check components tests tools
python tools/check_repository.py
python -m pytest -q
esphome compile tests/yaml/esp-idf.yaml
esphome compile tests/yaml/arduino.yaml
python tools/check_example.py nasa-ac --compile
python tools/check_example.py non-nasa-ac --compile
python tools/check_example.py capture --compile
```

The helper creates ephemeral non-deployed test secrets and only generates/compiles; it never uploads. CI runs real parser/sanitizer/catalog/schema/integration tests, direct dependency/license/format checks and five firmware cases. Read [testing](docs/development/testing.md), [architecture](docs/development/architecture.md), [catalog evidence](docs/protocol/message-catalog.md), [execution ledger](docs/development/execution-ledger.md) and [release gates](docs/development/release-process.md). A build or synthetic fixture is not hardware evidence.

Use [Discussions](https://github.com/omerfaruk-aran/esphome-samsung-hvac/discussions) for questions and [Issues](https://github.com/omerfaruk-aran/esphome-samsung-hvac/issues) for actionable reports, after reviewing [SUPPORT](SUPPORT.md). Report vulnerabilities privately through [SECURITY](SECURITY.md). Contributions follow [CONTRIBUTING](CONTRIBUTING.md) and the [Code of Conduct](CODE_OF_CONDUCT.md); public content uses professional US English.

## License and attribution

[MIT License](LICENSE). The legacy integration was originally developed by [Lanwin](https://github.com/lanwin) and maintained by [Ömer Faruk ARAN](https://github.com/omerfaruk-aran); applicable provenance/notices are preserved in [NOTICE](NOTICE). No third-party implementation source was transplanted. Reverse-engineered source facts are not official Samsung specifications. This project is independent and not endorsed by Samsung.
