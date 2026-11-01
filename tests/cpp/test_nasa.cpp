#include "nasa_packet.h"
#include "nasa_read_queue.h"
#include "nasa_engine.h"
#include "nasa_framer.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace esphome::samsung_hvac;
using namespace esphome::samsung_hvac::nasa;

Frame fixture(const std::string& directory, const char* name) {
  std::ifstream file(directory + "/" + name + ".hex");
  std::string hex;
  file >> hex;
  assert(!hex.empty() && hex.size() % 2 == 0);
  Frame frame;
  frame.size = hex.size() / 2;
  for (size_t i = 0; i < frame.size; ++i)
    frame.data[i] = static_cast<uint8_t>(std::stoul(hex.substr(i * 2, 2), nullptr, 16));
  return frame;
}

void test_types_and_roundtrip(const std::string& directory) {
  auto frame = fixture(directory, "mixed_notification");
  Packet packet;
  assert(decode(frame, packet) == DecodeError::NONE);
  assert(packet.source.value == 0x200000 && packet.destination.value == 0x80ff00);
  assert(packet.type == DataType::NOTIFICATION && packet.sequence == 7 &&
         packet.message_count == 4);
  assert(packet.messages[0].kind() == ValueType::ENUM && packet.messages[0].raw == 1);
  assert(packet.messages[1].kind() == ValueType::VARIABLE && packet.messages[1].raw == 0xffce);
  assert(signed_value(packet.messages[1]).value() == -50);
  assert(packet.messages[2].kind() == ValueType::LONG_VARIABLE &&
         packet.messages[2].raw == 0x89abcdefU);
  assert(signed_value(packet.messages[2]).value() == -1985229329LL);
  const auto& structure = packet.messages[3];
  assert(structure.kind() == ValueType::STRUCTURE && structure.length == 8);
  assert(packet.storage.data[structure.offset] == 1 &&
         packet.storage.data[structure.offset + 7] == 8);
  assert(!signed_value(structure).has_value());
  Frame encoded;
  assert(encode(packet, encoded));
  assert(encoded.size == frame.size &&
         std::equal(frame.data.begin(), frame.data.begin() + frame.size, encoded.data.begin()));
  assert(encode(packet, packet.storage));
  assert(
      packet.storage.size == frame.size &&
      std::equal(frame.data.begin(), frame.data.begin() + frame.size, packet.storage.data.begin()));
  assert(decode(fixture(directory, "unsigned_limits"), packet) == DecodeError::NONE);
  assert(packet.messages[2].raw == 0xffffffffU && signed_value(packet.messages[2]).value() == -1);
  assert(signed_value(packet.messages[0]).value() == -1 &&
         signed_value(packet.messages[1]).value() == -1);
  assert(decode(fixture(directory, "signed_min"), packet) == DecodeError::NONE);
  assert(signed_value(packet.messages[0]).value() == -128);
  assert(signed_value(packet.messages[1]).value() == -32768);
  assert(signed_value(packet.messages[2]).value() == -2147483648LL);
  assert(decode(fixture(directory, "signed_max"), packet) == DecodeError::NONE);
  assert(signed_value(packet.messages[0]).value() == 127);
  assert(signed_value(packet.messages[1]).value() == 32767);
  assert(signed_value(packet.messages[2]).value() == 2147483647LL);
  auto large = fixture(directory, "structure_max");
  assert(decode(large, packet) == DecodeError::NONE && packet.message_count == 1);
  assert(packet.messages[0].length == 1484);
  assert(encode(packet, encoded));
  assert(encoded.size == 1502 &&
         std::equal(large.data.begin(), large.data.begin() + 1502, encoded.data.begin()));
  auto many = fixture(directory, "count_255");
  assert(decode(many, packet) == DecodeError::NONE && packet.message_count == 255);
  assert(packet.messages[254].code == 0x4000 && packet.messages[254].raw == 1);
  assert(encode(packet, encoded) && encoded.size == many.size);
  packet.message_count = 256;
  assert(!encode(packet, encoded));
  assert(Address{0x200001}.role() == AddressRole::INDOOR);
  assert(Address{0x100000}.role() == AddressRole::OUTDOOR);
  assert(Address{0x80ff00}.role() == AddressRole::CONTROLLER);
  assert(Address{0xb00000}.role() == AddressRole::BROADCAST);
  assert(Address{0x990000}.role() == AddressRole::UNKNOWN);
}

void test_strict_rejection(const std::string& directory) {
  Packet packet;
  for (const char* name :
       {"short_variable", "bad_count", "structure_not_last", "trailing_payload", "unknown_type"}) {
    assert(decode(fixture(directory, name), packet) != DecodeError::NONE);
    assert(packet.message_count == 0);
  }
  auto valid = fixture(directory, "mixed_notification");
  for (size_t size = 0; size < valid.size; ++size) {
    auto truncated = valid;
    truncated.size = size;
    assert(decode(truncated, packet) != DecodeError::NONE && packet.message_count == 0);
  }
  auto invalid = valid;
  invalid.data[invalid.size - 2] ^= 1;
  assert(decode(invalid, packet) == DecodeError::CRC);
  invalid = valid;
  invalid.size = 1503;
  assert(decode(invalid, packet) == DecodeError::FRAME_SIZE);
  assert(decode(valid, packet) == DecodeError::NONE);
  packet.type = DataType::WRITE;
  Frame output;
  assert(!encode(packet, output) && output.size == 0);
  packet.type = DataType::REQUEST;
  assert(!encode(packet, output));
}

struct Results : ReadSink {
  std::vector<ReadResult> values;
  void on_read_complete(const ReadResult& result) override { values.push_back(result); }
};

Packet reply(uint8_t sequence, DataType type = DataType::RESPONSE) {
  Packet packet;
  packet.source = Address{0x200000};
  packet.destination = Address{0x80ff00};
  packet.sequence = sequence;
  packet.type = type;
  packet.message_count = type == DataType::ACK || type == DataType::NACK ? 0 : 1;
  packet.messages[0].code = 0x4201;
  packet.messages[0].raw = 250;
  return packet;
}

void test_read_golden_and_correlation(const std::string& directory) {
  Packet packet;
  packet.source = Address{0x80ff00};
  packet.destination = Address{0x200000};
  packet.sequence = 7;
  packet.type = DataType::READ;
  packet.message_count = 1;
  packet.messages[0].code = 0x4201;
  Frame encoded;
  assert(encode(packet, encoded));
  auto golden = fixture(directory, "read_target");
  assert(encoded.size == golden.size &&
         std::equal(golden.data.begin(), golden.data.begin() + golden.size, encoded.data.begin()));
  Results results;
  ReadQueue queue(Address{0x80ff00}, results);
  assert(!queue.enqueue(Address{0xb00000}, 0x4201, 0));
  assert(!queue.enqueue(Address{0x200000}, 0x4600, 0));
  assert(queue.enqueue(Address{0x200000}, 0x4201, 0));
  assert(!queue.enqueue(Address{0x200000}, 0x4201, 0));
  assert(queue.prepare(0, encoded));
  assert(decode(encoded, packet) == DecodeError::NONE && packet.type == DataType::READ);
  auto sequence = packet.sequence;
  auto response = reply(sequence);
  assert(!queue.on_packet(response, 1));  // Not transmitted yet.
  queue.tick(1001);
  assert(results.values.empty());  // Response timeout begins only after confirmed TX.
  queue.on_transmitted(sequence, 1500, true);
  auto wrong = response;
  wrong.source = Address{0x200001};
  assert(!queue.on_packet(wrong, 1501));
  wrong = response;
  wrong.destination = Address{0x80ff01};
  assert(!queue.on_packet(wrong, 1502));
  wrong = response;
  wrong.messages[0].code = 0x4204;
  assert(!queue.on_packet(wrong, 1503));
  assert(!queue.on_packet(reply(sequence, DataType::ACK), 1504));
  assert(!queue.on_packet(reply(sequence, DataType::NOTIFICATION), 1505));
  assert(queue.on_packet(response, 1506));
  assert(results.values.size() == 1 && results.values[0].status == ReadStatus::SUCCESS);
  assert(results.values[0].raw == 250);
  assert(!queue.on_packet(response, 1507));
  assert(queue.enqueue(Address{0x200000}, 0x4201, 1508));
  assert(queue.prepare(1508, encoded));
  assert(decode(encoded, packet) == DecodeError::NONE && packet.sequence != sequence);
  queue.on_transmitted(packet.sequence, 1509, true);
  assert(!queue.on_packet(response, 1510));
  assert(!queue.on_packet(reply(sequence, DataType::ACK), 1511));
  assert(results.values.size() == 1);
  assert(queue.on_packet(reply(packet.sequence, DataType::NACK), 1512));
  assert(results.values.back().status == ReadStatus::NACK);
}

void test_retries_and_deadlines() {
  Results results;
  ReadQueue queue(Address{0x80ff00}, results);
  Frame frame;
  Packet packet;
  assert(queue.enqueue(Address{0x200000}, 0x4201, 0));
  uint8_t first = 0;
  for (uint32_t attempt = 0; attempt < 3; ++attempt) {
    assert(queue.prepare(attempt * 1000, frame));
    assert(decode(frame, packet) == DecodeError::NONE);
    if (attempt == 0)
      first = packet.sequence;
    else
      assert(packet.sequence != first);
    queue.on_transmitted(packet.sequence, attempt * 1000, true);
    queue.tick(attempt * 1000 + 999);
    assert(results.values.empty());
    queue.tick(attempt * 1000 + 1000);
  }
  assert(results.values.size() == 1 && results.values[0].status == ReadStatus::TIMEOUT);
  assert(results.values[0].attempts == 3 && !queue.prepare(3001, frame));
  assert(queue.enqueue(Address{0x200000}, 0x4201, 4000));
  assert(queue.prepare(4000, frame));
  assert(decode(frame, packet) == DecodeError::NONE);
  queue.on_transmitted(packet.sequence, 4001, false);
  assert(results.values.back().status == ReadStatus::SEND_FAILED);
  assert(queue.enqueue(Address{0x200000}, 0x4201, 5000));
  assert(queue.prepare(5000, frame));
  queue.tick(10000);
  assert(results.values.back().status == ReadStatus::SEND_FAILED);
  assert(!queue.on_packet(reply(packet.sequence), 10001));

  // A late packet/completion must enforce deadlines even without a preceding tick.
  Results late_results;
  ReadQueue late(Address{0x80ff00}, late_results);
  assert(late.enqueue(Address{0x200000}, 0x4201, 0));
  assert(late.prepare(0, frame));
  assert(decode(frame, packet) == DecodeError::NONE);
  late.on_transmitted(packet.sequence, 0, true);
  assert(!late.on_packet(reply(packet.sequence), 1001));
  assert(late_results.values.empty());
  assert(late.prepare(1001, frame));
  assert(decode(frame, packet) == DecodeError::NONE);
  late.on_transmitted(packet.sequence, 7000, true);
  assert(late_results.values.size() == 1 &&
         late_results.values[0].status == ReadStatus::SEND_FAILED);
}

void test_capacity_sequence_and_rollover() {
  Results results;
  ReadQueue queue(Address{0x80ff00}, results);
  Frame frame;
  Packet packet;
  for (unsigned i = 0; i < 8; ++i) assert(queue.enqueue(Address{0x200000U + i}, 0x4201, 0));
  assert(!queue.enqueue(Address{0x200008}, 0x4201, 0));
  queue.tick(5000);
  assert(results.values.size() == 8);
  for (const auto& result : results.values) assert(result.status == ReadStatus::EXPIRED);
  Results wrapped_results;
  ReadQueue wrapped(Address{0x80ff00}, wrapped_results);
  uint32_t now = std::numeric_limits<uint32_t>::max() - 400;
  assert(wrapped.enqueue(Address{0x200000}, 0x4201, now));
  assert(wrapped.prepare(now, frame));
  assert(decode(frame, packet) == DecodeError::NONE);
  wrapped.on_transmitted(packet.sequence, now, true);
  wrapped.tick(now + 999);
  assert(wrapped_results.values.empty());
  wrapped.tick(now + 1000);
  assert(wrapped.prepare(now + 1000, frame));
  assert(decode(frame, packet) == DecodeError::NONE);
  wrapped.on_transmitted(packet.sequence, now + 1000, true);
  assert(wrapped.on_packet(reply(packet.sequence), now + 1001));
  Results exhausted_results;
  ReadQueue exhausted(Address{0x80ff00}, exhausted_results);
  for (unsigned i = 0; i < 255; ++i) {
    assert(exhausted.enqueue(Address{0x200000}, 0x4201, i));
    assert(exhausted.prepare(i, frame));
    assert(decode(frame, packet) == DecodeError::NONE && packet.sequence != 0);
    exhausted.on_transmitted(packet.sequence, i, true);
    assert(exhausted.on_packet(reply(packet.sequence), i));
  }
  assert(exhausted.enqueue(Address{0x200000}, 0x4201, 255));
  assert(!exhausted.prepare(255, frame));  // No safe sequence reuse within quarantine.
  exhausted.tick(5255);
  assert(exhausted_results.values.back().status == ReadStatus::EXPIRED);
  assert(exhausted.enqueue(Address{0x200000}, 0x4201, 31000));
  assert(exhausted.prepare(31000, frame));
}

struct Packets : PacketSink {
  unsigned received{0};
  DataType last{DataType::UNDEFINED};
  void on_nasa_packet(const Packet& packet) override {
    ++received;
    last = packet.type;
  }
};

void test_passive_engine(const std::string& directory) {
  Results results;
  Packets packets;
  Engine engine(Address{0x80ff00}, packets, results);
  for (const char* name :
       {"read_target", "write", "request", "mixed_notification", "response", "ack", "nack"})
    engine.receive(fixture(directory, name), 0);
  assert(packets.received == 7 && packets.last == DataType::NACK);
  for (unsigned category = 1; category <= 7; ++category)
    assert(engine.stats().categories[category] == 1);
  engine.receive(fixture(directory, "bad_count"), 1);
  assert(engine.stats().invalid_packets == 1 && packets.received == 7 && results.values.empty());
  assert(engine.reads().enqueue(Address{0x200000}, 0x4201, 2));
  Frame frame;
  Packet read;
  assert(engine.reads().prepare(2, frame));
  assert(decode(frame, read) == DecodeError::NONE);
  engine.reads().on_transmitted(read.sequence, 3, true);
  auto response = reply(read.sequence);
  assert(encode(response, frame));
  engine.receive(frame, 4);
  assert(results.values.size() == 1 && results.values[0].status == ReadStatus::SUCCESS);
  assert(packets.received == 8 && packets.last == DataType::RESPONSE);
  Engine isolated(Address{0x80ff00}, packets, results);
  assert(isolated.stats().decoded_packets == 0);
}

void test_inner_payload_fuzz() {
  uint32_t random = 0x1234abcd;
  Packet packet;
  Frame frame, encoded;
  unsigned accepted = 0;
  for (unsigned iteration = 0; iteration < 10000; ++iteration) {
    random = random * 1664525U + 1013904223U;
    frame.size = 16 + random % 256;
    for (size_t i = 0; i < frame.size; ++i) {
      random = random * 1664525U + 1013904223U;
      frame.data[i] = static_cast<uint8_t>(random >> 24);
    }
    if (iteration % 5 == 0) {
      frame.size = 19;
      frame.data[12] = 1;
      frame.data[13] = 0x40;
      frame.data[14] = 0x00;
      frame.data[15] = static_cast<uint8_t>(iteration);
    }
    frame.data[0] = 0x32;
    frame.data[1] = 0;
    frame.data[2] = static_cast<uint8_t>(frame.size - 2);
    if (frame.size - 2 > 255) frame.data[1] = 1;
    frame.data[9] = 0xc0;
    frame.data[10] = 0x14;
    auto crc = frame_crc(frame.data.data() + 3, frame.size - 6);
    frame.data[frame.size - 3] = static_cast<uint8_t>(crc >> 8);
    frame.data[frame.size - 2] = static_cast<uint8_t>(crc);
    frame.data[frame.size - 1] = 0x34;
    auto result = decode(frame, packet);
    if (result == DecodeError::NONE) {
      ++accepted;
      assert(encode(packet, encoded));
      assert(encoded.size == frame.size &&
             std::equal(frame.data.begin(), frame.data.begin() + frame.size, encoded.data.begin()));
    } else {
      assert(packet.message_count == 0);
    }
  }
  assert(accepted >= 2000);
}

int main(int argc, char** argv) {
  assert(argc == 2);
  test_types_and_roundtrip(argv[1]);
  test_strict_rejection(argv[1]);
  test_read_golden_and_correlation(argv[1]);
  test_retries_and_deadlines();
  test_capacity_sequence_and_rollover();
  test_passive_engine(argv[1]);
  test_inner_payload_fuzz();
  std::cout << "All NASA assertions passed (7 groups, 10000 CRC-valid inner fuzz cases)\n";
}
