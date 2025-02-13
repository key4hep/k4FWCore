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
#ifndef FWCORE_IUNIQUEIDGENSVC_H
#define FWCORE_IUNIQUEIDGENSVC_H

#include "edm4hep/EventHeader.h"

#include <cstddef>
#include <string>

#include <GaudiKernel/IInterface.h>

/** @class IUniqueIDGenSvc
 *
 *  Interface to the UniqueIDGenSvc
 *
 *  @author Placido Fernandez Declara
 */

struct GAUDI_API IUniqueIDGenSvc : extend_interfaces<IInterface> {
  using event_num_t = decltype(std::declval<edm4hep::EventHeader>().getEventNumber());
  using run_num_t   = decltype(std::declval<edm4hep::EventHeader>().getRunNumber());
  DeclareInterfaceID(IUniqueIDGenSvc, 2, 0);
  virtual size_t getUniqueID(event_num_t evt_num, run_num_t run_num, const std::string& name) const = 0;
};

#endif
