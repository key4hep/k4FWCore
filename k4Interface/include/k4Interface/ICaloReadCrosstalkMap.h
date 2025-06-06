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
#ifndef RECINTERFACE_ICALOREADCROSSTALKMAP_H
#define RECINTERFACE_ICALOREADCROSSTALKMAP_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

/** @class ICaloReadCrosstalkMap k4Interface/include/k4Interface/ICaloReadCrosstalkMap.h ICaloReadCrosstalkMap.h
 *
 *  Interface to the service reading the crosstalk map for ALLEGRO ECAL barrel.
 *
 *  @author Zhibo Wu
 */

class ICaloReadCrosstalkMap : virtual public IAlgTool {
public:
  DeclareInterfaceID(ICaloReadCrosstalkMap, 1, 0);

  virtual std::vector<uint64_t> const& getNeighbours(uint64_t cellID) = 0;
  virtual std::vector<double> const& getCrosstalks(uint64_t cellID) = 0;
};
#endif /* RECINTERFACE_ICALOREADCROSSTALKMAP_H */
