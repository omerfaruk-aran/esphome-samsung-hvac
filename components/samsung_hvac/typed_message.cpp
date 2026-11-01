#include "typed_message.h"
#include <algorithm>
#include <cmath>
namespace esphome::samsung_hvac::advanced {
unsigned Descriptor::bits() const {
  switch ((this->code >> 9) & 3) {
    case 0:
      return 8;
    case 1:
      return 16;
    case 2:
      return 32;
    default:
      return 0;
  }
}
bool Descriptor::valid() const {
  if (!this->bits() || !std::isfinite(this->scale) || this->scale == 0 ||
      !std::isfinite(this->offset) || this->sentinel_count > this->sentinels.size() ||
      this->value_count > this->values.size() || this->raw_min > this->raw_max)
    return false;
  const auto maximum = (uint64_t{1} << this->bits()) - 1;
  if (this->raw_min > maximum) return false;
  for (size_t i = 0; i < this->sentinel_count; ++i)
    if (this->sentinels[i] > maximum) return false;
  for (size_t i = 0; i < this->value_count; ++i)
    if (this->values[i] > maximum) return false;
  return true;
}
std::optional<double> Descriptor::decode(const nasa::Message& message) const {
  if (!this->valid() || message.code != this->code ||
      message.raw > ((uint64_t{1} << this->bits()) - 1) || message.raw < this->raw_min ||
      message.raw > this->raw_max)
    return {};
  for (size_t i = 0; i < this->sentinel_count; ++i)
    if (message.raw == this->sentinels[i]) return {};
  if (this->value_count && std::find(this->values.begin(), this->values.begin() + this->value_count,
                                     message.raw) == this->values.begin() + this->value_count)
    return {};
  const double raw =
      this->is_signed ? static_cast<double>(*nasa::signed_value(message)) : message.raw;
  const double value = raw * this->scale + this->offset;
  return std::isfinite(value) ? std::optional<double>(value) : std::nullopt;
}
void Observer::set_sentinel(size_t index, uint32_t value) {
  if (index >= this->descriptor_.sentinels.size()) return;
  this->descriptor_.sentinels[index] = value;
  this->descriptor_.sentinel_count = std::max<size_t>(this->descriptor_.sentinel_count, index + 1);
}
void Observer::set_valid_value(size_t index, uint32_t value) {
  if (index >= this->descriptor_.values.size()) return;
  this->descriptor_.values[index] = value;
  this->descriptor_.value_count = std::max<size_t>(this->descriptor_.value_count, index + 1);
}
void Observer::receive(const nasa::Packet& packet) {
  if (packet.source != this->source_ || packet.message_count > packet.messages.size() ||
      (this->source_.role() != nasa::AddressRole::INDOOR &&
       this->source_.role() != nasa::AddressRole::OUTDOOR) ||
      (packet.type != nasa::DataType::NOTIFICATION && packet.type != nasa::DataType::RESPONSE))
    return;
  for (size_t i = 0; i < packet.message_count; ++i)
    if (packet.messages[i].code == this->descriptor_.code)
      this->sink_.on_value(this->descriptor_.decode(packet.messages[i]));
}
}  // namespace esphome::samsung_hvac::advanced
