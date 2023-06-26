#include "k4FWCoreTest_CheckExampleEventData.h"
#include <GaudiAlg/GaudiAlgorithm.h>

// datamodel
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHitCollection.h"

DECLARE_COMPONENT(k4FWCoreTest_CheckExampleEventData)

k4FWCoreTest_CheckExampleEventData::k4FWCoreTest_CheckExampleEventData(const std::string& aName, ISvcLocator* aSvcLoc)
    : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("mcparticles", m_mcParticleHandle, "Dummy Particle collection (output)");
  declareProperty("vectorfloat", m_vectorFloatHandle, "Dummy collection (output)");
}

StatusCode k4FWCoreTest_CheckExampleEventData::initialize() { return GaudiAlgorithm::initialize(); }

StatusCode k4FWCoreTest_CheckExampleEventData::execute() {
  auto floatVector = m_vectorFloatHandle.get();
  if (floatVector->size() != 3 || (*floatVector)[2] != m_event) {
    fatal() << "Contents of vectorfloat collection is not as expected: size = " << floatVector->size()
            << " (expected 3), contents = " << *floatVector << " (expected [125., 25., " << m_event << "]) "
            << std::endl;
    return StatusCode::FAILURE;
  }

  auto particles = m_mcParticleHandle.get();
  auto particle  = (*particles)[0];
  if ((particle.getMomentum().x != m_magicNumberOffset + m_event + 5) ||
      (particle.getMass() != m_magicNumberOffset + m_event + 8)) {
    fatal() << "Contents of mcparticles collection is not as expected: momentum.x = " << particle.getMomentum().x
            << " (expected " << m_magicNumberOffset + m_event + 5 << "), mass = " << particle.getMass() << " (expected "
            << m_magicNumberOffset + m_event + 8 << ")" << std::endl;
    return StatusCode::FAILURE;
  }

  m_event++;
  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_CheckExampleEventData::finalize() { return GaudiAlgorithm::finalize(); }
