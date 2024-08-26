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
#include "k4FWCoreTest_cellID_reader.h"
#include "k4FWCoreTest_cellID_writer.h"

DECLARE_COMPONENT(k4FWCoreTest_cellID_reader)

k4FWCoreTest_cellID_reader::k4FWCoreTest_cellID_reader(const std::string& aName, ISvcLocator* aSvcLoc)
    : Gaudi::Algorithm(aName, aSvcLoc) {
  ;
  declareProperty("simtrackhits_r", m_simTrackerHitReaderHandle, "Dummy Hit collection Reader");
  // Set Cardinality to 1 because this algorithm is not prepared to run in parallel
  setProperty("Cardinality", 1).ignore();
}

k4FWCoreTest_cellID_reader::~k4FWCoreTest_cellID_reader() {}

StatusCode k4FWCoreTest_cellID_reader::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_reader::execute(const EventContext&) const {
  [[maybe_unused]] const auto simtrackerhits_coll = m_simTrackerHitReaderHandle.get();

  const auto cellIDstr = m_cellIDHandle.get();

  if (cellIDstr != cellIDtest) {
    error() << "ERROR cellID is: " << cellIDstr << "expected (" << cellIDtest << ")" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_reader::finalize() { return Gaudi::Algorithm::finalize(); }
