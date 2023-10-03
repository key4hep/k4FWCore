#include "Gaudi/Property.h"
#include "GaudiAlg/Transformer.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MutableMCParticle.h"

// Define BaseClass_t
#include "k4FWCore/BaseClass.h"

#include <string>

// Which type of collection we are reading and writing
using colltype_in  = edm4hep::MCParticleCollection;
using colltype_out = edm4hep::MCParticleCollection;

struct ExampleFunctionalTransformer final
    : Gaudi::Functional::Transformer<colltype_out(const colltype_in&), BaseClass_t> {
  ExampleFunctionalTransformer(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, KeyValue("InputCollection", "MCParticles"),
                    KeyValue("OutputCollection", "NewMCParticles")) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as all pointers to collections
  // we get from the input
  colltype_out operator()(const colltype_in& input) const override {
    auto  coll_out = edm4hep::MCParticleCollection();
    for (const auto& particle : input) {
      auto new_particle = edm4hep::MutableMCParticle();
      new_particle.setPDG(particle.getPDG() + 10);
      new_particle.setGeneratorStatus(particle.getGeneratorStatus() + 10);
      new_particle.setSimulatorStatus(particle.getSimulatorStatus() + 10);
      new_particle.setCharge(particle.getCharge() + 10);
      new_particle.setTime(particle.getTime() + 10);
      new_particle.setMass(particle.getMass() + 10);
      coll_out->push_back(new_particle);
    }
    return coll_out;
  }
};

DECLARE_COMPONENT(ExampleFunctionalTransformer)
