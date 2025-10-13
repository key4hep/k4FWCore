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

#include "k4FWCore/Consumer.h"

#include <stdexcept>
#include <string>
#include <vector>

struct ExampleFunctionalConsumerInputOutputLocations final
    : k4FWCore::Consumer<void(const std::vector<const edm4hep::MCParticleCollection*>& input,
                              const edm4hep::MCParticleCollection& anotherInput,
                              const edm4hep::MCParticleCollection& anotherInputWithKeyValues)> {
  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the output collection
  ExampleFunctionalConsumerInputOutputLocations(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc,
                 {
                     KeyValues("InputSeveralCollections", {"MCParticles0", "MCParticles1", "MCParticles2"}),
                     KeyValue("InputCollection", "MCParticles3"),
                     KeyValues("SingleInputWithKeyValues", {"MCParticles4"}),
                 }) {}

  StatusCode initialize() override {
    // The input locations can be changed from python
    const std::vector<std::string> inputsMultiple = {"MCParticles0", "MCParticles1", "MCParticles2"};
    const std::vector<std::string> inputsSingle = {"MCParticles3"};
    const std::vector<std::string> inputsWithKeyValues = {"MCParticles4"};
    if (!std::ranges::equal(inputLocations(0), inputsMultiple)) {
      throw std::runtime_error("Wrong default value for the input collection, expected {MCParticles0, MCParticles1, "
                               "MCParticles2}, got {" +
                               inputLocations(0)[0] + ", " + inputLocations(0)[1] + ", " + inputLocations(0)[2] + "}");
    }
    if (!std::ranges::equal(inputLocations("InputSeveralCollections"), inputsMultiple)) {
      throw std::runtime_error("Wrong default value for the input collection, expected {MCParticles0, MCParticles1, "
                               "MCParticles2}, got {" +
                               inputLocations(0)[0] + ", " + inputLocations(0)[1] + ", " + inputLocations(0)[2] + "}");
    }
    if (!std::ranges::equal(inputLocations(1), inputsSingle)) {
      throw std::runtime_error("Wrong default value for the input collection, expected {MCParticles3}, got {" +
                               inputLocations(1)[0] + "}");
    }
    if (!std::ranges::equal(inputLocations("InputCollection"), inputsSingle)) {
      throw std::runtime_error("Wrong default value for the input collection, expected {MCParticles3}, got {" +
                               inputLocations(1)[0] + "}");
    }
    if (!std::ranges::equal(inputLocations(2), inputsWithKeyValues)) {
      throw std::runtime_error("Wrong default value for the input collection, expected {MCParticles4}, got {" +
                               inputLocations(2)[0] + "}");
    }
    if (!std::ranges::equal(inputLocations("SingleInputWithKeyValues"), inputsWithKeyValues)) {
      throw std::runtime_error("Wrong default value for the input collection, expected {MCParticles4}, got {" +
                               inputLocations(2)[0] + "}");
    }
    return StatusCode::SUCCESS;
  }

  // This is the function that will be called to produce the data
  void operator()(const std::vector<const edm4hep::MCParticleCollection*>&, const edm4hep::MCParticleCollection&,
                  const edm4hep::MCParticleCollection&) const override {}
};

DECLARE_COMPONENT(ExampleFunctionalConsumerInputOutputLocations)
