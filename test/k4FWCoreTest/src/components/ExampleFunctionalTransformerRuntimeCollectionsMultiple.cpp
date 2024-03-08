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
#include "edm4hep/TrackerHit3DCollection.h"
#include "edm4hep/TrackCollection.h"
#include "podio/UserDataCollection.h"

#include "k4FWCore/Transformer.h"

#include <stdexcept>
#include <string>

// Which type of collection we are reading
using FloatColl         = std::map<std::string, std::shared_ptr<podio::UserDataCollection<float>>>;
using ParticleColl      = std::map<std::string, std::shared_ptr<edm4hep::MCParticleCollection>>;
using SimTrackerHitColl = std::map<std::string, std::shared_ptr<edm4hep::SimTrackerHitCollection>>;
using TrackerHitColl    = std::map<std::string, std::shared_ptr<edm4hep::TrackerHit3DCollection>>;
using TrackColl         = std::map<std::string, std::shared_ptr<edm4hep::TrackCollection>>;

using retType = std::tuple<std::map<std::string, podio::UserDataCollection<float>>,
                           std::map<std::string, edm4hep::MCParticleCollection>,
                           std::map<std::string, edm4hep::MCParticleCollection>,
                           std::map<std::string, edm4hep::SimTrackerHitCollection>,
                           std::map<std::string, edm4hep::TrackerHit3DCollection>,
                           std::map<std::string, edm4hep::TrackCollection>>;

struct ExampleFunctionalTransformerRuntimeCollectionsMultiple final
    : k4FWCore::MultiTransformer<retType(const FloatColl&, const ParticleColl&, const SimTrackerHitColl&,
                                         const TrackerHitColl&, const TrackColl&)> {
  // The pairs in KeyValue can be changed from python and they correspond
  // to the names of the input collection
  ExampleFunctionalTransformerRuntimeCollectionsMultiple(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(name, svcLoc,
                         {
                             KeyValues("InputCollectionFloat", {"VectorFloat"}),
                             KeyValues("InputCollectionParticles", {"MCParticles1"}),
                             KeyValues("InputCollectionSimTrackerHits", {"SimTrackerHits"}),
                             KeyValues("InputCollectionTrackerHits", {"TrackerHits"}),
                             KeyValues("InputCollectionTracks", {"Tracks"}),
                         },
                         {
                             KeyValues("OutputCollectionFloat", {"VectorFloat"}),
                             KeyValues("OutputCollectionParticles1", {"MCParticles1"}),
                             KeyValues("OutputCollectionParticles2", {"MCParticles2"}),
                             KeyValues("OutputCollectionSimTrackerHits", {"SimTrackerHits"}),
                             KeyValues("OutputCollectionTrackerHits", {"TrackerHits"}),
                             KeyValues("OutputCollectionTracks", {"Tracks"}),
                         }) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  retType operator()(const FloatColl& floatMap, const ParticleColl& particlesMap,
                     const SimTrackerHitColl& simTrackerHitMap, const TrackerHitColl& trackerHitMap,
                     const TrackColl& trackMap) const override {
    auto floatMapOut         = std::map<std::string, podio::UserDataCollection<float>>();
    auto particleMapOut      = std::map<std::string, edm4hep::MCParticleCollection>();
    auto particle2MapOut     = std::map<std::string, edm4hep::MCParticleCollection>();
    auto simTrackerHitMapOut = std::map<std::string, edm4hep::SimTrackerHitCollection>();
    auto trackerHitMapOut    = std::map<std::string, edm4hep::TrackerHit3DCollection>();
    auto trackMapOut         = std::map<std::string, edm4hep::TrackCollection>();

    if (floatMap.size() != 3) {
      throw std::runtime_error("Wrong size of the floatVector collection map, expected 3, got " +
                               std::to_string(floatMap.size()) + "");
    }
    for (const auto& [key, floatVector] : floatMap) {
      if (floatVector->size() != 3) {
        throw std::runtime_error("Wrong size of floatVector collection, expected 3, got " +
                                 std::to_string(floatVector->size()) + "");
      }
      if ((floatVector->vec()[0] != 125) || (floatVector->vec()[1] != 25) || (floatVector->vec()[2] != 0)) {
        std::stringstream error;
        error << "Wrong data in floatVector collection, expected 125, 25, " << 0 << " got " << floatVector->vec()[0]
              << ", " << floatVector->vec()[1] << ", " << floatVector->vec()[2] << "";
        throw std::runtime_error(error.str());
      }
      auto coll = podio::UserDataCollection<float>();
      coll.push_back(floatVector->vec()[0]);
      coll.push_back(floatVector->vec()[1]);
      coll.push_back(floatVector->vec()[2]);
      floatMapOut["New" + key] = std::move(coll);
    }

    if (particlesMap.size() != 3) {
      throw std::runtime_error("Wrong size of the particleMap map, expected 3, got " +
                               std::to_string(particleMapOut.size()) + "");
    }

    for (auto& [key, particles] : particlesMap) {
      auto coll = edm4hep::MCParticleCollection();
      int  i   = 0;
      for (const auto& particle : *particles) {
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
          coll.push_back(particle.clone());
        }
        i++;
        particleMapOut["New" + key] = std::move(coll);
      }
    }

    if (simTrackerHitMap.size() != 3) {
      throw std::runtime_error("Wrong size of the simTrackerHitMap map, expected 3, got " +
                               std::to_string(simTrackerHitMapOut.size()) + "");
    }

    for (auto& [key, simTrackerHits] : simTrackerHitMap) {
      auto coll = edm4hep::SimTrackerHitCollection();
      if ((simTrackerHits->at(0).getPosition()[0] != 3) || (simTrackerHits->at(0).getPosition()[1] != 4) ||
          (simTrackerHits->at(0).getPosition()[2] != 5)) {
        std::stringstream error;
        error << "Wrong data in simTrackerHits collection, expected 3, 4, 5 got "
              << simTrackerHits->at(0).getPosition()[0] << ", " << simTrackerHits->at(0).getPosition()[1] << ", "
              << simTrackerHits->at(0).getPosition()[2] << "";
        throw std::runtime_error(error.str());
      }
      coll.push_back(simTrackerHits->at(0).clone());
      simTrackerHitMapOut["New" + key] = std::move(coll);
    }

    if (trackerHitMap.size() != 3) {
      throw std::runtime_error("Wrong size of the trackerHitMap map, expected 3, got " +
                               std::to_string(trackerHitMapOut.size()) + "");
    }

    for (auto& [key, trackerHits] : trackerHitMap) {
      auto coll = edm4hep::TrackerHit3DCollection();
      if ((trackerHits->at(0).getPosition()[0] != 3) || (trackerHits->at(0).getPosition()[1] != 4) ||
          (trackerHits->at(0).getPosition()[2] != 5)) {
        std::stringstream error;
        error << "Wrong data in trackerHits collection, expected 3, 4, 5 got " << trackerHits->at(0).getPosition()[0]
              << ", " << trackerHits->at(0).getPosition()[1] << ", " << trackerHits->at(0).getPosition()[2] << "";
        throw std::runtime_error(error.str());
      }
      coll.push_back(trackerHits->at(0).clone());
      trackerHitMapOut["New" + key] = std::move(coll);
    }

    if (trackMap.size() != 3) {
      throw std::runtime_error("Wrong size of the trackMap map, expected 3, got " + std::to_string(trackMapOut.size()) +
                               "");
    }

    for (auto& [key, tracks] : trackMap) {
      auto coll = edm4hep::TrackCollection();
      if ((tracks->at(0).getType() != 1) || (std::abs(tracks->at(0).getChi2() - 2.1) > 1e-6) ||
          (tracks->at(0).getNdf() != 3) || (std::abs(tracks->at(0).getDEdx() - 4.1) > 1e-6) ||
          (std::abs(tracks->at(0).getDEdxError() - 5.1) > 1e-6) ||
          (std::abs(tracks->at(0).getRadiusOfInnermostHit() - 6.1) > 1e-6)) {
        std::stringstream error;
        error << "Wrong data in tracks collection, expected 1, 2.1, 3, 4.1, 5.1, 6.1 got " << tracks->at(0).getType()
              << ", " << tracks->at(0).getChi2() << ", " << tracks->at(0).getNdf() << ", " << tracks->at(0).getDEdx()
              << ", " << tracks->at(0).getDEdxError() << ", " << tracks->at(0).getRadiusOfInnermostHit() << "";
        throw std::runtime_error(error.str());
      }
      coll->push_back(tracks->at(0).clone());
      trackMapOut["New" + key] = std::move(coll);
    }

    return std::make_tuple(std::move(floatMapOut), std::move(particleMapOut), std::move(particle2MapOut),
                           std::move(simTrackerHitMapOut), std::move(trackerHitMapOut), std::move(trackMapOut));
  }

private:
  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalTransformerRuntimeCollectionsMultiple)
