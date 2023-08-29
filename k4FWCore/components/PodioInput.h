/*
 * Copyright (c) 2014-2023 Key4hep-Project.
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
#ifndef FWCORE_PODIOINPUT_H
#define FWCORE_PODIOINPUT_H
// Gaaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// STL
#include <string>
#include <vector>

// forward declarations
// from k4FWCore:
class PodioDataSvc;

/** @class PodioInput
 *
 *  Class that allows to read ROOT files written with PodioOutput
 *
 *  @author J. Lingemann
 */

class PodioInput : public GaudiAlgorithm {
public:
  /// Constructor.
  PodioInput(const std::string& name, ISvcLocator* svcLoc);
  /// Initialization of PodioInput. Acquires the data service, opens root file and creates trees.
  virtual StatusCode initialize();
  /// Execute. Re-creates collections that are specified to be read and sets references.
  virtual StatusCode execute();
  /// Finalize. Closes ROOT file.
  virtual StatusCode finalize();

private:
  /// Name of collections to read. Set by option collections (this is temporary)
  Gaudi::Property<std::vector<std::string>> m_collectionNames{this, "collections", {}, "Places of collections to read"};
  /// Data service: needed to register objects and get collection IDs. Just an observing pointer.
  PodioDataSvc* m_podioDataSvc;
};

#endif
