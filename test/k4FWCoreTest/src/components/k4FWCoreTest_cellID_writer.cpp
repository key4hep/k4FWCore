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
#include "k4FWCoreTest_cellID_writer.h"

DECLARE_COMPONENT(k4FWCoreTest_cellID_writer)

k4FWCoreTest_cellID_writer::k4FWCoreTest_cellID_writer(const std::string& aName, ISvcLocator* aSvcLoc)
    : Gaudi::Algorithm(aName, aSvcLoc) {
  ;
  declareProperty("simtrackhits_w", m_simTrackerHitWriterHandle, "Dummy Hit collection Writer");
  // Set Cardinality to 1 because this algorithm is not prepared to run in parallel
  setProperty("Cardinality", 1).ignore();
}

StatusCode k4FWCoreTest_cellID_writer::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  m_cellIDHandle.put(cellIDtest);

  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_cellID_writer::execute(const EventContext&) const {
  edm4hep::SimTrackerHitCollection* simTrackerHits = m_simTrackerHitWriterHandle.createAndPut();
  auto                              hit            = simTrackerHits->create();
  hit.setPosition({3, 4, 5});

  return StatusCode::SUCCESS;
}
