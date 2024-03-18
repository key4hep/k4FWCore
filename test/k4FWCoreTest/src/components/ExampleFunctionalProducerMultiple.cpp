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
#include "GaudiAlg/Producer.h"
#include "k4FWCore/BaseClass.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#if __has_include("edm4hep/TrackerHit3DCollection.h")
#include "edm4hep/TrackerHit3DCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
namespace edm4hep {
  using TrackerHit3DCollection = edm4hep::TrackerHitCollection;
}  // namespace edm4hep
#endif
#include "podio/UserDataCollection.h"

#include <string>

// Which type of collections we are producing
using Float         = podio::UserDataCollection<float>;
using Particle      = edm4hep::MCParticleCollection;
using SimTrackerHit = edm4hep::SimTrackerHitCollection;
using TrackerHit    = edm4hep::TrackerHit3DCollection;
using Track         = edm4hep::TrackCollection;

struct ExampleFunctionalProducerMultiple final
    : Gaudi::Functional::Producer<std::tuple<Float, Particle, Particle, SimTrackerHit, TrackerHit, Track>(),
                                  BaseClass_t> {
  // The pairs in KeyValue can be changed from python and they correspond
  // to the names of the output collections
  ExampleFunctionalProducerMultiple(const std::string& name, ISvcLocator* svcLoc)
      : Producer(
            name, svcLoc,
            {KeyValue("OutputCollectionFloat", "VectorFloat"), KeyValue("OutputCollectionParticles1", "MCParticles1"),
             KeyValue("OutputCollectionParticles2", "MCParticles2"),
             KeyValue("OutputCollectionSimTrackerHits", "SimTrackerHits"),
             KeyValue("OutputCollectionTrackerHits", "TrackerHits"), KeyValue("OutputCollectionTracks", "Tracks")}) {}

  // This is the function that will be called to produce the data
  std::tuple<Float, Particle, Particle, SimTrackerHit, TrackerHit, Track> operator()() const override {
    // The following was copied and adapted from the
    // k4FWCoreTest_CreateExampleEventData test

    auto floatVector = podio::UserDataCollection<float>();
    floatVector.push_back(125.);
    floatVector.push_back(25.);
    floatVector.push_back(m_event);

    auto particles = edm4hep::MCParticleCollection();
    auto particle  = particles.create();
    particle.setMomentum({m_magicNumberOffset + m_event + 5.0, m_magicNumberOffset + 6.0, m_magicNumberOffset + 7.0});
    particle.setMass(m_magicNumberOffset + m_event + 8);

    auto simTrackerHits = edm4hep::SimTrackerHitCollection();
    auto hit            = simTrackerHits.create();
    hit.setPosition({3, 4, 5});

    auto trackerHits = edm4hep::TrackerHit3DCollection();
    auto trackerHit  = trackerHits.create();
    trackerHit.setPosition({3, 4, 5});

    auto tracks = edm4hep::TrackCollection();
    auto track  = tracks.create();
    auto track2 = tracks.create();
    // set members
    track.setType(1);
    track.setChi2(2.1);
    track.setNdf(3);
    track.setDEdx(4.1);
    track.setDEdxError(5.1);
    track.setRadiusOfInnermostHit(6.1);
    track.addToSubdetectorHitNumbers(1);
    track.addToSubdetectorHitNumbers(4);
    track.addToTrackStates(edm4hep::TrackState());
    // set associatons
    track.addToTrackerHits(trackerHit);
    track.addToTracks(track2);

    return std::make_tuple(std::move(floatVector), std::move(particles), Particle(), std::move(simTrackerHits),
                           std::move(trackerHits), std::move(tracks));
  }

private:
  // We can define any property we want that can be set from python
  // and use it inside operator()
  Gaudi::Property<int> m_exampleInt{this, "ExampleInt", 3, "Example int that can be used in the algorithm"};
  // integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  int                  m_event{0};
};

DECLARE_COMPONENT(ExampleFunctionalProducerMultiple)
