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

#include <k4FWCore/Consumer.h>
#include <k4FWCore/ICollectionFromObjectSvc.h>

#include <edm4hep/MCParticleCollection.h>

struct TestCollectionFromObjectRetrieval final : k4FWCore::Consumer<void(const edm4hep::MCParticleCollection&)> {
  TestCollectionFromObjectRetrieval(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, {KeyValues("InputCollection", {"MCParticles"})}) {}

  StatusCode initialize() final {
    m_collFromObjSvc = service("CollectionFromObjSvc", false);
    if (!m_collFromObjSvc) {
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  void operator()(const edm4hep::MCParticleCollection& inputColl) const final {
    const auto mc = inputColl[0];
    debug() << "Retrieving collection for object with id " << mc.id() << endmsg;
    const auto* collFromObj = m_collFromObjSvc->getCollectionFor(mc);
    const auto checkMC = (*collFromObj)[0];
    if (mc != checkMC) {
      throw std::runtime_error("Could not get the expected collection from the object");
    }

    auto newMCParticle = edm4hep::MutableMCParticle{};
    const auto* invalidColl = m_collFromObjSvc->getCollectionFor(newMCParticle);
    if (invalidColl) {
      throw std::runtime_error("Could get a collection for an object that is not in a collection");
    }

    auto newCollection = edm4hep::MCParticleCollection();
    newCollection->push_back(newMCParticle);
    const auto* stillInvalidColl = m_collFromObjSvc->getCollectionFor(newMCParticle);
    if (stillInvalidColl) {
      throw std::runtime_error("Could get a collection for an object that is not in a collection");
    }
  }

private:
  SmartIF<ICollectionFromObjectSvc> m_collFromObjSvc;
};

DECLARE_COMPONENT(TestCollectionFromObjectRetrieval)
