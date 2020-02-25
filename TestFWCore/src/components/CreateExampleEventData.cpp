#include "CreateExampleEventData.h"


// datamodel
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackHitCollection.h"


DECLARE_COMPONENT(CreateExampleEventData)

CreateExampleEventData::CreateExampleEventData(const std::string& aName, ISvcLocator* aSvcLoc) : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("mcparticles", m_mcParticleHandle, "Dummy Particle collection (output)");
  declareProperty("trackhits", m_trackHitHandle, "Dummy Hit collection (output)");
}

CreateExampleEventData::~CreateExampleEventData() {}

StatusCode CreateExampleEventData::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode CreateExampleEventData::execute() {

  std::vector<int>* intVector = m_singleIntHandle.createAndPut();
  intVector->emplace_back(777);

  std::vector<float>* floatVector = m_somefloatHandle2.createAndPut();

  floatVector->emplace_back( 125.);
  floatVector->emplace_back( 25.);

  edm4hep::MCParticleCollection* particles = m_mcParticleHandle.createAndPut();

  edm4hep::MCParticle particle = particles->create();

  auto& p4 = particle.momentum();
  p4.x = m_magicNumberOffset + 5; 
  p4.y = m_magicNumberOffset + 6;
  p4.z = m_magicNumberOffset + 7;
  particle.mass(m_magicNumberOffset + 8);



  return StatusCode::SUCCESS;
}

StatusCode CreateExampleEventData::finalize() { return GaudiAlgorithm::finalize(); }
