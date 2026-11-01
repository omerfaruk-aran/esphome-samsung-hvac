# Contributing

Thank you for helping improve this project. The repository is in receive-only development; please discuss substantial design or compatibility work before implementing it.

## Workflow

1. Search existing issues and discussions. Open an issue for reproducible defects or a discussion for questions and proposals.
2. Describe the intended behavior and affected hardware. Avoid posting credentials, private network details, or unredacted captures.
3. Fork the repository, create a focused branch, and make a small, coherent change. Do not commit generated files or secrets.
4. Test the change where feasible, state exactly what was tested and what was not, and open a pull request using the template. Link the issue when applicable.
5. Address review comments and resolve conversations. Maintainers use squash merges.

Use professional US English for public documentation, comments, examples, release notes, and discussions. Preserve attribution and verify licenses before including third-party material. Do not claim support without evidence. Changes involving hardware or protocol behavior should include device models, protocol, interface, configuration, sanitized captures, and validation results where available.

Use Python 3.13, install requirements-dev.txt, and run the checks in README.md. The PR workflow runs real schema/code-generation tests and ESP32 Arduino/ESP-IDF firmware builds. Do not invent results; document exact commands, outcomes and limitations. Follow the [Code of Conduct](CODE_OF_CONDUCT.md) and report security vulnerabilities privately as described in [SECURITY.md](SECURITY.md).

The current runtime is deliberately passive. Read [architecture](docs/development/architecture.md), [testing](docs/development/testing.md), [catalog evidence](docs/protocol/message-catalog.md) and [release gates](docs/development/release-process.md) before changing protocol behavior. Product profiles/observers must not own UART readers or writers; NASA/Non-NASA state machines stay separate. Mapping contributions require explicit type/unit/scale/signedness/sentinel/source evidence and tests; model reports alone do not establish compatibility.

Run the pinned C++ formatter and Python/repository checks. Direct dependency or Action upgrades require corresponding license/source review and passing CI; update the inventory when versions change. Preserve the legacy repository as read-only. No contribution or software check authorizes a physical write, firmware upload or public release.
