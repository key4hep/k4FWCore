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
#ifndef K4FWCORE_K4FWCORETEST_CHECKEEXAMPLEEVENTDATA
#define K4FWCORE_K4FWCORETEST_CHECKEEXAMPLEEVENTDATA

#include "k4FWCore/DataHandle.h"

// GAUDI
#include "Gaudi/Algorithm.h"

#include "podio/UserDataCollection.h"

// datamodel
namespace edm4hep {
class MCParticleCollection;
} // namespace edm4hep

class k4FWCoreTest_CheckExampleEventData : public Gaudi::Algorithm {
public:
  explicit k4FWCoreTest_CheckExampleEventData(const std::string&, ISvcLocator*);
  ~k4FWCoreTest_CheckExampleEventData() = default;
  /**  Execute.
   *   @return status code
   */
  StatusCode execute(const EventContext&) const final;

private:
  /// integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  Gaudi::Property<bool> m_keepEventNumberZero{this, "keepEventNumberZero", false,
                                              "Don't add the event number to the dummy values written"};
  /// Handle for the MCParticles to be written
  mutable k4FWCore::DataHandle<edm4hep::MCParticleCollection> m_mcParticleHandle{"MCParticles",
                                                                                 Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<podio::UserDataCollection<float>> m_vectorFloatHandle{"VectorFloat",
                                                                                     Gaudi::DataHandle::Reader, this};

  mutable int m_event{0};
};

#endif // K4FWCORE_K4FWCORETEST_CHECKEEXAMPLEEVENTDATA
