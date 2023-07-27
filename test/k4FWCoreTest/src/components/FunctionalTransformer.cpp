#include "Gaudi/Property.h"
#include "Gaudi/Algorithm.h"
#include "GaudiAlg/Transformer.h"
#include "k4FWCore/DataWrapper.h"

#include "podio/CollectionBase.h"
#include "podio/UserDataCollection.h"
#include "edm4hep/MCParticleCollection.h"

#include <string>

// This will always be Gaudi::Algorithm
using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

// Which type of collection we are reading and writing
// Both have to be wrapped in DataWrapper
// For reading, the collection type is always podio::CollectionBase
using colltype_in = DataWrapper<podio::CollectionBase>;
using colltype_out = DataWrapper<podio::UserDataCollection<double>>;

struct ExampleFunctionalTransformer final :
  Gaudi::Functional::Transformer<colltype_out(const colltype_in&), BaseClass_t> {

  ExampleFunctionalTransformer( const std::string& name, ISvcLocator* svcLoc )
    : Transformer( name, svcLoc, KeyValue("InputLocation", "/ExampleInt"),
                   KeyValue( "OutputLocation", "/ExampleDouble" ) ) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as all pointers to collections
  // we get from the input
  colltype_out operator()(const colltype_in& input) const override {
    const auto* coll = input.getData();
    const auto* ptr = reinterpret_cast<const podio::UserDataCollection<float>*>(coll);
    auto coll_out = std::make_unique<podio::UserDataCollection<double>>();
    for (auto& val : *ptr) {
      coll_out->push_back(val);
    }
    colltype_out dw = DataWrapper<podio::UserDataCollection<double>>(std::move(coll_out));
    return dw;
  }

};
 
DECLARE_COMPONENT(ExampleFunctionalTransformer)
