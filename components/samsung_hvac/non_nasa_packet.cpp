#include "non_nasa_packet.h"
#include <algorithm>

namespace esphome::samsung_hvac::non_nasa {

namespace {
uint8_t checksum(const uint8_t *bytes) {
  uint8_t result = 0;
  for (size_t i = 1; i < 12; ++i) result ^= bytes[i];
  return result;
}
Temperature temperature(uint8_t byte) {
  return Temperature{(byte & 0x80) != 0, static_cast<int16_t>((byte & 0x80) != 0 ? byte & 0x7f : int(byte) - 55)};
}
int mode_bits(Mode mode) {
  switch (mode) {
    case Mode::AUTO: case Mode::AUTO_HEAT: return 0;
    case Mode::COOL: return 1;
    case Mode::DRY: return 2;
    case Mode::FAN: return 3;
    case Mode::HEAT: return 4;
    default: return -1;
  }
}
int fan_bits(Fan fan) {
  switch (fan) {
    case Fan::AUTO: return 0;
    case Fan::LOW_SPEED: return 64;
    case Fan::MEDIUM: return 128;
    case Fan::HIGH_SPEED: case Fan::FRESH: return 160;
    case Fan::TURBO: return 224;
    default: return -1;
  }
}
bool known_swing(Swing swing) {
  return swing == Swing::OFF || swing == Swing::VERTICAL || swing == Swing::HORIZONTAL || swing == Swing::BOTH;
}
}  // namespace

bool decode(const Frame &frame, Packet &packet) {
  packet = Packet{};
  if (frame.size != 14 || frame.data[0] != 0x32 || frame.data[13] != 0x34 ||
      checksum(frame.data.data()) != frame.data[12])
    return false;
  packet.source = frame.data[1];
  packet.destination = frame.data[2];
  packet.command = frame.data[3];
  std::copy_n(frame.data.data() + 4, 8, packet.payload.begin());
  return true;
}

bool encode(const Packet &packet, Frame &frame) {
  frame.size = 14;
  frame.data[0] = 0x32;
  frame.data[1] = packet.source;
  frame.data[2] = packet.destination;
  frame.data[3] = packet.command;
  std::copy(packet.payload.begin(), packet.payload.end(), frame.data.begin() + 4);
  frame.data[12] = checksum(frame.data.data());
  frame.data[13] = 0x34;
  return true;
}

bool decode_status(const Packet &packet, Status &status) {
  status = Status{};
  if (packet.command != 0x20) return false;
  status.target = temperature(packet.payload[0]);
  status.room = temperature(packet.payload[1]);
  status.pipe_in = temperature(packet.payload[2]);
  status.pipe_out = temperature(packet.payload[7]);
  status.mode = static_cast<Mode>(packet.payload[4] & 0x3f);
  status.power = (packet.payload[4] & 0x80) != 0;
  status.fan = static_cast<Fan>(packet.payload[3] & 7);
  uint8_t swing = packet.payload[3] >> 3;
  status.swing = swing == 0 ? Swing::OFF : static_cast<Swing>(swing);
  return true;
}

bool valid_control(const ControlState &control) {
  return control.target >= 16 && control.target <= 30 && control.room <= 50 &&
         mode_bits(control.mode) >= 0 && fan_bits(control.fan) >= 0 && known_swing(control.swing);
}

bool control_from_status(const Status &status, ControlState &control) {
  control = ControlState{};
  if (status.target.fahrenheit || status.room.fahrenheit || status.target.degrees < 16 ||
      status.target.degrees > 30 || status.room.degrees < 0 || status.room.degrees > 50)
    return false;
  control = ControlState{status.power, static_cast<uint8_t>(status.target.degrees),
                         static_cast<uint8_t>(status.room.degrees), status.mode, status.fan, status.swing};
  return valid_control(control);
}

bool encode_control(uint8_t destination, const ControlState &control, Frame &frame) {
  frame.size = 0;
  if (destination >= 0x80 || !valid_control(control)) return false;
  Packet packet;
  packet.source = 0xd0;
  packet.destination = destination;
  packet.command = 0xb0;
  packet.payload = {static_cast<uint8_t>(control.swing), control.room,
                    static_cast<uint8_t>(control.target | fan_bits(control.fan)),
                    static_cast<uint8_t>(mode_bits(control.mode)),
                    static_cast<uint8_t>(control.power ? 0xf4 : 0xc4), 0x21, 0, 0};
  return encode(packet, frame);
}

bool encode_registration(Frame &frame) {
  Packet packet;
  packet.source = 0xd0;
  packet.destination = 0xc8;
  packet.command = 0xd1;
  packet.payload[0] = 0xd2;
  return encode(packet, frame);
}

}  // namespace esphome::samsung_hvac::non_nasa
