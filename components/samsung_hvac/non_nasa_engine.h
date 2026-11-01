#pragma once

#include "non_nasa_control_queue.h"
#include "non_nasa_registration.h"

namespace esphome::samsung_hvac::non_nasa {

class PacketSink {
 public:
  virtual ~PacketSink() = default;
  // Borrowed until callback return; do not reenter receive().
  virtual void on_non_nasa_packet(const Packet& packet) = 0;
};
struct EngineStats {
  uint32_t decoded_packets{0};
  uint32_t invalid_packets{0};
};
class Engine {
 public:
  explicit Engine(PacketSink& sink) : sink_(sink) {}
  void receive(const Frame& frame, uint32_t now);
  void tick(uint32_t now) {
    this->controls_.tick(now);
    this->registration_.tick(now);
    if (this->controls_.needs_wakeup(now) && this->registration_.request_wakeup(now))
      this->controls_.require_fresh_status();
  }
  Registration& registration() { return this->registration_; }
  ControlQueue& controls() { return this->controls_; }
  const EngineStats& stats() const { return this->stats_; }

 private:
  PacketSink& sink_;
  Packet packet_;
  Registration registration_;
  ControlQueue controls_;
  EngineStats stats_;
};

}  // namespace esphome::samsung_hvac::non_nasa
