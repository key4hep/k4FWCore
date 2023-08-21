#include "Gaudi/Algorithm.h"
#include "Gaudi/Property.h"
#include "GaudiAlg/Producer.h"
#include "k4FWCore/FunctionalUtils.h"

#include "edm4hep/MCParticleCollection.h"

#include <string>

// Which collection we are producing
using colltype = edm4hep::MCParticleCollection;

struct ExampleFunctionalProducer final : Gaudi::Functional::Producer<colltype(), BaseClass_t> {
  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the output collection
  ExampleFunctionalProducer(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, KeyValue("OutputCollection", "MCParticles")) {}

  // This is the function that will be called to produce the data
  colltype operator()() const override {
    auto coll = edm4hep::MCParticleCollection();
    coll.push_back({1, 2, 3, 4, 5, 6, {}, {}, {}, {}, {}, {}});
    coll.push_back({2, 3, 4, 5, 6, 7, {}, {}, {}, {}, {}, {}});
    // We have to return whatever collection type we specified in the
    // template argument
    return coll;
  }

private:
  // We can define any property we want that can be set from python
  // and use it inside operator()
  Gaudi::Property<int> m_exampleInt{this, "ExampleInt", 3, "Example int that can be used in the algorithm"};
};

DECLARE_COMPONENT(ExampleFunctionalProducer)
