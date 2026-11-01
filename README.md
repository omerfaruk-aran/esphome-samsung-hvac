# Samsung HVAC Modular for ESPHome

A planned modular ESPHome integration for Samsung HVAC systems. This repository is in the foundation stage: it provides receive-only transport groundwork, but no HVAC control, verified hardware matrix, installation procedure, or migration path. Do not use it to control equipment yet.

## Project goals

The project aims to separate shared bus transport, NASA and Non-NASA protocols, climate profiles, and EHS heat-pump support. Compatibility and behavior must be established through implementation and hardware validation; a goal is not a support guarantee.

## For users

For the existing integration, see [esphome_samsung_hvac_bus](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus) and its documentation. This project is separate and no migration is currently available. Use [Discussions](https://github.com/omerfaruk-aran/esphome-samsung-hvac/discussions) for questions and [Issues](https://github.com/omerfaruk-aran/esphome-samsung-hvac/issues) for actionable reports. See [SUPPORT.md](SUPPORT.md).

HVAC wiring and bus interfaces can involve hazardous voltages or damage equipment. Use a suitable isolated interface and qualified assistance; never connect an ESP32 directly to an unknown bus. Follow equipment instructions and local electrical requirements.

## For contributors

Read [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md). Public documentation, examples, comments, release notes, and discussions use US English. Planned documentation belongs in `docs/`. The foundation and receive-only transport provide bounded framing, real host/schema/code-generation tests and a two-framework firmware workflow. Typed protocol engines and HVAC entities are not implemented yet.

## License and attribution

This project is licensed under the [MIT License](LICENSE). The legacy integration was originally developed by [Lanwin](https://github.com/lanwin) and maintained by [Ömer Faruk ARAN](https://github.com/omerfaruk-aran); see [NOTICE](NOTICE). This repository does not currently include legacy protocol source.

## Foundation development checks

Use Python 3.13 and a virtual environment, then install `requirements-dev.txt`.

```sh
python -m pip install -r requirements-dev.txt
python -m pytest -q
ruff check components tests
ruff format --check components tests
esphome config tests/yaml/esp-idf.yaml
esphome compile tests/yaml/esp-idf.yaml
esphome compile tests/yaml/arduino.yaml
```

The YAML files are build fixtures, not installation or wiring examples. Do not upload them to connected equipment. The transport requires explicit `protocol: nasa` or `protocol: non_nasa`, exclusive `uart_id`, and 9600 8E1 test framing. This is a packaging contract, not a universal device configuration. It reads and validates framing without transmitting any bus bytes and creates no Home Assistant HVAC entities. Unsupported write settings fail validation.

Read the [baseline inventory](docs/compatibility/legacy-feature-inventory.md), [architecture decisions](docs/architecture/adr/0001-public-component-packaging.md), [parity tracker](docs/architecture/legacy-parity.md) and [execution ledger](docs/development/execution-ledger.md). Firmware compilation does not establish physical compatibility. Hardware: NOT TESTED.

See [transport behavior and limits](docs/development/transport.md) for bounded buffers, diagnostics, scheduler tests and the native ESP32 UART scope.
