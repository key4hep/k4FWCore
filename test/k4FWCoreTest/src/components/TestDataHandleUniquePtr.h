#ifndef TESTFWCORE_TESTDATAHANDLEUNIQUEPTR
#define TESTFWCORE_TESTDATAHANDLEUNIQUEPTR

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

// edm4hep
#include "TTree.h"
#include "k4FWCore/DataHandle.h"

// datamodel
namespace edm4hep {
  class MCParticleCollection;
}

class TestDataHandleUniquePtr : public GaudiAlgorithm {
public:
  explicit TestDataHandleUniquePtr(const std::string&, ISvcLocator*);
  virtual ~TestDataHandleUniquePtr();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Execute.
   *   @return status code
   */
  virtual StatusCode execute() final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  DataHandle<edm4hep::MCParticleCollection> m_mcParticleHandle{"MCParticles", Gaudi::DataHandle::Writer, this};
};
#endif /* TESTFWCORE_TESTDATAHANDLEUNIQUEPTR */
