#include "bus_transport.h"
#include "nasa_framer.h"
#include "non_nasa_framer.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace esphome::samsung_hvac;

struct Sink : FrameSink {
  std::vector<Frame> frames;
  void on_frame(const Frame& frame) override { frames.push_back(frame); }
};

struct SimulatedIO : BusIO {
  std::deque<uint8_t> rx;
  std::vector<uint8_t> tx;
  std::vector<bool> directions;
  size_t capacity{64};
  TxStatus status{TxStatus::DONE};
  bool write_ok{true};
  bool read(uint8_t& byte) override {
    if (rx.empty()) return false;
    byte = rx.front();
    rx.pop_front();
    return true;
  }
  size_t write_capacity() override { return capacity; }
  bool write(const uint8_t* data, size_t size) override {
    if (!write_ok) return false;
    tx.insert(tx.end(), data, data + size);
    return true;
  }
  TxStatus tx_status() override { return status; }
  void set_direction(bool transmit) override { directions.push_back(transmit); }
};

Frame fixture(const std::string& directory, const char* name) {
  std::ifstream file(directory + "/" + name);
  std::string hex;
  file >> hex;
  assert(!hex.empty() && hex.size() % 2 == 0);
  Frame frame;
  frame.size = hex.size() / 2;
  assert(frame.size <= frame.data.size());
  for (size_t i = 0; i < frame.size; ++i)
    frame.data[i] = static_cast<uint8_t>(std::stoul(hex.substr(i * 2, 2), nullptr, 16));
  return frame;
}

void feed(FrameAssembler& framer, const Frame& frame, uint32_t now = 0) {
  for (size_t i = 0; i < frame.size; ++i) framer.feed(frame.data[i], now);
}

void test_framing(const Frame& nasa_frame, const Frame& non_frame, const Frame& maximum) {
  Sink a, b;
  Diagnostics da, db;
  nasa::Framer nasa_framer(a, da);
  non_nasa::Framer non_framer(b, db);
  nasa_framer.tick(100);
  nasa_framer.feed(0x11, 100);
  nasa_framer.feed(0x34, 100);
  assert(da.discarded_bytes == 2 && !nasa_framer.pending());
  for (size_t i = 0; i + 1 < nasa_frame.size; ++i) nasa_framer.feed(nasa_frame.data[i], 100);
  assert(a.frames.empty() && nasa_framer.pending());
  nasa_framer.feed(nasa_frame.data[nasa_frame.size - 1], 100);
  assert(a.frames.size() == 1 && !nasa_framer.pending());
  feed(nasa_framer, nasa_frame, 101);
  feed(nasa_framer, nasa_frame, 101);
  assert(a.frames.size() == 3);
  feed(non_framer, non_frame);
  feed(non_framer, non_frame);
  assert(b.frames.size() == 2 && da.rx_frames == 3 && db.rx_frames == 2);
  auto corrupt = nasa_frame;
  corrupt.data[15] ^= 1;
  feed(nasa_framer, corrupt, 102);
  assert(a.frames.size() == 3 && da.invalid_frames > 0);
  auto bad_checksum = non_frame;
  bad_checksum.data[12] ^= 1;
  feed(non_framer, bad_checksum, 102);
  assert(b.frames.size() == 2 && db.invalid_frames > 0);
  auto bad_end = non_frame;
  bad_end.data[13] = 0;
  feed(non_framer, bad_end, 103);
  assert(b.frames.size() == 2);
  nasa_framer.feed(0x32, 104);
  nasa_framer.feed(0xff, 104);
  nasa_framer.feed(0xff, 104);
  assert(!nasa_framer.pending() && da.invalid_frames >= 2);
  feed(nasa_framer, maximum, 105);
  assert(a.frames.back().size == 1502 && da.buffer_overflows == 0);
  assert(da.rx_frames == 4 && db.rx_frames == 2);

  // A corrupt longer candidate can contain a complete shorter frame and a tail.
  Sink recovered;
  Diagnostics recovery_stats;
  nasa::Framer recovery(recovered, recovery_stats);
  Frame overlap;
  overlap.size = 40;
  overlap.data[0] = 0x32;
  overlap.data[2] = 38;
  std::copy_n(nasa_frame.data.begin(), nasa_frame.size, overlap.data.begin() + 3);
  feed(recovery, overlap, 200);
  assert(recovered.frames.size() == 1 && recovered.frames[0].size == nasa_frame.size);
}

void test_timeout_and_rollover(const Frame& frame) {
  Sink sink;
  Diagnostics stats;
  nasa::Framer parser(sink, stats);
  const uint32_t start = std::numeric_limits<uint32_t>::max() - 20;
  parser.feed(0x32, start);
  parser.tick(start + 49);
  assert(parser.pending() && stats.frame_timeouts == 0);
  parser.tick(start + 50);
  assert(!parser.pending() && stats.frame_timeouts == 1);
  feed(parser, frame, start + 51);
  assert(sink.frames.size() == 1);
}

void test_bounded_random_input(const Frame& frame) {
  Sink sink;
  Diagnostics stats;
  nasa::Framer parser(sink, stats);
  uint32_t random = 1234567;
  for (uint32_t i = 0; i < 100000; ++i) {
    random = random * 1664525U + 1013904223U;
    parser.feed(static_cast<uint8_t>(random >> 24), i);
  }
  parser.tick(100100);
  feed(parser, frame, 100101);
  assert(sink.frames.size() == 1 && stats.buffer_overflows == 0);
}

struct Rig {
  Sink sink;
  Diagnostics stats;
  SimulatedIO io;
  non_nasa::Framer parser{sink, stats};
  Transport transport{io, parser, stats};
  void discover(const Frame& frame, uint32_t now = 0) {
    io.rx.insert(io.rx.end(), frame.data.begin(), frame.data.begin() + frame.size);
    transport.loop(now);
    assert(stats.rx_frames == 1);
  }
};

void test_passive_and_discovery(const Frame& frame) {
  Rig rig;
  assert(!rig.transport.enqueue(frame, 0));
  rig.transport.loop(200);
  assert(rig.io.tx.empty() && rig.io.directions.empty());
  rig.transport.set_tx_enabled(true);
  assert(rig.transport.enqueue(frame, 200));
  rig.transport.loop(1000);
  assert(rig.io.tx.empty());  // No valid bus frame observed.
  rig.discover(frame, 1001);
  rig.transport.loop(1100);
  assert(rig.io.tx.empty());
  rig.transport.loop(1101);
  assert(rig.io.tx == std::vector<uint8_t>(frame.data.begin(), frame.data.begin() + frame.size));
}

void test_queue_capacity_and_expiry(const Frame& frame) {
  Rig rig;
  rig.discover(frame);
  rig.transport.set_tx_enabled(true);
  for (unsigned i = 0; i < 4; ++i) assert(rig.transport.enqueue(frame, 1));
  assert(!rig.transport.enqueue(frame, 1));
  rig.transport.loop(5001);
  assert(rig.stats.tx_expired == 4 && rig.io.tx.empty());
  Frame invalid;
  assert(!rig.transport.enqueue(invalid, 5002));
  invalid.size = 1503;
  assert(!rig.transport.enqueue(invalid, 5002));
}

void test_serialized_tx_and_direction(const Frame& frame) {
  Rig rig;
  rig.discover(frame);
  rig.transport.set_tx_enabled(true);
  auto second = frame;
  second.data[4] = 0x99;  // Scheduler handles bytes; codecs own frame integrity.
  assert(rig.transport.enqueue(frame, 1));
  assert(rig.transport.enqueue(second, 1));
  rig.io.capacity = 0;
  rig.transport.loop(100);
  assert(rig.io.tx.empty() && rig.io.directions.empty());
  rig.io.capacity = 5;
  rig.transport.loop(101);
  assert(rig.io.tx.size() == 5 && rig.io.directions == std::vector<bool>{true});
  rig.transport.loop(102);
  assert(rig.io.tx.size() == 10);
  rig.transport.loop(103);
  assert(rig.io.tx.size() == 14);
  rig.io.status = TxStatus::BUSY;
  rig.transport.loop(500);
  assert(rig.stats.tx_frames == 0 && rig.io.directions.size() == 1);
  rig.io.status = TxStatus::DONE;
  rig.transport.loop(501);
  rig.transport.loop(503);
  assert(rig.io.directions.size() == 1);
  rig.transport.loop(504);
  assert(rig.io.directions == (std::vector<bool>{true, false}));
  assert(rig.stats.tx_frames == 1);
  rig.transport.loop(603);
  assert(rig.io.tx.size() == 14);
  rig.io.capacity = 64;
  rig.transport.loop(604);
  assert(rig.io.tx.size() == 28);
  assert(std::equal(frame.data.begin(), frame.data.begin() + 14, rig.io.tx.begin()));
  assert(std::equal(second.data.begin(), second.data.begin() + 14, rig.io.tx.begin() + 14));
}

void test_rx_budget_and_priority(const Frame& frame) {
  Rig rig;
  rig.discover(frame);
  rig.transport.set_tx_enabled(true);
  assert(rig.transport.enqueue(frame, 1));
  rig.io.rx.insert(rig.io.rx.end(), 200, 0x11);
  rig.transport.loop(100);
  assert(rig.io.rx.size() == 136 && rig.io.tx.empty());
  rig.transport.loop(200);
  rig.transport.loop(300);
  rig.transport.loop(400);
  assert(rig.io.rx.empty() && rig.io.tx.empty());
  rig.transport.loop(500);
  assert(rig.io.tx.size() == frame.size);
  rig.io.rx.push_back(0x32);
  rig.transport.loop(501);
  assert(rig.stats.echo_bytes == 1 && !rig.parser.pending());
}

void test_faults_and_clock_wrap(const Frame& frame) {
  {
    Rig failed_write;
    failed_write.discover(frame);
    failed_write.transport.set_tx_enabled(true);
    assert(failed_write.transport.enqueue(frame, 1));
    failed_write.io.write_ok = false;
    failed_write.transport.loop(100);
    assert(failed_write.stats.tx_faults == 1 && failed_write.io.tx.empty());
    assert(failed_write.io.directions == (std::vector<bool>{true, false}));
    assert(!failed_write.transport.enqueue(frame, 101));
  }
  for (bool driver_failure : {false, true}) {
    Rig rig;
    rig.discover(frame);
    rig.transport.set_tx_enabled(true);
    assert(rig.transport.enqueue(frame, 1));
    rig.transport.loop(100);
    rig.io.status = driver_failure ? TxStatus::FAILED : TxStatus::BUSY;
    rig.transport.loop(driver_failure ? 101 : 5100);
    assert(rig.stats.tx_faults == 1 && rig.io.directions.back() == false);
    assert(!rig.transport.enqueue(frame, 5101));
    rig.transport.set_tx_enabled(true);
    rig.transport.loop(6000);
    assert(rig.io.tx.size() == frame.size && rig.stats.tx_frames == 0);
  }
  Rig rig;
  uint32_t start = std::numeric_limits<uint32_t>::max() - 50;
  rig.discover(frame, start);
  rig.transport.set_tx_enabled(true);
  assert(rig.transport.enqueue(frame, start));
  rig.transport.loop(start + 99);
  assert(rig.io.tx.empty());
  rig.transport.loop(start + 100);
  assert(rig.io.tx.size() == frame.size);
}

int main(int argc, char** argv) {
  assert(argc == 2);
  auto nasa_frame = fixture(argv[1], "nasa.hex");
  auto non_frame = fixture(argv[1], "non_nasa.hex");
  auto maximum = fixture(argv[1], "nasa_max.hex");
  test_framing(nasa_frame, non_frame, maximum);
  test_timeout_and_rollover(nasa_frame);
  test_bounded_random_input(nasa_frame);
  test_passive_and_discovery(non_frame);
  test_queue_capacity_and_expiry(non_frame);
  test_serialized_tx_and_direction(non_frame);
  test_rx_budget_and_priority(non_frame);
  test_faults_and_clock_wrap(non_frame);
  std::cout << "All transport assertions passed (8 groups, 100000 random bytes)\n";
}
