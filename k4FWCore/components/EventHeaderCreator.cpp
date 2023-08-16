#include "EventHeaderCreator.h"
#include "edm4hep/EventHeaderCollection.h"

DECLARE_COMPONENT(EventHeaderCreator)

EventHeaderCreator::EventHeaderCreator(const std::string& name, ISvcLocator* svcLoc)
    : GaudiAlgorithm(name, svcLoc) {
      declareProperty("EventHeaderColllection", m_headerCol);
    }

StatusCode EventHeaderCreator::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

StatusCode EventHeaderCreator::execute() {
  static int eventNumber = 0;
  debug() << "Filling EventHeader with runNumber " << int(m_runNumber) << " and eventNumber " << eventNumber + m_eventNumberOffset << endmsg;
  auto headers = m_headerCol.createAndPut();
  auto header = headers->create();
  header.setRunNumber(m_runNumber);
  header.setEventNumber(eventNumber++ + m_eventNumberOffset);
  return StatusCode::SUCCESS;
}

StatusCode EventHeaderCreator::finalize() {
  if (GaudiAlgorithm::finalize().isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}
