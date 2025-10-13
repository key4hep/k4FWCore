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

#include <string>

// Which type of collection we are reading
using FloatColl = podio::UserDataCollection<float>;
using ParticleColl = edm4hep::MCParticleCollection;
using SimTrackerHitColl = edm4hep::SimTrackerHitCollection;
using TrackerHitColl = edm4hep::TrackerHit3DCollection;
using TrackColl = edm4hep::TrackCollection;
using RecoColl = edm4hep::ReconstructedParticleCollection;
using LinkColl = edm4hep::RecoMCParticleLinkCollection;

// As a simple example, we'll write an integer and a collection of MCParticles
using Counter = podio::UserDataCollection<int>;
using Particle = edm4hep::MCParticleCollection;

struct ExampleFunctionalTransformerMultiple final
    : k4FWCore::MultiTransformer<std::tuple<Counter, Particle, LinkColl>(
          const FloatColl&, const ParticleColl&, const SimTrackerHitColl&, const TrackerHitColl&, const TrackColl&,
          const RecoColl&, const LinkColl&)> {
  ExampleFunctionalTransformerMultiple(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(
            name, svcLoc,
            {KeyValue("InputCollectionFloat", "VectorFloat"), KeyValue("InputCollectionParticles", "MCParticles1"),
             KeyValue("InputCollectionSimTrackerHits", "SimTrackerHits"),
             KeyValue("InputCollectionTrackerHits", "TrackerHits"), KeyValue("InputCollectionTracks", "Tracks"),
             KeyValue("InputCollectionRecoParticles", "RecoParticles"), KeyValue("InputCollectionLinks", "Links")},
            {KeyValue("OutputCollectionCounter", {"Counter"}),
             KeyValue("OutputCollectionParticles", {"NewMCParticles"}),
             KeyValue("OutputLinkCollection", {"NewLinks"})}) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  std::tuple<Counter, Particle, LinkColl> operator()(const FloatColl&, const ParticleColl& particles,
                                                     const SimTrackerHitColl& simTrackerHits,
                                                     const TrackerHitColl& trackerHits, const TrackColl& tracks,
                                                     const RecoColl& recos, const LinkColl& links) const override {
    Counter counter;

    auto newParticlesColl = edm4hep::MCParticleCollection();
    for (const auto& p : particles) {
      // We need to create a new particle since the current one is already in a collection

      auto newParticle = newParticlesColl.create();
      newParticle.setPDG(p.getPDG() + m_offset);
      newParticle.setGeneratorStatus(p.getGeneratorStatus() + m_offset);
      newParticle.setSimulatorStatus(p.getSimulatorStatus() + m_offset);
      newParticle.setCharge(p.getCharge() + m_offset);
      newParticle.setTime(p.getTime() + m_offset);
      newParticle.setMass(p.getMass() + m_offset);
    }
    counter.push_back(particles.size());
    counter.push_back(simTrackerHits.size());
    counter.push_back(trackerHits.size());
    counter.push_back(tracks.size());

    auto newLinks = edm4hep::RecoMCParticleLinkCollection();
    for (size_t i = 0; i < links.size(); ++i) {
      auto link = newLinks.create();
      link.setFrom(recos[i]);
      link.setTo(newParticlesColl[i]);
    }

    return std::make_tuple(std::move(counter), std::move(newParticlesColl), std::move(newLinks));
  }

  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalTransformerMultiple)
