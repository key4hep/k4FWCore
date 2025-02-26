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

#include <cstddef>
#include <stdexcept>
#include <string>

constexpr size_t event_num_digits = std::numeric_limits<UniqueIDGenSvc::event_num_t>::digits;
constexpr size_t run_num_digits   = std::numeric_limits<UniqueIDGenSvc::run_num_t>::digits;
constexpr size_t seed_digits      = std::numeric_limits<UniqueIDGenSvc::seed_t>::digits;
constexpr size_t name_digits      = std::numeric_limits<size_t>::digits;

UniqueIDGenSvc::UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc) : base_class(name, svcLoc) {}

size_t UniqueIDGenSvc::getUniqueID(event_num_t evt_num, run_num_t run_num, const std::string& name) const {
  std::bitset<seed_digits>      seed_bits      = m_seed.value();
  std::bitset<event_num_digits> event_num_bits = evt_num;
  std::bitset<run_num_digits>   run_num_bits   = run_num;
  std::bitset<name_digits>      name_bits      = std::hash<std::string>{}(name);

  std::bitset<seed_digits + event_num_digits + run_num_digits + name_digits> combined_bits;

  for (size_t i = 0; i < name_digits; i++) {
    combined_bits[i] = name_bits[i];
  }
  for (size_t i = 0; i < run_num_digits; i++) {
    combined_bits[i + name_digits] = run_num_bits[i];
  }
  for (size_t i = 0; i < event_num_digits; i++) {
    combined_bits[i + run_num_digits + name_digits] = event_num_bits[i];
  }
  for (size_t i = 0; i < seed_digits; i++) {
    combined_bits[i + event_num_digits + run_num_digits + name_digits] = seed_bits[i];
  }

  auto hash = std::hash<std::bitset<seed_digits + event_num_digits + run_num_digits + name_digits>>{}(combined_bits);
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
