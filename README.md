# Samsung HVAC Modular for ESPHome

A planned modular ESPHome integration for Samsung HVAC systems. This repository is in the foundation stage: it does not yet provide a working component, supported hardware matrix, installation procedure, or migration path. Do not use it to control equipment yet.

## Project goals

The project aims to separate shared bus transport, NASA and Non-NASA protocols, climate profiles, and EHS heat-pump support. Compatibility and behavior must be established through implementation and hardware validation; a goal is not a support guarantee.

## For users

For the existing integration, see [esphome_samsung_hvac_bus](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus) and its documentation. This project is separate and no migration is currently available. Use [Discussions](https://github.com/omerfaruk-aran/esphome-samsung-hvac/discussions) for questions and [Issues](https://github.com/omerfaruk-aran/esphome-samsung-hvac/issues) for actionable reports. See [SUPPORT.md](SUPPORT.md).

HVAC wiring and bus interfaces can involve hazardous voltages or damage equipment. Use a suitable isolated interface and qualified assistance; never connect an ESP32 directly to an unknown bus. Follow equipment instructions and local electrical requirements.

## For contributors

Read [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md). Public documentation, examples, comments, release notes, and discussions use US English. Planned documentation belongs in `docs/`. No protocol implementation or CI checks are present yet.

## License and attribution

This project is licensed under the [MIT License](LICENSE). The legacy integration was originally developed by [Lanwin](https://github.com/lanwin) and maintained by [Ömer Faruk ARAN](https://github.com/omerfaruk-aran); see [NOTICE](NOTICE). This repository does not currently include legacy protocol source.
