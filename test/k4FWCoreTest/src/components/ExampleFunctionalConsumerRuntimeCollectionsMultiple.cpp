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
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"

#include "k4FWCore/Consumer.h"

#include <string>

struct ExampleFunctionalConsumerRuntimeCollectionsMultiple final
    : k4FWCore::Consumer<void(const std::map<std::string, const edm4hep::MCParticleCollection&>& particleMap,
                              const std::map<std::string, const edm4hep::TrackCollection&>&      trackMap,
                              const edm4hep::SimTrackerHitCollection&                            simTrackerHits)> {
  // The pair in KeyValues can be changed from python and it corresponds
  // to the name of the input collections
  ExampleFunctionalConsumerRuntimeCollectionsMultiple(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc,
                 {KeyValues("Particles", {"MCParticles"}), KeyValues("Tracks", {"MCParticles"}),
                  KeyValues("SimTrackerHits", {"MCParticles"})}) {}

  // This is the function that will be called to produce the data
  void operator()(const std::map<std::string, const edm4hep::MCParticleCollection&>& particleMap,
                  const std::map<std::string, const edm4hep::TrackCollection&>&      trackMap,
                  const edm4hep::SimTrackerHitCollection&                            simTrackerHits) const override {
    info() << "Received " << particleMap.size() << " particle collections and " << trackMap.size()
           << " track collections" << endmsg;
    if (particleMap.size() != 5) {
      throw std::runtime_error("Wrong size of the particleMap map, expected 5, got " +
                               std::to_string(particleMap.size()));
    }
    for (auto& [key, particles] : particleMap) {
      int i = 0;
      for (const auto& particle : particles) {
        if ((particle.getPDG() != 1 + i + m_offset) || (particle.getGeneratorStatus() != 2 + i + m_offset) ||
            (particle.getSimulatorStatus() != 3 + i + m_offset) || (particle.getCharge() != 4 + i + m_offset) ||
            (particle.getTime() != 5 + i + m_offset) || (particle.getMass() != 6 + i + m_offset)) {
          std::stringstream error;
          error << "Wrong data in MCParticle collection, expected " << 1 + i + m_offset << ", " << 2 + i + m_offset
                << ", " << 3 + i + m_offset << ", " << 4 + i + m_offset << ", " << 5 + i + m_offset << ", "
                << 6 + i + m_offset << " got " << particle.getPDG() << ", " << particle.getGeneratorStatus() << ", "
                << particle.getSimulatorStatus() << ", " << particle.getCharge() << ", " << particle.getTime() << ", "
                << particle.getMass();
          throw std::runtime_error(error.str());
        }
        i++;
      }
    }
    if (trackMap.size() != 3) {
      fatal() << "Wrong size of the tracks map, expected 3, got " << trackMap.size() << endmsg;
    }
    for (auto& [key, tracks] : trackMap) {
      if ((tracks[0].getType() != 1) || (std::abs(tracks[0].getChi2() - 2.1) > 1e-6) || (tracks[0].getNdf() != 3)) {
        std::stringstream error;
        error << "Wrong data in tracks collection, expected 1, 2.1, 3, 4.1, 5.1, 6.1 got " << tracks[0].getType()
              << ", " << tracks[0].getChi2() << ", " << tracks[0].getNdf();
        throw std::runtime_error(error.str());
      }
    }
  }

private:
  // We can define any property we want that can be set from python
  // and use it inside operator()
  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalConsumerRuntimeCollectionsMultiple)
