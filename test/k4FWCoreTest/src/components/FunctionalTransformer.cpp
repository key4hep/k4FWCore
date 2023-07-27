#include "Gaudi/Property.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Transformer.h"
#include "edm4hep/MCParticleCollection.h"
#include "podio/UserDataCollection.h"
#include "k4FWCore/DataHandle.h"

#include <string>

using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

using colltype_in = DataWrapper<podio::CollectionBase>;
using colltype_out = DataWrapper<podio::UserDataCollection<double>>;

struct FunctionalTransformer final : Gaudi::Functional::Transformer<colltype_out(const colltype_in&), BaseClass_t> {

  FunctionalTransformer( const std::string& name, ISvcLocator* svcLoc )
    : Transformer( name, svcLoc, KeyValue("InputLocation", "/ExampleInt"),
                   KeyValue( "OutputLocation", "/ExampleDouble" ) ) {}

  colltype_out operator()(const colltype_in& input) const override {
    const auto* coll = input.getData();
    const auto* ptr = reinterpret_cast<const podio::UserDataCollection<float>*>(coll);
    auto coll_out = new podio::UserDataCollection<double>();
    for (auto& val : *ptr) {
      coll_out->push_back(val);
    }
    colltype_out* dw = new DataWrapper<podio::UserDataCollection<double>>();
    dw->setData(coll_out);
    return *dw;
  }

};
 
DECLARE_COMPONENT(FunctionalTransformer)
