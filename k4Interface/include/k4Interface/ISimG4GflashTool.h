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
#ifndef SIMG4INTERFACE_ISIMG4GFLASHTOOL_H
#define SIMG4INTERFACE_ISIMG4GFLASHTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// Geant
#include "GVFlashShowerParameterisation.hh"

/** @class ISimG4GflashTool SimG4Interface/SimG4Interface/ISimG4GflashTool.h ISimG4GflashTool.h
 *
 *  Interface to the Gflash parametrisation tool.
 *  It returns the parametriation that should be attached to the GFlashShowerModel.
 *
 *  @author Anna Zaborowska
 */

class ISimG4GflashTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ISimG4GflashTool, 1, 0);

  /**  Get the parametrisation
   *   @return unique pointer to the parametrisation
   */
  virtual std::unique_ptr<GVFlashShowerParameterisation> parametrisation() = 0;
};
#endif /* SIMG4INTERFACE_ISIMG4GFLASHTOOL_H */
