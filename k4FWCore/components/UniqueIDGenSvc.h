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
#ifndef FWCORE_UNIQUEIDGENSVC_H
#define FWCORE_UNIQUEIDGENSVC_H

#include <cstdint>
#include <string>

#include <GaudiKernel/Service.h>
#include "k4Interface/IUniqueIDGenSvc.h"

/** @class UniqueIDGenSvc
 *  Generate unique, reproducible numbers using
 *  event number, run number, algorithm name, and global seed.
 *  To be used as reproducible per event seeds for random number generators used inside algorithms.
 *
 */
class UniqueIDGenSvc : public extends<Service, IUniqueIDGenSvc> {
public:
  UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc);
  StatusCode initialize() override;
  size_t     getUniqueID(uint32_t evt_num, uint32_t run_num, const std::string& name) const override;

private:
  Gaudi::Property<int64_t> m_seed{this, "Seed", {123456789}};
};

#endif
