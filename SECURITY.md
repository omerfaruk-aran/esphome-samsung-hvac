# Security Policy

## Supported versions

This project is in its receive-only development stage and has no released or supported versions. The legacy project has its own security policy.

## Reporting a vulnerability

Do not disclose a suspected vulnerability in a public issue or discussion. Use [GitHub private vulnerability reporting](https://github.com/omerfaruk-aran/esphome-samsung-hvac/security/advisories/new) to contact maintainers. Include the affected version or commit, impact, reproduction steps, and sanitized evidence. Never include active credentials or private keys.

Maintainers will review reports and coordinate disclosure and fixes when feasible. No response or remediation time is guaranteed. Please allow time for investigation before public disclosure.

## Development security boundaries

Production TX remains disabled; no public polling, registration, control, raw send or FSV writer is available. A typed custom observation cannot claim catalog confirmation. The offline write Guard has no live bridge or approved device policy; its verified-family assertion is a trusted future policy prerequisite, not proof of hardware safety.

YAML and custom C++ are trusted firmware configuration, not an untrusted-code sandbox. Declarative ownership checks reject competing UART consumers/actions and shared pins, but cannot establish physical topology or electrical safety. Keep captures/private network details out of public reports; use the [capture privacy procedure](docs/packet-capture.md).

CI uses read-only permissions, immutable official Actions, no persisted checkout credentials and ephemeral non-deployed example secrets. [Dependency/license checks](docs/development/dependencies.md) and [release gates](docs/development/release-process.md) state their scope; they do not certify every transitive dependency, secret pattern or physical action. Hardware and actual release readiness remain NOT TESTED/BLOCKED.
