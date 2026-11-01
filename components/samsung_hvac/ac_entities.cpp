#include "ac_entities.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome::samsung_hvac::profiles {
climate::ClimateTraits AcClimate::traits() {
  auto result = climate::ClimateTraits();
  result.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT_COOL,
                              climate::CLIMATE_MODE_COOL, climate::CLIMATE_MODE_DRY,
                              climate::CLIMATE_MODE_FAN_ONLY, climate::CLIMATE_MODE_HEAT});
  if (this->room_supported_)
    result.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  result.set_visual_min_temperature(16);
  result.set_visual_max_temperature(30);
  result.set_visual_temperature_step(1);
  return result;
}
void AcClimate::control(const climate::ClimateCall& call) {
  (void)call;
  ESP_LOGW("samsung_hvac.profile", "Control disabled for passive internal climate projection");
}
void AcClimate::observe(const State& state) {
  const auto power = state.get(Field::POWER), target = state.get(Field::TARGET),
             mode = state.get(Field::MODE);
  if (!power || !target || (*power != 0 && !mode)) return;
  if (*power == 0)
    this->mode = climate::CLIMATE_MODE_OFF;
  else {
    switch (static_cast<unsigned>(*mode)) {
      case 0:
        this->mode = climate::CLIMATE_MODE_HEAT_COOL;
        break;
      case 1:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
      case 2:
        this->mode = climate::CLIMATE_MODE_DRY;
        break;
      case 3:
        this->mode = climate::CLIMATE_MODE_FAN_ONLY;
        break;
      case 4:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      // Auto Heat has no exact ESPHome climate mode; preserve it in the text entity.
      default:
        return;
    }
  }
  this->target_temperature = *target;
  if (this->room_supported_) this->current_temperature = state.get(Field::ROOM).value_or(NAN);
  this->publish_state();
}
void AcDevice::on_state(const State& state, uint32_t mask) {
  for (size_t i = 0; i < FIELD_COUNT; ++i) {
    if ((mask & (1U << i)) == 0) continue;
    const auto value = state.values[i];
    if (this->sensors_[i]) this->sensors_[i]->publish_state(value.value_or(NAN));
    if (this->binaries_[i]) {
      if (value)
        this->binaries_[i]->publish_state(*value != 0);
      else
        this->binaries_[i]->invalidate_state();
    }
    if (this->texts_[i])
      this->texts_[i]->publish_state(
          value ? text_value(static_cast<Field>(i), static_cast<unsigned>(*value)) : "Unknown");
  }
  if (this->climate_ && (mask & (field_mask(Field::POWER) | field_mask(Field::TARGET) |
                                 field_mask(Field::ROOM) | field_mask(Field::MODE))))
    this->climate_->observe(state);
}
}  // namespace esphome::samsung_hvac::profiles
