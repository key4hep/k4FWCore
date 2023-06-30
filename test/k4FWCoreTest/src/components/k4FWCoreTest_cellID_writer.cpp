#include "k4FWCoreTest_cellID_writer.h"

DECLARE_COMPONENT(k4FWCoreTest_cellID_writer)

k4FWCoreTest_cellID_writer::k4FWCoreTest_cellID_writer(const std::string& aName, ISvcLocator* aSvcLoc)
    : GaudiAlgorithm(aName, aSvcLoc) {
  ;
  declareProperty("simtrackhits_w", m_simTrackerHitWriterHandle, "Dummy Hit collection Writer");
}

k4FWCoreTest_cellID_writer::~k4FWCoreTest_cellID_writer() {}

StatusCode k4FWCoreTest_cellID_writer::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  m_cellIDHandle.put(cellIDtest);

  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_writer::execute() {
  edm4hep::SimTrackerHitCollection* simTrackerHits = m_simTrackerHitWriterHandle.createAndPut();
  auto                              hit            = simTrackerHits->create();
  hit.setPosition({3, 4, 5});

  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_writer::finalize() { return GaudiAlgorithm::finalize(); }
