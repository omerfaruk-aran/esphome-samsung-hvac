#pragma once
#include "ehs_profile.h"
#include "profile_consumer.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome::samsung_hvac::profiles::ehs {
class Device : public PacketConsumer, private StateSink {
 public:
  Device(uint32_t address) : profile_(address, *this) {}
  using PacketConsumer::receive;
  void receive(const nasa::Packet& packet) override { this->profile_.receive(packet); }
  void set_outdoor_address(uint32_t address) { this->profile_.set_outdoor_address(address); }
  void set_zones(bool first, bool second) { this->profile_.set_zones(first, second); }
  void set_sensor(Field field, sensor::Sensor* entity) {
    this->sensors_[static_cast<size_t>(field)] = entity;
  }
  void set_binary_sensor(Field field, binary_sensor::BinarySensor* entity) {
    this->binaries_[static_cast<size_t>(field)] = entity;
  }
  void set_text_sensor(Field field, text_sensor::TextSensor* entity) {
    this->texts_[static_cast<size_t>(field)] = entity;
  }

 private:
  void on_state(const State& state, uint32_t mask) override;
  Profile profile_;
  std::array<sensor::Sensor*, FIELD_COUNT> sensors_{};
  std::array<binary_sensor::BinarySensor*, FIELD_COUNT> binaries_{};
  std::array<text_sensor::TextSensor*, FIELD_COUNT> texts_{};
};
}  // namespace esphome::samsung_hvac::profiles::ehs
