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

  ExampleFunctionalConsumer( const std::string& name, ISvcLocator* svcLoc )
    : Consumer( name, svcLoc, KeyValue("InputLocation", "MCParticles")) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as all pointers to collections
  // we get from the input
  void operator()(const colltype& input) const override {
    std::cout << "ExampleFunctionalConsumer: " << input << std::endl;
    const auto* coll = input.getData();
    const auto* ptr = reinterpret_cast<const edm4hep::MCParticleCollection*>(coll);
    int i = 0;
    for (const auto& p : *ptr) {
      assert(p.getPDG() == 1 + i);
      assert(p.getGeneratorStatus() == 2 + i);
      assert(p.getSimulatorStatus() == 3 + i);
      assert(p.getCharge() == 4 + i);
      assert(p.getTime() == 5 + i);
      assert(p.getMass() == 6 + i);
      i++;
    }
  }

};
 
DECLARE_COMPONENT(ExampleFunctionalConsumer)
