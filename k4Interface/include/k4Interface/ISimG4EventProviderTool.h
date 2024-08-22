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
#ifndef SIMG4INTERFACE_ISIMG4EVENTPROVIDERTOOL_H
#define SIMG4INTERFACE_ISIMG4EVENTPROVIDERTOOL_H

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

class G4Event;

/** @class ISimG4EventProviderTool SimG4Interface/ISimG4EventProviderTool.h ISimG4EventProviderTool.h
 *
 *  Abstract interface to Geant4 Primary Generator classes
 *
 *  @author Andrea Dell'Acqua, J. Lingemann
 *  @date   2014-10-02
 */

class ISimG4EventProviderTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ISimG4EventProviderTool, 1, 0);

  /** get initialization hook for the geometry
   *  @return pointer to the G4Event containing primary particles
   */
  virtual G4Event* g4Event() = 0;
};

#endif /* SIMG4INTERFACE_ISIMG4EVENTPROVIDERTOOL_H */
