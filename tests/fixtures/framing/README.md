# Framing fixtures

- nasa.hex: legacy test/main_test_nasa.cpp test_nasa_1 expected frame at pinned commit 6b1966bea8db9344d23763454f9cebed015d15c5. INFERRED source-derived framing reference; no capture/model provenance. It is only parsed, never transmitted to equipment.
- non_nasa.hex: synthetic Cmd20-shaped frame for framing tests. Checksum independently calculated as XOR of bytes 1–11.
- nasa_max.hex: synthetic 1502-byte Structure-shaped frame at the engineering buffer limit. CRC independently generated using Python binascii.crc_hqx(payload, 0). It tests framing, not semantic validity or a device's supported structure length.

None is a hardware-confirmed capture. No physical compatibility is claimed. Payload/type validation belongs to P2/P3.
