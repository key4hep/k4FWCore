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
#include "Gaudi/Functional/Transformer.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MutableMCParticle.h"

// Define BaseClass_t
#include "k4FWCore/BaseClass.h"

#include <string>

// Which type of collection we are reading and writing
using colltype_in  = edm4hep::MCParticleCollection;
using colltype_out = edm4hep::MCParticleCollection;

struct ExampleFunctionalTransformer final
    : Gaudi::Functional::Transformer<colltype_out(const colltype_in&), BaseClass_t> {
  ExampleFunctionalTransformer(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, KeyValue("InputCollection", "MCParticles"),
                    KeyValue("OutputCollection", "NewMCParticles")) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as all pointers to collections
  // we get from the input
  colltype_out operator()(const colltype_in& input) const override {
    auto coll_out = edm4hep::MCParticleCollection();
    for (const auto& particle : input) {
      auto new_particle = edm4hep::MutableMCParticle();
      new_particle.setPDG(particle.getPDG() + 10);
      new_particle.setGeneratorStatus(particle.getGeneratorStatus() + 10);
      new_particle.setSimulatorStatus(particle.getSimulatorStatus() + 10);
      new_particle.setCharge(particle.getCharge() + 10);
      new_particle.setTime(particle.getTime() + 10);
      new_particle.setMass(particle.getMass() + 10);
      coll_out->push_back(new_particle);
    }
    return coll_out;
  }
};

DECLARE_COMPONENT(ExampleFunctionalTransformer)
