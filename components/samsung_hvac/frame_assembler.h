#pragma once

#include "bus_types.h"

namespace esphome::samsung_hvac {

// Shared storage mechanics only. Protocol-specific boundary/integrity rules are separate.
class FrameAssembler {
 public:
  FrameAssembler(FrameSink &sink, Diagnostics &stats) : sink_(sink), stats_(stats) {}
  virtual ~FrameAssembler() = default;
  void feed(uint8_t byte, uint32_t now);
  void tick(uint32_t now);
  bool pending() const { return this->buffer_.size != 0; }

 protected:
  static constexpr size_t INVALID_SIZE = MAX_FRAME_SIZE + 1;
  virtual size_t expected_size() const = 0;
  virtual bool valid(size_t size) const = 0;
  Frame buffer_;

 private:
  void consume_(size_t size);
  FrameSink &sink_;
  Diagnostics &stats_;
  uint32_t last_byte_{0};
};

}  // namespace esphome::samsung_hvac
