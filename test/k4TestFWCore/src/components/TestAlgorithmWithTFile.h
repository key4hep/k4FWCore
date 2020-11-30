#ifndef TESTFWCORE_CREATEEXAMPLEEVENTDATA
#define TESTFWCORE_CREATEEXAMPLEEVENTDATA

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

// edm4hep
#include "k4FWCore/DataHandle.h"
#include "TTree.h"
#include "TFile.h"



// datamodel 
namespace edm4hep {
class MCParticleCollection;
class SimTrackerHitCollection;
class SimCaloHit;
}

/** @class TestAlgorithmWithTFile
 *  Lightweight producer for edm data for tests that do not depend on the actual
 *  data content and therefore do not need the simulation machinery.
 *  Fills data members with increasing integers, together with some offset so that different
 *  events can be easily distinguished. 
 *
 */
class TestAlgorithmWithTFile : public GaudiAlgorithm {
public:
  explicit TestAlgorithmWithTFile(const std::string&, ISvcLocator*);
  virtual ~TestAlgorithmWithTFile();
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
  /// integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0, "Integer to add to the dummy values written to the edm"};
  /// Handle for the genparticles to be written
  DataHandle<edm4hep::MCParticleCollection> m_mcParticleHandle{"MCParticles", Gaudi::DataHandle::Writer, this};
  /// Handle for the genvertices to be written
  DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitHandle{"SimTrackerHit", Gaudi::DataHandle::Writer, this};

  DataHandle<float> m_singleFloatHandle{"SingleFloat", Gaudi::DataHandle::Writer, this};
  DataHandle<std::vector<float>> m_vectorFloatHandle{"VectorFloat", Gaudi::DataHandle::Writer, this};
  DataHandle<int> m_singleIntHandle{"SingleInt", Gaudi::DataHandle::Writer, this};

  /// for testing: write a second TFile by user in an algorithm
  Float_t m_value;
  TFile* m_file;
  TTree* m_tree;
};
#endif /* TESTFWCORE_CREATEEXAMPLEEVENTDATA */
