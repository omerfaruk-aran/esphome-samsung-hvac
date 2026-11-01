#include "nasa_engine.h"

namespace esphome::samsung_hvac::nasa {

void Engine::receive(const Frame &frame, uint32_t now) {
  if (decode(frame, this->packet_) != DecodeError::NONE) {
    this->stats_.invalid_packets++;
    return;
  }
  this->stats_.decoded_packets++;
  this->stats_.categories[static_cast<uint8_t>(this->packet_.type)]++;
  this->reads_.on_packet(this->packet_, now);
  this->sink_.on_nasa_packet(this->packet_);
}

}  // namespace esphome::samsung_hvac::nasa
