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
#include "PodioLegacyInput.h"

#include "TFile.h"
#include "TROOT.h"

#include "k4FWCore/DataWrapper.h"
#include "k4FWCore/PodioLegacyDataSvc.h"

DECLARE_COMPONENT(PodioLegacyInput)

PodioLegacyInput::PodioLegacyInput(const std::string& name, ISvcLocator* svcLoc) : GaudiAlgorithm(name, svcLoc) {}

StatusCode PodioLegacyInput::initialize() {
  if (GaudiAlgorithm::initialize().isFailure())
    return StatusCode::FAILURE;

  // check whether we have the PodioEvtSvc active
  m_podioLegacyDataSvc = dynamic_cast<PodioLegacyDataSvc*>(evtSvc().get());
  if (nullptr == m_podioLegacyDataSvc)
    return StatusCode::FAILURE;

  auto idTable = m_podioLegacyDataSvc->getCollectionIDs();
  for (auto& name : m_collectionNames) {
    debug() << "Finding collection " << name << " in collection registry." << endmsg;
    if (!idTable->present(name)) {
      error() << "Requested product " << name << " not found." << endmsg;
      return StatusCode::FAILURE;
    }
    m_collectionIDs.push_back(idTable->collectionID(name));
  }
  return StatusCode::SUCCESS;
}

StatusCode PodioLegacyInput::execute() {
  size_t cntr = 0;
  // Re-create the collections from ROOT file
  for (auto& id : m_collectionIDs) {
    const std::string& collName = m_collectionNames.value().at(cntr++);
    debug() << "Registering collection to read " << collName << " with id " << id << endmsg;
    if (m_podioLegacyDataSvc->readCollection(collName, id).isFailure()) {
      return StatusCode::FAILURE;
    }
  }
  // Tell data service that we are done with requested collections
  m_podioLegacyDataSvc->endOfRead();
  return StatusCode::SUCCESS;
}

StatusCode PodioLegacyInput::finalize() {
  if (GaudiAlgorithm::finalize().isFailure())
    return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}
