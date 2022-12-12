#include "k4FWCoreTest_cellID_reader.h"
#include "k4FWCoreTest_cellID_writer.h"

DECLARE_COMPONENT(k4FWCoreTest_cellID_reader)

k4FWCoreTest_cellID_reader::k4FWCoreTest_cellID_reader(const std::string& aName, ISvcLocator* aSvcLoc)
    : GaudiAlgorithm(aName, aSvcLoc) {
  ;
  declareProperty("simtrackhits_r", m_simTrackerHitReaderHandle, "Dummy Hit collection Reader");
}

k4FWCoreTest_cellID_reader::~k4FWCoreTest_cellID_reader() {}

StatusCode k4FWCoreTest_cellID_reader::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_reader::execute() {
  const auto simtrackerhits_coll = m_simTrackerHitReaderHandle.get();

  auto       collID    = simtrackerhits_coll->getID();
  const auto cellIDstr = m_simTrackerHitReaderHandle.getCollMetadataCellID(collID);

  if (cellIDstr != cellIDtest) {
    std::cout << "ERROR cellID is: " << cellIDstr << std::endl;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_reader::finalize() { return GaudiAlgorithm::finalize(); }
