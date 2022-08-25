#ifndef K4FWCORE_K4FWCORETEST_CREATEEXAMPLEEVENTDATA
#define K4FWCORE_K4FWCORETEST_CREATEEXAMPLEEVENTDATA

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

// edm4hep
#include "TTree.h"
#include "k4FWCore/DataHandle.h"

// datamodel
namespace edm4hep {
  class MCParticleCollection;
  class SimTrackerHitCollection;
  class TrackerHitCollection;
  class SimCaloHitCollection;
  class TrackCollection;
}  // namespace edm4hep

/** @class k4FWCoreTest_CreateExampleEventData
 *  Lightweight producer for edm data for tests that do not depend on the actual
 *  data content and therefore do not need the simulation machinery.
 *  Fills data members with increasing integers, together with some offset so that different
 *  events can be easily distinguished.
 *
 */
class k4FWCoreTest_CreateExampleEventData : public GaudiAlgorithm {
public:
  explicit k4FWCoreTest_CreateExampleEventData(const std::string&, ISvcLocator*);
  virtual ~k4FWCoreTest_CreateExampleEventData();
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
  /// Handle for the MCParticles to be written
  DataHandle<edm4hep::MCParticleCollection> m_mcParticleHandle{"MCParticles", Gaudi::DataHandle::Writer, this};
  /// Handle for the SimTrackerHits to be written
  DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitHandle{"SimTrackerHits", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackerHitCollection>    m_TrackerHitHandle{"TrackerHits", Gaudi::DataHandle::Writer, this};

  /// Handle for the Tracks to be written
  DataHandle<edm4hep::TrackCollection> m_trackHandle{"Tracks", Gaudi::DataHandle::Writer, this};

  DataHandle<float>              m_singleFloatHandle{"SingleFloat", Gaudi::DataHandle::Writer, this};
  DataHandle<std::vector<float>> m_vectorFloatHandle{"VectorFloat", Gaudi::DataHandle::Writer, this};
  DataHandle<int>                m_singleIntHandle{"SingleInt", Gaudi::DataHandle::Writer, this};
};
#endif /* K4FWCORE_K4FWCORETEST_CREATEEXAMPLEEVENTDATA */
