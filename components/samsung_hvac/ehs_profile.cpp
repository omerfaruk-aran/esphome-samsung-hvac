#include "ehs_profile.h"

namespace esphome::samsung_hvac::profiles::ehs {
const char* text_value(Field field, unsigned value) {
  if (field == Field::DHW_MODE) {
    switch (value) {
      case 0:
        return "Economy";
      case 1:
        return "Standard";
      case 2:
        return "Power";
      case 3:
        return "Force";
    }
  }
  if (field == Field::MODE) {
    switch (value) {
      case 0:
        return "Auto";
      case 1:
        return "Cool";
      case 2:
        return "Dry";
      case 3:
        return "Fan";
      case 4:
        return "Heat";
    }
  }
  return "Unknown";
}
void Profile::put_(Field field, std::optional<float> value, uint32_t& mask) {
  this->state_.values[static_cast<size_t>(field)] = value;
  mask |= field_mask(field);
}
void Profile::receive(const nasa::Packet& packet) {
  if ((packet.type != nasa::DataType::NOTIFICATION && packet.type != nasa::DataType::RESPONSE) ||
      packet.message_count > packet.messages.size())
    return;
  uint32_t mask = 0;
  for (size_t i = 0; i < packet.message_count; ++i) {
    const auto& m = packet.messages[i];
    if (this->outdoor_ && packet.source.value == *this->outdoor_) {
      if (m.code == 0x8204) this->put_(Field::OUTDOOR, *nasa::signed_value(m) / 10.0f, mask);
      if (m.code == 0x8235) this->put_(Field::ERROR_CODE, m.raw, mask);
    }
    if (packet.source.value != this->address_) continue;
    switch (m.code) {
      case 0x4235:
        this->put_(Field::DHW_TARGET, m.raw / 10.0f, mask);
        break;
      case 0x4247:
        this->put_(Field::OUTLET_TARGET, m.raw / 10.0f, mask);
        break;
      case 0x42e9:
        this->put_(Field::FLOW, m.raw / 10.0f, mask);
        break;
      case 0x4066:
        this->put_(Field::DHW_MODE, m.raw <= 3 ? std::optional<float>(m.raw) : std::nullopt, mask);
        break;
      case 0x4001:
        this->put_(Field::MODE, m.raw <= 4 ? std::optional<float>(m.raw) : std::nullopt, mask);
        break;
      default: {
        std::optional<Field> temperature, boolean;
        switch (m.code) {
          case 0x4237:
            temperature = Field::DHW_CURRENT;
            break;
          case 0x4238:
            temperature = Field::OUTLET_CURRENT;
            break;
          case 0x4236:
            temperature = Field::RETURN;
            break;
          case 0x4248:
            temperature = Field::WATER_LAW_SHIFT;
            break;
          case 0x427f:
            temperature = Field::WATER_LAW_TARGET;
            break;
          case 0x4000:
            boolean = Field::POWER;
            break;
          case 0x4065:
            boolean = Field::DHW_POWER;
            break;
          case 0x4089:
            boolean = Field::PUMP;
            break;
          case 0x402e:
            boolean = Field::DEFROST;
            break;
          case 0x4119:
            if (this->zone1_) boolean = Field::ZONE1_POWER;
            break;
          case 0x411e:
            if (this->zone2_) boolean = Field::ZONE2_POWER;
            break;
        }
        if (temperature) this->put_(*temperature, *nasa::signed_value(m) / 10.0f, mask);
        if (boolean)
          this->put_(*boolean, m.raw <= 1 ? std::optional<float>(m.raw) : std::nullopt, mask);
      }
    }
  }
  if (mask) this->sink_.on_state(this->state_, mask);
}
}  // namespace esphome::samsung_hvac::profiles::ehs
