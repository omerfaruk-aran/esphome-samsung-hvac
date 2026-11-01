#pragma once
#include "nasa_packet.h"
#include "non_nasa_packet.h"
#include <array>
#include <optional>

namespace esphome::samsung_hvac::profiles {
enum class Family { NASA_AC, NON_NASA_AC };
enum class Field : uint8_t { TARGET, ROOM, PIPE_IN, PIPE_OUT, OUTDOOR, ERROR_CODE, HUMIDITY,
                            POWER, FILTER_WARNING, DEFROSTING, SILENCE, CLEANING,
                            VERTICAL_SWING, HORIZONTAL_SWING, MODE, FAN, PRESET, COUNT };
constexpr size_t FIELD_COUNT = static_cast<size_t>(Field::COUNT);
constexpr uint32_t field_mask(Field field) { return 1U << static_cast<uint8_t>(field); }
struct State {
  std::array<std::optional<float>, FIELD_COUNT> values{};
  std::optional<float> get(Field field) const { return this->values[static_cast<size_t>(field)]; }
};
class StateSink {
 public:
  virtual ~StateSink() = default;
  // Synchronous, borrowed state; mask identifies observations, including invalidations.
  virtual void on_state(const State &state, uint32_t mask) = 0;
};
enum class ControlStatus { CONTROL_DISABLED, UNSUPPORTED, INVALID };
const char *text_value(Field field, unsigned value);

class AcProfile {
 public:
  AcProfile(Family family, uint32_t address, StateSink &sink) : family_(family), address_(address), sink_(sink) {}
  void set_outdoor_address(uint32_t address) { this->outdoor_ = address; }
  void set_fans(uint32_t mask) { this->fans_ = mask; }
  void set_presets(uint32_t mask) { this->presets_ = mask; }
  void set_swing(bool vertical, bool horizontal) { this->vertical_ = vertical; this->horizontal_ = horizontal; }
  void receive(const nasa::Packet &packet);
  void receive(const non_nasa::Packet &packet);
  const State &state() const { return this->state_; }
  // Offline validation only. Profiles have no scheduler or bus handle and cannot transmit.
  ControlStatus assess_control(Field field, float value) const;
 private:
  void put_(Field field, std::optional<float> value, uint32_t &mask);
  void enum_(Field field, unsigned value, uint32_t &mask);
  bool capability_(Field field, unsigned value) const;
  Family family_;
  uint32_t address_;
  std::optional<uint32_t> outdoor_;
  StateSink &sink_;
  State state_;
  uint32_t fans_{0}, presets_{0};
  bool vertical_{false}, horizontal_{false};
};
}  // namespace esphome::samsung_hvac::profiles
