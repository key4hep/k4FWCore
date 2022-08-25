#include "TestDataHandleUniquePtr.h"


// datamodel
#include "edm4hep/MCParticleCollection.h"


DECLARE_COMPONENT(TestDataHandleUniquePtr)

TestDataHandleUniquePtr::TestDataHandleUniquePtr(const std::string& aName, ISvcLocator* aSvcLoc) : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("mcparticles", m_mcParticleHandle, "Dummy Particle collection (output)");
}

TestDataHandleUniquePtr::~TestDataHandleUniquePtr() {}

StatusCode TestDataHandleUniquePtr::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode TestDataHandleUniquePtr::execute() {

  auto coll = std::make_unique<edm4hep::MCParticleCollection>();

  auto particle = coll->create();

  auto& p4 = particle.momentum();
  p4.x = 5;
  p4.y = 6;
  p4.z = 7;
  particle.setMass(8);


  m_mcParticleHandle.put(std::move(coll));


  return StatusCode::SUCCESS;
}

StatusCode TestDataHandleUniquePtr::finalize() { return GaudiAlgorithm::finalize(); }
