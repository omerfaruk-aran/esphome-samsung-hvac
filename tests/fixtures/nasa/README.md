# NASA typed packet fixtures

All files here are synthetic, generated from source-derived wire rules with an independent Python binascii.crc_hqx(payload, 0) CRC reference. None is a captured or hardware-confirmed packet. No device model or capture provenance is available.

Research references (INFERRED): legacy6b1966bea8db9344d23763454f9cebed015d15c5 protocol_nasa.h/.cpp, and nasactl629c43fce1c6a0595b5070028e34cf4ad0f25386 nasa_message.h/nasa_packet.cpp/nasa_command.h. Both were MIT-licensed at inspection; no implementation source was transplanted.

The fixtures cover explicit wire types and packet categories, signed/unsigned boundary bit patterns, CRC-valid malformed inner counts/lengths/structure ordering and trailing payload. Field IDs in these fixtures exercise encoding and do not establish universal device meanings; 0x4204 remains disputed for product mapping. Structure bytes remain opaque. The read_target packet has a zero Variable payload as in nasactl's READ constructor and is not transmitted to hardware.
