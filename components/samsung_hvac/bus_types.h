#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace esphome::samsung_hvac {

constexpr size_t MAX_FRAME_SIZE = 1502;

struct Frame {
  std::array<uint8_t, MAX_FRAME_SIZE> data{};
  size_t size{0};
};

struct Diagnostics {
  uint32_t rx_bytes{0};
  uint32_t rx_frames{0};
  uint32_t discarded_bytes{0};
  uint32_t invalid_frames{0};
  uint32_t frame_timeouts{0};
  uint32_t buffer_overflows{0};
  uint32_t echo_bytes{0};
  uint32_t tx_frames{0};
  uint32_t tx_rejected{0};
  uint32_t tx_expired{0};
  uint32_t tx_faults{0};
};

class FrameSink {
 public:
  virtual ~FrameSink() = default;
  // Borrowed until this synchronous callback returns; do not reenter the assembler.
  virtual void on_frame(const Frame& frame) = 0;
};

}  // namespace esphome::samsung_hvac
