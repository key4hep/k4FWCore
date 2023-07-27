#include "Gaudi/Property.h"
#include "Gaudi/Algorithm.h"
#include "GaudiAlg/Consumer.h"
#include "k4FWCore/DataWrapper.h"

#include "podio/UserDataCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHitCollection.h"

#include <string>

// This will always be Gaudi::Algorithm
using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

// Which type of collection we are reading
// this will always be podio::CollectionBase
// Has to be wrapped in DataWrapper
using colltype = DataWrapper<podio::CollectionBase>;

struct ExampleFunctionalConsumerMultiple final : Gaudi::Functional::Consumer<void(const colltype&,
                                                                                  const colltype&,
                                                                                  const colltype&,
                                                                                  const colltype&,
                                                                                  const colltype&), BaseClass_t> {

  ExampleFunctionalConsumerMultiple( const std::string& name, ISvcLocator* svcLoc )
    : Consumer( name, svcLoc,
                {
                  KeyValue("InputLocationFloat", "VectorFloat"),
                  KeyValue("InputLocationParticles", "MCParticles"),
                  KeyValue("InputLocationSimTrackerHits", "SimTrackerHits"),
                  KeyValue("InputLocationTrackerHits", "TrackerHits"),
                  KeyValue("InputLocationTracks", "Tracks"),
                }
                ) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  void operator()(const colltype& floatVector,
                  const colltype& particles,
                  const colltype& simTrackerHits,
                  const colltype& trackerHits,
                  const colltype& tracks) const override {
    // auto* floatVectorColl = dynamic_cast<const podio::UserDataCollection<float>*>(floatVector.getData());
    // assert(*floatVectorColl[0] == 125);
    // assert(*floatVectorColl[1] == 25);
    // assert(*floatVectorColl[2] == m_event);

    // auto particlesColl = dynamic_cast<const edm4hep::MCParticleCollection*>(particles.getData());
    // auto p4 = particlesColl[0].momentum();
    // assert(p4.x == m_magicNumberOffset + m_event + 5);
    // assert(p4.y == m_magicNumberOffset + 6);
    // assert(p4.z == m_magicNumberOffset + 7);
    // assert(particlesColl[0].getMass() == m_magicNumberOffset + m_event + 8);

    // auto simTrackerHitsColl = dynamic_cast<const edm4hep::SimTrackerHitCollection*>(simTrackerHits.getData());
    // assert(simTrackerHitsColl[0].getPosition()[0] == 3);
    // assert(simTrackerHitsColl[0].getPosition()[1] == 4);
    // assert(simTrackerHitsColl[0].getPosition()[2] == 5);

    // auto trackerHitsColl = dynamic_cast<const edm4hep::TrackerHitCollection*>(trackerHits.getData());
    // assert(trackerHitsColl[0].getPosition()[0] == 3);
    // assert(trackerHitsColl[0].getPosition()[1] == 4);
    // assert(trackerHitsColl[0].getPosition()[2] == 5);

    // auto tracksColl = dynamic_cast<const edm4hep::TrackCollection*>(tracks.getData());

  }

};
 
DECLARE_COMPONENT(ExampleFunctionalConsumerMultiple)
