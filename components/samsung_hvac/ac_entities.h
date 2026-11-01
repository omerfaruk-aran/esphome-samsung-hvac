#pragma once
#include "ac_profile.h"
#include "profile_consumer.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/climate/climate.h"

namespace esphome::samsung_hvac::profiles {
class AcClimate : public climate::Climate {
 public:
  void set_room_supported(bool value) { this->room_supported_ = value; }
  void observe(const State& state);

 protected:
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall& call) override;
  bool room_supported_{false};
};
class AcDevice : public PacketConsumer, private StateSink {
 public:
  AcDevice(Family family, uint32_t address) : profile_(family, address, *this) {}
  void set_outdoor_address(uint32_t address) { this->profile_.set_outdoor_address(address); }
  void set_fans(uint32_t mask) { this->profile_.set_fans(mask); }
  void set_presets(uint32_t mask) { this->profile_.set_presets(mask); }
  void set_swing(bool vertical, bool horizontal) { this->profile_.set_swing(vertical, horizontal); }
  void set_sensor(Field field, sensor::Sensor* entity) {
    this->sensors_[static_cast<size_t>(field)] = entity;
  }
  void set_binary_sensor(Field field, binary_sensor::BinarySensor* entity) {
    this->binaries_[static_cast<size_t>(field)] = entity;
  }
  void set_text_sensor(Field field, text_sensor::TextSensor* entity) {
    this->texts_[static_cast<size_t>(field)] = entity;
  }
  void set_climate(AcClimate* entity) { this->climate_ = entity; }
  void receive(const nasa::Packet& packet) override { this->profile_.receive(packet); }
  void receive(const non_nasa::Packet& packet) override { this->profile_.receive(packet); }

 private:
  void on_state(const State& state, uint32_t mask) override;
  AcProfile profile_;
  std::array<sensor::Sensor*, FIELD_COUNT> sensors_{};
  std::array<binary_sensor::BinarySensor*, FIELD_COUNT> binaries_{};
  std::array<text_sensor::TextSensor*, FIELD_COUNT> texts_{};
  AcClimate* climate_{nullptr};
};
}  // namespace esphome::samsung_hvac::profiles
