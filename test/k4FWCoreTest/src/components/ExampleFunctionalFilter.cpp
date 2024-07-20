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

#include "Gaudi/Accumulators.h"
#include "edm4hep/MCParticleCollection.h"

#include "k4FWCore/FilterPredicate.h"

#include <string>

struct ExampleFunctionalFilter final : k4FWCore::FilterPredicate<bool(const edm4hep::MCParticleCollection& input)> {
  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the input collection
  ExampleFunctionalFilter(const std::string& name, ISvcLocator* svcLoc)
      : FilterPredicate(name, svcLoc, KeyValue("InputCollection", {"MCParticles"})) {}

  // This is the function that will be called to check if the event passes
  // Note that the function has to be const, as well as the collections
  // we get from the input
  bool operator()(const edm4hep::MCParticleCollection& input) const override {
    ++m_counter;
    // Only pass for half of the events
    bool condition = m_counter.sum() % 2;
    if (condition) {
      info() << "Event passed" << endmsg;
    } else {
      info() << "Event did not pass" << endmsg;
    }
    return condition;
  }

  // Thread-safe counter
  mutable Gaudi::Accumulators::Counter<> m_counter{this, "Counter"};
};

DECLARE_COMPONENT(ExampleFunctionalFilter)
