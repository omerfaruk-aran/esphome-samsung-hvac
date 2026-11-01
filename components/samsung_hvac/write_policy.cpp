#include "write_policy.h"
#include <cmath>
#include <algorithm>
#include <limits>
namespace esphome::samsung_hvac::advanced {
namespace {
bool near_integer(double value) {
  return std::isfinite(value) &&
         std::abs(value - std::round(value)) <=
             8 * std::numeric_limits<double>::epsilon() * std::max(1.0, std::abs(value));
}
bool same_physical_value(double first, double second) {
  return std::abs(first - second) <=
         8 * std::numeric_limits<double>::epsilon() * std::max(std::abs(first), std::abs(second));
}
}
Guard::Guard(const WritePolicy& policy) : policy_(policy) {}
bool Guard::valid_policy_() const {
  const auto role = this->policy_.device.role();
  if (!this->policy_.opt_in || !this->policy_.verified_family ||
      (role != nasa::AddressRole::INDOOR && role != nasa::AddressRole::OUTDOOR) ||
      this->policy_.local.role() != nasa::AddressRole::CONTROLLER || !this->policy_.rule_count ||
      this->policy_.rule_count > this->policy_.rules.size() || this->policy_.rate_ms < 1000 ||
      this->policy_.rate_ms > 60000 || this->policy_.preparation_ms < 100 ||
      this->policy_.preparation_ms > 5000 || this->policy_.readback_ms < 100 ||
      this->policy_.readback_ms > 60000)
    return false;
  for (size_t i = 0; i < this->policy_.rule_count; ++i) {
    const auto& r = this->policy_.rules[i];
    if (!r.descriptor.valid() || !std::isfinite(r.minimum) || !std::isfinite(r.maximum) ||
        r.minimum > r.maximum || !std::isfinite(r.step) || r.step <= 0)
      return false;
    for (size_t j = 0; j < i; ++j)
      if (this->policy_.rules[j].descriptor.code == r.descriptor.code) return false;
  }
  return true;
}
Admission Guard::prepare(nasa::Address device, uint16_t code, double value, uint32_t now,
                         Ticket& ticket) {
  ticket = {};
  this->tick(now);
  if (this->stage_ == Stage::FAULT || !this->valid_policy_() || device != this->policy_.device)
    return Admission::BLOCKED;
  if (this->stage_ != Stage::IDLE) return Admission::BUSY;
  const WriteRule* rule = nullptr;
  for (size_t i = 0; i < this->policy_.rule_count; ++i)
    if (this->policy_.rules[i].descriptor.code == code) rule = &this->policy_.rules[i];
  if (!rule) return Admission::BLOCKED;
  if (!std::isfinite(value) || value < rule->minimum || value > rule->maximum)
    return Admission::INVALID;
  const double steps = (value - rule->minimum) / rule->step;
  const double raw = (value - rule->descriptor.offset) / rule->descriptor.scale;
  if (!near_integer(steps) || !near_integer(raw)) return Admission::INVALID;
  const unsigned bits = rule->descriptor.bits();
  const double minimum =
      rule->descriptor.is_signed ? -static_cast<double>(uint64_t{1} << (bits - 1)) : 0;
  const double maximum =
      static_cast<double>((uint64_t{1} << (rule->descriptor.is_signed ? bits - 1 : bits)) - 1);
  const double rounded = std::round(raw);
  if (rounded < minimum || rounded > maximum) return Admission::INVALID;
  const uint32_t wire =
      static_cast<uint32_t>(rounded < 0 ? static_cast<int64_t>(rounded) + (int64_t{1} << bits)
                                        : static_cast<int64_t>(rounded));
  const auto decoded = rule->descriptor.decode({code, wire});
  if (!decoded || *decoded < rule->minimum || *decoded > rule->maximum ||
      !same_physical_value(*decoded, value) ||
      !near_integer((*decoded - rule->minimum) / rule->step))
    return Admission::INVALID;
  if (this->attempted_ && static_cast<uint32_t>(now - this->last_attempt_) < this->policy_.rate_ms)
    return Admission::RATE_LIMITED;
  if (++this->generation_ == 0) ++this->generation_;
  this->ticket_ = {this->generation_, code, wire};
  ticket = this->ticket_;
  this->rule_ = rule;
  this->started_ = now;
  this->last_attempt_ = now;
  this->attempted_ = true;
  this->stage_ = Stage::PREPARED;
  this->result_ = Result::PENDING;
  return Admission::READY;
}
void Guard::on_transmitted(uint32_t ticket, bool success, uint32_t now) {
  this->tick(now);
  if (this->stage_ != Stage::PREPARED || ticket != this->ticket_.id) return;
  if (!success) {
    this->stage_ = Stage::FAULT;
    this->result_ = Result::SEND_FAILED;
    return;
  }
  this->stage_ = Stage::WAITING;
  this->started_ = now;
  this->last_attempt_ = now;
}
void Guard::tick(uint32_t now) {
  if (this->stage_ == Stage::PREPARED &&
      static_cast<uint32_t>(now - this->started_) >= this->policy_.preparation_ms) {
    this->stage_ = Stage::FAULT;
    this->result_ = Result::UNKNOWN_OUTCOME;
  } else if (this->stage_ == Stage::WAITING &&
             static_cast<uint32_t>(now - this->started_) >= this->policy_.readback_ms) {
    this->stage_ = Stage::IDLE;
    this->result_ = Result::TIMED_OUT;
  }
}
void Guard::observe(const nasa::Packet& packet, uint32_t now) {
  this->tick(now);
  if (this->stage_ != Stage::WAITING || packet.source != this->policy_.device ||
      packet.destination != this->policy_.local || packet.message_count > packet.messages.size() ||
      (packet.type != nasa::DataType::NOTIFICATION && packet.type != nasa::DataType::RESPONSE))
    return;
  for (size_t i = 0; i < packet.message_count; ++i) {
    const auto value = this->rule_->descriptor.decode(packet.messages[i]);
    if (value && packet.messages[i].raw == this->ticket_.raw) {
      this->stage_ = Stage::IDLE;
      this->result_ = Result::OBSERVED_MATCH;
      return;
    }
  }
}
}  // namespace esphome::samsung_hvac::advanced
