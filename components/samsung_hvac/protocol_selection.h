#pragma once

namespace esphome::samsung_hvac {

enum class Protocol { NASA, NON_NASA };
const char* protocol_name(Protocol protocol);

}  // namespace esphome::samsung_hvac
