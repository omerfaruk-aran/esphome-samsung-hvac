#include "ac_profile.h"
#include <cmath>

namespace esphome::samsung_hvac::profiles {
const char *text_value(Field field, unsigned value) {
  if (field == Field::MODE) {
    switch (value) { case 0: return "Auto"; case 1: return "Cool"; case 2: return "Dry";
      case 3: return "Fan"; case 4: return "Heat"; case 5: return "Auto Heat"; }
  } else if (field == Field::FAN) {
    switch (value) { case 0: return "Auto"; case 1: return "Low"; case 2: return "Medium";
      case 3: return "High"; case 4: return "Turbo"; case 5: return "Fresh"; }
  } else if (field == Field::PRESET) {
    switch (value) { case 0: return "None"; case 1: return "Sleep"; case 2: return "Quiet";
      case 3: return "Fast"; case 6: return "Long Reach"; case 7: return "Eco"; case 9: return "WindFree"; }
  }
  return "Unknown";
}
void AcProfile::put_(Field field, std::optional<float> value, uint32_t &mask) {
  this->state_.values[static_cast<size_t>(field)] = value;
  mask |= field_mask(field);
}
bool AcProfile::capability_(Field field, unsigned value) const {
  if (field == Field::FAN) return value < 6 && (this->fans_ & (1U << value));
  if (field == Field::PRESET) return this->family_ == Family::NASA_AC && value < 10 &&
      (this->presets_ & (1U << value)) && value != 4 && value != 5 && value != 8;
  if (field == Field::VERTICAL_SWING) return this->vertical_;
  if (field == Field::HORIZONTAL_SWING) return this->horizontal_;
  return true;
}
void AcProfile::enum_(Field field, unsigned value, uint32_t &mask) {
  bool valid = this->capability_(field, value);
  if (field == Field::MODE) valid &= value <= (this->family_ == Family::NASA_AC ? 4U : 5U);
  this->put_(field, valid ? std::optional<float>(value) : std::nullopt, mask);
}
void AcProfile::receive(const nasa::Packet &packet) {
  if (this->family_ != Family::NASA_AC ||
      (packet.type != nasa::DataType::NOTIFICATION && packet.type != nasa::DataType::RESPONSE)) return;
  uint32_t mask = 0;
  for (size_t i = 0; i < packet.message_count; ++i) {
    const auto &m = packet.messages[i];
    if (this->outdoor_ && packet.source.value == *this->outdoor_) {
      if (m.code == 0x8204) this->put_(Field::OUTDOOR, static_cast<int16_t>(m.raw) / 10.0f, mask);
      if (m.code == 0x8235) this->put_(Field::ERROR_CODE, m.raw, mask);
    }
    if (packet.source.value != this->address_) continue;
    switch (m.code) {
      // Disputed 0x4203/0x4204 meanings are deliberately absent.
      case 0x4201: this->put_(Field::TARGET, m.raw / 10.0f, mask); break;
      case 0x4205: this->put_(Field::PIPE_IN, static_cast<int16_t>(m.raw) / 10.0f, mask); break;
      case 0x4206: this->put_(Field::PIPE_OUT, static_cast<int16_t>(m.raw) / 10.0f, mask); break;
      case 0x4038: this->put_(Field::HUMIDITY, m.raw <= 100 ? std::optional<float>(m.raw) : std::nullopt, mask); break;
      case 0x4001: this->enum_(Field::MODE, m.raw, mask); break;
      case 0x4006: this->enum_(Field::FAN, m.raw, mask); break;
      case 0x4060: this->enum_(Field::PRESET, m.raw, mask); break;
      default: {
        std::optional<Field> field;
        switch (m.code) {
          case 0x4000: field = Field::POWER; break;
          case 0x4027: field = Field::FILTER_WARNING; break;
          case 0x402e: field = Field::DEFROSTING; break;
          case 0x4046: field = Field::SILENCE; break;
          case 0x4111: field = Field::CLEANING; break;
          case 0x4011: if (this->vertical_) field = Field::VERTICAL_SWING; break;
          case 0x407e: if (this->horizontal_) field = Field::HORIZONTAL_SWING; break;
        }
        if (field) this->put_(*field, m.raw <= 1 ? std::optional<float>(m.raw) : std::nullopt, mask);
      }
    }
  }
  if (mask) this->sink_.on_state(this->state_, mask);
}
void AcProfile::receive(const non_nasa::Packet &packet) {
  if (this->family_ != Family::NON_NASA_AC || packet.source != this->address_) return;
  non_nasa::Status status;
  if (!non_nasa::decode_status(packet, status)) return;
  uint32_t mask = 0;
  this->put_(Field::TARGET, status.target.celsius(), mask);
  this->put_(Field::ROOM, status.room.celsius(), mask);
  this->put_(Field::PIPE_IN, status.pipe_in.celsius(), mask);
  this->put_(Field::PIPE_OUT, status.pipe_out.celsius(), mask);
  this->put_(Field::POWER, status.power ? 1 : 0, mask);
  unsigned mode = 255, fan = 255;
  switch (status.mode) {
    case non_nasa::Mode::AUTO: mode = 0; break;
    case non_nasa::Mode::COOL: mode = 1; break;
    case non_nasa::Mode::DRY: mode = 2; break;
    case non_nasa::Mode::FAN: mode = 3; break;
    case non_nasa::Mode::HEAT: mode = 4; break;
    case non_nasa::Mode::AUTO_HEAT: mode = 5; break;
    default: break;
  }
  switch (status.fan) {
    case non_nasa::Fan::AUTO: fan = 0; break;
    case non_nasa::Fan::LOW_SPEED: fan = 1; break;
    case non_nasa::Fan::MEDIUM: fan = 2; break;
    case non_nasa::Fan::HIGH_SPEED: fan = 3; break;
    case non_nasa::Fan::TURBO: fan = 4; break;
    case non_nasa::Fan::FRESH: fan = 5; break;
    default: break;
  }
  this->enum_(Field::MODE, mode, mask);
  this->enum_(Field::FAN, fan, mask);
  const bool known = status.swing == non_nasa::Swing::VERTICAL || status.swing == non_nasa::Swing::HORIZONTAL ||
      status.swing == non_nasa::Swing::BOTH || status.swing == non_nasa::Swing::OFF;
  if (this->vertical_) this->put_(Field::VERTICAL_SWING, known ?
      std::optional<float>(status.swing == non_nasa::Swing::VERTICAL || status.swing == non_nasa::Swing::BOTH) : std::nullopt, mask);
  if (this->horizontal_) this->put_(Field::HORIZONTAL_SWING, known ?
      std::optional<float>(status.swing == non_nasa::Swing::HORIZONTAL || status.swing == non_nasa::Swing::BOTH) : std::nullopt, mask);
  this->sink_.on_state(this->state_, mask);
}
ControlStatus AcProfile::assess_control(Field field, float value) const {
  if (!std::isfinite(value)) return ControlStatus::INVALID;
  switch (field) {
    case Field::TARGET: if (value < 16 || value > 30 || std::floor(value) != value) return ControlStatus::INVALID; break;
    case Field::POWER: if (value != 0 && value != 1) return ControlStatus::INVALID; break;
    case Field::MODE: case Field::FAN: case Field::PRESET:
      if (value < 0 || value > 255 || std::floor(value) != value) return ControlStatus::INVALID;
      if (!this->capability_(field, static_cast<unsigned>(value)) ||
          (field == Field::MODE && value > (this->family_ == Family::NASA_AC ? 4 : 5))) return ControlStatus::UNSUPPORTED;
      break;
    case Field::VERTICAL_SWING: case Field::HORIZONTAL_SWING:
      if (value != 0 && value != 1) return ControlStatus::INVALID;
      if (!this->capability_(field, 0)) return ControlStatus::UNSUPPORTED;
      break;
    default: return ControlStatus::UNSUPPORTED;
  }
  return ControlStatus::CONTROL_DISABLED;
}
}  // namespace esphome::samsung_hvac::profiles
