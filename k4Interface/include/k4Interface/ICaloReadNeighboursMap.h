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
#ifndef RECINTERFACE_ICALOREADNEIGHBOURSMAP_H
#define RECINTERFACE_ICALOREADNEIGHBOURSMAP_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

/** @class ICaloReadNeighboursMap RecInterface/RecInterface/ICaloReadNeighboursMap.h ICaloReadNeighboursMap.h
 *
 *  Interface to the service creating a map for the calorimetry.
 *
 *  @author Anna Zaborowska
 */

class ICaloReadNeighboursMap : virtual public IAlgTool {
public:
  DeclareInterfaceID(ICaloReadNeighboursMap, 1, 0);

  virtual std::vector<uint64_t>& neighbours(uint64_t aCellId) = 0;
};
#endif /* RECINTERFACE_ICALOREADNEIGHBOURSMAP_H */
