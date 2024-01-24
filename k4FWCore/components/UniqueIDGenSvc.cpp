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

DECLARE_COMPONENT(UniqueIDGenSvc)

UniqueIDGenSvc::UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc) : base_class(name, svcLoc) {}

StatusCode UniqueIDGenSvc::initialize() {
  StatusCode sc = Service::initialize();
  return sc;
}

const size_t bits32    = std::numeric_limits<uint32_t>::digits;
const size_t bits64    = std::numeric_limits<uint64_t>::digits;
const size_t bitsSizeT = std::numeric_limits<size_t>::digits;

size_t UniqueIDGenSvc::getUniqueID(uint32_t evt_num, uint32_t run_num, const std::string& name) const {
  std::bitset<bits64>    seed_bits(this->m_seed);
  std::bitset<bits32>    event_num_bits(evt_num), run_num_bits(run_num);
  size_t                 str_hash = std::hash<std::string>{}(name);
  std::bitset<bitsSizeT> name_bits(str_hash);

  std::bitset<bits64 + bits32 + bits32 + bitsSizeT> combined_bits(seed_bits.to_string() + event_num_bits.to_string() +
                                                                  run_num_bits.to_string() + name_bits.to_string());

  return std::hash<std::bitset<bits64 + bits32 + bits32 + bitsSizeT>>{}(combined_bits);
}
