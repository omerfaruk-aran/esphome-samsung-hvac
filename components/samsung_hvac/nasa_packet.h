#pragma once

#include "bus_types.h"
#include <optional>

namespace esphome::samsung_hvac::nasa {

enum class AddressRole { OUTDOOR, INDOOR, CONTROLLER, BROADCAST, UNKNOWN };
struct Address {
  uint32_t value{0};
  AddressRole role() const;
  bool operator==(Address other) const { return this->value == other.value; }
  bool operator!=(Address other) const { return !(*this == other); }
};

enum class DataType : uint8_t { UNDEFINED = 0, READ = 1, WRITE = 2, REQUEST = 3, NOTIFICATION = 4,
                              RESPONSE = 5, ACK = 6, NACK = 7 };
enum class ValueType : uint8_t { ENUM, VARIABLE, LONG_VARIABLE, STRUCTURE };
enum class DecodeError { NONE, FRAME_SIZE, ENVELOPE, CRC, HEADER, PAYLOAD };

struct Message {
  uint16_t code{0};
  uint32_t raw{0};
  uint16_t offset{0};
  uint16_t length{0};
  ValueType kind() const { return static_cast<ValueType>((this->code >> 9) & 3); }
};

struct Packet {
  Address source;
  Address destination;
  uint8_t control{0xc0};
  uint8_t packet_type{1};
  DataType type{DataType::UNDEFINED};
  uint8_t sequence{0};
  uint16_t message_count{0};
  std::array<Message, 255> messages{};
  Frame storage;
};

// On failure message_count is zero; callers must check the return before using fields.
DecodeError decode(const Frame &frame, Packet &packet);
// Control WRITE/REQUEST encoding is deliberately unavailable in this phase.
bool encode(const Packet &packet, Frame &frame);
bool encode_read(Address local, Address device, uint8_t sequence, uint16_t code, Frame &frame);
std::optional<int64_t> signed_value(const Message &message);

}  // namespace esphome::samsung_hvac::nasa
