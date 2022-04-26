#include "UniqueIDGenSvc.h"

DECLARE_COMPONENT( UniqueIDGenSvc )

UniqueIDGenSvc::UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc) : 
  base_class(name, svcLoc) {}

StatusCode UniqueIDGenSvc::initialize() {
  StatusCode sc = Service::initialize();
  return sc;
}

const size_t bits32 = std::numeric_limits<uint32_t>::digits;
const size_t bits64 = std::numeric_limits<uint64_t>::digits;
const size_t bitsSizeT = std::numeric_limits<size_t>::digits;

size_t UniqueIDGenSvc::getUniqueID(
  int64_t seed, uint32_t evt_num, uint32_t run_num, std::string name) const
{
  std::bitset<bits64> seed_bits(seed);
  std::bitset<bits32> event_num_bits(evt_num), run_num_bits(run_num);
  size_t str_hash = std::hash<std::string>{}(name);
  std::bitset<bitsSizeT> name_bits(str_hash);

  std::bitset<bits64+bits32+bits32+bitsSizeT> combined_bits(
    seed_bits.to_string() +
    event_num_bits.to_string() +
    run_num_bits.to_string() +
    name_bits.to_string());

  return std::hash<std::bitset<bits64+bits32+bits32+bitsSizeT>>{}(combined_bits);
}
