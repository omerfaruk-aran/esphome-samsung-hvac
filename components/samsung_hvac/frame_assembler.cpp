#include "frame_assembler.h"
#include <cstring>

namespace esphome::samsung_hvac {

void FrameAssembler::consume_(size_t size) {
  this->buffer_.size -= size;
  std::memmove(this->buffer_.data.data(), this->buffer_.data.data() + size, this->buffer_.size);
}

void FrameAssembler::tick(uint32_t now) {
  if (this->pending() && static_cast<uint32_t>(now - this->last_byte_) >= 50) {
    this->stats_.frame_timeouts++;
    this->stats_.discarded_bytes += this->buffer_.size;
    this->buffer_.size = 0;
  }
}

void FrameAssembler::feed(uint8_t byte, uint32_t now) {
  this->tick(now);
  this->last_byte_ = now;
  if (this->buffer_.size == MAX_FRAME_SIZE) {
    this->stats_.buffer_overflows++;
    this->buffer_.size = 0;
  }
  this->buffer_.data[this->buffer_.size++] = byte;
  while (this->pending()) {
    if (this->buffer_.data[0] != 0x32) {
      this->stats_.discarded_bytes++;
      this->consume_(1);
      continue;
    }
    size_t expected = this->expected_size();
    if (expected == INVALID_SIZE) {
      this->stats_.invalid_frames++;
      this->consume_(1);
      continue;
    }
    if (expected == 0 || this->buffer_.size < expected) return;
    if (!this->valid(expected)) {
      this->stats_.invalid_frames++;
      this->consume_(1);
      continue;
    }
    // Resynchronization can leave bytes after the recovered frame.
    size_t buffered = this->buffer_.size;
    this->buffer_.size = expected;
    this->stats_.rx_frames++;
    this->sink_.on_frame(this->buffer_);
    this->buffer_.size = buffered;
    this->consume_(expected);
  }
}

}  // namespace esphome::samsung_hvac
