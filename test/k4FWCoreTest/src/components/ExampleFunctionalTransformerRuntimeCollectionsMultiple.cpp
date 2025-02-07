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

#include "k4FWCore/Transformer.h"

#include <stdexcept>
#include <string>

// Which type of collection we are reading
using FloatColl         = std::vector<const podio::UserDataCollection<float>*>;
using ParticleColl      = std::vector<const edm4hep::MCParticleCollection*>;
using SimTrackerHitColl = std::vector<const edm4hep::SimTrackerHitCollection*>;
using TrackerHitColl    = std::vector<const edm4hep::TrackerHit3DCollection*>;
using TrackColl         = std::vector<const edm4hep::TrackCollection*>;
using RecoColl          = std::vector<const edm4hep::ReconstructedParticleCollection*>;
using LinkColl          = std::vector<const edm4hep::RecoMCParticleLinkCollection*>;

using retType = std::tuple<std::vector<podio::UserDataCollection<float>>, std::vector<edm4hep::MCParticleCollection>,
                           std::vector<edm4hep::MCParticleCollection>, std::vector<edm4hep::SimTrackerHitCollection>,
                           std::vector<edm4hep::TrackerHit3DCollection>, std::vector<edm4hep::TrackCollection>,
                           std::vector<edm4hep::ReconstructedParticleCollection>,
                           std::vector<edm4hep::RecoMCParticleLinkCollection>>;

struct ExampleFunctionalTransformerRuntimeCollectionsMultiple final
    : k4FWCore::MultiTransformer<retType(const FloatColl&, const ParticleColl&, const SimTrackerHitColl&,
                                         const TrackerHitColl&, const TrackColl&, const RecoColl&, const LinkColl&)> {
  // The pairs in KeyValue can be changed from python and they correspond
  // to the names of the input collections
  ExampleFunctionalTransformerRuntimeCollectionsMultiple(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(name, svcLoc,
                         {
                             KeyValues("InputCollectionFloat", {"VectorFloat"}),
                             KeyValues("InputCollectionParticles", {"MCParticles1"}),
                             KeyValues("InputCollectionSimTrackerHits", {"SimTrackerHits"}),
                             KeyValues("InputCollectionTrackerHits", {"TrackerHits"}),
                             KeyValues("InputCollectionTracks", {"Tracks"}),
                             KeyValues("InputCollectionRecoParticles", {"Recos"}),
                             KeyValues("InputCollectionLinks", {"Links"}),
                         },
                         {
                             KeyValues("OutputCollectionFloat", {"VectorFloat"}),
                             KeyValues("OutputCollectionParticles1", {"MCParticles1"}),
                             KeyValues("OutputCollectionParticles2", {"MCParticles2"}),
                             KeyValues("OutputCollectionSimTrackerHits", {"SimTrackerHits"}),
                             KeyValues("OutputCollectionTrackerHits", {"TrackerHits"}),
                             KeyValues("OutputCollectionTracks", {"Tracks"}),
                             KeyValues("OutputCollectionRecos", {"Recos"}),
                             KeyValues("OutputCollectionLinks", {"Links"}),
                         }) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  retType operator()(const FloatColl& floatVec, const ParticleColl& particlesVec,
                     const SimTrackerHitColl& simTrackerHitVec, const TrackerHitColl& trackerHitVec,
                     const TrackColl& trackVec, const RecoColl& recoVec, const LinkColl& linkVec) const override {
    auto floatVecOut         = std::vector<podio::UserDataCollection<float>>();
    auto particleVecOut      = std::vector<edm4hep::MCParticleCollection>();
    auto particle2VecOut     = std::vector<edm4hep::MCParticleCollection>();
    auto simTrackerHitVecOut = std::vector<edm4hep::SimTrackerHitCollection>();
    auto trackerHitVecOut    = std::vector<edm4hep::TrackerHit3DCollection>();
    auto trackVecOut         = std::vector<edm4hep::TrackCollection>();
    auto recoVecOut          = std::vector<edm4hep::ReconstructedParticleCollection>();
    auto linkVecOut          = std::vector<edm4hep::RecoMCParticleLinkCollection>();

    if (floatVec.size() != 3) {
      throw std::runtime_error("Wrong size of the float vector, expected 3, got " + std::to_string(floatVec.size()) +
                               "");
    }
    for (const auto& floatVector : floatVec) {
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
      floatVecOut.emplace_back(std::move(coll));
    }

    if (particlesVec.size() != 3) {
      throw std::runtime_error("Wrong size of the particle vector, expected 3, got " +
                               std::to_string(particleVecOut.size()) + "");
    }

    for (auto& particles : particlesVec) {
      auto coll = edm4hep::MCParticleCollection();
      int  i    = 0;
      for (const auto& particle : *particles) {
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
        coll.push_back(particle.clone());
        i++;
      }
      particleVecOut.emplace_back(std::move(coll));
    }

    if (simTrackerHitVec.size() != 3) {
      throw std::runtime_error("Wrong size of the simTrackerHit vector, expected 3, got " +
                               std::to_string(simTrackerHitVecOut.size()) + "");
    }

    for (auto& simTrackerHits : simTrackerHitVec) {
      auto coll = edm4hep::SimTrackerHitCollection();
      if ((simTrackerHits->at(0).getPosition()[0] != 3) || (simTrackerHits->at(0).getPosition()[1] != 4) ||
          (simTrackerHits->at(0).getPosition()[2] != 5)) {
        std::stringstream error;
        error << "Wrong data in simTrackerHits collection, expected 3, 4, 5 got "
              << simTrackerHits->at(0).getPosition()[0] << ", " << simTrackerHits->at(0).getPosition()[1] << ", "
              << simTrackerHits->at(0).getPosition()[2];
        throw std::runtime_error(error.str());
      }
      coll.push_back(simTrackerHits->at(0).clone());
      simTrackerHitVecOut.emplace_back(std::move(coll));
    }

    if (trackerHitVec.size() != 3) {
      throw std::runtime_error("Wrong size of the trackerHit vector, expected 3, got " +
                               std::to_string(trackerHitVecOut.size()) + "");
    }

    for (auto& trackerHits : trackerHitVec) {
      auto coll = edm4hep::TrackerHit3DCollection();
      if ((trackerHits->at(0).getPosition()[0] != 3) || (trackerHits->at(0).getPosition()[1] != 4) ||
          (trackerHits->at(0).getPosition()[2] != 5)) {
        std::stringstream error;
        error << "Wrong data in trackerHits collection, expected 3, 4, 5 got " << trackerHits->at(0).getPosition()[0]
              << ", " << trackerHits->at(0).getPosition()[1] << ", " << trackerHits->at(0).getPosition()[2] << "";
        throw std::runtime_error(error.str());
      }
      coll.push_back(trackerHits->at(0).clone());
      trackerHitVecOut.emplace_back(std::move(coll));
    }

    if (trackVec.size() != 3) {
      throw std::runtime_error("Wrong size of the track vector, expected 3, got " + std::to_string(trackVecOut.size()) +
                               "");
    }

    for (auto& tracks : trackVec) {
      auto coll = edm4hep::TrackCollection();
      if ((tracks->at(0).getType() != 1) || (std::abs(tracks->at(0).getChi2() - 2.1) > 1e-6) ||
          (tracks->at(0).getNdf() != 3)) {
        std::stringstream error;
        error << "Wrong data in tracks collection, expected 1, 2.1, 3, 4.1, 5.1, 6.1 got " << tracks->at(0).getType()
              << ", " << tracks->at(0).getChi2() << ", " << tracks->at(0).getNdf();
        throw std::runtime_error(error.str());
      }
      coll->push_back(tracks->at(0).clone());
      trackVecOut.emplace_back(std::move(coll));
    }

    if (recoVec.size() != 3) {
      throw std::runtime_error("Wrong size of the reco vector, expected 3, got " + std::to_string(recoVecOut.size()) +
                               "");
    }
    for (auto& recoColl : recoVec) {
      auto coll = edm4hep::ReconstructedParticleCollection();
      if (recoColl->size() != 5 || recoColl->at(1).getPDG() != 1) {
        std::stringstream error;
        error << "Wrong data in reco collection " << ", expected 5, 1 got " << recoColl->size();
        if (recoColl->size() > 1) {
          error << ", expected PDG 1, got " << recoColl->at(1).getPDG();
        }
        throw std::runtime_error(error.str());
      }
      for (const auto& reco : *recoColl) {
        coll.push_back(reco.clone());
      }
      recoVecOut.emplace_back(std::move(coll));
    }

    if (linkVec.size() != 3) {
      throw std::runtime_error("Wrong size of the link vector, expected 3, got " + std::to_string(linkVecOut.size()) +
                               "");
    }
    for (size_t i = 0; i < linkVec.size(); ++i) {
      const auto& linkColl = linkVec[i];
      auto        coll     = edm4hep::RecoMCParticleLinkCollection();
      if (linkColl->size() != 2) {
        std::stringstream error;
        error << "Wrong data in link collection " << i << ", expected 2, got " << linkColl->size();
        throw std::runtime_error(error.str());
      }
      for (size_t j = 0; j < linkColl->size(); j++) {
        auto link = coll.create();
        link.setFrom(recoVecOut[i].at(j));
        link.setTo(particleVecOut[i].at(j));
      }
      linkVecOut.emplace_back(std::move(coll));
    }

    return std::make_tuple(std::move(floatVecOut), std::move(particleVecOut), std::move(particle2VecOut),
                           std::move(simTrackerHitVecOut), std::move(trackerHitVecOut), std::move(trackVecOut),
                           std::move(recoVecOut), std::move(linkVecOut));
  }

private:
  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalTransformerRuntimeCollectionsMultiple)
