#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/uart/uart_component_esp_idf.h"
#include "esphome/core/gpio.h"
#include "bus_transport.h"
#include "protocol_selection.h"
#include <memory>

namespace esphome::samsung_hvac {

class SamsungHVAC : public Component, public uart::UARTDevice, private BusIO, private FrameSink {
 public:
  void set_protocol(Protocol protocol) { this->protocol_ = protocol; }
  void set_flow_control_pin(GPIOPin *pin) { this->flow_control_pin_ = pin; }
  void setup() override;
  void loop() override;
  void on_shutdown() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  Protocol protocol_{Protocol::NASA};
  GPIOPin *flow_control_pin_{nullptr};

 private:
  bool read(uint8_t &byte) override;
  size_t write_capacity() override;
  bool write(const uint8_t *data, size_t size) override;
  TxStatus tx_status() override;
  void set_direction(bool transmit) override;
  void on_frame(const Frame &frame) override;
  Diagnostics stats_;
  uart::IDFUARTComponent *native_uart_{nullptr};
  std::unique_ptr<FrameAssembler> framer_;
  std::unique_ptr<Transport> transport_;
};

}  // namespace esphome::samsung_hvac
