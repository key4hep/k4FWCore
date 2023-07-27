#include "Gaudi/Property.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Consumer.h"
#include "edm4hep/MCParticleCollection.h"
#include "podio/UserDataCollection.h"
#include "k4FWCore/DataHandle.h"

#include <string>

using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

using colltype = DataWrapper<podio::CollectionBase>;

struct FunctionalConsumer final : Gaudi::Functional::Consumer<void(const colltype& input), BaseClass_t> {

  FunctionalConsumer( const std::string& name, ISvcLocator* svcLoc )
    : Consumer( name, svcLoc, KeyValue("InputLocation", "/ExampleInt")) {}

  void operator()(const colltype& input) const override {
    std::cout << "FunctionalConsumer: " << input << std::endl;
    const auto* coll = input.getData();
    const auto* ptr = reinterpret_cast<const podio::UserDataCollection<float>*>(coll);
    for (const auto& val : *ptr) {
      std::cout << val << std::endl;
    }
  }

};
 
DECLARE_COMPONENT(FunctionalConsumer)
