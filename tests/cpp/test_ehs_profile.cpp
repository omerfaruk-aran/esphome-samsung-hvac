#include "ehs_profile.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include "bus_transport.h"
#include "nasa_engine.h"
#include "nasa_framer.h"
using namespace esphome::samsung_hvac;
using namespace esphome::samsung_hvac::profiles::ehs;
struct Sink : StateSink {
  unsigned calls{0};
  uint32_t fields{0};
  void on_state(const State&, uint32_t mask) override {
    ++calls;
    fields = mask;
  }
};
Frame fixture(const std::string& path) {
  std::ifstream file(path);
  assert(file);
  std::string hex;
  file >> hex;
  Frame result;
  assert(hex.size() % 2 == 0 && hex.size() / 2 <= result.data.size());
  result.size = hex.size() / 2;
  for (size_t i = 0; i < result.size; ++i)
    result.data[i] = static_cast<uint8_t>(std::stoul(hex.substr(2 * i, 2), nullptr, 16));
  return result;
}
struct SimulatedIO : BusIO {
  Frame bytes;
  size_t offset{0};
  unsigned writes{0}, directions{0};
  explicit SimulatedIO(const Frame& frame) : bytes(frame) {}
  bool read(uint8_t& value) override {
    if (offset == bytes.size) return false;
    value = bytes.data[offset++];
    return true;
  }
  size_t write_capacity() override { return 64; }
  bool write(const uint8_t*, size_t) override {
    ++writes;
    return true;
  }
  TxStatus tx_status() override { return TxStatus::DONE; }
  void set_direction(bool transmit) override {
    if (transmit) ++directions;
  }
};
struct Router : FrameSink, nasa::PacketSink, nasa::ReadSink {
  Profile& profile;
  nasa::Engine engine;
  explicit Router(Profile& p) : profile(p), engine({0x80ff00}, *this, *this) {}
  void on_frame(const Frame& frame) override { engine.receive(frame, 1); }
  void on_nasa_packet(const nasa::Packet& packet) override { profile.receive(packet); }
  void on_read_complete(const nasa::ReadResult&) override { assert(false); }
};
int main(int argc, char** argv) {
  assert(argc == 2);
  Sink golden_sink;
  Profile golden(0x200000, golden_sink);
  golden.set_zones(true, true);
  golden.set_outdoor_address(0x100000);
  Router router(golden);
  Diagnostics diagnostics;
  nasa::Framer framer(router, diagnostics);
  SimulatedIO io(fixture(std::string(argv[1]) + "/hydro_state.hex"));
  Transport transport(io, framer, diagnostics);
  for (uint32_t time = 1; time < 20; ++time) transport.loop(time);
  assert(diagnostics.rx_frames == 1 && router.engine.stats().decoded_packets == 1);
  assert(io.writes == 0 && io.directions == 0 && golden_sink.calls == 1);
  assert(golden.state().get(Field::DHW_CURRENT) == 46.5f);
  assert(golden.state().get(Field::OUTLET_CURRENT) == 33.5f);
  assert(golden.state().get(Field::RETURN) == 27.5f);
  assert(golden.state().get(Field::WATER_LAW_TARGET) == 34);
  assert(golden.state().get(Field::WATER_LAW_SHIFT) == -2.5f);
  assert(golden.state().get(Field::FLOW) == 12.3f);
  assert(golden.state().get(Field::POWER) == 1);
  assert(golden.state().get(Field::DHW_POWER) == 0);
  assert(golden.state().get(Field::PUMP) == 1);
  assert(golden.state().get(Field::DEFROST) == 0);
  assert(golden.state().get(Field::MODE) == 4);
  assert(golden.state().get(Field::DHW_MODE) == 1);
  assert(std::string(text_value(Field::MODE, 4)) == "Heat");
  assert(golden.state().get(Field::ZONE1_POWER) == 1 &&
         golden.state().get(Field::ZONE2_POWER) == 0);
  router.engine.receive(fixture(std::string(argv[1]) + "/outdoor_state.hex"), 20);
  assert(golden.state().get(Field::OUTDOOR) == -5 && golden.state().get(Field::ERROR_CODE) == 101);

  Sink sink, second_sink;
  Profile first(0x200000, sink), second(0x200001, second_sink);
  nasa::Packet p;
  p.source = {0x200000};
  p.type = nasa::DataType::NOTIFICATION;
  p.message_count = 4;
  p.messages[0] = {0x4237, 465};
  p.messages[1] = {0x4235, 500};
  p.messages[2] = {0x4248, 0xffe7};
  p.messages[3] = {0x4247, 350};
  first.receive(p);
  second.receive(p);
  assert(sink.calls == 1 && second_sink.calls == 0);
  assert(first.state().get(Field::DHW_CURRENT) == 46.5f);
  assert(first.state().get(Field::DHW_TARGET) == 50);
  assert(first.state().get(Field::WATER_LAW_SHIFT) == -2.5f);
  assert(first.state().get(Field::OUTLET_TARGET) == 35);
  assert(!first.state().get(Field::ZONE1_POWER) && !first.state().get(Field::ZONE2_POWER));
  p.type = nasa::DataType::ACK;
  p.messages[0].raw = 0;
  first.receive(p);
  assert(first.state().get(Field::DHW_CURRENT) == 46.5f);
  p.type = nasa::DataType::RESPONSE;
  p.message_count = 1;
  p.messages[0] = {0x411e, 1};
  first.receive(p);  // Zone 2 is not configured.
  assert(!first.state().get(Field::ZONE2_POWER));
  first.set_zones(true, true);
  first.receive(p);
  assert(first.state().get(Field::ZONE2_POWER) == 1);
  assert(!first.state().get(Field::ZONE1_POWER));
  assert(sink.fields == field_mask(Field::ZONE2_POWER));
  p.messages[0] = {0x4119, 0};
  first.receive(p);
  assert(first.state().get(Field::ZONE1_POWER) == 0);
  assert(first.state().get(Field::ZONE2_POWER) == 1);
  assert(sink.fields == field_mask(Field::ZONE1_POWER));
  p.messages[0] = {0x411e, 255};
  first.receive(p);
  assert(!first.state().get(Field::ZONE2_POWER));
  assert(first.state().get(Field::ZONE1_POWER) == 0);
  auto calls = sink.calls;
  p.messages[0] = {0x42d6, 400};
  first.receive(p);
  p.messages[0] = {0x4204, 210};
  first.receive(p);
  p.messages[0] = {0x4203, 220};
  first.receive(p);
  assert(sink.calls == calls);  // Ambiguous zone/room aliases remain disabled.
  first.set_outdoor_address(0x100000);
  p.messages[0] = {0x8204, 0xffce};
  first.receive(p);
  assert(!first.state().get(Field::OUTDOOR));
  p.source = {0x100000};
  first.receive(p);
  assert(first.state().get(Field::OUTDOOR) == -5);
  p.messages[0] = {0x4235, 300};
  first.receive(p);
  assert(first.state().get(Field::DHW_TARGET) == 50);
  p.source = {0x200000};
  p.messages[0] = {0x4066, 3};
  first.receive(p);
  assert(first.state().get(Field::DHW_MODE) == 3);
  assert(std::string(text_value(Field::DHW_MODE, 3)) == "Force");
  p.messages[0] = {0x4066, 4};
  first.receive(p);
  assert(!first.state().get(Field::DHW_MODE));
  p.messages[0] = {0x4236, 0xff9c};
  first.receive(p);
  assert(first.state().get(Field::RETURN) == -10);
  p.messages[0] = {0x42e9, 123};
  first.receive(p);
  assert(first.state().get(Field::FLOW) == 12.3f);
  std::cout << "All EHS profile assertions passed\n";
}
