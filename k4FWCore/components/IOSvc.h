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
#ifndef FWCORE_IOSVC_H
#define FWCORE_IOSVC_H

#include "Gaudi/Property.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Service.h"

#include "podio/Reader.h"
#include "podio/Writer.h"

#include "IIOSvc.h"
#include "k4FWCore/IMetadataSvc.h"
#include "k4FWCore/KeepDropSwitch.h"

#include <string>
#include <vector>

class IOSvc : public extends<Service, IIOSvc, IIncidentListener> {
  using extends::extends;

public:
  StatusCode initialize() override;
  StatusCode finalize() override;

  std::tuple<std::vector<podio::CollectionBase*>, std::vector<std::string>, podio::Frame> next() override;

  std::vector<std::string> getAvailableCollections() override;

protected:
  Gaudi::Property<std::vector<std::string>> m_collectionNames{
      this, "CollectionNames", {}, "List of collections to read"};
  Gaudi::Property<std::vector<std::string>> m_readingFileNames{this, "Input", {}, "List of files to read"};
  Gaudi::Property<std::string> m_writingFileName{this, "Output", {}, "List of files to write output to"};
  Gaudi::Property<std::vector<std::string>> m_outputCommands{
      this, "outputCommands", {"keep *"}, "A set of commands to declare which collections to keep or drop."};
  Gaudi::Property<std::string> m_outputType{this, "OutputType", "default",
                                            "Type of the output file (ROOT or RNTuple, or default)"};

  Gaudi::Property<bool> m_importedFromk4FWCore{
      this, "ImportedFromk4FWCore", false,
      "This is set to true when IOSvc is imported from k4FWCore instead of Configurables in python"};
  Gaudi::Property<int> m_firstEventEntry{this, "FirstEventEntry", 0, "First event entry to read"};

  std::mutex m_changeBufferLock;

  KeepDropSwitch m_switch;

  std::optional<podio::Reader> m_reader;
  std::optional<podio::Writer> m_writer;

  podio::Writer& getWriter() override {
    if (!m_writer) {
      m_writer = podio::makeWriter(m_writingFileName.value(), m_outputType);
    }
    return *m_writer;
  }

  // Gaudi doesn't always run the destructor of the Services so we have to
  // manually ask for the writer to be deleted so it will call finish()
  void deleteWriter() override { m_writer.reset(); }
  void deleteReader() override { m_reader.reset(); }

  SmartIF<IDataProviderSvc> m_dataSvc;
  SmartIF<IIncidentSvc> m_incidentSvc;
  SmartIF<IHiveWhiteBoard> m_hiveWhiteBoard;
  SmartIF<IMetadataSvc> m_metadataSvc;
  void handle(const Incident& incident) override;

  int m_entries{0};
  int m_nextEntry{0};

  bool checkIfWriteCollection(const std::string& collName) override;
};

#endif
