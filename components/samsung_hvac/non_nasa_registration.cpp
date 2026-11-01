#include "non_nasa_registration.h"

namespace esphome::samsung_hvac::non_nasa {

void Registration::tick(uint32_t now) {
  if (this->keepalive_pending_ && static_cast<uint32_t>(now - this->keepalive_at_) >= 100)
    this->keepalive_pending_ = false;
  if (this->wake_pending_ && static_cast<uint32_t>(now - this->wake_at_) >= 100) this->wake_pending_ = false;
  if (this->prepared_ && static_cast<uint32_t>(now - this->prepared_at_) >= 5000) {
    this->prepared_ = false;
    this->enabled_ = false;
    this->faulted_ = true;
  }
}

bool Registration::request_wakeup(uint32_t now) {
  this->tick(now);
  if (!this->enabled_ || !this->seen_valid_ || this->wake_pending_ || this->prepared_ || this->attempts_ >= 3 ||
      (this->has_attempt_ && static_cast<uint32_t>(now - this->last_attempt_) < 10000))
    return false;
  this->wake_pending_ = true;
  this->wake_at_ = now;
  return true;
}

void Registration::observe(const Packet &packet, uint32_t now) {
  this->tick(now);
  if ((packet.source < 0x80 && packet.command == 0x20) ||
      (packet.source == 0xc8 && (packet.command == 0xc6 || packet.command == 0xc0 || packet.command == 0xf3)))
    this->seen_valid_ = true;
  if (packet.source == 0xc8 && packet.destination == 0xd0 && packet.command == 0xc6 && packet.payload[0] == 1) {
    this->registered_ = true;
    this->seen_valid_ = true;
    this->attempts_ = 0;
    // Keep an issued ticket until its TX outcome arrives, even if C6 confirms the seat meanwhile.
    this->wake_pending_ = false;
  }
  if (this->registered_ && this->keepalive_ && packet.source == 0xc8 && packet.destination == 0xad &&
      (packet.payload[0] & 1) != 0 && !this->keepalive_pending_ &&
      (!this->has_attempt_ || static_cast<uint32_t>(now - this->last_attempt_) >= 10000)) {
    this->keepalive_pending_ = true;
    this->keepalive_at_ = now;
  }
}

bool Registration::prepare(uint32_t now, Frame &frame, uint32_t &token) {
  frame.size = 0;
  this->tick(now);
  if (!this->enabled_ || !this->seen_valid_ || this->prepared_ || this->attempts_ >= 3 ||
      (this->has_attempt_ && static_cast<uint32_t>(now - this->last_attempt_) < 10000))
    return false;
  if (this->registered_ && !this->wake_pending_ && (!this->keepalive_ || !this->keepalive_pending_ ||
                                                  static_cast<uint32_t>(now - this->keepalive_at_) < 30))
    return false;
  if (!encode_registration(frame)) return false;
  this->prepared_ = true;
  this->prepared_at_ = now;
  this->last_attempt_ = now;
  this->has_attempt_ = true;
  this->keepalive_pending_ = false;
  this->wake_pending_ = false;
  this->attempts_++;
  if (++this->token_ == 0) ++this->token_;
  token = this->token_;
  return true;
}

void Registration::on_transmitted(uint32_t token, uint32_t now, bool success) {
  this->tick(now);
  if (!this->prepared_ || token != this->token_) return;
  this->prepared_ = false;
  this->last_attempt_ = now;
  if (!success) { this->enabled_ = false; this->faulted_ = true; }
}

}  // namespace esphome::samsung_hvac::non_nasa
