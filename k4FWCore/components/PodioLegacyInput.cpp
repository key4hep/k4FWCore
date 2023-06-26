#include "PodioLegacyInput.h"

#include "TFile.h"
#include "TROOT.h"

#include "k4FWCore/DataWrapper.h"
#include "k4FWCore/PodioLegacyDataSvc.h"

DECLARE_COMPONENT(PodioLegacyInput)

PodioLegacyInput::PodioLegacyInput(const std::string& name, ISvcLocator* svcLoc) : GaudiAlgorithm(name, svcLoc) {}

StatusCode PodioLegacyInput::initialize() {
  if (GaudiAlgorithm::initialize().isFailure())
    return StatusCode::FAILURE;

  // check whether we have the PodioEvtSvc active
  m_podioLegacyDataSvc = dynamic_cast<PodioLegacyDataSvc*>(evtSvc().get());
  if (nullptr == m_podioLegacyDataSvc)
    return StatusCode::FAILURE;

  auto idTable = m_podioLegacyDataSvc->getCollectionIDs();
  for (auto& name : m_collectionNames) {
    debug() << "Finding collection " << name << " in collection registry." << endmsg;
    if (!idTable->present(name)) {
      error() << "Requested product " << name << " not found." << endmsg;
      return StatusCode::FAILURE;
    }
    m_collectionIDs.push_back(idTable->collectionID(name));
  }
  return StatusCode::SUCCESS;
}

StatusCode PodioLegacyInput::execute() {
  size_t cntr = 0;
  // Re-create the collections from ROOT file
  for (auto& id : m_collectionIDs) {
    const std::string& collName = m_collectionNames.value().at(cntr++);
    debug() << "Registering collection to read " << collName << " with id " << id << endmsg;
    if (m_podioLegacyDataSvc->readCollection(collName, id).isFailure()) {
      return StatusCode::FAILURE;
    }
  }
  // Tell data service that we are done with requested collections
  m_podioLegacyDataSvc->endOfRead();
  return StatusCode::SUCCESS;
}

StatusCode PodioLegacyInput::finalize() {
  if (GaudiAlgorithm::finalize().isFailure())
    return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}
