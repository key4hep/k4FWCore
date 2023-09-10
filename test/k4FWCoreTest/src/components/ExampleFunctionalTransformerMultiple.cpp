#include "Gaudi/Algorithm.h"
#include "Gaudi/Property.h"
#include "GaudiAlg/Transformer.h"
#include "k4FWCore/DataWrapper.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "podio/UserDataCollection.h"

// Define BaseClass_t
#include "k4FWCore/FunctionalUtils.h"

#include <string>

// Which type of collection we are reading
using FloatColl = podio::UserDataCollection<float>;
using ParticleColl = edm4hep::MCParticleCollection;
using SimTrackerHitColl = edm4hep::SimTrackerHitCollection;
using TrackerHitColl = edm4hep::TrackerHitCollection;
using TrackColl = edm4hep::TrackCollection;

// As a simple example, we'll write an integer and a collection of MCParticles
using Counter_t  = podio::UserDataCollection<int>;
using Particle_t = edm4hep::MCParticleCollection;

struct ExampleFunctionalTransformerMultiple final
    : Gaudi::Functional::MultiTransformer<std::tuple<Counter_t, Particle_t>(const FloatColl&, const ParticleColl&, const SimTrackerHitColl&, const TrackerHitColl&, const TrackColl&),
                                          BaseClass_t> {
  ExampleFunctionalTransformerMultiple(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(
            name, svcLoc,
            {KeyValue("InputCollectionFloat", "VectorFloat"), KeyValue("InputCollectionParticles", "MCParticles"),
             KeyValue("InputCollectionSimTrackerHits", "SimTrackerHits"),
             KeyValue("InputCollectionTrackerHits", "TrackerHits"), KeyValue("InputCollectionTracks", "Tracks")},
            {KeyValue("OutputCollectionCounter", "Counter"), KeyValue("OutputCollectionParticles", "NewMCParticles")}) {
  }

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  std::tuple<Counter_t, Particle_t> operator()(const FloatColl& floatVector, const ParticleColl& particles,
                                               const SimTrackerHitColl& simTrackerHits, const TrackerHitColl& trackerHits,
                                               const TrackColl& tracks) const override {
    Counter_t counter;

    counter.push_back(floatVector.size());

    auto newParticlesColl = edm4hep::MCParticleCollection();
    for (const auto& p : particles) {
      // We need to create a new particle since the current one is already in a collection

      // We could create a new one
      auto newParticle = newParticlesColl->create();
      newParticle.setPDG(p.getPDG());
      newParticle.setGeneratorStatus(p.getGeneratorStatus() + 1);
      newParticle.setSimulatorStatus(p.getSimulatorStatus() + 1);
      newParticle.setCharge(p.getCharge() + 2);
      newParticle.setTime(p.getTime() + 3);
      newParticle.setMass(p.getMass() + 4);
    }
    counter.push_back(particles.size());

    counter.push_back(simTrackerHits.size());

    counter.push_back(trackerHits.size());

    counter.push_back(tracks.size());

    return std::make_tuple(std::move(counter), std::move(newParticlesColl));
  }
};

DECLARE_COMPONENT(ExampleFunctionalTransformerMultiple)
