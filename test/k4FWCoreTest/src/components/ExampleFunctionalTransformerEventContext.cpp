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
#include "edm4hep/MutableMCParticle.h"

#include "k4FWCore/Transformer.h"

#include <string>

struct ExampleFunctionalTransformerEventContext final
    : k4FWCore::Transformer<edm4hep::MCParticleCollection(const EventContext&, const edm4hep::MCParticleCollection&)> {
  ExampleFunctionalTransformerEventContext(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValue("InputCollection", "MCParticles")},
                    {KeyValue("OutputCollection", "NewMCParticles")}) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as all pointers to collections
  // we get from the input
  edm4hep::MCParticleCollection operator()(const EventContext& ctx,
                                           const edm4hep::MCParticleCollection& input) const override {
    info() << "Transforming " << input.size() << " particles" << endmsg;
    info() << "Event number is " << ctx.evt() << endmsg;
    auto coll_out = edm4hep::MCParticleCollection();
    for (const auto& particle : input) {
      auto new_particle = coll_out.create();
      new_particle.setPDG(particle.getPDG() + m_offset);
      new_particle.setGeneratorStatus(particle.getGeneratorStatus() + m_offset);
      new_particle.setSimulatorStatus(particle.getSimulatorStatus() + m_offset);
      new_particle.setCharge(particle.getCharge() + m_offset);
      new_particle.setTime(particle.getTime() + m_offset);
      new_particle.setMass(particle.getMass() + m_offset);
    }

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_eventNumbersSeen.find(ctx.evt()) != m_eventNumbersSeen.end()) {
        throw std::runtime_error("Event number " + std::to_string(ctx.evt()) + " seen multiple times");
      }
      m_eventNumbersSeen.insert(ctx.evt());
    }

    return coll_out;
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

private:
  // integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
  mutable std::set<unsigned long> m_eventNumbersSeen{};
  mutable std::mutex m_mutex{};
};

DECLARE_COMPONENT(ExampleFunctionalTransformerEventContext)
