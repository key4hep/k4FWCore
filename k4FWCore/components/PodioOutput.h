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
#ifndef FWCORE_PODIOOUTPUT_H
#define FWCORE_PODIOOUTPUT_H

#include "Gaudi/Algorithm.h"
#include "k4FWCore/KeepDropSwitch.h"
#include "podio/CollectionBase.h"
#include "podio/podioVersion.h"
#if PODIO_BUILD_VERSION >= PODIO_VERSION(0, 99, 0)
#include "podio/ROOTWriter.h"
#else
#include "podio/ROOTFrameWriter.h"
namespace podio {
  using ROOTWriter = podio::ROOTFrameWriter;
}
#endif

#include <vector>

// forward declarations
class k4DataSvc;

class PodioOutput : public Gaudi::Algorithm {
public:
  /// Constructor.
  PodioOutput(const std::string& name, ISvcLocator* svcLoc);

  /// Initialization of PodioOutput. Acquires the data service, creates trees and root file.
  StatusCode initialize();
  /// Execute. For the first event creates branches for all collections known to PodioDataSvc and prepares them for
  /// writing. For the following events it reconnects the branches with collections and prepares them for write.
  StatusCode execute(const EventContext&) const;
  /// Finalize. Writes the meta data tree; writes file and cleans up all ROOT-pointers.
  StatusCode finalize();

private:
  /// First event or not
  mutable bool m_firstEvent;
  /// Root file name the output is written to
  Gaudi::Property<std::string> m_filename{this, "filename", "output.root", "Name of the file to create"};
  /// Commands which output is to be kept
  Gaudi::Property<std::vector<std::string>> m_outputCommands{
      this, "outputCommands", {"keep *"}, "A set of commands to declare which collections to keep or drop."};
  Gaudi::Property<std::string> m_filenameRemote{this, "remoteFilename", "",
                                                "An optional file path to copy the outputfile to."};
  /// Switch for keeping or dropping outputs
  KeepDropSwitch m_switch;
  k4DataSvc*     m_podioDataSvc;
  /// The actual ROOT frame writer
  std::unique_ptr<podio::ROOTWriter> m_framewriter;
  /// The stored collections
  std::vector<podio::CollectionBase*> m_storedCollections;
  /// The collections to write out
  mutable std::vector<std::string> m_collection_names_to_write;
};

#endif
