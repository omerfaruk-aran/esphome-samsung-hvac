#include "nasa_packet.h"
#include "nasa_framer.h"
#include <algorithm>

namespace esphome::samsung_hvac::nasa {

namespace {
size_t width(ValueType type) {
  switch (type) {
    case ValueType::ENUM: return 1;
    case ValueType::VARIABLE: return 2;
    case ValueType::LONG_VARIABLE: return 4;
    default: return 0;
  }
}
uint32_t read_be(const uint8_t *bytes, size_t size) {
  uint32_t value = 0;
  for (size_t i = 0; i < size; ++i) value = (value << 8) | bytes[i];
  return value;
}
void write_be(uint8_t *bytes, size_t size, uint32_t value) {
  for (size_t i = 0; i < size; ++i) bytes[size - i - 1] = static_cast<uint8_t>(value >> (i * 8));
}
void finish_frame(Frame &frame, size_t size) {
  frame.size = size;
  frame.data[0] = 0x32;
  write_be(frame.data.data() + 1, 2, size - 2);
  write_be(frame.data.data() + size - 3, 2, frame_crc(frame.data.data() + 3, size - 6));
  frame.data[size - 1] = 0x34;
}
}  // namespace

AddressRole Address::role() const {
  if (this->value > 0xffffff) return AddressRole::UNKNOWN;
  uint8_t prefix = static_cast<uint8_t>(this->value >> 16);
  if (prefix == 0x10) return AddressRole::OUTDOOR;
  if (prefix == 0x20) return AddressRole::INDOOR;
  if (prefix == 0x80) return AddressRole::CONTROLLER;
  if (prefix >= 0xb0 && prefix <= 0xbf) return AddressRole::BROADCAST;
  return AddressRole::UNKNOWN;
}

DecodeError decode(const Frame &frame, Packet &packet) {
  packet.message_count = 0;
  if (frame.size < 16 || frame.size > MAX_FRAME_SIZE) return DecodeError::FRAME_SIZE;
  const auto *bytes = frame.data.data();
  if (bytes[0] != 0x32 || bytes[frame.size - 1] != 0x34 || read_be(bytes + 1, 2) + 2 != frame.size)
    return DecodeError::ENVELOPE;
  size_t end = frame.size - 3;
  if (frame_crc(bytes + 3, frame.size - 6) != read_be(bytes + end, 2)) return DecodeError::CRC;
  uint8_t category = bytes[10] & 0x0f;
  if (category == 0 || category > 7 || (bytes[10] >> 4) > 4 || (bytes[9] & 7) != 0 ||
      ((bytes[9] >> 5) & 3) != 2)
    return DecodeError::HEADER;
  packet.source.value = read_be(bytes + 3, 3);
  packet.destination.value = read_be(bytes + 6, 3);
  packet.control = bytes[9];
  packet.packet_type = bytes[10] >> 4;
  packet.type = static_cast<DataType>(category);
  packet.sequence = bytes[11];
  size_t count = bytes[12];
  size_t cursor = 13;
  for (size_t i = 0; i < count; ++i) {
    if (cursor > end || end - cursor < 2) return DecodeError::PAYLOAD;
    Message message;
    message.code = static_cast<uint16_t>(read_be(bytes + cursor, 2));
    cursor += 2;
    size_t length = width(message.kind());
    if (message.kind() == ValueType::STRUCTURE) {
      if (i + 1 != count) return DecodeError::PAYLOAD;
      length = end - cursor;
    }
    if (cursor > end || length > end - cursor) return DecodeError::PAYLOAD;
    message.offset = static_cast<uint16_t>(cursor);
    message.length = static_cast<uint16_t>(length);
    if (message.kind() != ValueType::STRUCTURE) message.raw = read_be(bytes + cursor, length);
    packet.messages[i] = message;
    cursor += length;
  }
  if (cursor != end) return DecodeError::PAYLOAD;
  packet.storage = frame;
  packet.message_count = static_cast<uint16_t>(count);
  return DecodeError::NONE;
}

std::optional<int64_t> signed_value(const Message &message) {
  size_t size = width(message.kind());
  if (size == 0) return {};
  unsigned bits = static_cast<unsigned>(size * 8);
  uint64_t mask = (uint64_t{1} << bits) - 1;
  uint64_t raw = message.raw & mask;
  if ((raw & (uint64_t{1} << (bits - 1))) != 0) return static_cast<int64_t>(raw) - (int64_t{1} << bits);
  return static_cast<int64_t>(raw);
}

bool encode(const Packet &packet, Frame &frame) {
  if (&frame == &packet.storage) {
    Frame scratch;
    bool success = encode(packet, scratch);
    if (success) frame = scratch;
    else frame.size = 0;
    return success;
  }
  frame.size = 0;
  uint8_t category = static_cast<uint8_t>(packet.type);
  if (packet.type == DataType::WRITE || packet.type == DataType::REQUEST || category == 0 || category > 7 ||
      packet.packet_type != 1 || packet.source.value > 0xffffff || packet.destination.value > 0xffffff ||
      packet.message_count > packet.messages.size() || (packet.control & 7) != 0 ||
      ((packet.control >> 5) & 3) != 2)
    return false;
  size_t size = 16;
  for (size_t i = 0; i < packet.message_count; ++i) {
    const auto &message = packet.messages[i];
    size_t length = width(message.kind());
    if (message.kind() == ValueType::STRUCTURE) {
      if (packet.type == DataType::READ || i + 1 != packet.message_count ||
          packet.storage.size > MAX_FRAME_SIZE || message.offset > packet.storage.size ||
          message.length > packet.storage.size - message.offset)
        return false;
      length = message.length;
    } else if ((length < 4 && message.raw >= (uint32_t{1} << (length * 8))) ||
               (packet.type == DataType::READ && message.raw != 0)) {
      return false;
    }
    if (length + 2 > MAX_FRAME_SIZE - size) return false;
    size += length + 2;
  }
  auto *bytes = frame.data.data();
  write_be(bytes + 3, 3, packet.source.value);
  write_be(bytes + 6, 3, packet.destination.value);
  bytes[9] = packet.control;
  bytes[10] = static_cast<uint8_t>((packet.packet_type << 4) | category);
  bytes[11] = packet.sequence;
  bytes[12] = static_cast<uint8_t>(packet.message_count);
  size_t cursor = 13;
  for (size_t i = 0; i < packet.message_count; ++i) {
    const auto &message = packet.messages[i];
    write_be(bytes + cursor, 2, message.code);
    cursor += 2;
    size_t length = width(message.kind());
    if (message.kind() == ValueType::STRUCTURE) {
      length = message.length;
      std::copy_n(packet.storage.data.data() + message.offset, length, bytes + cursor);
    } else {
      write_be(bytes + cursor, length, message.raw);
    }
    cursor += length;
  }
  finish_frame(frame, size);
  return true;
}

bool encode_read(Address local, Address device, uint8_t sequence, uint16_t code, Frame &frame) {
  frame.size = 0;
  size_t size = width(Message{code}.kind());
  if (local.role() != AddressRole::CONTROLLER ||
      (device.role() != AddressRole::INDOOR && device.role() != AddressRole::OUTDOOR) ||
      sequence == 0 || code == 0 || size == 0)
    return false;
  auto *bytes = frame.data.data();
  write_be(bytes + 3, 3, local.value);
  write_be(bytes + 6, 3, device.value);
  bytes[9] = 0xc0;
  bytes[10] = 0x11;
  bytes[11] = sequence;
  bytes[12] = 1;
  write_be(bytes + 13, 2, code);
  std::fill_n(bytes + 15, size, 0);
  finish_frame(frame, 18 + size);
  return true;
}

}  // namespace esphome::samsung_hvac::nasa
