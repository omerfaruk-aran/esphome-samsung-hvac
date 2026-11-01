# NASA engine: passive decoding and read lifecycle

The engine consumes P1-validated frames and independently validates the outer envelope, CRC and typed payload. Each physical bus owns its own engine, packet storage and read queue. No NASA state is shared with Non-NASA. Production remains passive: it never queues or transmits a read, ACK, registration, request or control command. There is no new YAML TX setting or HVAC entity in P2.

## Typed wire data

| Type bits in message ID | Payload | Representation |
|---|---|---|
| 0 Enum | 1 byte | unsigned raw bits |
| 1 Variable | 2 bytes, big-endian | unsigned raw bits |
| 2 LongVariable | 4 bytes, big-endian | unsigned raw bits |
| 3 Structure | remaining body, only as final message | opaque bytes retained in full |

These are reverse-engineered source observations. The full MIT license of [nasactl at the inspected commit](https://github.com/paolostivanin/nasactl/blob/629c43fce1c6a0595b5070028e34cf4ad0f25386/LICENSE) was verified; [message](https://github.com/paolostivanin/nasactl/blob/629c43fce1c6a0595b5070028e34cf4ad0f25386/components/nasactl/nasa_message.h), packet and command sources were studied. No implementation was transplanted. Legacy decode also uses big-endian LongVariable, while its encoder differs; the new codec uses big-endian consistently and fixtures record this choice. No Samsung specification or identified device capture validates that choice universally.

Payload lengths, count, trailing bytes, structure placement, envelope/CRC and supported header fields are checked before exposing message_count. A failed decode exposes zero messages. At most 255 messages and a 1502-byte frame are represented without growing allocations. Opaque Structure data indexes the packet's owned frame storage. Callback data is borrowed only until callback return; consumers copy any state they retain.

Raw scalar values have no automatic unit, scale, signedness or sentinel interpretation. signed_value is an explicit two's-complement helper for 8/16/32-bit values; product descriptors must decide whether using it is justified. 0x4203/0x4204/0x42D6 meanings remain unassigned here. Engine statistics distinguish Read/Write/Request/Notification/Response/ACK/NACK; no control packet is treated as a notification.

The decoder supports source-derived protocol version 2 headers with reserved low bits clear and known packet/data categories; unsupported headers fail closed. Address role helpers classify 0x10 outdoor, 0x20 indoor, 0x80 test/controller and0xB0–0xBF broadcast; other roles remain opaque/unknown. They do not discover or authorize a physical device.

## Encoding and read lifecycle

The encoder supports typed read/passive packet fixtures but rejects Write and Request control categories. READ scalar payloads must be zero; active Structure reads are rejected. A dedicated single-message read encoder restricts the source to the inspected controller role, the target to a unicast indoor/outdoor role and the sequence to nonzero. None of these functions sends UART bytes.

ReadQueue has 8 intents including the active item, rejects duplicate device/code intents, and allows one in-flight request. prepare returns a bounded READ frame and enters PREPARED. The caller must report actual transmission completion using on_transmitted(sequence, time, success); merely accepting the frame into another queue does not start the response timer. Failure or a 5000ms preparation deadline terminates without retrying uncertain sends.

After successful TX, the response timeout is 1000ms, at most 3 transmitted attempts (2 retries). Retries use a new sequence. The queue matches source device, destination controller, sequence and expected message ID; ACK and Notification never complete reads. A matching Response returns raw bits; a matching NACK terminates. Packet/completion entry points check deadlines themselves, so a late event cannot succeed because a periodic tick was delayed.

Nonzero 8-bit sequences are quarantined for 30000ms after retirement, and preparation defers when all 255 are unavailable. This is a conservative software policy, not a proven maximum device response delay. The wire protocol cannot disambiguate an arbitrarily delayed response after eventual sequence reuse; hardware validation and protocol evidence remain prerequisites for active operation. Queued intents expire after 5000ms. All elapsed-time arithmetic uses uint32_t subtraction and has rollover tests.

## Integration boundary and evidence

The root routes passive NASA frames into this decoder and exposes counters in configuration logs. The read queue's prepare/transmission-completion interface is exercised with simulated lifecycle events; it is not connected to production scheduler submission in P2. Future active-read work must bridge confirmed TX completion and cancellation safely, plus enforce device/message policy and explicit opt-in. The P1 transport remains TX-disabled.

Fixtures and metadata are under tests/fixtures/nasa. They are synthetic/source-derived INFERRED evidence with no device models or capture references. Tests include all types/categories, malformed inner payloads, signed boundaries, read correlation, duplicate/foreign ACKs, deadlines, retries, quarantine, capacity and clock rollover under ASan/UBSan. Firmware compilation is software validation only. Hardware: NOT TESTED.
