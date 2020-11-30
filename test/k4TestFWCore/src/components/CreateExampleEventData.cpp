#include "CreateExampleEventData.h"


// datamodel
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"


DECLARE_COMPONENT(CreateExampleEventData)

CreateExampleEventData::CreateExampleEventData(const std::string& aName, ISvcLocator* aSvcLoc) : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("mcparticles", m_mcParticleHandle, "Dummy Particle collection (output)");
  declareProperty("trackhits", m_simTrackerHitHandle, "Dummy Hit collection (output)");
}

CreateExampleEventData::~CreateExampleEventData() {}

StatusCode CreateExampleEventData::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode CreateExampleEventData::execute() {

  m_singleIntHandle.put(new int(12345));

  std::vector<float>* floatVector = m_vectorFloatHandle.createAndPut();
  floatVector->emplace_back( 125.);
  floatVector->emplace_back( 25.);

  edm4hep::MCParticleCollection* particles = m_mcParticleHandle.createAndPut();

  auto particle = particles->create();

  auto& p4 = particle.momentum();
  p4.x = m_magicNumberOffset + 5; 
  p4.y = m_magicNumberOffset + 6;
  p4.z = m_magicNumberOffset + 7;
  particle.setMass(m_magicNumberOffset + 8);

  auto* hits = m_simTrackerHitHandle.createAndPut();
  auto hit = hits->create();
  hit.setPosition({3, 4, 5});


  return StatusCode::SUCCESS;
}

StatusCode CreateExampleEventData::finalize() { return GaudiAlgorithm::finalize(); }
