#pragma once
#include "typed_message.h"
namespace esphome::samsung_hvac::advanced {
struct WriteRule { Descriptor descriptor; double minimum{0}, maximum{0}, step{1}; };
struct WritePolicy {
  bool opt_in{false}, verified_family{false};
  nasa::Address local, device;
  std::array<WriteRule,8> rules{};
  uint8_t rule_count{0};
  uint32_t rate_ms{1000}, preparation_ms{5000}, readback_ms{1000};
};
struct Ticket { uint32_t id{0}; uint16_t code{0}; uint32_t raw{0}; };
enum class Admission { READY, BLOCKED, BUSY, INVALID, RATE_LIMITED };
enum class Result { NONE, PENDING, OBSERVED_MATCH, TIMED_OUT, UNKNOWN_OUTCOME, SEND_FAILED };
// Offline policy/lifecycle only. No frame encoder, scheduler, UART, retry or public enable API.
class Guard {
 public:
  explicit Guard(const WritePolicy &policy);
  Guard(const Guard &)=delete;
  Guard &operator=(const Guard &)=delete;
  Guard(Guard &&)=delete;
  Guard &operator=(Guard &&)=delete;
  Admission prepare(nasa::Address device,uint16_t code,double value,uint32_t now,Ticket &ticket);
  void on_transmitted(uint32_t ticket,bool success,uint32_t now);
  void observe(const nasa::Packet &packet,uint32_t now);
  void tick(uint32_t now);
  Result result() const { return this->result_; }
 private:
  enum class Stage { IDLE, PREPARED, WAITING, FAULT };
  bool valid_policy_() const;
  const WritePolicy policy_;
  Stage stage_{Stage::IDLE};
  Result result_{Result::NONE};
  Ticket ticket_;
  const WriteRule *rule_{nullptr};
  uint32_t generation_{0}, started_{0}, last_attempt_{0};
  bool attempted_{false};
};
}  // namespace esphome::samsung_hvac::advanced
