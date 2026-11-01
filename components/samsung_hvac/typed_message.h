#pragma once
#include "nasa_packet.h"
#include <array>
#include <optional>
namespace esphome::samsung_hvac::advanced {
struct Descriptor {
  uint16_t code{0};
  bool is_signed{false};
  double scale{1}, offset{0};
  std::array<uint32_t, 4> sentinels{};
  uint8_t sentinel_count{0};
  std::array<uint32_t, 16> values{};
  uint8_t value_count{0};
  uint32_t raw_min{0}, raw_max{UINT32_MAX};
  unsigned bits() const;
  bool valid() const;
  std::optional<double> decode(const nasa::Message &message) const;
};
class ValueSink {
 public:
  virtual ~ValueSink() = default;
  virtual void on_value(std::optional<double> value) = 0;
};
class Observer {
 public:
  Observer(nasa::Address source, const Descriptor &descriptor, ValueSink &sink)
      : source_(source), descriptor_(descriptor), sink_(sink) {}
  void receive(const nasa::Packet &packet);
  void set_sentinel(size_t index, uint32_t value);
  void set_valid_value(size_t index, uint32_t value);
  void set_raw_range(uint32_t minimum, uint32_t maximum) { this->descriptor_.raw_min=minimum; this->descriptor_.raw_max=maximum; }
 private:
  nasa::Address source_;
  Descriptor descriptor_;
  ValueSink &sink_;
};
}  // namespace esphome::samsung_hvac::advanced
