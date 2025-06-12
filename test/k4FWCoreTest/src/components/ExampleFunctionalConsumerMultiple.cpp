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
#include "edm4hep/RecoMCParticleLinkCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHit3DCollection.h"
#include "podio/UserDataCollection.h"

#include "k4FWCore/Consumer.h"

#include <sstream>
#include <stdexcept>
#include <string>

// Which type of collection we are reading
using FloatColl = podio::UserDataCollection<float>;
using ParticleColl = edm4hep::MCParticleCollection;
using SimTrackerHitColl = edm4hep::SimTrackerHitCollection;
using TrackerHitColl = edm4hep::TrackerHit3DCollection;
using TrackColl = edm4hep::TrackCollection;
using RecoColl = edm4hep::ReconstructedParticleCollection;
using LinkColl = edm4hep::RecoMCParticleLinkCollection;

struct ExampleFunctionalConsumerMultiple final
    : k4FWCore::Consumer<void(const FloatColl&, const ParticleColl&, const SimTrackerHitColl&, const TrackerHitColl&,
                              const TrackColl&, const RecoColl&, const LinkColl&)> {
  // The pairs in KeyValue can be changed from python and they correspond
  // to the names of the input collections
  ExampleFunctionalConsumerMultiple(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc,
                 {
                     KeyValues("InputCollectionFloat", {"VectorFloat"}),
                     KeyValues("InputCollectionParticles", {"MCParticles1"}),
                     KeyValues("InputCollectionSimTrackerHits", {"SimTrackerHits"}),
                     KeyValues("InputCollectionTrackerHits", {"TrackerHits"}),
                     KeyValues("InputCollectionTracks", {"Tracks"}),
                     KeyValues("InputCollectionRecoParticles", {"RecoParticles"}),
                     KeyValues("InputCollectionLinks", {"Links"}),
                 }) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  void operator()(const FloatColl& floatVector, const ParticleColl& particles, const SimTrackerHitColl& simTrackerHits,
                  const TrackerHitColl& trackerHits, const TrackColl& tracks, const RecoColl& recos,
                  const LinkColl& links) const override {
    if (floatVector.size() != 3) {
      throw std::runtime_error("Wrong size of floatVector collection, expected 3, got " +
                               std::to_string(floatVector.size()) + "");
    }
    if ((floatVector[0] != 125) || (floatVector[1] != 25) || (floatVector[2] != 0)) {
      std::stringstream error;
      error << "Wrong data in floatVector collection, expected 125, 25, " << 0 << " got " << floatVector[0] << ", "
            << floatVector[1] << ", " << floatVector[2];
      throw std::runtime_error(error.str());
    }

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

    if ((simTrackerHits[0].getPosition()[0] != 3) || (simTrackerHits[0].getPosition()[1] != 4) ||
        (simTrackerHits[0].getPosition()[2] != 5)) {
      std::stringstream error;
      error << "Wrong data in simTrackerHits collection, expected 3, 4, 5 got " << simTrackerHits[0].getPosition()[0]
            << ", " << simTrackerHits[0].getPosition()[1] << ", " << simTrackerHits[0].getPosition()[2];
      throw std::runtime_error(error.str());
    }

    if ((trackerHits[0].getPosition()[0] != 3) || (trackerHits[0].getPosition()[1] != 4) ||
        (trackerHits[0].getPosition()[2] != 5)) {
      std::stringstream error;
      error << "Wrong data in trackerHits collection, expected 3, 4, 5 got " << trackerHits[0].getPosition()[0] << ", "
            << trackerHits[0].getPosition()[1] << ", " << trackerHits[0].getPosition()[2];
      throw std::runtime_error(error.str());
    }

    if ((tracks[0].getType() != 1) || (std::abs(tracks[0].getChi2() - 2.1) > 1e-6) || (tracks[0].getNdf() != 3)) {
      std::stringstream error;
      error << "Wrong data in tracks collection, expected 1, 2.1, 3, 4.1, 5.1, 6.1 got " << tracks[0].getType() << ", "
            << tracks[0].getChi2() << ", " << tracks[0].getNdf();
      throw std::runtime_error(error.str());
    }

    for (size_t j = 0; j < links.size(); j++) {
      if (links[j].getFrom() != recos[j] || links[j].getTo() != particles[j]) {
        std::stringstream error;
        error << "Wrong data in links collection, link" << j << " expected " << recos[j].id() << ", "
              << particles[j].id() << " got " << links[j].getFrom().id() << ", " << links[j].getTo().id();
        throw std::runtime_error(error.str());
      }
    }
  }

private:
  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalConsumerMultiple)
