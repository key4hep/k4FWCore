#include "TestUniqueIDGenSvc.h"


DECLARE_COMPONENT(TestUniqueIDGenSvc)

TestUniqueIDGenSvc::TestUniqueIDGenSvc(const std::string& aName, ISvcLocator* aSvcLoc) : 
  GaudiAlgorithm(aName, aSvcLoc) {}

TestUniqueIDGenSvc::~TestUniqueIDGenSvc() {}

StatusCode TestUniqueIDGenSvc::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }

  m_service = serviceLocator()->service("UniqueIDGenSvc");

  return StatusCode::SUCCESS;
}

StatusCode TestUniqueIDGenSvc::execute() {

  // Get seed set for random engine
  std::vector<long> got_seeds{};
  StatusCode got_seeds_sc = randSvc()->engine()->seeds(got_seeds);
  if (!got_seeds_sc.isSuccess())
    warning() << "Random Service seeds could not be set" << endmsg;

  if (got_seeds[0] != 4072012) {
    return StatusCode::FAILURE;
  }

  uint evt_num = 4;
  uint run_num = 3;
  std::string name = "Some algorithm name";

  auto uid = m_service->getUniqueID(got_seeds[0], evt_num, run_num, name);
  auto uid_again = m_service->getUniqueID(got_seeds[0], evt_num, run_num, name);

  if (uid != uid_again) {
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode TestUniqueIDGenSvc::finalize() { 
  return GaudiAlgorithm::finalize(); 
}
