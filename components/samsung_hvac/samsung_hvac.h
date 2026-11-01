#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "protocol_selection.h"

namespace esphome::samsung_hvac {

// Foundation only: no parser, entity, RX loop, or transmit path is installed.
class SamsungHVAC : public Component, public uart::UARTDevice {
 public:
  void set_protocol(Protocol protocol) { this->protocol_ = protocol; }
  void dump_config() override;

 protected:
  Protocol protocol_{Protocol::NASA};
};

}  // namespace esphome::samsung_hvac
