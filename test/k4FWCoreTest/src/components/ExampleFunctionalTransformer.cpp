#include "Gaudi/Property.h"
#include "Gaudi/Algorithm.h"
#include "GaudiAlg/Transformer.h"
#include "k4FWCore/DataWrapper.h"

#include "podio/CollectionBase.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MutableMCParticle.h"

#include <string>

// This will always be Gaudi::Algorithm
using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

// Which type of collection we are reading and writing
// Both have to be wrapped in DataWrapper
// For reading, the collection type is always podio::CollectionBase
using colltype_in = DataWrapper<podio::CollectionBase>;
using colltype_out = DataWrapper<edm4hep::MCParticleCollection>;

struct ExampleFunctionalTransformer final :
  Gaudi::Functional::Transformer<colltype_out(const colltype_in&), BaseClass_t> {

  ExampleFunctionalTransformer( const std::string& name, ISvcLocator* svcLoc )
    : Transformer( name, svcLoc, KeyValue("InputLocation", "MCParticles"),
                   KeyValue( "OutputLocation", "NewMCParticles" ) ) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as all pointers to collections
  // we get from the input
  colltype_out operator()(const colltype_in& input) const override {
    auto* coll = dynamic_cast<const edm4hep::MCParticleCollection*>(input.getData());
    auto coll_out = std::make_unique<edm4hep::MCParticleCollection>();
    for (const auto& particle : *coll) {
      auto new_particle = edm4hep::MutableMCParticle();
      new_particle.setPDG(particle.getPDG() + 10);
      new_particle.setGeneratorStatus(particle.getGeneratorStatus() + 10);
      new_particle.setSimulatorStatus(particle.getSimulatorStatus() + 10);
      new_particle.setCharge(particle.getCharge() + 10);
      new_particle.setTime(particle.getTime() + 10);
      new_particle.setMass(particle.getMass() + 10);
      // new_particle.
      coll_out->push_back(new_particle);
    }
    colltype_out dw = DataWrapper<edm4hep::MCParticleCollection>(std::move(coll_out));
    return dw;
  }

};
 
DECLARE_COMPONENT(ExampleFunctionalTransformer)
