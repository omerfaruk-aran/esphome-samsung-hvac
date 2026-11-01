#pragma once
#include "typed_message.h"
#include "profile_consumer.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
namespace esphome::samsung_hvac::advanced {
class MessageEntity : public profiles::PacketConsumer, private ValueSink {
 public:
  MessageEntity(uint32_t source,uint16_t code,bool is_signed,double scale,double offset)
      : observer_({source},Descriptor{code,is_signed,scale,offset},*this) {}
  using profiles::PacketConsumer::receive;
  void receive(const nasa::Packet &packet) override { this->observer_.receive(packet); }
  void set_sensor(sensor::Sensor *entity) { this->sensor_=entity; }
  void set_binary_sensor(binary_sensor::BinarySensor *entity) { this->binary_=entity; }
  void set_sentinel(size_t index,uint32_t value) { this->observer_.set_sentinel(index,value); }
  void set_valid_value(size_t index,uint32_t value) { this->observer_.set_valid_value(index,value); }
  void set_raw_range(uint32_t minimum,uint32_t maximum) { this->observer_.set_raw_range(minimum,maximum); }
 private:
  void on_value(std::optional<double> value) override;
  Observer observer_;
  sensor::Sensor *sensor_{nullptr};
  binary_sensor::BinarySensor *binary_{nullptr};
};
}  // namespace esphome::samsung_hvac::advanced
