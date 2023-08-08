#include "Gaudi/Algorithm.h"
#include "Gaudi/Property.h"
#include "GaudiAlg/Producer.h"
#include "k4FWCore/FunctionalUtils.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "podio/UserDataCollection.h"

#include <string>

// Which type of collections we are producing
using Float_t         = podio::UserDataCollection<float>;
using Particle_t      = edm4hep::MCParticleCollection;
using SimTrackerHit_t = edm4hep::SimTrackerHitCollection;
using TrackerHit_t    = edm4hep::TrackerHitCollection;
using Track_t         = edm4hep::TrackCollection;

struct ExampleFunctionalProducerMultiple final
    : Gaudi::Functional::Producer<std::tuple<Float_t, Particle_t, SimTrackerHit_t, TrackerHit_t, Track_t>(),
                                  BaseClass_t> {
  // The pairs in KeyValue can be changed from python and they correspond
  // to the names of the output collections
  ExampleFunctionalProducerMultiple(const std::string& name, ISvcLocator* svcLoc)
      : Producer(
            name, svcLoc,
            {KeyValue("OutputCollectionFloat", "VectorFloat"), KeyValue("OutputCollectionParticles", "MCParticles"),
             KeyValue("OutputCollectionSimTrackerHits", "SimTrackerHits"),
             KeyValue("OutputCollectionTrackerHits", "TrackerHits"), KeyValue("OutputCollectionTracks", "Tracks")}) {}

  // This is the function that will be called to produce the data
  std::tuple<Float_t, Particle_t, SimTrackerHit_t, TrackerHit_t, Track_t> operator()() const override {
    // The following was copied and adapted from the
    // k4FWCoreTest_CreateExampleEventData test

    auto floatVector = podio::UserDataCollection<float>();
    floatVector.push_back(125.);
    floatVector.push_back(25.);
    floatVector.push_back(m_event);

    auto  particles = edm4hep::MCParticleCollection();
    auto  particle  = particles.create();
    auto& p4        = particle.momentum();
    p4.x            = m_magicNumberOffset + m_event + 5;
    p4.y            = m_magicNumberOffset + 6;
    p4.z            = m_magicNumberOffset + 7;
    particle.setMass(m_magicNumberOffset + m_event + 8);

    auto simTrackerHits = edm4hep::SimTrackerHitCollection();
    auto hit            = simTrackerHits.create();
    hit.setPosition({3, 4, 5});

    auto trackerHits = edm4hep::TrackerHitCollection();
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

    return std::make_tuple(std::move(floatVector), std::move(particles), std::move(simTrackerHits),
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
