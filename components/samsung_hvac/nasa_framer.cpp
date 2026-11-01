#include "nasa_framer.h"

namespace esphome::samsung_hvac::nasa {

uint16_t frame_crc(const uint8_t *data, size_t size) {
  uint16_t crc = 0;
  for (size_t i = 0; i < size; ++i) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (unsigned bit = 0; bit < 8; ++bit)
      crc = (crc & 0x8000) != 0 ? static_cast<uint16_t>((crc << 1) ^ 0x1021) : static_cast<uint16_t>(crc << 1);
  }
  return crc;
}

size_t Framer::expected_size() const {
  if (this->buffer_.size < 3) return 0;
  size_t size = (static_cast<size_t>(this->buffer_.data[1]) << 8) | this->buffer_.data[2];
  return size < 14 || size > MAX_FRAME_SIZE - 2 ? INVALID_SIZE : size + 2;
}

bool Framer::valid(size_t size) const {
  if (this->buffer_.data[size - 1] != 0x34) return false;
  uint16_t expected = (static_cast<uint16_t>(this->buffer_.data[size - 3]) << 8) | this->buffer_.data[size - 2];
  return frame_crc(this->buffer_.data.data() + 3, size - 6) == expected;
}

}  // namespace esphome::samsung_hvac::nasa
