#include "samsung_hvac.h"
#include "esphome/core/log.h"
#include "nasa_framer.h"
#include "non_nasa_framer.h"
#include <cinttypes>

namespace esphome::samsung_hvac {

static const char *const TAG = "samsung_hvac";

void SamsungHVAC::setup() {
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->setup();
    this->flow_control_pin_->digital_write(false);
  }
  // The final schema requires this concrete native UART implementation.
  this->native_uart_ = static_cast<uart::IDFUARTComponent *>(this->parent_);
  if (this->native_uart_ == nullptr || this->native_uart_->is_failed()) {
    this->mark_failed();
    return;
  }
  auto &sink = static_cast<FrameSink &>(*this);
  if (this->protocol_ == Protocol::NASA) {
    this->framer_ = std::make_unique<nasa::Framer>(sink, this->stats_);
    this->nasa_engine_ = std::make_unique<nasa::Engine>(nasa::Address{0x80ff00},
        static_cast<nasa::PacketSink &>(*this), static_cast<nasa::ReadSink &>(*this));
  } else {
    this->framer_ = std::make_unique<non_nasa::Framer>(sink, this->stats_);
    this->non_nasa_engine_ = std::make_unique<non_nasa::Engine>(static_cast<non_nasa::PacketSink &>(*this));
  }
  this->transport_ = std::make_unique<Transport>(static_cast<BusIO &>(*this), *this->framer_, this->stats_);
  // No production API enables TX in P1. Receive-only discovery is the default.
}

void SamsungHVAC::loop() {
  if (this->transport_ == nullptr) return;
  this->transport_->loop(millis());
  if (this->nasa_engine_ != nullptr) this->nasa_engine_->tick(millis());
  if (this->non_nasa_engine_ != nullptr) this->non_nasa_engine_->tick(millis());
  if (this->stats_.tx_faults != 0) {
    ESP_LOGE(TAG, "Transport TX fault; restart required before any further transmission");
    this->mark_failed();
  }
}

void SamsungHVAC::on_shutdown() { this->set_direction(false); }

bool SamsungHVAC::read(uint8_t &byte) {
  return this->native_uart_->available() != 0 && this->native_uart_->read_byte(&byte);
}

size_t SamsungHVAC::write_capacity() { return this->native_uart_->available_for_write(); }

bool SamsungHVAC::write(const uint8_t *data, size_t size) {
  if (this->native_uart_->is_failed() || size > this->write_capacity()) return false;
  this->native_uart_->write_array(data, size);
  return !this->native_uart_->is_failed();
}

TxStatus SamsungHVAC::tx_status() {
  auto port = static_cast<uart_port_t>(this->native_uart_->get_hw_serial_number());
  if (!uart_is_driver_installed(port)) return TxStatus::FAILED;
  esp_err_t result = uart_wait_tx_done(port, 0);
  if (result == ESP_OK) return TxStatus::DONE;
  return result == ESP_ERR_TIMEOUT ? TxStatus::BUSY : TxStatus::FAILED;
}

void SamsungHVAC::set_direction(bool transmit) {
  if (this->flow_control_pin_ != nullptr) this->flow_control_pin_->digital_write(transmit);
}

void SamsungHVAC::on_frame(const Frame &frame) {
  if (this->nasa_engine_ != nullptr) this->nasa_engine_->receive(frame, millis());
  if (this->non_nasa_engine_ != nullptr) this->non_nasa_engine_->receive(frame, millis());
}

void SamsungHVAC::on_nasa_packet(const nasa::Packet &packet) {
  for (size_t i = 0; i < this->message_count_; ++i) this->messages_[i]->receive(packet);
  for (size_t i = 0; i < this->profile_count_; ++i) this->profiles_[i]->receive(packet);
}

void SamsungHVAC::on_read_complete(const nasa::ReadResult &result) {
  // Production never queues active reads in P2; host tests exercise the lifecycle.
  (void) result;
}

void SamsungHVAC::on_non_nasa_packet(const non_nasa::Packet &packet) {
  for (size_t i = 0; i < this->profile_count_; ++i) this->profiles_[i]->receive(packet);
}

void SamsungHVAC::dump_config() {
  ESP_LOGCONFIG(TAG, "Samsung HVAC transport:");
  ESP_LOGCONFIG(TAG, "  Protocol: %s", protocol_name(this->protocol_));
  ESP_LOGCONFIG(TAG, "  Receive-only; no active polling, registration, or control");
  ESP_LOGCONFIG(TAG, "  Passive device profiles: %u", static_cast<unsigned>(this->profile_count_));
  ESP_LOGCONFIG(TAG, "  Passive typed observers: %u", static_cast<unsigned>(this->message_count_));
  LOG_PIN("  Direction pin: ", this->flow_control_pin_);
  ESP_LOGCONFIG(TAG, "  RX: %" PRIu32 " bytes, %" PRIu32 " valid frames, %" PRIu32 " invalid candidates",
                this->stats_.rx_bytes, this->stats_.rx_frames, this->stats_.invalid_frames);
  ESP_LOGCONFIG(TAG, "  RX timeouts: %" PRIu32 ", discarded bytes: %" PRIu32, this->stats_.frame_timeouts,
                this->stats_.discarded_bytes);
  if (this->nasa_engine_ != nullptr) {
    const auto &stats = this->nasa_engine_->stats();
    ESP_LOGCONFIG(TAG, "  NASA decoded: %" PRIu32 ", invalid typed packets: %" PRIu32,
                  stats.decoded_packets, stats.invalid_packets);
  }
  if (this->non_nasa_engine_ != nullptr) {
    const auto &stats = this->non_nasa_engine_->stats();
    ESP_LOGCONFIG(TAG, "  Non-NASA decoded: %" PRIu32 ", invalid packets: %" PRIu32,
                  stats.decoded_packets, stats.invalid_packets);
  }
}

}  // namespace esphome::samsung_hvac
