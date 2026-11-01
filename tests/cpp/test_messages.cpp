#include "typed_message.h"
#include "write_policy.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <fstream>
#include <string>
using namespace esphome::samsung_hvac;
using namespace esphome::samsung_hvac::advanced;
struct Sink : ValueSink {
  unsigned calls{0}; std::optional<double> value;
  void on_value(std::optional<double> observed) override { ++calls; value=observed; }
};
static_assert(!std::is_copy_constructible_v<Guard> && !std::is_move_constructible_v<Guard>);
int main(int argc, char **argv) {
  assert(argc==2); std::ifstream input(std::string(argv[1])+"/passive_fsv.hex"); assert(input);
  std::string hex; input >> hex; Frame frame; frame.size=hex.size()/2;
  assert(frame.size<=frame.data.size());
  for(size_t i=0;i<frame.size;++i) frame.data[i]=static_cast<uint8_t>(std::stoul(hex.substr(2*i,2),nullptr,16));
  nasa::Packet fixture; assert(nasa::decode(frame,fixture)==nasa::DecodeError::NONE);
  Sink fsv_sink; Observer fsv({0x200000},Descriptor{0x424e,true,.1,0},fsv_sink);
  fsv.receive(fixture); assert(fsv_sink.calls==1 && fsv_sink.value==45);

  Descriptor d{0x4201,false,0.1,0};
  assert(d.decode({0x4201,235})==23.5);
  assert(!d.decode({0x4205,235}));
  assert(!d.decode({0x4201,65536}));
  Descriptor s8{0x4002,true,1,0}, s16{0x4205,true,.1,0}, s32{0x4400,true,1,0}, u32{0x4400,false,1,0};
  assert(s8.decode({0x4002,255})==-1);
  assert(s16.decode({0x4205,0xff9c})==-10);
  assert(s32.decode({0x4400,0x80000000})==-2147483648.0);
  assert(u32.decode({0x4400,0xffffffff})==4294967295.0);
  Descriptor structure{0x4600,false,1,0}; assert(!structure.valid());
  d.sentinels[0]=65535; d.sentinel_count=1; assert(!d.decode({0x4201,65535}));
  d.raw_max=300; assert(!d.decode({0x4201,301}));
  d.scale=NAN; assert(!d.valid()); d.scale=.1;
  Sink sink; Observer observer({0x200000},d,sink);
  nasa::Packet p; p.source={0x200001}; p.destination={0x80ff00}; p.type=nasa::DataType::NOTIFICATION;
  p.message_count=1; p.messages[0]={0x4201,235}; observer.receive(p); assert(sink.calls==0);
  p.source={0x200000}; p.type=nasa::DataType::ACK; observer.receive(p); assert(sink.calls==0);
  p.type=nasa::DataType::RESPONSE; observer.receive(p); assert(sink.calls==1 && sink.value==23.5);
  p.messages[0].raw=65535; observer.receive(p); assert(sink.calls==2 && !sink.value);
  // The positive policy below is a synthetic unit-test policy, not verified hardware evidence.
  WritePolicy policy; policy.device={0x200000}; policy.local={0x80ff00};
  policy.rules[0]={Descriptor{0x4201,false,.1,0},16,30,1}; policy.rule_count=1;
  Ticket ticket;
  Guard disabled(policy); assert(disabled.prepare({0x200000},0x4201,24,0,ticket)==Admission::BLOCKED);
  policy.opt_in=true;
  Guard unverified(policy); assert(unverified.prepare({0x200000},0x4201,24,0,ticket)==Admission::BLOCKED);
  policy.verified_family=true;
  Guard gate(policy);
  assert(gate.prepare({0x200001},0x4201,24,0,ticket)==Admission::BLOCKED);
  assert(gate.prepare({0xb0ff00},0x4201,24,0,ticket)==Admission::BLOCKED);
  assert(gate.prepare({0x200000},0x4235,24,0,ticket)==Admission::BLOCKED);
  assert(gate.prepare({0x200000},0x4201,31,0,ticket)==Admission::INVALID);
  assert(gate.prepare({0x200000},0x4201,24.5,0,ticket)==Admission::INVALID);
  assert(gate.prepare({0x200000},0x4201,NAN,0,ticket)==Admission::INVALID);
  assert(gate.prepare({0x200000},0x4201,24,0,ticket)==Admission::READY);
  assert(ticket.raw==240 && ticket.code==0x4201);
  const auto first=ticket;
  assert(gate.prepare({0x200000},0x4201,25,1,ticket)==Admission::BUSY);
  p.messages[0]={0x4201,240}; p.type=nasa::DataType::NOTIFICATION; gate.observe(p,99);
  assert(gate.result()==Result::PENDING); // No readback before actual TX.
  gate.on_transmitted(first.id,true,100);
  p.messages[0]={0x4201,240}; p.type=nasa::DataType::ACK; gate.observe(p,101);
  assert(gate.result()==Result::PENDING);
  p.type=nasa::DataType::NOTIFICATION; p.source={0x200001}; gate.observe(p,102);
  assert(gate.result()==Result::PENDING);
  p.source={0x200000}; p.destination={0x800001}; gate.observe(p,103);
  assert(gate.result()==Result::PENDING);
  p.destination={0x80ff00}; gate.observe(p,104); assert(gate.result()==Result::OBSERVED_MATCH);
  assert(gate.prepare({0x200000},0x4201,25,500,ticket)==Admission::RATE_LIMITED);
  assert(gate.prepare({0x200000},0x4201,25,1100,ticket)==Admission::READY);
  gate.on_transmitted(ticket.id,true,1200);
  p.messages[0].raw=250; gate.observe(p,2200); // Deadline enforced without a preceding tick.
  assert(gate.result()==Result::TIMED_OUT);
  assert(gate.prepare({0x200000},0x4201,26,2300,ticket)==Admission::READY);
  const auto expired=ticket;
  gate.tick(7300); assert(gate.result()==Result::UNKNOWN_OUTCOME);
  gate.on_transmitted(expired.id,true,7301);
  assert(gate.prepare({0x200000},0x4201,26,9000,ticket)==Admission::BLOCKED);
  // Wrap-safe actual-TX rate/readback timing and stale token rejection.
  Guard wrap(policy); assert(wrap.prepare({0x200000},0x4201,24,0xfffffff0,ticket)==Admission::READY);
  wrap.on_transmitted(ticket.id+1,true,0xfffffff1); assert(wrap.result()==Result::PENDING);
  wrap.on_transmitted(ticket.id,true,0xfffffff2); wrap.tick(0x000003da);
  assert(wrap.result()==Result::TIMED_OUT);
  Guard failed(policy);
  assert(failed.prepare({0x200000},0x4201,24,0,ticket)==Admission::READY);
  failed.on_transmitted(ticket.id,false,1); assert(failed.result()==Result::SEND_FAILED);
  assert(failed.prepare({0x200000},0x4201,24,2000,ticket)==Admission::BLOCKED);
  WritePolicy malformed=policy; malformed.rate_ms=999;
  Guard too_fast(malformed); assert(too_fast.prepare({0x200000},0x4201,24,0,ticket)==Admission::BLOCKED);
  malformed=policy; malformed.rule_count=2; malformed.rules[1]=malformed.rules[0];
  Guard duplicate(malformed); assert(duplicate.prepare({0x200000},0x4201,24,0,ticket)==Admission::BLOCKED);
  WritePolicy signed_policy=policy;
  signed_policy.rules[0]={Descriptor{0x4248,true,.1,0},-5,5,.5};
  Guard negative(signed_policy);
  assert(negative.prepare({0x200000},0x4248,-2.5,0,ticket)==Admission::READY && ticket.raw==0xffe7);
  WritePolicy quantized=policy;
  quantized.rules[0]={Descriptor{0x4201,false,1000,0},0.0001,0.0001,1};
  Guard outside(quantized);
  assert(outside.prepare({0x200000},0x4201,0.0001,0,ticket)==Admission::INVALID);
  quantized.rules[0].minimum=0; quantized.rules[0].maximum=1000; quantized.rules[0].step=0.0001;
  Guard unequal(quantized);
  assert(unequal.prepare({0x200000},0x4201,0.0001,0,ticket)==Admission::INVALID);
  quantized.rules[0]={Descriptor{0x4201,false,.1,0},.3,.3,1};
  Guard strict_boundary(quantized);
  assert(strict_boundary.prepare({0x200000},0x4201,.3,0,ticket)==Admission::INVALID); // Decoded raw3 exceeds the strict double maximum.
  std::cout << "All typed message and policy assertions passed\n";
}
