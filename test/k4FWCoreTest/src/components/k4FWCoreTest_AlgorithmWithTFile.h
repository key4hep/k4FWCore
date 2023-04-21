#ifndef K4FWCORE_K4FWCORETEST_ALGORITHMWITHTFILE
#define K4FWCORE_K4FWCORETEST_ALGORITHMWITHTFILE

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

// edm4hep
#include "TFile.h"
#include "TTree.h"
#include "k4FWCore/DataHandle.h"

#include "podio/UserDataCollection.h"

// datamodel
namespace edm4hep {
  class MCParticleCollection;
  class SimTrackerHitCollection;
  class SimCaloHit;
}  // namespace edm4hep

/** @class k4FWCoreTest_AlgorithmWithTFile
 *  Test producer to check that data can still be written to
 *  a user-declared TFile when using the PodioDataSvc
 *
 */
class k4FWCoreTest_AlgorithmWithTFile : public GaudiAlgorithm {
public:
  explicit k4FWCoreTest_AlgorithmWithTFile(const std::string&, ISvcLocator*);
  virtual ~k4FWCoreTest_AlgorithmWithTFile();
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
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  /// Handle for the genparticles to be written
  DataHandle<edm4hep::MCParticleCollection> m_mcParticleHandle{"MCParticles", Gaudi::DataHandle::Writer, this};
  /// Handle for the genvertices to be written
  DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitHandle{"SimTrackerHit", Gaudi::DataHandle::Writer, this};

  DataHandle<podio::UserDataCollection<float>> m_vectorFloatHandle{"VectorFloat", Gaudi::DataHandle::Writer, this};

  /// for testing: write a second TFile by user in an algorithm
  Float_t m_value;
  TFile*  m_file;
  TTree*  m_tree;
};
#endif /* K4FWCORE_K4FWCORETEST_ALGORITHMWITHTFILE */
