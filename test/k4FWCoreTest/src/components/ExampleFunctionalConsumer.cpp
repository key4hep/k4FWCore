#include "Gaudi/Property.h"
#include "Gaudi/Algorithm.h"
#include "GaudiAlg/Consumer.h"
#include "k4FWCore/DataWrapper.h"

#include "podio/CollectionBase.h"
#include "edm4hep/MCParticleCollection.h"

#include <string>

// This will always be Gaudi::Algorithm
using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

// Which type of collection we are reading
// this will always be podio::CollectionBase
// Has to be wrapped in DataWrapper
using colltype = DataWrapper<podio::CollectionBase>;

struct ExampleFunctionalConsumer final : Gaudi::Functional::Consumer<void(const colltype& input), BaseClass_t> {

  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the input collection
  ExampleFunctionalConsumer( const std::string& name, ISvcLocator* svcLoc )
    : Consumer( name, svcLoc, KeyValue("InputCollection", "MCParticles")) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  void operator()(const colltype& input) const override {
    auto* coll = dynamic_cast<const edm4hep::MCParticleCollection*>(input.getData());
    int i = 0;
    for (const auto& particle : *coll) {
      if ((particle.getPDG() != 1 + i) ||
          (particle.getGeneratorStatus() != 2 + i) ||
          (particle.getSimulatorStatus() != 3 + i) ||
          (particle.getCharge() != 4 + i) ||
          (particle.getTime() != 5 + i) ||
          (particle.getMass() != 6 + i)) {
        fatal() << "Wrong data in MCParticle collection";
      }
      i++;
    }
  }

};
 
DECLARE_COMPONENT(ExampleFunctionalConsumer)
