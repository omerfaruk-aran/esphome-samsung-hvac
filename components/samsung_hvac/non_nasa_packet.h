#pragma once

#include "bus_types.h"

namespace esphome::samsung_hvac::non_nasa {

struct Packet {
  uint8_t source{0};
  uint8_t destination{0};
  uint8_t command{0};
  std::array<uint8_t, 8> payload{};
};
enum class Mode : uint8_t {
  HEAT = 1,
  COOL = 2,
  DRY = 4,
  FAN = 8,
  AUTO_HEAT = 0x21,
  AUTO = 0x22,
  UNKNOWN = 0xff
};
enum class Fan : uint8_t {
  AUTO = 0,
  LOW_SPEED = 2,
  MEDIUM = 4,
  HIGH_SPEED = 5,
  FRESH = 6,
  TURBO = 7,
  UNKNOWN = 0xff
};
enum class Swing : uint8_t { VERTICAL = 26, HORIZONTAL = 27, BOTH = 28, OFF = 31, UNKNOWN = 0xff };
struct Temperature {
  bool fahrenheit{false};
  int16_t degrees{0};
  float celsius() const {
    return this->fahrenheit ? (this->degrees - 32) * (5.0f / 9.0f) : this->degrees;
  }
};
struct Status {
  Temperature target, room, pipe_in, pipe_out;
  bool power{false};
  Mode mode{Mode::UNKNOWN};
  Fan fan{Fan::UNKNOWN};
  Swing swing{Swing::UNKNOWN};
};
struct ControlState {
  bool power{false};
  uint8_t target{0};
  uint8_t room{0};
  Mode mode{Mode::UNKNOWN};
  Fan fan{Fan::UNKNOWN};
  Swing swing{Swing::UNKNOWN};
};

bool decode(const Frame& frame, Packet& packet);
bool encode(const Packet& packet, Frame& frame);
bool decode_status(const Packet& packet, Status& status);
bool control_from_status(const Status& status, ControlState& control);
bool valid_control(const ControlState& control);
bool encode_control(uint8_t destination, const ControlState& control, Frame& frame);
bool encode_registration(Frame& frame);

}  // namespace esphome::samsung_hvac::non_nasa
