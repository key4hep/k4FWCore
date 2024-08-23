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
#ifndef K4FWCORE_K4FWCORETEST_CELLID_READER
#define K4FWCORE_K4FWCORETEST_CELLID_READER

// GAUDI
#include "Gaudi/Algorithm.h"

// key4hep
#include "k4FWCore/DataHandle.h"
#include "k4FWCore/MetaDataHandle.h"

#include "edm4hep/Constants.h"

#include "edm4hep/SimTrackerHitCollection.h"

/** @class k4FWCoreTest_cellID
 *  Lightweight reader for edm data to test cellID reading
 */
class k4FWCoreTest_cellID_reader : public Gaudi::Algorithm {
public:
  explicit k4FWCoreTest_cellID_reader(const std::string&, ISvcLocator*);
  /**  Execute.
   *   @return status code
   */
  StatusCode execute(const EventContext&) const final;

private:
  /// Handle for the SimTrackerHits to be read
  mutable DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitReaderHandle{"SimTrackerHits",
                                                                                   Gaudi::DataHandle::Reader, this};
  mutable MetaDataHandle<std::string> m_cellIDHandle{m_simTrackerHitReaderHandle, edm4hep::labels::CellIDEncoding,
                                                     Gaudi::DataHandle::Reader};
};
#endif /* K4FWCORE_K4FWCORETEST_CELLID */
