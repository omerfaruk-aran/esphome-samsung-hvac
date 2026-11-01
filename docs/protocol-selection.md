# Protocol and profile selection

Select protocol explicitly on each samsung_hvac root. There is no automatic detector or inference from address-string length.

| Root protocol | Product profile | Source address | Available state scope |
|---|---|---|---|
| nasa | nasa_ac | indoor 0x20xxxx; optional outdoor 0x10xxxx | configured passive AC observations |
| non_nasa | non_nasa_ac | indoor 0x00–0x7F | passive Cmd20 AC observations |
| nasa | ehs | hydro 0x20xxxx; optional outdoor 0x10xxxx | source-labelled DHW/outlet/Water Law/diagnostics and distinct configured zone enables |

The configured NASA roles are a software scope, not a universal device-address map. Discover actual addresses only through a verified receive-only setup/capture; do not send speculative queries or guess an active controller identity. Public model reports are leads. Separate NASA/Non-NASA parsers/state machines do not share mutable protocol state.

At most8product profiles and 32NASA scalar observers share one root's RX path. Duplicate product source addresses, wrong protocol/profile, unsupported capability/entity combinations and conflicting UART/direction pins fail validation. Advanced observers may intentionally observe the same source separately and retain their own state. Structures, generic Non-NASA raw entities and named disputed room/zone targets are unavailable.

See [AC configuration](development/ac-profiles.md), [EHS](development/ehs-profile.md), [advanced observations](development/advanced-messages.md) and [mapping evidence](protocol/message-catalog.md). Supported frameworks compile for the esp32dev fixture under Arduino/ESP-IDF; other chip/board/interface compatibility is not established by that result.
