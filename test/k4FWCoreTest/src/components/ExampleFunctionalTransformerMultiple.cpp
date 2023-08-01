#include "Gaudi/Property.h"
#include "Gaudi/Algorithm.h"
#include "GaudiAlg/Transformer.h"
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

// As a simple example, we'll write an integer and a collection of MCParticles
using Counter_t = DataWrapper<podio::UserDataCollection<int>>;
using Particle_t = DataWrapper<edm4hep::MCParticleCollection>;

struct ExampleFunctionalTransformerMultiple final : Gaudi::Functional::MultiTransformer<std::tuple<Counter_t, Particle_t>(const colltype&,
                                                                                  const colltype&,
                                                                                  const colltype&,
                                                                                  const colltype&,
                                                                                  const colltype&), BaseClass_t> {

  ExampleFunctionalTransformerMultiple( const std::string& name, ISvcLocator* svcLoc )
    : MultiTransformer( name, svcLoc,
                {
                  KeyValue("InputCollectionFloat", "VectorFloat"),
                  KeyValue("InputCollectionParticles", "MCParticles"),
                  KeyValue("InputCollectionSimTrackerHits", "SimTrackerHits"),
                  KeyValue("InputCollectionTrackerHits", "TrackerHits"),
                  KeyValue("InputCollectionTracks", "Tracks")
                },
                {
                  KeyValue("OutputCollectionCounter", "Counter"),
                  KeyValue("OutputCollectionParticles", "NewMCParticles")
                }
                ) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  std::tuple<Counter_t, Particle_t> operator()(const colltype& floatVector,
                  const colltype& particles,
                  const colltype& simTrackerHits,
                  const colltype& trackerHits,
                  const colltype& tracks) const override {
    auto counter = std::make_unique<podio::UserDataCollection<int>>();

    auto* floatVectorColl = dynamic_cast<const podio::UserDataCollection<float>*>(floatVector.getData());
    counter->push_back(floatVectorColl->size());


    auto particlesColl = dynamic_cast<const edm4hep::MCParticleCollection*>(particles.getData());
    auto newParticlesColl = std::make_unique<edm4hep::MCParticleCollection>();
    for (const auto& p : *particlesColl) {
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
    auto particleDW = DataWrapper<edm4hep::MCParticleCollection>(std::move(newParticlesColl));
    counter->push_back(particlesColl->size());

    auto simTrackerHitsColl = dynamic_cast<const edm4hep::SimTrackerHitCollection*>(simTrackerHits.getData());
    counter->push_back(simTrackerHitsColl->size());

    auto trackerHitsColl = dynamic_cast<const edm4hep::TrackerHitCollection*>(trackerHits.getData());
    counter->push_back(trackerHitsColl->size());

    auto tracksColl = dynamic_cast<const edm4hep::TrackCollection*>(tracks.getData());
    counter->push_back(tracksColl->size());

    auto counterDW = DataWrapper<podio::UserDataCollection<int>>(std::move(counter));

    return std::make_tuple(counterDW, particleDW);
  }

};
 
DECLARE_COMPONENT(ExampleFunctionalTransformerMultiple)
