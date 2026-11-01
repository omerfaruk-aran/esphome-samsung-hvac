#include "ac_profile.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
using namespace esphome::samsung_hvac;
using namespace esphome::samsung_hvac::profiles;
struct Sink : StateSink {
  unsigned calls{0};
  uint32_t fields{0};
  void on_state(const State&, uint32_t mask) override {
    ++calls;
    fields = mask;
  }
};
Frame fixture(const std::string& path) {
  Frame f;
  std::ifstream input(path);
  assert(input);
  std::string byte;
  input >> byte;
  assert(byte.size() % 2 == 0);
  for (size_t i = 0; i < byte.size(); i += 2) {
    assert(f.size < f.data.size());
    f.data[f.size++] = static_cast<uint8_t>(std::stoul(byte.substr(i, 2), nullptr, 16));
  }
  return f;
}
int main(int argc, char** argv) {
  assert(argc == 2);
  Sink golden_sink;
  AcProfile golden(Family::NASA_AC, 0x200000, golden_sink);
  golden.set_fans(1U << 2);
  golden.set_presets(1U << 9);
  golden.set_swing(true, true);
  nasa::Packet golden_packet;
  assert(nasa::decode(fixture(std::string(argv[1]) + "/nasa_ac_state.hex"), golden_packet) ==
         nasa::DecodeError::NONE);
  golden.receive(golden_packet);
  assert(golden.state().get(Field::TARGET) == 23.5f);
  assert(golden.state().get(Field::PIPE_IN) == -10.0f);
  assert(golden.state().get(Field::PIPE_OUT) == 12.5f);
  assert(golden.state().get(Field::HUMIDITY) == 50);
  assert(golden.state().get(Field::MODE) == 4);
  assert(golden.state().get(Field::FAN) == 2);
  assert(golden.state().get(Field::FILTER_WARNING) == 1);
  assert(golden.state().get(Field::DEFROSTING) == 0);
  assert(golden.state().get(Field::CLEANING) == 1);
  assert(golden.state().get(Field::VERTICAL_SWING) == 1);
  assert(golden.state().get(Field::HORIZONTAL_SWING) == 0);
  Sink legacy_golden_sink;
  AcProfile legacy_golden(Family::NON_NASA_AC, 0, legacy_golden_sink);
  legacy_golden.set_fans(1U << 1);
  non_nasa::Packet legacy_packet;
  assert(non_nasa::decode(fixture(std::string(argv[1]) + "/non_nasa_ac_state.hex"), legacy_packet));
  legacy_golden.receive(legacy_packet);
  assert(legacy_golden.state().get(Field::TARGET) == 18);
  assert(legacy_golden.state().get(Field::MODE) == 4);
  assert(legacy_golden.state().get(Field::FAN) == 1);

  Sink sink, other_sink;
  AcProfile first(Family::NASA_AC, 0x200000, sink), second(Family::NASA_AC, 0x200001, other_sink);
  nasa::Packet p;
  p.source = {0x200000};
  p.type = nasa::DataType::NOTIFICATION;
  p.message_count = 2;
  p.messages[0] = {0x4201, 235};
  p.messages[1] = {0x4000, 1};
  first.receive(p);
  second.receive(p);
  assert(sink.calls == 1 && other_sink.calls == 0);
  assert(first.state().get(Field::TARGET) == 23.5f);
  assert(first.state().get(Field::POWER) == 1);
  assert(!first.state().get(Field::ROOM));
  assert(sink.fields == (field_mask(Field::TARGET) | field_mask(Field::POWER)));
  p.type = nasa::DataType::ACK;
  p.messages[0].raw = 270;
  first.receive(p);
  assert(sink.calls == 1 && first.state().get(Field::TARGET) == 23.5f);
  p.type = nasa::DataType::RESPONSE;
  p.message_count = 1;
  p.messages[0] = {0x4204, 240};
  first.receive(p);
  assert(sink.calls == 1 && !first.state().get(Field::ROOM));
  p.messages[0] = {0x4205, 0xff9c};
  first.receive(p);
  assert(first.state().get(Field::PIPE_IN) == -10.0f);
  p.messages[0] = {0x4000, 255};
  first.receive(p);
  assert(!first.state().get(Field::POWER));
  // Repeat observations notify only the fields present, including invalidation.
  assert(sink.fields == field_mask(Field::POWER));
  p.messages[0] = {0x4006, 4};
  first.receive(p);
  assert(!first.state().get(Field::FAN));
  first.set_fans(1U << 4);
  first.receive(p);
  assert(first.state().get(Field::FAN) == 4);
  p.messages[0] = {0x4060, 9};
  first.receive(p);
  assert(!first.state().get(Field::PRESET));
  first.set_presets(1U << 9);
  first.receive(p);
  assert(first.state().get(Field::PRESET) == 9);
  assert(std::string(text_value(Field::PRESET, 9)) == "WindFree");
  p.messages[0] = {0x4011, 1};
  first.receive(p);
  assert(!first.state().get(Field::VERTICAL_SWING));
  first.set_swing(true, false);
  first.receive(p);
  assert(first.state().get(Field::VERTICAL_SWING) == 1);
  assert(!first.state().get(Field::HORIZONTAL_SWING));
  // An outdoor unit is an explicit separate source; no broadcast state sharing.
  first.set_outdoor_address(0x100000);
  p.source = {0x200000};
  p.messages[0] = {0x8204, 0xffce};
  first.receive(p);
  assert(!first.state().get(Field::OUTDOOR));
  p.source = {0x100000};
  first.receive(p);
  assert(first.state().get(Field::OUTDOOR) == -5.0f);
  p.messages[0] = {0x4201, 300};
  first.receive(p);
  assert(first.state().get(Field::TARGET) == 23.5f);
  // Non-NASA uses only Cmd20, with its own conversion and device address.
  Sink legacy_sink;
  AcProfile legacy(Family::NON_NASA_AC, 0, legacy_sink);
  non_nasa::Packet n;
  n.source = 0;
  n.command = 0x20;
  n.payload = {79, 80, 81, 0, 0x82, 2, 0, 82};
  legacy.receive(n);
  non_nasa::Status decoded;
  assert(non_nasa::decode_status(n, decoded));
  assert(legacy.state().get(Field::ROOM) == decoded.room.celsius());
  assert(legacy.state().get(Field::TARGET) == 24);
  assert(legacy.state().get(Field::ROOM) == 25);
  assert(legacy.state().get(Field::PIPE_IN) == 26);
  assert(legacy.state().get(Field::PIPE_OUT) == 27);
  const auto calls = legacy_sink.calls;
  n.source = 1;
  legacy.receive(n);
  n.source = 0;
  n.command = 0x54;
  legacy.receive(n);
  assert(legacy_sink.calls == calls);
  first.receive(n);
  legacy.receive(p);
  assert(legacy_sink.calls == calls);
  legacy.set_swing(true, true);
  n.command = 0x20;
  n.payload[3] = static_cast<uint8_t>(28U << 3);
  legacy.receive(n);
  assert(legacy.state().get(Field::VERTICAL_SWING) == 1);
  assert(legacy.state().get(Field::HORIZONTAL_SWING) == 1);
  n.payload[3] = 0x08;
  legacy.receive(n);  // Unknown raw swing enum 1, not the OFF enum.
  assert(!legacy.state().get(Field::VERTICAL_SWING));
  assert(!legacy.state().get(Field::HORIZONTAL_SWING));
  // No profile control assessment can schedule a bus write.
  assert(first.assess_control(Field::TARGET, 24) == ControlStatus::CONTROL_DISABLED);
  assert(first.assess_control(Field::TARGET, NAN) == ControlStatus::INVALID);
  assert(first.assess_control(Field::TARGET, 31) == ControlStatus::INVALID);
  assert(first.assess_control(Field::ROOM, 24) == ControlStatus::UNSUPPORTED);
  assert(legacy.assess_control(Field::PRESET, 9) == ControlStatus::UNSUPPORTED);
  std::cout << "All AC profile assertions passed\n";
}
