#include "non_nasa_engine.h"

namespace esphome::samsung_hvac::non_nasa {

void Engine::receive(const Frame &frame, uint32_t now) {
  if (!decode(frame, this->packet_)) {
    this->stats_.invalid_packets++;
    return;
  }
  this->stats_.decoded_packets++;
  this->registration_.observe(this->packet_, now);
  this->controls_.observe(this->packet_, now);
  this->sink_.on_non_nasa_packet(this->packet_);
}

}  // namespace esphome::samsung_hvac::non_nasa
