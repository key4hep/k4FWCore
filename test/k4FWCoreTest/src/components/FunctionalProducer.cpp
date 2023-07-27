#include "Gaudi/Property.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Producer.h"
#include "edm4hep/MCParticleCollection.h"
#include "podio/UserDataCollection.h"
#include "k4FWCore/DataHandle.h"

#include <string>

using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

using colltype = DataWrapper<podio::UserDataCollection<float>>;

struct FunctionalProducer final : Gaudi::Functional::Producer<colltype(), BaseClass_t> {

  FunctionalProducer( const std::string& name, ISvcLocator* svcLoc )
    : Producer( name, svcLoc, KeyValue( "OutputLocation", "/ExampleInt" ) ) {}

  colltype operator()() const override {
    auto coll = new podio::UserDataCollection<float>();
    coll->push_back(1.5);
    coll->push_back(2.5);
    colltype* dw = new DataWrapper<podio::UserDataCollection<float>>();
    dw->setData(coll);
    return *dw;
  }

  Gaudi::Property<int> m_exampleInt{this, "ExampleInt", 3,
                                    "Example int to be produced"};

};
 
DECLARE_COMPONENT(FunctionalProducer)
