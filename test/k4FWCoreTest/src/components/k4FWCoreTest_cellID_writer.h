/*
 * Copyright (c) 2014-2023 Key4hep-Project.
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
#ifndef K4FWCORE_K4FWCORETEST_CELLID_WRITER
#define K4FWCORE_K4FWCORETEST_CELLID_WRITER

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

// key4hep
#include "k4FWCore/DataHandle.h"
#include "k4FWCore/MetaDataHandle.h"

// datamodel
#include "edm4hep/SimTrackerHitCollection.h"

const std::string cellIDtest = "M:3,S-1:3,I:9,J:9,K-1:6";

/** @class k4FWCoreTest_cellID
 *  Lightweight producer for edm data to test cellID
 */
class k4FWCoreTest_cellID_writer : public GaudiAlgorithm {
public:
  explicit k4FWCoreTest_cellID_writer(const std::string&, ISvcLocator*);
  virtual ~k4FWCoreTest_cellID_writer();
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
  DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitWriterHandle{"SimTrackerHits", Gaudi::DataHandle::Writer,
                                                                           this};
  MetaDataHandle<std::string>                  m_cellIDHandle{m_simTrackerHitWriterHandle, "CellIDEncodingString",
                                             Gaudi::DataHandle::Writer};
};
#endif /* K4FWCORE_K4FWCORETEST_CELLID_WRITER */
