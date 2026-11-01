#pragma once

#include "frame_assembler.h"

namespace esphome::samsung_hvac::nasa {

uint16_t frame_crc(const uint8_t* data, size_t size);

class Framer : public FrameAssembler {
 public:
  using FrameAssembler::FrameAssembler;

 protected:
  size_t expected_size() const override;
  bool valid(size_t size) const override;
};

}  // namespace esphome::samsung_hvac::nasa
