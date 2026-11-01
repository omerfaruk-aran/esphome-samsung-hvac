#include "non_nasa_framer.h"

namespace esphome::samsung_hvac::non_nasa {

bool Framer::valid(size_t size) const {
  if (size != 14 || this->buffer_.data[13] != 0x34) return false;
  uint8_t checksum = 0;
  for (size_t i = 1; i < 12; ++i) checksum ^= this->buffer_.data[i];
  return checksum == this->buffer_.data[12];
}

}  // namespace esphome::samsung_hvac::non_nasa
