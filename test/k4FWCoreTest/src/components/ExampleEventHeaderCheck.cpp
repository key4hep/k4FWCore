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

#include "edm4hep/Constants.h"
#include "edm4hep/EventHeaderCollection.h"

#include "k4FWCore/Consumer.h"

#include <Gaudi/Property.h>

#include <fmt/core.h>

#include <iterator>
#include <mutex>
#include <stdexcept>
#include <string>

struct ExampleEventHeaderCheck final : k4FWCore::Consumer<void(const edm4hep::EventHeaderCollection&)> {
  ExampleEventHeaderCheck(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, {KeyValues("EventHeaderName", {edm4hep::labels::EventHeader})}) {}

  void operator()(const edm4hep::EventHeaderCollection& evtHeaderColl) const override {
    if (evtHeaderColl.empty()) {
      throw std::runtime_error("EventHeader collection is empty");
    }
    const auto evtHeader = evtHeaderColl[0];
    if (!evtHeader.isAvailable()) {
      throw std::runtime_error("Cannot get a valid EventHeader");
    }

    if (evtHeader.getRunNumber() != m_runNumber) {
      throw std::runtime_error(fmt::format("Run number is not set correctly (expected {}, actual {})",
                                           m_runNumber.value(), evtHeader.getRunNumber()));
    }
    if (std::find(std::begin(m_expectedEventNumbers), std::end(m_expectedEventNumbers), evtHeader.getEventNumber()) ==
        std::end(m_expectedEventNumbers)) {
      throw std::runtime_error(fmt::format("Event number {} is not in expected numbers", evtHeader.getEventNumber()));
    }
    auto lock = std::scoped_lock(m_mutex);
    if (m_seenEventNumbers.contains(evtHeader.getEventNumber())) {
      throw std::runtime_error(fmt::format("Event number {} is duplicated", evtHeader.getEventNumber()));
    }
    m_seenEventNumbers.insert(evtHeader.getEventNumber());
  }

  Gaudi::Property<int>              m_runNumber{this, "runNumber", 0, "The expected run number"};
  Gaudi::Property<std::vector<int>> m_expectedEventNumbers{this, "expectedEventNumbers", 0, "Expected event numbers"};
  mutable std::mutex                m_mutex;
  mutable std::set<int>             m_seenEventNumbers;
};

DECLARE_COMPONENT(ExampleEventHeaderCheck)
