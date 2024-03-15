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
#include "EventHeaderCreator.h"
#include "edm4hep/EventHeaderCollection.h"

DECLARE_COMPONENT(EventHeaderCreator)

EventHeaderCreator::EventHeaderCreator(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("EventHeaderCollection", m_headerCol,
                  "Name of the EventHeaderCollection that will be stored in the output root file.");
}

StatusCode EventHeaderCreator::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure())
    return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

StatusCode EventHeaderCreator::execute(const EventContext&) const {
  static int eventNumber = 0;
  debug() << "Filling EventHeader with runNumber " << int(m_runNumber) << " and eventNumber "
          << eventNumber + m_eventNumberOffset << endmsg;
  auto headers = m_headerCol.createAndPut();
  auto header  = headers->create();
  header.setRunNumber(m_runNumber);
  header.setEventNumber(eventNumber++ + m_eventNumberOffset);
  return StatusCode::SUCCESS;
}

StatusCode EventHeaderCreator::finalize() {
  if (Gaudi::Algorithm::finalize().isFailure())
    return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}
