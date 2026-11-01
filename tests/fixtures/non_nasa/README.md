# Non-NASA fixtures

These are synthetic, source-derived INFERRED frames at legacy commit 6b1966bea8db9344d23763454f9cebed015d15c5. XOR bytes 1–11 were computed independently in Python. They cover Cmd20, Cmd54 echo/non-echo, CmdC6, B0, D1, a broadcast and raw CmdF3 telemetry. metadata.json records provenance. No capture or device model has been verified; no frame is sent to equipment.

Known source data: test/main_test_non_nasa_queue.cpp contains the C6 poll and regression scenarios; protocol_non_nasa.cpp contains the B0/D1 layouts and status conversions. MIT license and Lanwin attribution are recorded in the baseline inventory/NOTICE. The tests are independently written, not transplanted. CmdF3 power scaling remains unresolved; this fixture preserves raw bytes without asserting physical units or power.
