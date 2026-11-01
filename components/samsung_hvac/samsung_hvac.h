#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/uart/uart_component_esp_idf.h"
#include "esphome/core/gpio.h"
#include "bus_transport.h"
#include "nasa_engine.h"
#include "non_nasa_engine.h"
#include "protocol_selection.h"
#include "ac_entities.h"
#include "ehs_entities.h"
#include "message_entity.h"
#include <memory>

namespace esphome::samsung_hvac {

class SamsungHVAC : public Component, public uart::UARTDevice, private BusIO, private FrameSink,
                    private nasa::PacketSink, private nasa::ReadSink, private non_nasa::PacketSink {
 public:
  void set_protocol(Protocol protocol) { this->protocol_ = protocol; }
  void set_flow_control_pin(GPIOPin *pin) { this->flow_control_pin_ = pin; }
  void add_profile(profiles::PacketConsumer *profile) {
    if (this->profile_count_ < this->profiles_.size()) this->profiles_[this->profile_count_++] = profile;
  }
  void add_message(advanced::MessageEntity *message) {
    if (this->message_count_ < this->messages_.size()) this->messages_[this->message_count_++] = message;
  }
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
  void on_nasa_packet(const nasa::Packet &packet) override;
  void on_read_complete(const nasa::ReadResult &result) override;
  void on_non_nasa_packet(const non_nasa::Packet &packet) override;
  std::array<profiles::PacketConsumer *, 8> profiles_{};
  size_t profile_count_{0};
  std::array<advanced::MessageEntity *, 32> messages_{};
  size_t message_count_{0};
  Diagnostics stats_;
  uart::IDFUARTComponent *native_uart_{nullptr};
  std::unique_ptr<FrameAssembler> framer_;
  std::unique_ptr<Transport> transport_;
  std::unique_ptr<nasa::Engine> nasa_engine_;
  std::unique_ptr<non_nasa::Engine> non_nasa_engine_;
};

}  // namespace esphome::samsung_hvac
