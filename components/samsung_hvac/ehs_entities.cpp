#include "ehs_entities.h"
#include <cmath>
namespace esphome::samsung_hvac::profiles::ehs {
void Device::on_state(const State &state, uint32_t mask) {
  for (size_t i = 0; i < FIELD_COUNT; ++i) {
    if ((mask & (1U << i)) == 0) continue;
    const auto value = state.values[i];
    if (this->sensors_[i]) this->sensors_[i]->publish_state(value.value_or(NAN));
    if (this->binaries_[i]) {
      if (value) this->binaries_[i]->publish_state(*value != 0);
      else this->binaries_[i]->invalidate_state();
    }
    if (this->texts_[i]) this->texts_[i]->publish_state(value ? text_value(static_cast<Field>(i), static_cast<unsigned>(*value)) : "Unknown");
  }
}
}  // namespace esphome::samsung_hvac::profiles::ehs
