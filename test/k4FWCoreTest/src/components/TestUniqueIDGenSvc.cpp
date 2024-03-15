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

DECLARE_COMPONENT(TestUniqueIDGenSvc)

TestUniqueIDGenSvc::TestUniqueIDGenSvc(const std::string& aName, ISvcLocator* aSvcLoc)
    : Gaudi::Algorithm(aName, aSvcLoc) {}

TestUniqueIDGenSvc::~TestUniqueIDGenSvc() {}

StatusCode TestUniqueIDGenSvc::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }

  m_service = serviceLocator()->service("UniqueIDGenSvc");

  return StatusCode::SUCCESS;
}

StatusCode TestUniqueIDGenSvc::execute(const EventContext&) const {
  uint        evt_num = 4;
  uint        run_num = 3;
  std::string name    = "Some algorithm name";

  auto uid       = m_service->getUniqueID(evt_num, run_num, name);
  auto uid_again = m_service->getUniqueID(evt_num, run_num, name);

  if (uid != uid_again) {
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode TestUniqueIDGenSvc::finalize() { return Gaudi::Algorithm::finalize(); }
