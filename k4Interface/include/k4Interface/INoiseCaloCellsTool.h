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
#ifndef RECINTERFACE_INOISECALOCELLSTOOL_H
#define RECINTERFACE_INOISECALOCELLSTOOL_H

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

/** @class INoiseCaloCellsTool
 *
 *  Abstract interface for tools that add or filter noise to calorimeter cells
 *
 *  @author Jana Faltova
 *  @date   2016-09
 */

class INoiseCaloCellsTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(INoiseCaloCellsTool, 1, 0);

  virtual void addRandomCellNoise(std::unordered_map<uint64_t, double>& aCells) = 0;
  virtual void filterCellNoise(std::unordered_map<uint64_t, double>& aCells) = 0;
};

#endif /* RECINTERFACE_INOISECALOCELLSTOOL_H */
