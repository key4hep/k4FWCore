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
#include "GaudiAlg/Transformer.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MutableMCParticle.h"

#include "k4FWCore/Transformer.h"

#include <string>

struct ExampleFunctionalTransformer final
    : k4FWCore::Transformer<  edm4hep::MCParticleCollection(const edm4hep::MCParticleCollection&)> {
  ExampleFunctionalTransformer(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, KeyValue("InputCollection", "MCParticles"),
                    KeyValue("OutputCollection", "NewMCParticles")) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as all pointers to collections
  // we get from the input
  edm4hep::MCParticleCollection operator()(const edm4hep::MCParticleCollection& input) const override {
    info() << "Transforming " << input.size() << " particles" << endmsg;
    auto coll_out = edm4hep::MCParticleCollection();
    for (const auto& particle : input) {
      auto new_particle = edm4hep::MutableMCParticle();
      new_particle.setPDG(particle.getPDG() + m_offset);
      new_particle.setGeneratorStatus(particle.getGeneratorStatus() + m_offset);
      new_particle.setSimulatorStatus(particle.getSimulatorStatus() + m_offset);
      new_particle.setCharge(particle.getCharge() + m_offset);
      new_particle.setTime(particle.getTime() + m_offset);
      new_particle.setMass(particle.getMass() + m_offset);
      coll_out->push_back(new_particle);
    }
    return coll_out;
  }

private:
  // integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_offset{this, "Offset", 10,
      "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalTransformer)
