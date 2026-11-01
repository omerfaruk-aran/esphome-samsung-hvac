#include "non_nasa_control_queue.h"

namespace esphome::samsung_hvac::non_nasa {

namespace {
bool same_mode(Mode a, Mode b) {
  auto automatic = [](Mode mode) { return mode == Mode::AUTO || mode == Mode::AUTO_HEAT; };
  return a == b || (automatic(a) && automatic(b));
}
bool same_fan(Fan a, Fan b) {
  auto high = [](Fan fan) { return fan == Fan::HIGH_SPEED || fan == Fan::FRESH; };
  return a == b || (high(a) && high(b));
}
}  // namespace

ControlQueue::Device *ControlQueue::find_(uint8_t address) {
  for (auto &device : this->devices_) if (device.used && device.address == address) return &device;
  return nullptr;
}
const ControlQueue::Device *ControlQueue::find_(uint8_t address) const {
  for (const auto &device : this->devices_) if (device.used && device.address == address) return &device;
  return nullptr;
}

bool ControlQueue::matches_(const ControlState &a, const ControlState &b, uint8_t mask) {
  return ((mask & POWER) == 0 || a.power == b.power) && ((mask & MODE) == 0 || same_mode(a.mode, b.mode)) &&
         ((mask & TARGET) == 0 || a.target == b.target) && ((mask & FAN) == 0 || same_fan(a.fan, b.fan)) &&
         ((mask & SWING) == 0 || a.swing == b.swing);
}

ControlState ControlQueue::merge_(const ControlState &base, const ControlState &desired, uint8_t mask) {
  ControlState result = base;
  if ((mask & POWER) != 0) result.power = desired.power;
  if ((mask & MODE) != 0) result.mode = desired.mode;
  if ((mask & TARGET) != 0) result.target = desired.target;
  if ((mask & FAN) != 0) result.fan = desired.fan;
  if ((mask & SWING) != 0) result.swing = desired.swing;
  return result;
}

void ControlQueue::finish_(Device &device, ControlResult result) {
  device.phase = Phase::IDLE;
  device.result = result;
  device.mask = 0;
  device.ack_seen = false;
  device.token = 0;
  device.desired = device.observed;
}

void ControlQueue::set_enabled(bool enabled) {
  this->enabled_ = enabled;
  if (!enabled) {
    this->poll_pending_ = false;
    for (auto &device : this->devices_)
      if (device.phase != Phase::IDLE) finish_(device, ControlResult::CANCELLED);
  }
}

void ControlQueue::tick(uint32_t now) {
  if (this->poll_pending_ && static_cast<uint32_t>(now - this->poll_at_) >= 100) this->poll_pending_ = false;
  for (auto &device : this->devices_) {
    if (!device.used || device.phase == Phase::IDLE) continue;
    if (device.phase == Phase::PREPARED && static_cast<uint32_t>(now - device.phase_at) >= 5000) {
      finish_(device, ControlResult::SEND_FAILED);
    } else if (static_cast<uint32_t>(now - device.queued_at) >= 15000) {
      finish_(device, ControlResult::TIMEOUT);
    } else if (device.phase == Phase::WAITING && static_cast<uint32_t>(now - device.phase_at) >= 4500) {
      if (device.ack_seen) finish_(device, ControlResult::READBACK_TIMEOUT);
      else if (device.attempts >= 3) finish_(device, ControlResult::TIMEOUT);
      else device.phase = Phase::PENDING;
    }
  }
}

void ControlQueue::observe(const Packet &packet, uint32_t now) {
  this->tick(now);
  if (packet.command == 0xc6 && packet.source == 0xc8 && packet.destination == 0xd0 && packet.payload[0] == 1) {
    this->poll_pending_ = true;
    this->poll_at_ = now;
    return;
  }
  if (packet.command == 0x54 && packet.destination == 0xd0) {
    auto *device = this->find_(packet.source);
    if (device != nullptr && device->phase == Phase::WAITING && device->sent_generation == device->generation &&
        packet.payload == device->sent_payload)
      device->ack_seen = true;
    return; // No ACK, including an exact echo, completes an intent.
  }
  if (packet.command != 0x20 || packet.source >= 0x80) return;
  auto *device = this->find_(packet.source);
  if (device == nullptr) {
    for (auto &candidate : this->devices_) {
      if (!candidate.used) {
        candidate.used = true;
        candidate.address = packet.source;
        device = &candidate;
        break;
      }
    }
  }
  if (device == nullptr) return;
  Status status;
  ControlState fresh;
  if (!decode_status(packet, status) || !control_from_status(status, fresh)) {
    device->valid = false;
    if (device->phase != Phase::IDLE) finish_(*device, ControlResult::SEND_FAILED);
    return;
  }
  bool achieved = device->phase != Phase::IDLE && device->phase != Phase::PREPARED &&
                  matches_(device->desired, fresh, device->mask);
  bool superseded = false;
  if (device->phase == Phase::WAITING && device->ack_seen) {
    for (uint8_t bit : {uint8_t(POWER), uint8_t(MODE), uint8_t(TARGET), uint8_t(FAN), uint8_t(SWING)}) {
      if ((device->mask & bit) == 0 || ((!fresh.power) && (bit == FAN || bit == SWING))) continue;
      if (!matches_(device->observed, fresh, bit) && !matches_(device->desired, fresh, bit)) superseded = true;
    }
  }
  device->observed = fresh;
  device->observed_at = now;
  device->valid = true;
  if (achieved) finish_(*device, ControlResult::VERIFIED);
  else if (superseded) finish_(*device, ControlResult::SUPERSEDED);
  else if (device->phase == Phase::IDLE) device->desired = fresh;
  else {
    auto refreshed = merge_(fresh, device->desired, device->mask);
    constexpr uint8_t all_fields = POWER | MODE | TARGET | FAN | SWING;
    bool wire_changed = !matches_(device->desired, refreshed, all_fields) || device->desired.room != refreshed.room;
    if (device->phase == Phase::PREPARED && device->generation == device->prepared_generation &&
        (wire_changed || matches_(device->desired, fresh, device->mask))) {
      if (++device->generation == 0) ++device->generation;
    }
    device->desired = refreshed;
  }
}

bool ControlQueue::submit(uint8_t address, const Patch &patch, uint32_t now) {
  this->tick(now);
  auto *device = this->find_(address);
  if (!this->enabled_ || device == nullptr || !device->valid ||
      static_cast<uint32_t>(now - device->observed_at) >= 30000 ||
      (!patch.power && !patch.mode && !patch.target && !patch.fan && !patch.swing) ||
      (patch.mode && patch.power && !*patch.power))
    return false;
  ControlState desired = device->phase == Phase::IDLE ? device->observed : device->desired;
  uint8_t mask = device->phase == Phase::IDLE ? 0 : device->mask;
  if (patch.mode) { desired.mode = *patch.mode; desired.power = true; mask |= MODE | POWER; }
  if (patch.power) { desired.power = *patch.power; mask |= POWER; }
  if (patch.target) { desired.target = *patch.target; mask |= TARGET; }
  if (patch.fan) { desired.fan = *patch.fan; mask |= FAN; }
  if (patch.swing) { desired.swing = *patch.swing; mask |= SWING; }
  if (!valid_control(desired)) return false;
  device->desired = desired;
  device->mask = mask;
  if (++device->generation == 0) ++device->generation;
  if (device->phase != Phase::PREPARED) device->phase = Phase::PENDING;
  device->queued_at = now;
  device->attempts = 0;
  device->ack_seen = false;
  device->result = ControlResult::PENDING;
  return true;
}

bool ControlQueue::prepare(uint32_t now, Frame &frame, uint32_t &token) {
  frame.size = 0;
  this->tick(now);
  if (!this->enabled_ || !this->poll_pending_) return false;
  for (const auto &device : this->devices_) if (device.phase == Phase::PREPARED) return false;
  for (size_t step = 0; step < this->devices_.size(); ++step) {
    size_t index = (this->cursor_ + step) % this->devices_.size();
    auto &device = this->devices_[index];
    if (device.phase != Phase::PENDING || !device.valid ||
        static_cast<uint32_t>(now - device.observed_at) >= 30000)
      continue;
    if (matches_(device.desired, device.observed, device.mask)) {
      finish_(device, ControlResult::VERIFIED);
      continue;
    }
    if (!encode_control(device.address, device.desired, frame)) return false;
    if (++this->next_token_ == 0) ++this->next_token_;
    device.token = this->next_token_;
    device.prepared_generation = device.generation;
    for (size_t i = 0; i < 8; ++i) device.prepared_payload[i] = frame.data[4 + i];
    device.phase = Phase::PREPARED;
    device.phase_at = now;
    this->cursor_ = (index + 1) % this->devices_.size();
    this->poll_pending_ = false;
    token = device.token;
    return true;
  }
  return false;
}

bool ControlQueue::is_current(uint32_t token) const {
  for (const auto &device : this->devices_)
    if (device.phase == Phase::PREPARED && device.token == token && device.generation == device.prepared_generation)
      return true;
  return false;
}

void ControlQueue::on_transmitted(uint32_t token, uint32_t now, bool success) {
  this->tick(now);
  for (auto &device : this->devices_) {
    if (device.phase != Phase::PREPARED || device.token != token) continue;
    if (device.prepared_generation != device.generation) {
      device.phase = Phase::PENDING;
      return;
    }
    if (!success) { finish_(device, ControlResult::SEND_FAILED); return; }
    device.phase = Phase::WAITING;
    device.phase_at = now;
    device.sent_generation = device.generation;
    device.sent_payload = device.prepared_payload;
    device.attempts++;
    return;
  }
}

bool ControlQueue::pending(uint8_t address) const {
  const auto *device = this->find_(address);
  return device != nullptr && device->phase != Phase::IDLE;
}
ControlResult ControlQueue::result(uint8_t address) const {
  const auto *device = this->find_(address);
  return device == nullptr ? ControlResult::NONE : device->result;
}

bool ControlQueue::needs_wakeup(uint32_t now) const {
  if (!this->enabled_) return false;
  for (const auto &device : this->devices_)
    if (device.phase == Phase::PENDING && static_cast<uint32_t>(now - device.queued_at) >= 1000 &&
        static_cast<uint32_t>(now - device.queued_at) < 15000)
      return true;
  return false;
}

void ControlQueue::require_fresh_status() {
  for (auto &device : this->devices_) if (device.phase == Phase::PENDING) device.valid = false;
}

}  // namespace esphome::samsung_hvac::non_nasa
