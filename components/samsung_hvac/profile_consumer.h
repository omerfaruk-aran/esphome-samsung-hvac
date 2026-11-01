#pragma once
#include "nasa_packet.h"
#include "non_nasa_packet.h"

namespace esphome::samsung_hvac::profiles {
// Decoded observations only; no UART, scheduler, ownership or mutable engine state.
class PacketConsumer {
 public:
  virtual ~PacketConsumer() = default;
  virtual void receive(const nasa::Packet& packet) { (void)packet; }
  virtual void receive(const non_nasa::Packet& packet) { (void)packet; }
};
}  // namespace esphome::samsung_hvac::profiles
