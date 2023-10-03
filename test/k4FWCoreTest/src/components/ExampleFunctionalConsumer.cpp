#include "Gaudi/Property.h"
#include "GaudiAlg/Consumer.h"

#include "edm4hep/MCParticleCollection.h"

// Define BaseClass_t
#include "k4FWCore/BaseClass.h"

#include <stdexcept>
#include <string>

// Which type of collection we are reading
using colltype = edm4hep::MCParticleCollection;

struct ExampleFunctionalConsumer final : Gaudi::Functional::Consumer<void(const colltype& input), BaseClass_t> {
  // The pair in KeyValue can be changed from python and it corresponds
  // to the name of the input collection
  ExampleFunctionalConsumer(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValue("InputCollection", "MCParticles")) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  void operator()(const colltype& input) const override {
    int   i    = 0;
    for (const auto& particle : input) {
      if ((particle.getPDG() != 1 + i + m_possibleOffset) || (particle.getGeneratorStatus() != 2 + i + m_possibleOffset) ||
          (particle.getSimulatorStatus() != 3 + i + m_possibleOffset) || (particle.getCharge() != 4 + i + m_possibleOffset) ||
          (particle.getTime() != 5 + i + m_possibleOffset) || (particle.getMass() != 6 + i + m_possibleOffset)) {
        std::stringstream error;
        error << "Wrong data in MCParticle collection, expected " << 1 + i + m_possibleOffset << ", " << 2 + i + m_possibleOffset << ", "
              << 3 + i + m_possibleOffset << ", " << 4 + i + m_possibleOffset << ", " << 5 + i + m_possibleOffset << ", " << 6 + i + m_possibleOffset << " got "
              << particle.getPDG() << ", " << particle.getGeneratorStatus() << ", " << particle.getSimulatorStatus() << ", " << particle.getCharge() << ", "
              << particle.getTime() << ", " << particle.getMass() << "";
        throw std::runtime_error(error.str());
      }
      i++;
    }
  }

  Gaudi::Property<int> m_possibleOffset{this, "PossibleOffset", 0, "Possible offset in the values data"};
};

DECLARE_COMPONENT(ExampleFunctionalConsumer)
