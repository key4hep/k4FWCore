#ifndef K4FWCORE_K4FWCORETEST_CELLID_READER
#define K4FWCORE_K4FWCORETEST_CELLID_READER

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

// edm4hep
#include "k4FWCore/DataHandle.h"
#include "TTree.h"

// datamodel
#include "edm4hep/SimTrackerHitCollection.h"

/** @class k4FWCoreTest_cellID
 *  Lightweight producer for edm data for tests that do not depend on the actual
 *  data content and therefore do not need the simulation machinery.
 *  Fills data members with increasing integers, together with some offset so that different
 *  events can be easily distinguished. 
 *
 */
class k4FWCoreTest_cellID_reader : public GaudiAlgorithm {
public:
  explicit k4FWCoreTest_cellID_reader(const std::string&, ISvcLocator*);
  virtual ~k4FWCoreTest_cellID_reader();
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
  /// Handle for the SimTrackerHits to be written
  // DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitWriterHandle{"SimTrackerHits", Gaudi::DataHandle::Writer, this};
  /// Handle for the SimTrackerHits to be read
   DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitReaderHandle{"SimTrackerHits", Gaudi::DataHandle::Reader, this};
};
#endif /* K4FWCORE_K4FWCORETEST_CELLID */
