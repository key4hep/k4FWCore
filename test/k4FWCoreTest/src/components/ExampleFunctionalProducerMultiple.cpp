#include "Gaudi/Property.h"
#include "Gaudi/Algorithm.h"
#include "GaudiAlg/Producer.h"
#include "k4FWCore/DataWrapper.h"

#include "podio/UserDataCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHitCollection.h"

#include <string>

// This will always be Gaudi::Algorithm
using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

// Which type of collections we are producing
// They have to be wrapped in DataWrapper
using Float_t = DataWrapper<podio::UserDataCollection<float>>;
using Particle_t = DataWrapper<edm4hep::MCParticleCollection>;
using SimTrackerHit_t = DataWrapper<edm4hep::SimTrackerHitCollection>;
using TrackerHit_t = DataWrapper<edm4hep::TrackerHitCollection>;
using Track_t = DataWrapper<edm4hep::TrackCollection>;

struct ExampleFunctionalProducerMultiple final : Gaudi::Functional::Producer<std::tuple<Float_t, Particle_t, SimTrackerHit_t, TrackerHit_t, Track_t>(), BaseClass_t> {

  ExampleFunctionalProducerMultiple( const std::string& name, ISvcLocator* svcLoc )
    : Producer( name, svcLoc,
                {
                KeyValue( "OutputLocationFloat", "VectorFloat" ),
                KeyValue( "OutputLocationParticles", "MCParticles" ),
                KeyValue( "OutputLocationSimTrackerHits", "SimTrackerHits" ),
                KeyValue( "OutputLocationTrackerHits", "TrackerHits" ),
                KeyValue( "OutputLocationTracks", "Tracks" )
                }
                ) {}

  // This is the function that will be called to produce the data
  std::tuple<Float_t, Particle_t, SimTrackerHit_t, TrackerHit_t, Track_t> operator()() const override {
    // The following was copied and adapted from the
    // k4FWCoreTest_CreateExampleEventData test

    auto floatVector = std::make_unique<podio::UserDataCollection<float>>();
    floatVector->push_back(125.);
    floatVector->push_back(25.);
    floatVector->push_back(m_event);
    Float_t floatVectorDW = DataWrapper<podio::UserDataCollection<float>>(std::move(floatVector));

    auto particles = std::make_unique<edm4hep::MCParticleCollection>();
    auto particle = particles->create();
    auto& p4 = particle.momentum();
    p4.x     = m_magicNumberOffset + m_event + 5;
    p4.y     = m_magicNumberOffset + 6;
    p4.z     = m_magicNumberOffset + 7;
    particle.setMass(m_magicNumberOffset + m_event + 8);
    Particle_t particleDW = DataWrapper<edm4hep::MCParticleCollection>(std::move(particles));

    auto simTrackerHits = std::make_unique<edm4hep::SimTrackerHitCollection>();
    auto hit = simTrackerHits->create();
    hit.setPosition({3, 4, 5});
    SimTrackerHit_t simTrackerHitDW = DataWrapper<edm4hep::SimTrackerHitCollection>(std::move(simTrackerHits)); 

    auto trackerHits = std::make_unique<edm4hep::TrackerHitCollection>();
    auto trackerHit = trackerHits->create();
    trackerHit.setPosition({3, 4, 5});
    TrackerHit_t trackerHitDW = DataWrapper<edm4hep::TrackerHitCollection>(std::move(trackerHits));

    auto tracks = std::make_unique<edm4hep::TrackCollection>();
    auto track  = tracks->create();
    auto track2 = tracks->create();
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
    Track_t trackDW = DataWrapper<edm4hep::TrackCollection>(std::move(tracks));

    return std::make_tuple(floatVectorDW, particleDW, simTrackerHitDW, trackerHitDW, trackDW);
  }

private:
  // integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  int m_event{0};
};
 
DECLARE_COMPONENT(ExampleFunctionalProducerMultiple)
