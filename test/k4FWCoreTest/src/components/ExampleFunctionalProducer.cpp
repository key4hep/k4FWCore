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

#include "k4FWCore/Producer.h"

#include <string>

struct ExampleFunctionalProducer final : k4FWCore::Producer<edm4hep::MCParticleCollection()> {
  // The pair in KeyValues can be changed from python and it corresponds
  // to the name of the output collection
  ExampleFunctionalProducer(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {}, KeyValues("OutputCollection", {"MCParticles"})) {}

  // This is the function that will be called to produce the data
  edm4hep::MCParticleCollection operator()() const override {
    auto coll = edm4hep::MCParticleCollection();
    // This will create a particle and set PDG, generatorStatus, simulatorStatus
    // ... (the fields of a MCParticle) to 1, 2, 3...
    coll.create(1, 2, 3, 4.f, 5.f, 6.f);
    // We can also set the fields of the particle manually
    auto particle = coll.create(2, 3, 4, 5.f, 6.f, 7.f);
    particle.setPDG(2);
    particle.setGeneratorStatus(3);
    particle.setSimulatorStatus(4);
    particle.setCharge(5.f);
    particle.setTime(6.f);
    particle.setMass(7.f);
    // We have to return whatever collection type we specified in the
    // template argument
    return coll;
  }

private:
  // We can define any property we want that can be set from python
  // and use it inside operator()
  Gaudi::Property<int> m_exampleInt{this, "ExampleInt", 3, "Example int that can be used in the algorithm"};
};

DECLARE_COMPONENT(ExampleFunctionalProducer)
