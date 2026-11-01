#pragma once

#include "frame_assembler.h"

namespace esphome::samsung_hvac {

enum class TxStatus { BUSY, DONE, FAILED };

class BusIO {
 public:
  virtual ~BusIO() = default;
  virtual bool read(uint8_t &byte) = 0;
  virtual size_t write_capacity() = 0;
  virtual bool write(const uint8_t *data, size_t size) = 0;
  virtual TxStatus tx_status() = 0;
  virtual void set_direction(bool transmit) = 0;
};

class Transport {
 public:
  Transport(BusIO &io, FrameAssembler &framer, Diagnostics &stats) : io_(io), framer_(framer), stats_(stats) {}
  void loop(uint32_t now);
  bool enqueue(const Frame &frame, uint32_t now);
  void set_tx_enabled(bool enabled);

 private:
  enum class State { IDLE, SENDING, DRAINING, GUARD, FAULT };
  struct QueuedFrame {
    Frame frame;
    uint32_t queued_at{0};
  };
  void fault_();
  void pop_();
  BusIO &io_;
  FrameAssembler &framer_;
  Diagnostics &stats_;
  std::array<QueuedFrame, 4> queue_{};
  size_t head_{0};
  size_t count_{0};
  size_t offset_{0};
  State state_{State::IDLE};
  bool tx_enabled_{false};
  uint32_t last_activity_{0};
  uint32_t tx_started_{0};
  uint32_t guard_started_{0};
};

}  // namespace esphome::samsung_hvac
