#pragma once

#include "frame_assembler.h"

namespace esphome::samsung_hvac::non_nasa {

class Framer : public FrameAssembler {
 public:
  using FrameAssembler::FrameAssembler;

 protected:
  size_t expected_size() const override { return 14; }
  bool valid(size_t size) const override;
};

}  // namespace esphome::samsung_hvac::non_nasa
