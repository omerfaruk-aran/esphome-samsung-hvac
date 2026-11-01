#include "protocol_selection.h"

namespace esphome::samsung_hvac {

const char *protocol_name(Protocol protocol) {
  return protocol == Protocol::NASA ? "NASA" : "Non-NASA";
}

}  // namespace esphome::samsung_hvac
