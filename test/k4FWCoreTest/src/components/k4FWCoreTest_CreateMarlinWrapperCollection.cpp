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
#include "k4FWCoreTest_CreateMarlinWrapperCollection.h"

#include "edm4hep/MCParticleCollection.h"
#include "podio/CollectionBase.h"

#include "k4FWCore/DataWrapper.h"

k4FWCoreTest_CreateMarlinWrapperCollection::k4FWCoreTest_CreateMarlinWrapperCollection(const std::string& aName,
                                                                                       ISvcLocator* aSvcLoc)
    : Gaudi::Algorithm(aName, aSvcLoc) {}

StatusCode k4FWCoreTest_CreateMarlinWrapperCollection::initialize() {
  m_eventDataSvc = service("EventDataSvc");
  if (!m_eventDataSvc) {
    error() << "Could not retrieve EventDataSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_CreateMarlinWrapperCollection::execute(const EventContext&) const {
  auto wrapper = new DataWrapper<podio::CollectionBase>();
  auto coll = new edm4hep::MCParticleCollection();

  coll->create(1, 2, 3, 4.f, 5.f, 6.f);
  // We can also set the fields of the particle manually
  auto particle = coll->create(2, 3, 4, 5.f, 6.f, 7.f);
  particle.setPDG(2);
  particle.setGeneratorStatus(3);
  particle.setSimulatorStatus(4);
  particle.setCharge(5.f);
  particle.setTime(6.f);
  particle.setMass(7.f);

  wrapper->setData(coll);
  auto sc = m_eventDataSvc->registerObject("/Event/" + m_outputCollectionName, wrapper);
  if (!sc) {
    error() << "Could not register collection " << endmsg;
  }

  return StatusCode::SUCCESS;
}

DECLARE_COMPONENT(k4FWCoreTest_CreateMarlinWrapperCollection)
