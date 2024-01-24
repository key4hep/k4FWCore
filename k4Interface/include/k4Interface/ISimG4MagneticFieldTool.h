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
#ifndef SIMG4INTERFACE_ISIMG4MAGNETICFIELDTOOL_H
#define SIMG4INTERFACE_ISIM4MAGNETICFIELDTOOL_H

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

// Geant 4:
#include "G4MagneticField.hh"

/** @class ISimG4MagneticFieldTool SimG4Interface/SimG4Interface/ISimG4MagneticFieldTool.h ISimG4MagneticFieldTool.h
 *
 *  Abstract interface to Geant4 field classes
 *
 *  @author Andrea Dell'Acqua
 *  @date   2016-02-22
 */

// FIXME this tool should rather be a MagneticFieldSvc since it deals with G4 singletons

class ISimG4MagneticFieldTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ISimG4MagneticFieldTool, 1, 0);

  /** get initialization hook for the magnetic field
   *  @return pointer to G4MagneticField
   */
  virtual const G4MagneticField* field() const = 0;
};

#endif /* SIMG4INTERFACE_ISIM4MAGNETICFIELDTOOL_H */
