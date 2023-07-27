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
#include "PodioInput.h"
#include "GaudiAlg/Consumer.h"

#include "k4FWCore/PodioDataSvc.h"

DECLARE_COMPONENT(PodioInput)

PodioInput::PodioInput(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {
  // check whether we have the PodioEvtSvc active
  m_podioDataSvc = dynamic_cast<PodioDataSvc*>(evtSvc().get());
  if (!m_podioDataSvc) {
    error() << "Could not get PodioDataSvc" << endmsg;
  }
}

void PodioInput::operator()() const {

  for (auto& collName : m_collectionNames) {
    debug() << "Registering collection to read " << collName << endmsg;
    if (m_podioDataSvc->readCollection(collName).isFailure()) {
      error() << "Failed to register collection " << collName << endmsg;
    }
  }

  // Tell data service that we are done with requested collections
  m_podioDataSvc->endOfRead();
}
