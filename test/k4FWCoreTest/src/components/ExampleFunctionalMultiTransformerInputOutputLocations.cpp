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
#include "k4FWCore/Transformer.h"

#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

struct ExampleFunctionalMultiTransformerInputOutputLocations final
    : k4FWCore::MultiTransformer<std::tuple<std::vector<edm4hep::MCParticleCollection>, edm4hep::MCParticleCollection,
                                            edm4hep::MCParticleCollection>(
          const std::vector<const edm4hep::MCParticleCollection*>&, const edm4hep::MCParticleCollection&,
          const edm4hep::MCParticleCollection&)> {
  // The pairs in KeyValue/KeyValues can be changed from python and they correspond
  // to the names of the input and output collections
  ExampleFunctionalMultiTransformerInputOutputLocations(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(
            name, svcLoc,
            {
                KeyValues("InputSeveralCollections", {"MCParticles0", "MCParticles1", "MCParticles2"}),
                KeyValue("InputCollection", "MCParticles3"),
                KeyValues("SingleInputWithKeyValues", {"MCParticles4"}),
            },
            {
                KeyValues("OutputSeveralCollections", {"OutMCParticles0", "OutMCParticles1", "OutMCParticles2"}),
                KeyValue("OutputCollection", "OutMCParticles3"),
                KeyValues("SingleOutputWithKeyValues", {"OutMCParticles4"}),
            }) {}

  StatusCode initialize() override {
    // The input and output locations can be changed from python
    const std::vector<std::string> inputsMultiple = {"MCParticles0", "MCParticles1", "MCParticles2"};
    const std::vector<std::string> inputsSingle = {"MCParticles3"};
    const std::vector<std::string> inputsWithKeyValues = {"MCParticles4"};
    const std::vector<std::string> outputsMultiple = {"OutMCParticles0", "OutMCParticles1", "OutMCParticles2"};
    const std::vector<std::string> outputsSingle = {"OutMCParticles3"};
    const std::vector<std::string> outputsWithKeyValues = {"OutMCParticles4"};
    if (!std::ranges::equal(inputLocations(0), inputsMultiple)) {
      throw std::runtime_error(
          "Wrong default value for the input collection, expected {MCParticles0, MCParticles1, MCParticles2}, got {" +
          inputLocations(0)[0] + ", " + inputLocations(0)[1] + ", " + inputLocations(0)[2] + "}");
    }
    if (!std::ranges::equal(inputLocations("InputSeveralCollections"), inputsMultiple)) {
      throw std::runtime_error(
          "Wrong default value for the input collection, expected {MCParticles0, MCParticles1, MCParticles2}, got {" +
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
    if (!std::ranges::equal(outputLocations(0), outputsMultiple)) {
      throw std::runtime_error("Wrong default value for the output collection, expected {OutMCParticles0, "
                               "OutMCParticles1, OutMCParticles2}, got {" +
                               outputLocations(0)[0] + ", " + outputLocations(0)[1] + ", " + outputLocations(0)[2] +
                               "}");
    }
    if (!std::ranges::equal(outputLocations("OutputSeveralCollections"), outputsMultiple)) {
      throw std::runtime_error("Wrong default value for the output collection, expected {OutMCParticles0, "
                               "OutMCParticles1, OutMCParticles2}, got {" +
                               outputLocations(0)[0] + ", " + outputLocations(0)[1] + ", " + outputLocations(0)[2] +
                               "}");
    }
    if (!std::ranges::equal(outputLocations(1), outputsSingle)) {
      throw std::runtime_error("Wrong default value for the output collection, expected {OutMCParticles3}, got {" +
                               outputLocations(1)[0] + "}");
    }
    if (!std::ranges::equal(outputLocations("OutputCollection"), outputsSingle)) {
      throw std::runtime_error("Wrong default value for the output collection, expected {OutMCParticles3}, got {" +
                               outputLocations(1)[0] + "}");
    }
    if (!std::ranges::equal(outputLocations(2), outputsWithKeyValues)) {
      throw std::runtime_error("Wrong default value for the output collection, expected {OutMCParticles4}, got {" +
                               outputLocations(2)[0] + "}");
    }
    if (!std::ranges::equal(outputLocations("SingleOutputWithKeyValues"), outputsWithKeyValues)) {
      throw std::runtime_error("Wrong default value for the output collection, expected {OutMCParticles4}, got {" +
                               outputLocations(2)[0] + "}");
    }
    return StatusCode::SUCCESS;
  }

  // This is the function that will be called to transform the data
  std::tuple<std::vector<edm4hep::MCParticleCollection>, edm4hep::MCParticleCollection, edm4hep::MCParticleCollection>
  operator()(const std::vector<const edm4hep::MCParticleCollection*>&, const edm4hep::MCParticleCollection&,
             const edm4hep::MCParticleCollection&) const override {
    return std::make_tuple(std::vector<edm4hep::MCParticleCollection>(3), edm4hep::MCParticleCollection{},
                           edm4hep::MCParticleCollection{});
  }
};

DECLARE_COMPONENT(ExampleFunctionalMultiTransformerInputOutputLocations)
