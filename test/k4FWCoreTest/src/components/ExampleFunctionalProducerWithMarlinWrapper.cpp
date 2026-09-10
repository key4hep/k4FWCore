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

#include "edm4hep/MCParticleCollection.h"
#include "podio/CollectionBase.h"

#include "k4FWCore/Producer.h"

#include <string>

// Produce a collection through a CollectionBase pointer, as the Marlin wrapper
// does when it registers a collection in the event store.
struct ExampleFunctionalProducerWithMarlinWrapper final : k4FWCore::Producer<podio::CollectionBase*()> {
  ExampleFunctionalProducerWithMarlinWrapper(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {}, KeyValue("OutputCollection", "MCParticles")) {}

  podio::CollectionBase* operator()() const override {
    auto coll = new edm4hep::MCParticleCollection();
    coll->create(1, 2, 3, 4.f, 5.f, 6.);
    auto particle = coll->create(2, 3, 4, 5.f, 6.f, 7.);
    particle.setPDG(2);
    particle.setGeneratorStatus(3);
    particle.setSimulatorStatus(4);
    particle.setCharge(5.f);
    particle.setTime(6.f);
    particle.setMass(7.);
    return coll;
  }
};

DECLARE_COMPONENT(ExampleFunctionalProducerWithMarlinWrapper)
