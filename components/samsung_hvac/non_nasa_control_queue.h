#pragma once

#include "non_nasa_packet.h"
#include <optional>

namespace esphome::samsung_hvac::non_nasa {

struct Patch {
  std::optional<bool> power;
  std::optional<uint8_t> target;
  std::optional<Mode> mode;
  std::optional<Fan> fan;
  std::optional<Swing> swing;
};
enum class ControlResult { NONE, PENDING, VERIFIED, TIMEOUT, READBACK_TIMEOUT, SEND_FAILED, CANCELLED, SUPERSEDED };

class ControlQueue {
 public:
  void set_enabled(bool enabled);
  void observe(const Packet &packet, uint32_t now);
  bool submit(uint8_t address, const Patch &patch, uint32_t now);
  bool prepare(uint32_t now, Frame &frame, uint32_t &token);
  bool is_current(uint32_t token) const;
  void on_transmitted(uint32_t token, uint32_t now, bool success);
  void tick(uint32_t now);
  bool pending(uint8_t address) const;
  ControlResult result(uint8_t address) const;
  bool needs_wakeup(uint32_t now) const;
  void require_fresh_status();

 private:
  enum class Phase { IDLE, PENDING, PREPARED, WAITING };
  enum Mask : uint8_t { POWER = 1, MODE = 2, TARGET = 4, FAN = 8, SWING = 16 };
  struct Device {
    bool used{false};
    bool valid{false};
    uint8_t address{0};
    ControlState observed, desired;
    uint32_t observed_at{0};
    uint32_t queued_at{0};
    uint32_t phase_at{0};
    uint32_t generation{0};
    uint32_t prepared_generation{0};
    uint32_t sent_generation{0};
    uint32_t token{0};
    uint8_t mask{0};
    uint8_t attempts{0};
    bool ack_seen{false};
    std::array<uint8_t, 8> prepared_payload{}, sent_payload{};
    Phase phase{Phase::IDLE};
    ControlResult result{ControlResult::NONE};
  };
  Device *find_(uint8_t address);
  const Device *find_(uint8_t address) const;
  static bool matches_(const ControlState &a, const ControlState &b, uint8_t mask);
  static ControlState merge_(const ControlState &base, const ControlState &desired, uint8_t mask);
  static void finish_(Device &device, ControlResult result);
  std::array<Device, 8> devices_{};
  bool enabled_{false};
  bool poll_pending_{false};
  uint32_t poll_at_{0};
  uint32_t next_token_{0};
  size_t cursor_{0};
};

}  // namespace esphome::samsung_hvac::non_nasa
