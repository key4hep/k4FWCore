#include "k4FWCore/UniqueIDGenSvc.h"

DECLARE_COMPONENT( UniqueIDGenSvc )

UniqueIDGenSvc::UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc) : 
  base_class(name, svcLoc) {}

StatusCode UniqueIDGenSvc::initialize() {
  StatusCode sc = Service::initialize();
  return sc;
}

uint64_t UniqueIDGenSvc::getUniqueID(
  uint32_t evt_num) const 
{
  std::bitset<std::numeric_limits<uint32_t>::digits> event_num_bits(evt_num);

  return std::hash<std::bitset<32>>{}(event_num_bits);
}

uint64_t UniqueIDGenSvc::getUniqueID(
  uint32_t evt_num, uint32_t run_num) const 
{
  std::bitset<std::numeric_limits<uint32_t>::digits> event_num_bits(evt_num);
  std::bitset<std::numeric_limits<uint32_t>::digits> run_num_bits(run_num);

  std::bitset<64> combined_bits(event_num_bits.to_string() + run_num_bits.to_string());

  return std::hash<std::bitset<64>>{}(combined_bits);
}

uint64_t UniqueIDGenSvc::getUniqueID(
  uint32_t evt_num, uint32_t run_num, std::string name) const 
{
  std::bitset<std::numeric_limits<uint32_t>::digits> event_num_bits(evt_num);
  std::bitset<std::numeric_limits<uint32_t>::digits> run_num_bits(run_num);
  size_t str_hash = std::hash<std::string>{}(name);
  std::bitset<std::numeric_limits<size_t>::digits> name_bits(str_hash);

  std::bitset<128> combined_bits(event_num_bits.to_string() + run_num_bits.to_string() + name_bits.to_string());

  return std::hash<std::bitset<128>>{}(combined_bits);
}
