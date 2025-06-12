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

#include "k4FWCore/Producer.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/RecoMCParticleLinkCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHit3DCollection.h"
#include "podio/UserDataCollection.h"

#include <string>

// Which type of collections we are producing

using retType =
    std::tuple<podio::UserDataCollection<float>, edm4hep::MCParticleCollection, edm4hep::MCParticleCollection,
               edm4hep::SimTrackerHitCollection, edm4hep::TrackerHit3DCollection, edm4hep::TrackCollection,
               edm4hep::ReconstructedParticleCollection, edm4hep::RecoMCParticleLinkCollection>;

struct ExampleFunctionalProducerMultiple final : k4FWCore::Producer<retType()> {
  // The pairs in KeyValue can be changed from python and they correspond
  // to the names of the output collections
  ExampleFunctionalProducerMultiple(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {},
                 {KeyValues("OutputCollectionFloat", {"VectorFloat"}),
                  KeyValues("OutputCollectionParticles1", {"MCParticles1"}),
                  KeyValues("OutputCollectionParticles2", {"MCParticles2"}),
                  KeyValues("OutputCollectionSimTrackerHits", {"SimTrackerHits"}),
                  KeyValues("OutputCollectionTrackerHits", {"TrackerHits"}),
                  KeyValues("OutputCollectionTracks", {"Tracks"}),
                  KeyValues("OutputCollectionRecoParticles", {"RecoParticles"}),
                  KeyValues("OutputCollectionLinks", {"Links"})}) {}

  // This is the function that will be called to produce the data
  retType operator()() const override {
    // The following was copied and adapted from the
    // k4FWCoreTest_CreateExampleEventData test

    auto floatVector = podio::UserDataCollection<float>();
    floatVector.push_back(125.);
    floatVector.push_back(25.);
    floatVector.push_back(m_event);

    auto particles = edm4hep::MCParticleCollection();
    edm4hep::Vector3d v{0, 0, 0};
    auto part1 = particles.create(1, 2, 3, 4.f, 5.f, 6.f, v, v, v);
    auto part2 = particles.create(2, 3, 4, 5.f, 6.f, 7.f);

    auto simTrackerHits = edm4hep::SimTrackerHitCollection();
    auto hit = simTrackerHits.create();
    hit.setPosition({3, 4, 5});

    auto trackerHits = edm4hep::TrackerHit3DCollection();
    auto trackerHit = trackerHits.create();
    trackerHit.setPosition({3, 4, 5});

    auto tracks = edm4hep::TrackCollection();
    auto track = tracks.create();
    auto track2 = tracks.create();
    // set members
    track.setType(1);
    track.setChi2(2.1);
    track.setNdf(3);
    track.addToSubdetectorHitNumbers(1);
    track.addToSubdetectorHitNumbers(4);
    track.addToTrackStates(edm4hep::TrackState());
    // set associatons
    track.addToTrackerHits(trackerHit);
    track.addToTracks(track2);

    auto recos = edm4hep::ReconstructedParticleCollection();
    for (int i = 0; i < 5; ++i) {
      auto reco = recos.create();
      reco.setPDG(i);
    }

    auto links = edm4hep::RecoMCParticleLinkCollection();
    for (size_t i = 0; i < 2; ++i) {
      auto link = links.create();
      link.setFrom(recos[i]);
      link.setTo(particles[i]);
    }

    return std::make_tuple(std::move(floatVector), std::move(particles), edm4hep::MCParticleCollection(),
                           std::move(simTrackerHits), std::move(trackerHits), std::move(tracks), std::move(recos),
                           std::move(links));
  }

private:
  // We can define any property we want that can be set from python
  // and use it inside operator()
  Gaudi::Property<int> m_exampleInt{this, "ExampleInt", 3, "Example int that can be used in the algorithm"};
  // integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  int m_event{0};
};

DECLARE_COMPONENT(ExampleFunctionalProducerMultiple)
