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
#include "GaudiAlg/Producer.h"

#include "edm4hep/MCParticleCollection.h"

// #include "k4FWCore/ProducerMany.h"
#include "k4FWCore/TransformerMany.h"

#include <string>

struct ExampleFunctionalProducerRuntimeCollections final : k4FWCore::ProducerMany<std::map<std::string, std::shared_ptr<podio::CollectionBase>>()> {
  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the output collection
  ExampleFunctionalProducerRuntimeCollections(const std::string& name, ISvcLocator* svcLoc)
      : ProducerMany(name, svcLoc, KeyValue("OutputCollection", "MCParticles")) {}

  // This is the function that will be called to produce the data
  std::map<std::string, std::shared_ptr<podio::CollectionBase>> operator()() const override {
    std::map<std::string, std::shared_ptr<podio::CollectionBase>> m_outputCollections;
    for (int i = 0; i < m_numberOfCollections; ++i) {
      std::string name = "MCParticles" + std::to_string(i);
      auto coll = std::make_shared<edm4hep::MCParticleCollection>();
      coll->create(1, 2, 3, 4.f, 5.f, 6.f);
      coll->create(2, 3, 4, 5.f, 6.f, 7.f);
      m_outputCollections[name] = std::dynamic_pointer_cast<podio::CollectionBase>(coll);
    }
    return m_outputCollections;

  }

private:
  // We can define any property we want that can be set from python
  // and use it inside operator()
  Gaudi::Property<int> m_numberOfCollections{this, "NumberOfCollections", 1, "Example int that can be used in the algorithm"};
};

DECLARE_COMPONENT(ExampleFunctionalProducerRuntimeCollections)