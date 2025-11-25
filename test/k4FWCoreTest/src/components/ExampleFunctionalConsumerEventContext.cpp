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

#include "Gaudi/Property.h"

#include "edm4hep/MCParticleCollection.h"

#include "k4FWCore/Consumer.h"

#include <limits>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>

struct ExampleFunctionalConsumerEventContext final
    : k4FWCore::Consumer<void(const EventContext&, const edm4hep::MCParticleCollection& input)> {
  // The pair in KeyValues can be changed from python and it corresponds
  // to the name of the input collection
  ExampleFunctionalConsumerEventContext(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValue("InputCollection", "MCParticles")) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  void operator()(const EventContext& ctx, const edm4hep::MCParticleCollection& input) const override {
    debug() << "Received MCParticle collection with " << input.size() << " elements" << endmsg;
    info() << "Event number is " << ctx.evt() << endmsg;
    if (input.size() != 2) {
      fatal() << "Wrong size of MCParticle collection, expected 2 got " << input.size() << endmsg;
      throw std::runtime_error("Wrong size of MCParticle collection");
    }
    int i = 0;
    for (const auto& particle : input) {
      if ((particle.getPDG() != 1 + i + m_offset) || (particle.getGeneratorStatus() != 2 + i + m_offset) ||
          (particle.getSimulatorStatus() != 3 + i + m_offset) || (particle.getCharge() != 4 + i + m_offset) ||
          (particle.getTime() != 5 + i + m_offset) || (particle.getMass() != 6 + i + m_offset)) {
        std::stringstream error;
        error << "Wrong data in MCParticle collection, expected " << 1 + i + m_offset << ", " << 2 + i + m_offset
              << ", " << 3 + i + m_offset << ", " << 4 + i + m_offset << ", " << 5 + i + m_offset << ", "
              << 6 + i + m_offset << " got " << particle.getPDG() << ", " << particle.getGeneratorStatus() << ", "
              << particle.getSimulatorStatus() << ", " << particle.getCharge() << ", " << particle.getTime() << ", "
              << particle.getMass();
        throw std::runtime_error(error.str());
      }
      i++;
    }
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_eventNumbersSeen.find(ctx.evt()) != m_eventNumbersSeen.end()) {
        throw std::runtime_error("Event number " + std::to_string(ctx.evt()) + " seen multiple times");
      }
      m_eventNumbersSeen.insert(ctx.evt());
    }
  }

  StatusCode finalize() override {
    unsigned long value = std::numeric_limits<unsigned long>::max();
    if (m_eventNumbersSeen.empty()) {
      fatal() << "No events were processed!" << endmsg;
      return StatusCode::FAILURE;
    }
    for (auto number : m_eventNumbersSeen) {
      if (value != std::numeric_limits<unsigned long>::max() && number != value + 1) {
        fatal() << "Event numbers are not consecutive, missing event between " << value << " and " << number << endmsg;
        return StatusCode::FAILURE;
      }
      info() << "Found event number " << number << endmsg;
      value = number;
    }
    return StatusCode::SUCCESS;
  }

  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
  mutable std::set<unsigned long> m_eventNumbersSeen{};
  mutable std::mutex m_mutex{};
};

DECLARE_COMPONENT(ExampleFunctionalConsumerEventContext)
