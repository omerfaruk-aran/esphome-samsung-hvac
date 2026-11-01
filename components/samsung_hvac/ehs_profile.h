#pragma once
#include "nasa_packet.h"
#include <array>
#include <optional>

namespace esphome::samsung_hvac::profiles::ehs {
enum class Field : uint8_t { DHW_CURRENT, DHW_TARGET, OUTLET_CURRENT, OUTLET_TARGET, RETURN,
  WATER_LAW_SHIFT, WATER_LAW_TARGET, FLOW, OUTDOOR, ERROR_CODE, POWER, DHW_POWER, DHW_MODE,
  PUMP, DEFROST, ZONE1_POWER, ZONE2_POWER, MODE, COUNT };
constexpr size_t FIELD_COUNT = static_cast<size_t>(Field::COUNT);
constexpr uint32_t field_mask(Field field) { return 1U << static_cast<uint8_t>(field); }
struct State {
  std::array<std::optional<float>, FIELD_COUNT> values{};
  std::optional<float> get(Field field) const { return this->values[static_cast<size_t>(field)]; }
};
class StateSink {
 public:
  virtual ~StateSink() = default;
  virtual void on_state(const State &state, uint32_t mask) = 0;
};
const char *text_value(Field field, unsigned value);
class Profile {
 public:
  Profile(uint32_t address, StateSink &sink) : address_(address), sink_(sink) {}
  void set_outdoor_address(uint32_t address) { this->outdoor_ = address; }
  void set_zones(bool first, bool second) { this->zone1_ = first; this->zone2_ = second; }
  void receive(const nasa::Packet &packet);
  const State &state() const { return this->state_; }
 private:
  void put_(Field field, std::optional<float> value, uint32_t &mask);
  uint32_t address_;
  std::optional<uint32_t> outdoor_;
  StateSink &sink_;
  State state_;
  bool zone1_{false}, zone2_{false};
};
}  // namespace esphome::samsung_hvac::profiles::ehs
