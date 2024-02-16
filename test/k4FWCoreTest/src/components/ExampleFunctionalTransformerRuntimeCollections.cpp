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

#include "k4FWCore/Transformer.h"

#include <string>

using mapType = std::map<std::string, std::shared_ptr<edm4hep::MCParticleCollection>>;

struct ExampleFunctionalTransformerRuntimeCollections final : k4FWCore::Transformer<mapType(const mapType& input)> {
  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the output collection
  ExampleFunctionalTransformerRuntimeCollections(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("InputCollections", {"MCParticles"})},
                    {KeyValues("OutputCollections", {"MCParticles"})}) {}

  // This is the function that will be called to produce the data
  mapType operator()(const mapType& input) const override {
    std::map<std::string, std::shared_ptr<edm4hep::MCParticleCollection>> m_outputCollections;
    for (int i = 0; i < input.size(); ++i) {
      std::string name     = "NewMCParticles" + std::to_string(i);
      auto        old_coll = input.at("MCParticles" + std::to_string(i));
      auto        coll     = std::make_shared<edm4hep::MCParticleCollection>();
      coll->push_back(old_coll->at(0).clone());
      coll->push_back(old_coll->at(1).clone());
      m_outputCollections[name] = coll;
    }
    return m_outputCollections;
  }

private:
  // We can define any property we want that can be set from python
  // and use it inside operator()
  Gaudi::Property<int> m_numberOfCollections{this, "NumberOfCollections", 3,
                                             "Example int that can be used in the algorithm"};
};

DECLARE_COMPONENT(ExampleFunctionalTransformerRuntimeCollections)
