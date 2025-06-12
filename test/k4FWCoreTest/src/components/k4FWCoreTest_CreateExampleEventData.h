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

#include "Gaudi/Algorithm.h"

#include "k4FWCore/DataHandle.h"

#include "podio/UserDataCollection.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/RecoMCParticleLinkCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHit3DCollection.h"

/** @class k4FWCoreTest_CreateExampleEventData
 *  Lightweight producer for edm data for tests that do not depend on the actual
 *  data content and therefore do not need the simulation machinery.
 *  Fills data members with increasing integers, together with some offset so that different
 *  events can be easily distinguished.
 *
 */
class k4FWCoreTest_CreateExampleEventData : public Gaudi::Algorithm {
public:
  explicit k4FWCoreTest_CreateExampleEventData(const std::string&, ISvcLocator*);
  /**  Execute.
   *   @return status code
   */
  StatusCode execute(const EventContext&) const final;

private:
  /// integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  mutable k4FWCore::DataHandle<edm4hep::MCParticleCollection> m_mcParticleHandle{"MCParticles",
                                                                                 Gaudi::DataHandle::Writer, this};
  mutable k4FWCore::DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitHandle{"SimTrackerHits",
                                                                                       Gaudi::DataHandle::Writer, this};
  mutable k4FWCore::DataHandle<edm4hep::TrackerHit3DCollection> m_TrackerHitHandle{"TrackerHits",
                                                                                   Gaudi::DataHandle::Writer, this};

  mutable k4FWCore::DataHandle<edm4hep::TrackCollection> m_trackHandle{"Tracks", Gaudi::DataHandle::Writer, this};

  mutable k4FWCore::DataHandle<podio::UserDataCollection<float>> m_vectorFloatHandle{"VectorFloat",
                                                                                     Gaudi::DataHandle::Writer, this};
  mutable k4FWCore::DataHandle<edm4hep::ReconstructedParticleCollection> m_recoHandle{"RecoParticles",
                                                                                      Gaudi::DataHandle::Writer, this};
  mutable k4FWCore::DataHandle<edm4hep::RecoMCParticleLinkCollection> m_linkHandle{"Links", Gaudi::DataHandle::Writer,
                                                                                   this};

  mutable int m_event{0};
};
#endif /* K4FWCORE_K4FWCORETEST_CREATEEXAMPLEEVENTDATA */
