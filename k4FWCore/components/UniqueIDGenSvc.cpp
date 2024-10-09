/*
 * Copyright (c) 2014-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "UniqueIDGenSvc.h"

#include <cstdint>
#include <stdexcept>
#include <string>

UniqueIDGenSvc::UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc) : base_class(name, svcLoc) {}

constexpr size_t bits32    = std::numeric_limits<uint32_t>::digits;
constexpr size_t bits64    = std::numeric_limits<uint64_t>::digits;
constexpr size_t bitsSizeT = std::numeric_limits<size_t>::digits;

size_t UniqueIDGenSvc::getUniqueID(uint32_t evt_num, uint32_t run_num, const std::string& name) const {
  std::bitset<bits64>    seed_bits      = this->m_seed.value();
  std::bitset<bits32>    event_num_bits = evt_num, run_num_bits = run_num;
  size_t                 str_hash  = std::hash<std::string>{}(name);
  std::bitset<bitsSizeT> name_bits = str_hash;

  std::bitset<bits64 + bits32 + bits32 + bitsSizeT> combined_bits;

  for (size_t i = 0; i < bitsSizeT; i++) {
    combined_bits[i] = name_bits[i];
  }
  for (size_t i = 0; i < bits32; i++) {
    combined_bits[i + bitsSizeT] = run_num_bits[i];
  }
  for (size_t i = 0; i < bits32; i++) {
    combined_bits[i + bits32 + bitsSizeT] = event_num_bits[i];
  }
  for (size_t i = 0; i < bits64; i++) {
    combined_bits[i + bits32 + bits32 + bitsSizeT] = seed_bits[i];
  }

  auto hash     = std::hash<std::bitset<bits64 + bits32 + bits32 + bitsSizeT>>{}(combined_bits);
  bool inserted = false;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::tie(std::ignore, inserted) = m_uniqueIDs.insert(hash);
  }
  if (!inserted) {
    error() << "Event number " << evt_num << ", run number " << run_num << " and algorithm name \"" << name
            << "\" have already been used. Please check the uniqueness of the event number, run number and name."
            << endmsg;
    if (m_throwIfDuplicate) {
      throw std::runtime_error("Duplicate event number, run number and algorithm name");
    }
  }

  return hash;
}

DECLARE_COMPONENT(UniqueIDGenSvc)
