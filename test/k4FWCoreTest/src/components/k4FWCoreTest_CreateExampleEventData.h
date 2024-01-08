/*
 * Copyright (c) 2014-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef K4FWCORE_K4FWCORETEST_CREATEEXAMPLEEVENTDATA
#define K4FWCORE_K4FWCORETEST_CREATEEXAMPLEEVENTDATA

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

// key4hep
#include "k4FWCore/DataHandle.h"

// podio
#include "podio/UserDataCollection.h"

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

  DataHandle<podio::UserDataCollection<float>> m_vectorFloatHandle{"VectorFloat", Gaudi::DataHandle::Writer, this};

  int m_event{0};
};
#endif /* K4FWCORE_K4FWCORETEST_CREATEEXAMPLEEVENTDATA */
