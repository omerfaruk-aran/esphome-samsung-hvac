#include "bus_transport.h"
#include <algorithm>

namespace esphome::samsung_hvac {

void Transport::pop_() {
  this->head_ = (this->head_ + 1) % this->queue_.size();
  this->count_--;
}

void Transport::fault_() {
  this->io_.set_direction(false);
  this->state_ = State::FAULT;
  this->tx_enabled_ = false;
  this->count_ = 0;
  this->stats_.tx_faults++;
}

void Transport::set_tx_enabled(bool enabled) {
  if (this->state_ == State::FAULT) return;
  this->tx_enabled_ = enabled;
  if (!enabled) {
    if (this->state_ != State::IDLE) this->fault_();
    this->count_ = 0;
  }
}

bool Transport::enqueue(const Frame& frame, uint32_t now) {
  if (!this->tx_enabled_ || this->state_ == State::FAULT || this->count_ == this->queue_.size() ||
      frame.size == 0 || frame.size > MAX_FRAME_SIZE) {
    this->stats_.tx_rejected++;
    return false;
  }
  auto& slot = this->queue_[(this->head_ + this->count_) % this->queue_.size()];
  slot.frame = frame;
  slot.queued_at = now;
  this->count_++;
  return true;
}

void Transport::loop(uint32_t now) {
  if (this->state_ == State::FAULT) return;
  uint8_t byte;
  size_t received = 0;
  while (received < 64 && this->io_.read(byte)) {
    ++received;
    this->stats_.rx_bytes++;
    this->last_activity_ = now;
    if (this->state_ == State::IDLE)
      this->framer_.feed(byte, now);
    else
      this->stats_.echo_bytes++;
  }
  this->framer_.tick(now);

  if (this->state_ == State::IDLE) {
    while (this->count_ != 0 &&
           static_cast<uint32_t>(now - this->queue_[this->head_].queued_at) >= 5000) {
      this->stats_.tx_expired++;
      this->pop_();
    }
    if (!this->tx_enabled_ || this->count_ == 0 || this->stats_.rx_frames == 0 ||
        this->framer_.pending() || received == 64 ||
        static_cast<uint32_t>(now - this->last_activity_) < 100 || this->io_.write_capacity() == 0)
      return;
    this->state_ = State::SENDING;
    this->offset_ = 0;
    this->tx_started_ = now;
    this->io_.set_direction(true);
  }

  if (static_cast<uint32_t>(now - this->tx_started_) >= 5000) {
    this->fault_();
    return;
  }
  if (this->state_ == State::SENDING) {
    const auto& frame = this->queue_[this->head_].frame;
    size_t chunk = std::min({size_t{64}, this->io_.write_capacity(), frame.size - this->offset_});
    if (chunk == 0) return;
    if (!this->io_.write(frame.data.data() + this->offset_, chunk)) {
      this->fault_();
      return;
    }
    this->offset_ += chunk;
    if (this->offset_ == frame.size) this->state_ = State::DRAINING;
    return;
  }
  if (this->state_ == State::DRAINING) {
    auto status = this->io_.tx_status();
    if (status == TxStatus::FAILED) {
      this->fault_();
    } else if (status == TxStatus::DONE) {
      this->state_ = State::GUARD;
      this->guard_started_ = now;
    }
    return;
  }
  if (this->state_ == State::GUARD && static_cast<uint32_t>(now - this->guard_started_) >= 3) {
    this->io_.set_direction(false);
    this->stats_.tx_frames++;
    this->pop_();
    this->last_activity_ = now;
    this->state_ = State::IDLE;
  }
}

}  // namespace esphome::samsung_hvac
