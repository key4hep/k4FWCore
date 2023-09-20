#include "Gaudi/Algorithm.h"
#include "Gaudi/Property.h"
#include "GaudiAlg/Consumer.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "podio/UserDataCollection.h"

// Define BaseClass_t
#include "k4FWCore/BaseClass.h"

#include <string>


// Which type of collection we are reading
using FloatColl = podio::UserDataCollection<float>;
using ParticleColl = edm4hep::MCParticleCollection;
using SimTrackerHitColl = edm4hep::SimTrackerHitCollection;
using TrackerHitColl = edm4hep::TrackerHitCollection;
using TrackColl = edm4hep::TrackCollection;


struct ExampleFunctionalConsumerMultiple final
    : Gaudi::Functional::Consumer<
          void(const FloatColl&, const ParticleColl&, const SimTrackerHitColl&, const TrackerHitColl&, const TrackColl&), BaseClass_t> {
  // The pairs in KeyValue can be changed from python and they correspond
  // to the names of the input collection
  ExampleFunctionalConsumerMultiple(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc,
                 {
                     KeyValue("InputCollectionFloat", "VectorFloat"),
                     KeyValue("InputCollectionParticles", "MCParticles1"),
                     KeyValue("InputCollectionSimTrackerHits", "SimTrackerHits"),
                     KeyValue("InputCollectionTrackerHits", "TrackerHits"),
                     KeyValue("InputCollectionTracks", "Tracks"),
                 }) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  void operator()(
                  const FloatColl& floatVector,
                  const ParticleColl& particles, const SimTrackerHitColl& simTrackerHits,
                  const TrackerHitColl& trackerHits, const  TrackColl& tracks) const override {
    // if ((floatVector[0] != 125) || (floatVector[1] != 25) || (floatVector[2] != m_event)) {
    //   fatal() << "Wrong data in floatVector collection";
    // }

    auto  p4            = particles.momentum()[0];
    if ((p4.x != m_magicNumberOffset + m_event + 5) || (p4.y != m_magicNumberOffset + 6) ||
        (p4.z != m_magicNumberOffset + 7) || (particles[0].getMass() != m_magicNumberOffset + m_event + 8)) {
      fatal() << "Wrong data in particles collection";
    }

    if ((simTrackerHits[0].getPosition()[0] != 3) || (simTrackerHits[0].getPosition()[1] != 4) ||
        (simTrackerHits[0].getPosition()[2] != 5)) {
      fatal() << "Wrong data in simTrackerHits collection";
    }

    if ((trackerHits[0].getPosition()[0] != 3) || (trackerHits[0].getPosition()[1] != 4) ||
        (trackerHits[0].getPosition()[2] != 5)) {
      fatal() << "Wrong data in trackerHits collection";
    }

    if ((tracks[0].getType() != 1) || (tracks[0].getChi2() != 2.1) || (tracks[0].getNdf() != 3) ||
        (tracks[0].getDEdx() != 4.1) || (tracks[0].getDEdxError() != 5.1) ||
        (tracks[0].getRadiusOfInnermostHit() != 6.1)
        // (tracks[0].getSubdetectorHitNumbers() != {1, 4})
    ) {
      fatal() << "Wrong data in tracks collection";
    }
  }

private:
  // integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  int                  m_event{0};
};

DECLARE_COMPONENT(ExampleFunctionalConsumerMultiple)
