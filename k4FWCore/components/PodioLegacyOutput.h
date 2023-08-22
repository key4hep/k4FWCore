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
#ifndef FWCORE_PODIOLEGACYOUTPUT_H
#define FWCORE_PODIOLEGACYOUTPUT_H

#include "GaudiAlg/GaudiAlgorithm.h"
#include "k4FWCore/KeepDropSwitch.h"
#include "podio/CollectionBase.h"

#include "TTree.h"

#include <vector>

// forward declarations
class TFile;
class PodioLegacyDataSvc;

class PodioLegacyOutput : public GaudiAlgorithm {
public:
  /// Constructor.
  PodioLegacyOutput(const std::string& name, ISvcLocator* svcLoc);

  /// Initialization of PodioOutput. Acquires the data service, creates trees and root file.
  virtual StatusCode initialize();
  /// Execute. For the first event creates branches for all collections known to PodioDataSvc and prepares them for
  /// writing. For the following events it reconnects the branches with collections and prepares them for write.
  virtual StatusCode execute();
  /// Finalize. Writes the meta data tree; writes file and cleans up all ROOT-pointers.
  virtual StatusCode finalize();

private:
  void resetBranches(const std::vector<std::pair<std::string, podio::CollectionBase*>>& collections);
  void createBranches(const std::vector<std::pair<std::string, podio::CollectionBase*>>& collections);
  /// First event or not
  bool m_firstEvent;
  /// Root file name the output is written to
  Gaudi::Property<std::string> m_filename{this, "filename", "output.root", "Name of the file to create"};
  /// Commands which output is to be kept
  Gaudi::Property<std::vector<std::string>> m_outputCommands{
      this, "outputCommands", {"keep *"}, "A set of commands to declare which collections to keep or drop."};
  Gaudi::Property<std::string> m_filenameRemote{this, "remoteFilename", "",
                                                "An optional file path to copy the outputfile to."};
  /// Switch for keeping or dropping outputs
  KeepDropSwitch m_switch;
  /// Needed for collection ID table
  PodioLegacyDataSvc* m_podioLegacyDataSvc;
  /// The actual ROOT file
  std::unique_ptr<TFile> m_file;
  /// The tree to be filled with collections
  TTree* m_datatree;
  /// The tree to be filled with meta data
  TTree* m_metadatatree;
  TTree* m_runMDtree;
  TTree* m_evtMDtree;
  TTree* m_colMDtree;

  /// The stored collections
  std::vector<podio::CollectionBase*> m_storedCollections;
};

#endif
