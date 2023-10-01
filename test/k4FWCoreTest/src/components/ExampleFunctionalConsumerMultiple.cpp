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

#include <stdexcept>
#include <string>
#include <sstream>


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
    if (floatVector.size() != 3) {
      throw std::runtime_error("Wrong size of floatVector collection, expected 3, got " + std::to_string(floatVector.size()) + "");
    }
    if ((floatVector[0] != 125) || (floatVector[1] != 25) || (floatVector[2] != 0)) {
      std::stringstream error;
      error << "Wrong data in floatVector collection, expected 125, 25, " << 0 << " got " << floatVector[0] << ", " << floatVector[1] << ", " << floatVector[2] << "";
      throw std::runtime_error(error.str());
    }

    auto  p4            = particles.momentum()[0];
    if ((p4.x != m_magicNumberOffset + 5) || (p4.y != m_magicNumberOffset + 6) ||
        (p4.z != m_magicNumberOffset + 7) || (particles[0].getMass() != m_magicNumberOffset + 8)) {
      std::stringstream error;
      error << "Wrong data in particles collection, expected " << m_magicNumberOffset + 5 << ", "
            << m_magicNumberOffset + 6 << ", " << m_magicNumberOffset + 7 << ", " <<
        m_magicNumberOffset + 8 << " got " << p4.x << ", " << p4.y << ", " <<
        p4.z << ", " << particles[0].getMass() << "";
      throw std::runtime_error(error.str());
    }

    if ((simTrackerHits[0].getPosition()[0] != 3) || (simTrackerHits[0].getPosition()[1] != 4) ||
        (simTrackerHits[0].getPosition()[2] != 5)) {
      std::stringstream error;
      error << "Wrong data in simTrackerHits collection, expected 3, 4, 5 got " << simTrackerHits[0].getPosition()[0] << ", " << simTrackerHits[0].getPosition()[1] << ", " << simTrackerHits[0].getPosition()[2] << "";
      throw std::runtime_error(error.str());
    }

    if ((trackerHits[0].getPosition()[0] != 3) || (trackerHits[0].getPosition()[1] != 4) ||
        (trackerHits[0].getPosition()[2] != 5)) {
      std::stringstream error;
      error << "Wrong data in trackerHits collection, expected 3, 4, 5 got " << trackerHits[0].getPosition()[0] << ", " << trackerHits[0].getPosition()[1] << ", " << trackerHits[0].getPosition()[2] << "";
      throw std::runtime_error(error.str());
    }

    if ((tracks[0].getType() != 1) || (std::abs(tracks[0].getChi2() - 2.1) > 1e-6) || (tracks[0].getNdf() != 3) ||
        (std::abs(tracks[0].getDEdx() - 4.1) > 1e-6) || (std::abs(tracks[0].getDEdxError() - 5.1) > 1e-6) ||
        (std::abs(tracks[0].getRadiusOfInnermostHit() - 6.1) > 1e-6)
    ) {
      std::stringstream error;
      error << "Wrong data in tracks collection, expected 1, 2.1, 3, 4.1, 5.1, 6.1 got " << tracks[0].getType() << ", " << tracks[0].getChi2() << ", " << tracks[0].getNdf() << ", " << tracks[0].getDEdx() << ", " << tracks[0].getDEdxError() << ", " << tracks[0].getRadiusOfInnermostHit() << "";
      throw std::runtime_error(error.str());
    }
  }

private:
  // integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalConsumerMultiple)
