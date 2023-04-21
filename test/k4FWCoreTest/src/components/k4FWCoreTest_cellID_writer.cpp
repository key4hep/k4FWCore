#include "k4FWCoreTest_cellID_writer.h"

DECLARE_COMPONENT(k4FWCoreTest_cellID_writer)

k4FWCoreTest_cellID_writer::k4FWCoreTest_cellID_writer(const std::string& aName, ISvcLocator* aSvcLoc)
    : GaudiAlgorithm(aName, aSvcLoc), m_eventDataSvc("EventDataSvc", "k4FWCoreTest_cellID_writer") {
  ;
  declareProperty("simtrackhits_w", m_simTrackerHitWriterHandle, "Dummy Hit collection Writer");
}

k4FWCoreTest_cellID_writer::~k4FWCoreTest_cellID_writer() {}

StatusCode k4FWCoreTest_cellID_writer::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }

  StatusCode sc  = m_eventDataSvc.retrieve();
  m_podioDataSvc = dynamic_cast<PodioDataSvc*>(m_eventDataSvc.get());

  if (sc == StatusCode::FAILURE) {
    error() << "Error retrieving Event Data Service" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_writer::execute() {
  edm4hep::SimTrackerHitCollection* simTrackerHits = m_simTrackerHitWriterHandle.createAndPut();
  auto                              hit            = simTrackerHits->create();
  hit.setPosition({3, 4, 5});

  // TODO: decide on collection metadata interface in the data service 
  // something along the lines of 
  // m_podioDataSvc->addCollectionMetaData(collID, "name", value);

  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_writer::finalize() { return GaudiAlgorithm::finalize(); }
