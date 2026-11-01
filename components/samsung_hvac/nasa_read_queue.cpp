#include "nasa_read_queue.h"

namespace esphome::samsung_hvac::nasa {

bool ReadQueue::enqueue(Address device, uint16_t code, uint32_t now) {
  if (this->local_.role() != AddressRole::CONTROLLER || this->count_ == this->entries_.size() ||
      code == 0 || Message{code}.kind() == ValueType::STRUCTURE ||
      (device.role() != AddressRole::INDOOR && device.role() != AddressRole::OUTDOOR))
    return false;
  for (size_t i = 0; i < this->count_; ++i) {
    const auto& entry = this->entries_[(this->head_ + i) % this->entries_.size()];
    if (entry.device == device && entry.code == code) return false;
  }
  this->entries_[(this->head_ + this->count_) % this->entries_.size()] =
      Entry{device, code, now, 0};
  this->count_++;
  return true;
}

uint8_t ReadQueue::allocate_sequence_(uint32_t now) {
  for (unsigned i = 0; i < 255; ++i) {
    uint8_t candidate = this->next_sequence_;
    this->next_sequence_ = candidate == 255 ? 1 : static_cast<uint8_t>(candidate + 1);
    if (!this->used_[candidate] ||
        static_cast<uint32_t>(now - this->last_used_[candidate]) >= 30000) {
      this->used_[candidate] = true;
      this->last_used_[candidate] = now;
      return candidate;
    }
  }
  return 0;
}

void ReadQueue::finish_(ReadStatus status, uint32_t now, uint32_t raw) {
  const auto entry = this->entries_[this->head_];
  ReadResult result{entry.device, entry.code, status, raw, entry.attempts};
  if (this->sequence_ != 0) this->last_used_[this->sequence_] = now;
  this->head_ = (this->head_ + 1) % this->entries_.size();
  this->count_--;
  this->sequence_ = 0;
  this->state_ = State::IDLE;
  this->sink_.on_read_complete(result);
}

void ReadQueue::tick(uint32_t now) {
  if (this->count_ == 0) return;
  if (this->state_ == State::PREPARED &&
      static_cast<uint32_t>(now - this->state_started_) >= 5000) {
    this->finish_(ReadStatus::SEND_FAILED, now);
  } else if (this->state_ == State::WAITING &&
             static_cast<uint32_t>(now - this->state_started_) >= 1000) {
    if (this->entries_[this->head_].attempts >= 3) {
      this->finish_(ReadStatus::TIMEOUT, now);
    } else {
      this->last_used_[this->sequence_] = now;
      this->sequence_ = 0;
      this->state_ = State::IDLE;
    }
  }
  while (this->count_ != 0 && this->state_ == State::IDLE &&
         static_cast<uint32_t>(now - this->entries_[this->head_].queued_at) >= 5000)
    this->finish_(ReadStatus::EXPIRED, now);
}

bool ReadQueue::prepare(uint32_t now, Frame& frame) {
  frame.size = 0;
  this->tick(now);
  if (this->count_ == 0 || this->state_ != State::IDLE) return false;
  uint8_t sequence = this->allocate_sequence_(now);
  if (sequence == 0) return false;
  const auto& entry = this->entries_[this->head_];
  if (!encode_read(this->local_, entry.device, sequence, entry.code, frame)) return false;
  this->sequence_ = sequence;
  this->state_ = State::PREPARED;
  this->state_started_ = now;
  return true;
}

void ReadQueue::on_transmitted(uint8_t sequence, uint32_t now, bool success) {
  this->tick(now);
  if (this->state_ != State::PREPARED || this->sequence_ != sequence) return;
  if (!success) {
    this->finish_(ReadStatus::SEND_FAILED, now);
    return;
  }
  this->entries_[this->head_].attempts++;
  this->state_ = State::WAITING;
  this->state_started_ = now;
}

bool ReadQueue::on_packet(const Packet& packet, uint32_t now) {
  this->tick(now);
  if (this->count_ == 0 || this->state_ != State::WAITING || packet.sequence != this->sequence_ ||
      packet.source != this->entries_[this->head_].device || packet.destination != this->local_ ||
      packet.message_count > packet.messages.size() || packet.packet_type != 1)
    return false;
  bool found = false;
  uint32_t raw = 0;
  for (size_t i = 0; i < packet.message_count; ++i) {
    if (packet.messages[i].code == this->entries_[this->head_].code) {
      found = true;
      raw = packet.messages[i].raw;
      break;
    }
  }
  if (packet.type == DataType::NACK && (packet.message_count == 0 || found)) {
    this->finish_(ReadStatus::NACK, now);
    return true;
  }
  if (packet.type != DataType::RESPONSE || !found) return false;
  this->finish_(ReadStatus::SUCCESS, now, raw);
  return true;
}

}  // namespace esphome::samsung_hvac::nasa
