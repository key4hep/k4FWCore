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
#include "GaudiAlg/Consumer.h"

#include "edm4hep/MCParticleCollection.h"

// Define BaseClass_t
#include "k4FWCore/BaseClass.h"

#include <stdexcept>
#include <string>
#include <thread>

#include "k4FWCore/Consumer.h"


struct ExampleFunctionalConsumer final : Gaudi::Functional::CConsumer<void(const edm4hep::MCParticleCollection& input)> {
// struct ExampleFunctionalConsumer final : Gaudi::Functional::Consumer<void(const edm4hep::MCParticleCollection& input), BaseClass_t> {
  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the input collection
  ExampleFunctionalConsumer(const std::string& name, ISvcLocator* svcLoc)
      : CConsumer(name, svcLoc, KeyValue("InputCollection", "MCParticles")) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  void operator()(const edm4hep::MCParticleCollection& input) const override {
  // void operator()(const edm4hep::MCParticleCollection& input) const override {
  // void operator()(const edm4hep::MCParticleCollection& input) const override {
    info() << "ExampleFunctionalConsumer::operator()" << endmsg;
    info() << "Size of collection " << input.size() << endmsg;
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    int i = 0;
    for (const auto& particle : input) {
      info() << particle.getPDG() << endmsg;
      if ((particle.getPDG() != 1 + i + m_possibleOffset) ||
          (particle.getGeneratorStatus() != 2 + i + m_possibleOffset) ||
          (particle.getSimulatorStatus() != 3 + i + m_possibleOffset) ||
          (particle.getCharge() != 4 + i + m_possibleOffset) || (particle.getTime() != 5 + i + m_possibleOffset) ||
          (particle.getMass() != 6 + i + m_possibleOffset)) {
        std::stringstream error;
        error << "Wrong data in MCParticle collection, expected " << 1 + i + m_possibleOffset << ", "
              << 2 + i + m_possibleOffset << ", " << 3 + i + m_possibleOffset << ", " << 4 + i + m_possibleOffset
              << ", " << 5 + i + m_possibleOffset << ", " << 6 + i + m_possibleOffset << " got " << particle.getPDG()
              << ", " << particle.getGeneratorStatus() << ", " << particle.getSimulatorStatus() << ", "
              << particle.getCharge() << ", " << particle.getTime() << ", " << particle.getMass() << "";
        // throw std::runtime_error(error.str());
      }
      i++;
    }
  }

  Gaudi::Property<int> m_possibleOffset{this, "PossibleOffset", 0, "Possible offset in the values data"};
};


// struct ExampleFunctionalConsumer final : Gaudi::Functional::Consumer<void(const std::shared_ptr<podio::CollectionBase>& input)> {
// // struct ExampleFunctionalConsumer final : Gaudi::Functional::Consumer<void(const edm4hep::MCParticleCollection& input), BaseClass_t> {
//   // The pair in KeyValue can be changed from python and it corresponds
//   // to the name of the input collection
//   ExampleFunctionalConsumer(const std::string& name, ISvcLocator* svcLoc)
//       : Consumer(name, svcLoc, KeyValue("InputCollection", "MCParticles")) {}

//   // This is the function that will be called to transform the data
//   // Note that the function has to be const, as well as the collections
//   // we get from the input
//   void operator()(const std::shared_ptr<podio::CollectionBase>& input) const override {
//   // void operator()(const edm4hep::MCParticleCollection& input) const override {
//     info() << "ExampleFunctionalConsumer::operator()" << endmsg;
//     info() << "Size of collection " << input->size() << endmsg;
//     // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
//     int i = 0;
//     for (const auto& particle : static_cast<const edm4hep::MCParticleCollection&>(*input)) {
//       if ((particle.getPDG() != 1 + i + m_possibleOffset) ||
//           (particle.getGeneratorStatus() != 2 + i + m_possibleOffset) ||
//           (particle.getSimulatorStatus() != 3 + i + m_possibleOffset) ||
//           (particle.getCharge() != 4 + i + m_possibleOffset) || (particle.getTime() != 5 + i + m_possibleOffset) ||
//           (particle.getMass() != 6 + i + m_possibleOffset)) {
//         std::stringstream error;
//         error << "Wrong data in MCParticle collection, expected " << 1 + i + m_possibleOffset << ", "
//               << 2 + i + m_possibleOffset << ", " << 3 + i + m_possibleOffset << ", " << 4 + i + m_possibleOffset
//               << ", " << 5 + i + m_possibleOffset << ", " << 6 + i + m_possibleOffset << " got " << particle.getPDG()
//               << ", " << particle.getGeneratorStatus() << ", " << particle.getSimulatorStatus() << ", "
//               << particle.getCharge() << ", " << particle.getTime() << ", " << particle.getMass() << "";
//         // throw std::runtime_error(error.str());
//       }
//       i++;
//     }
//   }

//   Gaudi::Property<int> m_possibleOffset{this, "PossibleOffset", 0, "Possible offset in the values data"};
// };

DECLARE_COMPONENT(ExampleFunctionalConsumer)
