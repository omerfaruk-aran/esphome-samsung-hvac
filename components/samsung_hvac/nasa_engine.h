#pragma once

#include "nasa_read_queue.h"

namespace esphome::samsung_hvac::nasa {

class PacketSink {
 public:
  virtual ~PacketSink() = default;
  // Packet storage is borrowed until this callback returns; do not reenter receive().
  virtual void on_nasa_packet(const Packet& packet) = 0;
};

struct EngineStats {
  uint32_t decoded_packets{0};
  uint32_t invalid_packets{0};
  std::array<uint32_t, 8> categories{};
};

class Engine {
 public:
  Engine(Address local, PacketSink& sink, ReadSink& read_sink)
      : sink_(sink), reads_(local, read_sink) {}
  void receive(const Frame& frame, uint32_t now);
  void tick(uint32_t now) { this->reads_.tick(now); }
  ReadQueue& reads() { return this->reads_; }
  const EngineStats& stats() const { return this->stats_; }

 private:
  PacketSink& sink_;
  ReadQueue reads_;
  Packet packet_;
  EngineStats stats_;
};

}  // namespace esphome::samsung_hvac::nasa
