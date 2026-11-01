#pragma once

#include "nasa_packet.h"

namespace esphome::samsung_hvac::nasa {

enum class ReadStatus { SUCCESS, NACK, TIMEOUT, SEND_FAILED, EXPIRED };
struct ReadResult {
  Address device;
  uint16_t code{0};
  ReadStatus status{ReadStatus::EXPIRED};
  uint32_t raw{0};
  uint8_t attempts{0};
};
class ReadSink {
 public:
  virtual ~ReadSink() = default;
  virtual void on_read_complete(const ReadResult& result) = 0;
};

class ReadQueue {
 public:
  ReadQueue(Address local, ReadSink& sink) : local_(local), sink_(sink) {}
  bool enqueue(Address device, uint16_t code, uint32_t now);
  bool prepare(uint32_t now, Frame& frame);
  void on_transmitted(uint8_t sequence, uint32_t now, bool success);
  bool on_packet(const Packet& packet, uint32_t now);
  void tick(uint32_t now);

 private:
  enum class State { IDLE, PREPARED, WAITING };
  struct Entry {
    Address device;
    uint16_t code{0};
    uint32_t queued_at{0};
    uint8_t attempts{0};
  };
  uint8_t allocate_sequence_(uint32_t now);
  void finish_(ReadStatus status, uint32_t now, uint32_t raw = 0);
  Address local_;
  ReadSink& sink_;
  std::array<Entry, 8> entries_{};
  std::array<uint32_t, 256> last_used_{};
  std::array<bool, 256> used_{};
  size_t head_{0};
  size_t count_{0};
  State state_{State::IDLE};
  uint8_t next_sequence_{1};
  uint8_t sequence_{0};
  uint32_t state_started_{0};
};

}  // namespace esphome::samsung_hvac::nasa
