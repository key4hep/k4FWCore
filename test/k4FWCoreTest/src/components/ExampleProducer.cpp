#include "Gaudi/Property.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Producer.h"

#include <string>

using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

struct FunctionalProducer final : Gaudi::Functional::Producer<int(), BaseClass_t> {

  FunctionalProducer( const std::string& name, ISvcLocator* svcLoc )
    : Producer( name, svcLoc, KeyValue( "OutputLocation", "/ExampleInt" ) ) {}

  int operator()() const override {
    info() << "Producing " << m_exampleInt << endmsg;
    return m_exampleInt;
  }

  Gaudi::Property<int> m_exampleInt{this, "ExampleInt", 3,
                                    "Example int to be produced"};

};
 
DECLARE_COMPONENT(FunctionalProducer)
