#include "non_nasa_packet.h"
#include "non_nasa_registration.h"
#include "non_nasa_control_queue.h"
#include "non_nasa_engine.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

using namespace esphome::samsung_hvac;
using namespace esphome::samsung_hvac::non_nasa;

Frame fixture(const std::string& directory, const char* name) {
  std::ifstream file(directory + "/" + name + ".hex");
  std::string hex;
  file >> hex;
  assert(hex.size() == 28);
  Frame result;
  result.size = 14;
  for (size_t i = 0; i < 14; ++i)
    result.data[i] = static_cast<uint8_t>(std::stoul(hex.substr(2 * i, 2), nullptr, 16));
  return result;
}

Packet packet(const Frame& frame) {
  Packet result;
  assert(decode(frame, result));
  return result;
}

bool equal(const Frame& a, const Frame& b) {
  return a.size == b.size && std::equal(a.data.begin(), a.data.begin() + a.size, b.data.begin());
}

Packet status_packet(uint8_t address, uint8_t target, bool power, Mode mode = Mode::COOL,
                     Fan fan = Fan::AUTO, Swing swing = Swing::OFF) {
  Packet result;
  result.source = address;
  result.destination = 0xc8;
  result.command = 0x20;
  result.payload = {
      static_cast<uint8_t>(target + 55),
      80,
      80,
      static_cast<uint8_t>((static_cast<uint8_t>(swing) << 3) | static_cast<uint8_t>(fan)),
      static_cast<uint8_t>((power ? 0x80 : 0) | static_cast<uint8_t>(mode)),
      0,
      0,
      80};
  return result;
}

Packet ack_for(const Frame& frame) {
  auto result = packet(frame);
  result.source = result.destination;
  result.destination = 0xd0;
  result.command = 0x54;
  return result;
}

void test_codec(const std::string& directory) {
  auto raw = fixture(directory, "status_off");
  auto data = packet(raw);
  Status status;
  assert(decode_status(data, status));
  assert(status.target.degrees == 23 && status.room.degrees == 25 && status.pipe_in.degrees == -5);
  assert(!status.power && status.mode == Mode::COOL && status.swing == Swing::OFF);
  Frame encoded;
  assert(encode(data, encoded) && equal(raw, encoded));
  data = packet(fixture(directory, "status_fahrenheit"));
  assert(decode_status(data, status));
  assert(status.target.fahrenheit && std::fabs(status.target.celsius() - 22.222222F) < 0.001F);
  ControlState control;
  assert(!control_from_status(status, control));
  control = ControlState{true, 18, 25, Mode::HEAT, Fan::LOW_SPEED, Swing::VERTICAL};
  assert(encode_control(0, control, encoded));
  assert(equal(encoded, fixture(directory, "control_heat18_low")));
  control.target = 15;
  assert(!encode_control(0, control, encoded) && encoded.size == 0);
  control.target = 18;
  assert(!encode_control(0xc8, control, encoded));
  control.mode = Mode::UNKNOWN;
  assert(!encode_control(0, control, encoded));
  control.mode = Mode::HEAT;
  control.fan = Fan::UNKNOWN;
  assert(!encode_control(0, control, encoded));
  control.fan = Fan::LOW_SPEED;
  control.swing = Swing::UNKNOWN;
  assert(!encode_control(0, control, encoded));
  assert(encode_registration(encoded) && equal(encoded, fixture(directory, "registration")));
  for (size_t length = 0; length < 14; ++length) {
    auto bad = raw;
    bad.size = length;
    assert(!decode(bad, data));
  }
  auto bad = raw;
  bad.data[12] ^= 1;
  assert(!decode(bad, data));
  auto telemetry = packet(fixture(directory, "telemetry_f3"));
  assert(telemetry.command == 0xf3 && telemetry.payload[4] == 41 && telemetry.payload[5] == 163);
}

void test_registration(const std::string& directory) {
  Registration registration;
  Frame frame;
  uint32_t token;
  assert(!registration.prepare(0, frame, token));
  registration.set_enabled(true);
  assert(!registration.prepare(0, frame, token));
  registration.observe(packet(fixture(directory, "status_off")), 0);
  assert(registration.prepare(0, frame, token));
  assert(equal(frame, fixture(directory, "registration")));
  registration.on_transmitted(token, 0, true);
  assert(!registration.prepare(9999, frame, token));
  assert(registration.prepare(10000, frame, token));
  registration.on_transmitted(token, 10000, true);
  assert(registration.prepare(20000, frame, token));
  registration.on_transmitted(token, 20000, true);
  assert(!registration.prepare(30000, frame, token));  // Bounded attempts without C6 confirmation.
  registration.observe(packet(fixture(directory, "poll")), 30001);
  assert(registration.registered());
  registration.set_keepalive(true);
  registration.observe(packet(fixture(directory, "broadcast")), 40000);
  assert(!registration.prepare(40029, frame, token));
  assert(registration.prepare(40030, frame, token));
  registration.on_transmitted(token, 40030, true);
  registration.observe(packet(fixture(directory, "broadcast")), 45000);
  assert(!registration.prepare(45030, frame, token));
  assert(!registration.request_wakeup(45030));  // Shared registration/keepalive/wake limiter.
  assert(!registration.prepare(60000, frame,
                               token));  // A rate-limited broadcast must not become a stale reply.
  assert(registration.request_wakeup(60001));
  assert(registration.prepare(60001, frame, token));
  registration.on_transmitted(token, 60001, true);
  Registration wrapped;
  wrapped.set_enabled(true);
  uint32_t start = std::numeric_limits<uint32_t>::max() - 1000;
  wrapped.observe(packet(fixture(directory, "status_off")), start);
  assert(wrapped.prepare(start, frame, token));
  wrapped.on_transmitted(token, start, true);
  assert(!wrapped.prepare(start + 9999, frame, token));
  assert(wrapped.prepare(start + 10000, frame, token));
}

Patch target(uint8_t value) {
  Patch patch;
  patch.target = value;
  return patch;
}
Patch fan(Fan value) {
  Patch patch;
  patch.fan = value;
  return patch;
}

void test_coalescing_and_echo(const std::string& directory) {
  ControlQueue queue;
  Frame frame;
  uint32_t token;
  queue.observe(status_packet(0, 23, false), 0);
  assert(!queue.submit(0, target(18), 0));
  queue.set_enabled(true);
  Patch mode;
  mode.mode = Mode::HEAT;
  assert(queue.submit(0, mode, 1));
  assert(queue.submit(0, target(18), 2));
  assert(queue.submit(0, fan(Fan::LOW_SPEED), 3));
  Patch swing;
  swing.swing = Swing::VERTICAL;
  assert(queue.submit(0, swing, 4));
  assert(!queue.prepare(4, frame, token));  // C6 grants one preparation window.
  queue.observe(packet(fixture(directory, "poll")), 100);
  assert(queue.prepare(100, frame, token));
  assert(equal(frame, fixture(directory, "control_heat18_low")));
  queue.on_transmitted(token, 100, true);
  queue.observe(ack_for(frame), 101);
  assert(queue.pending(0));  // Echo ACK alone never verifies control.
  assert(queue.submit(0, target(19), 102));
  auto first = frame;
  queue.observe(ack_for(first), 103);
  assert(queue.pending(0));
  queue.observe(packet(fixture(directory, "poll")), 1000);
  assert(queue.prepare(1000, frame, token));
  assert((frame.data[6] & 31) == 19 && frame.data[7] == 4 && frame.data[8] == 0xf4);
  queue.on_transmitted(token, 1000, true);
  queue.observe(ack_for(first), 1001);
  queue.observe(packet(fixture(directory, "ack_non_echo")), 1002);
  queue.observe(status_packet(0, 18, true, Mode::HEAT, Fan::LOW_SPEED, Swing::VERTICAL), 1003);
  assert(queue.pending(0));
  queue.observe(status_packet(0, 19, true, Mode::HEAT, Fan::LOW_SPEED, Swing::VERTICAL), 1004);
  assert(!queue.pending(0) && queue.result(0) == ControlResult::VERIFIED);
  queue.observe(status_packet(0, 26, true, Mode::HEAT),
                1500);  // Remote change becomes the new base.
  assert(queue.submit(0, fan(Fan::HIGH_SPEED), 1501));
  queue.observe(packet(fixture(directory, "poll")), 2000);
  assert(queue.prepare(2000, frame, token));
  assert((frame.data[6] & 31) == 26 && (frame.data[6] & 0xe0) == 160 && frame.data[8] == 0xf4);
}

void test_prepared_generations_and_devices(const std::string& directory) {
  ControlQueue queue;
  queue.set_enabled(true);
  queue.observe(status_packet(0, 23, false), 0);
  queue.observe(status_packet(1, 24, true), 0);
  assert(queue.submit(0, target(18), 1));
  assert(!queue.needs_wakeup(1000) && queue.needs_wakeup(1001));
  Frame first, second;
  uint32_t old_token, token;
  auto poll = packet(fixture(directory, "poll"));
  queue.observe(poll, 100);
  assert(queue.prepare(100, first, old_token));
  assert(queue.submit(0, target(20), 101));
  assert(!queue.is_current(old_token));
  queue.on_transmitted(old_token, 102, false);  // Cancellation cannot fail the newer intent.
  assert(queue.pending(0));
  queue.observe(poll, 1000);
  assert(queue.prepare(1000, second, token));
  assert(token != old_token && (second.data[6] & 31) == 20);
  queue.on_transmitted(old_token, 1001, true);
  queue.on_transmitted(token, 1002, true);
  queue.observe(status_packet(1, 20, false), 1003);
  assert(queue.pending(0));
  queue.observe(status_packet(0, 20, false), 1004);
  assert(!queue.pending(0));
  assert(queue.submit(1, target(21), 1005));
  queue.observe(poll, 2000);
  assert(queue.prepare(2000, second, token) && second.data[2] == 1);
  ControlQueue isolated;
  isolated.set_enabled(true);
  assert(!isolated.submit(0, target(18), 2000));
}

void test_retries_sleep_and_remote_override(const std::string& directory) {
  ControlQueue queue;
  queue.set_enabled(true);
  Frame frame;
  uint32_t token;
  auto poll = packet(fixture(directory, "poll"));
  queue.observe(status_packet(0, 23, true), 0);
  assert(queue.submit(0, target(18), 1));
  for (uint32_t attempt = 0; attempt < 3; ++attempt) {
    uint32_t now = 100 + attempt * 4500;
    queue.observe(poll, now);
    assert(queue.prepare(now, frame, token));
    queue.on_transmitted(token, now, true);
    queue.tick(now + 4500);
  }
  assert(!queue.pending(0) && queue.result(0) == ControlResult::TIMEOUT);
  queue.observe(status_packet(0, 23, true), 15000);
  assert(queue.submit(0, target(18), 15001));
  queue.tick(30001);  // No polls / sleeping device.
  assert(!queue.pending(0) && queue.result(0) == ControlResult::TIMEOUT);
  assert(!queue.needs_wakeup(30001));
  queue.observe(status_packet(0, 23, true), 31000);
  Patch off;
  off.power = false;
  assert(queue.submit(0, off, 31001));
  assert(queue.submit(0, fan(Fan::LOW_SPEED), 31002));
  queue.observe(poll, 32000);
  assert(queue.prepare(32000, frame, token));
  queue.on_transmitted(token, 32000, true);
  queue.observe(ack_for(frame), 32001);
  queue.observe(status_packet(0, 23, false),
                32002);  // Off normalizes fan, readback cannot verify the fan.
  queue.observe(status_packet(0, 23, true), 33000);  // Remote turned it back on.
  assert(!queue.pending(0) && queue.result(0) == ControlResult::SUPERSEDED);
  assert(queue.submit(0, fan(Fan::HIGH_SPEED), 33001));
  queue.observe(poll, 34000);
  assert(queue.prepare(34000, frame, token) && frame.data[8] == 0xf4);
}

struct Packets : PacketSink {
  unsigned count{0};
  void on_non_nasa_packet(const Packet&) override { ++count; }
};

void test_engine_and_bounds(const std::string& directory) {
  Packets sink;
  Engine engine(sink);
  auto status = fixture(directory, "status_off");
  engine.receive(status, 0);
  engine.receive(fixture(directory, "poll"), 1);
  assert(sink.count == 2 && engine.stats().decoded_packets == 2);
  status.data[12] ^= 1;
  engine.receive(status, 2);
  assert(sink.count == 2 && engine.stats().invalid_packets == 1);
  Frame output;
  uint32_t token;
  assert(!engine.registration().prepare(10000, output, token));
  assert(!engine.controls().submit(0, target(18), 10000));
  Engine active(sink);
  active.receive(fixture(directory, "status_off"), 0);
  active.receive(fixture(directory, "poll"), 1);
  active.registration().set_enabled(true);
  active.controls().set_enabled(true);
  assert(active.controls().submit(0, target(18), 2));
  active.tick(1002);
  assert(active.registration().prepare(1002, output, token));
  active.registration().on_transmitted(token, 1002, true);
  active.receive(fixture(directory, "poll"), 1003);
  assert(!active.controls().prepare(1003, output,
                                    token));  // Indoor must produce fresh Cmd20 after wake.
  active.receive(fixture(directory, "status_off"), 1004);
  active.receive(fixture(directory, "poll"), 1005);
  assert(active.controls().prepare(1005, output, token));
  ControlQueue queue;
  queue.set_enabled(true);
  for (uint8_t address = 0; address < 8; ++address) {
    queue.observe(status_packet(address, 23, true), 0);
    assert(queue.submit(address, target(18), 1));
  }
  queue.observe(status_packet(8, 23, true), 0);
  assert(!queue.submit(8, target(18), 1));
  assert(!queue.submit(0, target(31), 1));
  assert(!queue.submit(0, target(18), 30000));  // Stale observed full-state base.
  ControlQueue wrapped;
  wrapped.set_enabled(true);
  uint32_t now = std::numeric_limits<uint32_t>::max() - 100;
  wrapped.observe(status_packet(0, 23, true), now);
  assert(wrapped.submit(0, target(18), now));
  wrapped.observe(packet(fixture(directory, "poll")), now);
  assert(wrapped.prepare(now, output, token));
  wrapped.on_transmitted(token, now, true);
  wrapped.tick(now + 4499);
  assert(wrapped.pending(0));
  wrapped.observe(status_packet(0, 18, true), now + 4500);
  assert(!wrapped.pending(0) && wrapped.result(0) == ControlResult::VERIFIED);
}

void test_prepared_observation_races(const std::string& directory) {
  auto poll = packet(fixture(directory, "poll"));
  ControlQueue queue;
  queue.set_enabled(true);
  queue.observe(status_packet(0, 23, true), 0);
  assert(queue.submit(0, target(18), 1));
  Frame frame;
  uint32_t token;
  queue.observe(poll, 100);
  assert(queue.prepare(100, frame, token) && frame.data[8] == 0xf4);
  queue.observe(status_packet(0, 23, false),
                101);  // Remote power-off is not part of our target-only intent.
  assert(!queue.is_current(token));
  queue.on_transmitted(token, 102, false);
  queue.observe(poll, 200);
  assert(queue.prepare(200, frame, token) && frame.data[8] == 0xc4 && (frame.data[6] & 31) == 18);

  ControlQueue achieved;
  achieved.set_enabled(true);
  achieved.observe(status_packet(0, 23, true), 0);
  assert(achieved.submit(0, target(18), 1));
  achieved.observe(poll, 100);
  assert(achieved.prepare(100, frame, token));
  achieved.observe(status_packet(0, 18, true), 101);
  assert(!achieved.is_current(
      token));  // Already observed desired state must suppress a redundant old frame.
  achieved.on_transmitted(token, 102, false);
  achieved.observe(poll, 200);
  assert(!achieved.prepare(200, frame, token) && achieved.result(0) == ControlResult::VERIFIED);

  Registration registration;
  registration.set_enabled(true);
  registration.observe(packet(fixture(directory, "status_off")), 0);
  assert(registration.prepare(0, frame, token));
  registration.observe(poll, 1);
  registration.on_transmitted(token, 4000, true);
  assert(!registration.request_wakeup(10000));  // Only 6000 ms since actual TX completion.
  assert(registration.request_wakeup(14000));

  Registration late;
  late.set_enabled(true);
  late.observe(packet(fixture(directory, "status_off")), 0);
  assert(late.prepare(0, frame, token));
  late.tick(5000);
  late.on_transmitted(token, 6000, true);
  late.set_enabled(true);
  assert(
      !late.prepare(16000, frame, token));  // Unknown outcome remains faulted until reconstruction.
}

int main(int argc, char** argv) {
  assert(argc == 2);
  test_codec(argv[1]);
  test_registration(argv[1]);
  test_coalescing_and_echo(argv[1]);
  test_prepared_generations_and_devices(argv[1]);
  test_retries_sleep_and_remote_override(argv[1]);
  test_engine_and_bounds(argv[1]);
  test_prepared_observation_races(argv[1]);
  std::cout << "All Non-NASA assertions passed (7 groups; no NASA source linked)\n";
}
