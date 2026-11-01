#include "samsung_hvac.h"
#include "esphome/core/log.h"

namespace esphome::samsung_hvac {

static const char *const TAG = "samsung_hvac";

void SamsungHVAC::dump_config() {
  ESP_LOGCONFIG(TAG, "Samsung HVAC foundation:");
  ESP_LOGCONFIG(TAG, "  Protocol: %s", protocol_name(this->protocol_));
  ESP_LOGCONFIG(TAG, "  No bus processing or control is implemented in this foundation.");
}

}  // namespace esphome::samsung_hvac
