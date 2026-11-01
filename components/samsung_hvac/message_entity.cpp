#include "message_entity.h"
#include <cmath>
#include <limits>
namespace esphome::samsung_hvac::advanced {
void MessageEntity::on_value(std::optional<double> value) {
  if (this->sensor_)
    this->sensor_->publish_state(value && std::abs(*value) <= std::numeric_limits<float>::max()
                                     ? static_cast<float>(*value)
                                     : NAN);
  if (this->binary_) {
    if (value && (*value == 0 || *value == 1))
      this->binary_->publish_state(*value == 1);
    else
      this->binary_->invalidate_state();
  }
}
}  // namespace esphome::samsung_hvac::advanced
