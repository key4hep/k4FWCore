#ifndef K4FWCORE_K4FWCORETEST_CHECKEEXAMPLEEVENTDATA
#define K4FWCORE_K4FWCORETEST_CHECKEEXAMPLEEVENTDATA

#include "k4FWCore/DataHandle.h"
#include "k4FWCore/MetaDataHandle.h"
// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

#include "podio/UserDataCollection.h"

// datamodel
namespace edm4hep {
  class MCParticleCollection;
  class SimTrackerHitCollection;
  class TrackerHitCollection;
  class SimCaloHitCollection;
  class TrackCollection;
}  // namespace edm4hep

class k4FWCoreTest_CheckExampleEventData : public GaudiAlgorithm {
public:
  explicit k4FWCoreTest_CheckExampleEventData(const std::string&, ISvcLocator*);
  ~k4FWCoreTest_CheckExampleEventData() = default;
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
  DataHandle<edm4hep::MCParticleCollection>    m_mcParticleHandle{"MCParticles", Gaudi::DataHandle::Reader, this};
  DataHandle<podio::UserDataCollection<float>> m_vectorFloatHandle{"VectorFloat", Gaudi::DataHandle::Reader, this};

  // Some test MetaData
  MetaDataHandle<float> m_metadatafloat{&m_vectorFloatHandle, "someCollectionMetaData",  Gaudi::DataHandle::Writer, this};

  int m_event{0};
};

#endif  // K4FWCORE_K4FWCORETEST_CHECKEEXAMPLEEVENTDATA
