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

#include <string>

struct ExampleFunctionalConsumerRuntimeCollections final
    : k4FWCore::Consumer<void(const std::map<std::string, edm4hep::MCParticleCollection&>& input)> {
  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the output collection
  ExampleFunctionalConsumerRuntimeCollections(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValues("InputCollection", {"DefaultValue"})) {}

  // This is the function that will be called to produce the data
  void operator()(const std::map<std::string, edm4hep::MCParticleCollection&>& input) const override {
    if (input.size() != 3) {
      fatal() << "Wrong size of the input map, expected 3, got " << input.size() << endmsg;
    }
    for (auto& [key, val] : input) {
      int i = 0;
      for (const auto& particle : val) {
        if ((particle.getPDG() != 1 + i + m_offset) || (particle.getGeneratorStatus() != 2 + i + m_offset) ||
            (particle.getSimulatorStatus() != 3 + i + m_offset) || (particle.getCharge() != 4 + i + m_offset) ||
            (particle.getTime() != 5 + i + m_offset) || (particle.getMass() != 6 + i + m_offset)) {
          std::stringstream error;
          error << "Wrong data in MCParticle collection, expected " << 1 + i + m_offset << ", " << 2 + i + m_offset
                << ", " << 3 + i + m_offset << ", " << 4 + i + m_offset << ", " << 5 + i + m_offset << ", "
                << 6 + i + m_offset << " got " << particle.getPDG() << ", " << particle.getGeneratorStatus() << ", "
                << particle.getSimulatorStatus() << ", " << particle.getCharge() << ", " << particle.getTime() << ", "
                << particle.getMass() << "";
          throw std::runtime_error(error.str());
        }
        i++;
      }
    }
  }

private:
  // We can define any property we want that can be set from python
  // and use it inside operator()
  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalConsumerRuntimeCollections)
