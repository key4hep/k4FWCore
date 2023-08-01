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
    auto* floatVectorColl = dynamic_cast<const podio::UserDataCollection<float>*>(floatVector.getData());
    if (((*floatVectorColl)[0] != 125) ||
        ((*floatVectorColl)[1] != 25) ||
        ((*floatVectorColl)[2] != m_event)) {
      fatal() << "Wrong data in floatVector collection";
    }

    auto* particlesColl = dynamic_cast<const edm4hep::MCParticleCollection*>(particles.getData());
    auto p4 = (*particlesColl).momentum()[0];
    if ((p4.x != m_magicNumberOffset + m_event + 5) ||
        (p4.y != m_magicNumberOffset + 6) ||
        (p4.z != m_magicNumberOffset + 7) ||
        ((*particlesColl)[0].getMass() != m_magicNumberOffset + m_event + 8)) {
      fatal() << "Wrong data in particles collection";
    }

    auto* simTrackerHitsColl = dynamic_cast<const edm4hep::SimTrackerHitCollection*>(simTrackerHits.getData());
    if (((*simTrackerHitsColl)[0].getPosition()[0] != 3) || 
        ((*simTrackerHitsColl)[0].getPosition()[1] != 4) ||
        ((*simTrackerHitsColl)[0].getPosition()[2] != 5)) {
      fatal() << "Wrong data in simTrackerHits collection";
    }

    auto trackerHitsColl = dynamic_cast<const edm4hep::TrackerHitCollection*>(trackerHits.getData());
    if (((*trackerHitsColl)[0].getPosition()[0] != 3) ||
        ((*trackerHitsColl)[0].getPosition()[1] != 4) ||
        ((*trackerHitsColl)[0].getPosition()[2] != 5)) {
      fatal() << "Wrong data in trackerHits collection";
    }

    auto tracksColl = dynamic_cast<const edm4hep::TrackCollection*>(tracks.getData());
    if (((*tracksColl)[0].getType() != 1) ||
        ((*tracksColl)[0].getChi2() != 2.1) ||
        ((*tracksColl)[0].getNdf() != 3) ||
        ((*tracksColl)[0].getDEdx() != 4.1) ||
        ((*tracksColl)[0].getDEdxError() != 5.1) ||
        ((*tracksColl)[0].getRadiusOfInnermostHit() != 6.1) 
        // ((*tracksColl)[0].getSubdetectorHitNumbers() != {1, 4})
        ) {
      fatal() << "Wrong data in tracks collection";
    }

  }
private:
  // integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  int m_event{0};

};
 
DECLARE_COMPONENT(ExampleFunctionalConsumerMultiple)
