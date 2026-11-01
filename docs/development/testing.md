# Development tests and CI

Use Python 3.13, install requirements-dev.txt and run from the repository:

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

check_repository validates immutable Action pins/read-only permissions, direct tool version/license metadata, local documentation links, example secret references, tracked secret filenames and clang-format 23.1.3. It is not a general secret scanner, legal certification, complete dependency vulnerability audit or firmware SBOM. pip check validates installed dependency consistency. GitHub scanning/Dependabot checks are separately dated in the ledger.

Host tests compile real C++17 core modules with -Wall/-Wextra/-Werror/-pedantic, ASan and UBSan. Transport tests simulate bounded UART traffic/faults; NASA tests exercise typed bounds/correlation/fuzz; Non-NASA tests independently exercise registration/coalescing/ACK/readback; profiles test source/category/capability/zone isolation; advanced tests cover scalar/sentinel/precision and offline policy. Golden fixtures are source-labelled/synthetic unless actual capture provenance is explicitly present. No host binary accesses real UART hardware.

Real ESPHome CLI tests load/validate/generate positive and negative YAML, including both frameworks, competing owners, unsupported capabilities, typed descriptors and legacy migration. Example tests create non-deployed ephemeral credentials in temporary directories and mask values on failures. The helper permits only codegen/compile, with no upload/run passthrough. Native API/Wi-Fi compilation does not prove networking, registry/restore or device compatibility.

CI uses Ubuntu 24.04/Python 3.13, read-only contents permission, no stored checkout credentials and immutable official Actions. Required workflow jobs are Quality and tests plus five firmware entries: bus-esp-idf, bus-arduino, encrypted-nasa-ac, encrypted-non-nasa-ac and receive-capture. All applicable jobs and review must pass before squash merge. These jobs are the maintainer acceptance policy. The current main-protection ruleset does not enforce required status checks, so every applicable hosted result must be explicitly verified before merge. P8 does not alter that remote ruleset. No release/deployment job exists. Direct tool upgrades require matching license inventory review; Dependabot PRs do not auto-merge.

C++ formatting uses a pinned tool and preserves include/using order, string literals and comments. The P8 initial 48-file pass preserved non-whitespace content and is validated by the same compiler/sanitizer/firmware matrix. Current platform evidence is the esp32dev Arduino/ESP-IDF fixtures on local macOS and hosted Linux; no inherited ESP8266/LibreTiny/other-board guarantee exists.

Hardware, actual bus timing/readback, real API publication, Home Assistant identity/restore and recovery are NOT TESTED until recorded on an identified authorized setup. A skipped/unavailable criterion is never a passing hardware result. See [execution evidence](execution-ledger.md) and [release gates](release-process.md).
