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
#ifndef FWCORE_UNIQUEIDGENSVC_H
#define FWCORE_UNIQUEIDGENSVC_H

#include "GaudiKernel/Service.h"
#include "k4Interface/IUniqueIDGenSvc.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

/** @class UniqueIDGenSvc
 *  Generate unique, reproducible numbers using
 *  event number, run number, algorithm name, and global seed.
 *  To be used as reproducible per event seeds for random number generators used inside algorithms.
 *
 */
class UniqueIDGenSvc : public extends<Service, IUniqueIDGenSvc> {
public:
  using seed_t = uint64_t;

  UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc);
  size_t getUniqueID(const event_num_t evt_num, const run_num_t run_num, const std::string& name) const override;

private:
  Gaudi::Property<seed_t> m_seed{this, "Seed", {123456789}};
  mutable std::unordered_map<size_t, std::tuple<event_num_t, run_num_t, std::string>, std::identity> m_uniqueIDs;
  mutable std::mutex                                                                                 m_mutex;
  Gaudi::Property<bool>                                                                              m_checkDuplicates{
      this, "CheckDuplicates", m_isDebugBuild,
      "Caches obtained ID and throws an exception if a duplicate would be returned"};
#ifdef NDEBUG
  constexpr static bool m_isDebugBuild = false;
#else
  constexpr static bool m_isDebugBuild = true;
#endif
};

#endif
