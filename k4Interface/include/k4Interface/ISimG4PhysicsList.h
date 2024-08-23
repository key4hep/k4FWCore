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
#ifndef SIMG4INTERFACE_ISIMG4PHYSICSLIST_H
#define SIMG4INTERFACE_ISIMG4PHYSICSLIST_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// Geant4
class G4VModularPhysicsList;

/** @class ISimG4PhysicsList SimG4Interface/SimG4Interface/ISimG4PhysicsList.h ISimG4PhysicsList.h
 *
 *  abstract interface to load physics lists
 *
 *  @author Benedikt HEGNER
 */

class ISimG4PhysicsList : virtual public IAlgTool {
public:
  /// Retrieve interface ID
  DeclareInterfaceID(ISimG4PhysicsList, 1, 0);

  /** get initialization hook for the physics list
   *  @return  pointer to G4VModularPhysicsList
   */
  virtual G4VModularPhysicsList* physicsList() = 0;
};

#endif /* SIMG4INTERFACE_ISIMG4PHYSICSLIST_H */
