#pragma once

#include "non_nasa_packet.h"

namespace esphome::samsung_hvac::non_nasa {

class Registration {
 public:
  void set_enabled(bool enabled) {
    if (!enabled && this->prepared_) this->faulted_ = true;
    this->enabled_ = enabled && !this->faulted_;
    if (!this->enabled_) { this->prepared_ = false; this->wake_pending_ = false; this->keepalive_pending_ = false; }
  }
  void set_keepalive(bool enabled) { this->keepalive_ = enabled; if (!enabled) this->keepalive_pending_ = false; }
  bool registered() const { return this->registered_; }
  bool request_wakeup(uint32_t now);
  void observe(const Packet &packet, uint32_t now);
  bool prepare(uint32_t now, Frame &frame, uint32_t &token);
  void on_transmitted(uint32_t token, uint32_t now, bool success);
  void tick(uint32_t now);

 private:
  bool enabled_{false};
  bool faulted_{false};
  bool keepalive_{false};
  bool seen_valid_{false};
  bool registered_{false};
  bool has_attempt_{false};
  bool prepared_{false};
  bool keepalive_pending_{false};
  bool wake_pending_{false};
  uint8_t attempts_{0};
  uint32_t last_attempt_{0};
  uint32_t prepared_at_{0};
  uint32_t keepalive_at_{0};
  uint32_t wake_at_{0};
  uint32_t token_{0};
};

}  // namespace esphome::samsung_hvac::non_nasa
