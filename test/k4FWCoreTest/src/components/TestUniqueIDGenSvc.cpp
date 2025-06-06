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
#include "TestUniqueIDGenSvc.h"
#include "edm4hep/MutableEventHeader.h"
#include <cstdint>

DECLARE_COMPONENT(TestUniqueIDGenSvc)

TestUniqueIDGenSvc::TestUniqueIDGenSvc(const std::string& aName, ISvcLocator* aSvcLoc)
    : Gaudi::Algorithm(aName, aSvcLoc) {}

StatusCode TestUniqueIDGenSvc::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }

  m_service = serviceLocator()->service("UniqueIDGenSvc");

  return StatusCode::SUCCESS;
}

// This is meant to run up to two times
// For the first event, check that when giving two different event numbers, the unique IDs are different
// For the second event, the service throws when trying to get the same ID twice
StatusCode TestUniqueIDGenSvc::execute(const EventContext&) const {
  ++m_counter;
  auto evt_header = edm4hep::MutableEventHeader{};
  evt_header.setEventNumber(4);
  evt_header.setRunNumber(3 + m_counter.sum());
  std::string name = "Some algorithm name";

  auto uid = m_service->getUniqueID(evt_header, name);
  auto uid_again =
      m_service->getUniqueID(evt_header.getEventNumber() + (m_counter.sum() % 2), evt_header.getRunNumber(), name);
  if (uid == uid_again) {
    throw std::runtime_error("Unique IDs are the same");
  }

  return StatusCode::SUCCESS;
}
