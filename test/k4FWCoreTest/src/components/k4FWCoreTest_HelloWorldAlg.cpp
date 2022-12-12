#include "k4FWCoreTest_HelloWorldAlg.h"
#include "GaudiKernel/MsgStream.h"

DECLARE_COMPONENT(k4FWCoreTest_HelloWorldAlg)

k4FWCoreTest_HelloWorldAlg::k4FWCoreTest_HelloWorldAlg(const std::string& aName, ISvcLocator* aSvcLoc)
    : GaudiAlgorithm(aName, aSvcLoc) {}

k4FWCoreTest_HelloWorldAlg::~k4FWCoreTest_HelloWorldAlg() {}

StatusCode k4FWCoreTest_HelloWorldAlg::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_HelloWorldAlg::execute() {
  info() << endmsg;
  info() << endmsg;
  info() << theMessage << endmsg;
  info() << endmsg;
  info() << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_HelloWorldAlg::finalize() { return GaudiAlgorithm::finalize(); }
