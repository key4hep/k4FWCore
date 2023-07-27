#include "Gaudi/Property.h"
#include "Gaudi/Algorithm.h"
#include "GaudiAlg/Producer.h"
#include "k4FWCore/DataWrapper.h"

#include "edm4hep/MCParticleCollection.h"

#include <string>

// This will always be Gaudi::Algorithm
using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

// Which type of collection we are producing
// Has to be wrapped in DataWrapper
using colltype = DataWrapper<edm4hep::MCParticleCollection>;

struct ExampleFunctionalProducer final : Gaudi::Functional::Producer<colltype(), BaseClass_t> {

  ExampleFunctionalProducer( const std::string& name, ISvcLocator* svcLoc )
    : Producer( name, svcLoc, KeyValue( "OutputLocation", "/ExampleInt" ) ) {}

  // This is the function that will be called to produce the data
  colltype operator()() const override {
    auto coll = std::make_unique<edm4hep::MCParticleCollection>();
    coll->push_back({1, 2, 3, 4, 5, 6, {}, {}, {}, {}, {}, {}});
    coll->push_back({2, 3, 4, 5, 6, 7, {}, {}, {}, {}, {}, {}});
    colltype dw = DataWrapper<edm4hep::MCParticleCollection>(std::move(coll));
    return dw;
  }

  Gaudi::Property<int> m_exampleInt{this, "ExampleInt", 3,
                                    "Example int to be produced"};
};
 
DECLARE_COMPONENT(ExampleFunctionalProducer)
