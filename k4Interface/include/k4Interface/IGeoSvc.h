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

#ifndef IGEOSVC_H
#define IGEOSVC_H

#include "GaudiKernel/IService.h"

namespace dd4hep {
  class Detector;
  class DetElement;
}  // namespace dd4hep

class G4VUserDetectorConstruction;

class GAUDI_API IGeoSvc : virtual public IService {
public:
  DeclareInterfaceID(IGeoSvc, 1, 0);
  virtual dd4hep::DetElement                                            getDD4HepGeo()                            = 0;
  virtual dd4hep::Detector*                                             getDetector()                             = 0;
  virtual G4VUserDetectorConstruction*                                  getGeant4Geo()                            = 0;
  virtual std::string                                                   constantAsString(std::string const& name) = 0;
  virtual ~IGeoSvc() {}
};

#endif  // IGEOSVC_H
