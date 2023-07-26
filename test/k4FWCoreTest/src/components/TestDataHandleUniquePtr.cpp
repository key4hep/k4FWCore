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
#include "TestDataHandleUniquePtr.h"

// datamodel
#include "edm4hep/MCParticleCollection.h"

DECLARE_COMPONENT(TestDataHandleUniquePtr)

TestDataHandleUniquePtr::TestDataHandleUniquePtr(const std::string& aName, ISvcLocator* aSvcLoc)
    : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("mcparticles", m_mcParticleHandle, "Dummy Particle collection (output)");
}

TestDataHandleUniquePtr::~TestDataHandleUniquePtr() {}

StatusCode TestDataHandleUniquePtr::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode TestDataHandleUniquePtr::execute() {
  auto coll = std::make_unique<edm4hep::MCParticleCollection>();

  auto particle = coll->create();

  auto& p4 = particle.momentum();
  p4.x     = 5;
  p4.y     = 6;
  p4.z     = 7;
  particle.setMass(8);

  m_mcParticleHandle.put(std::move(coll));

  return StatusCode::SUCCESS;
}

StatusCode TestDataHandleUniquePtr::finalize() { return GaudiAlgorithm::finalize(); }
