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
#ifndef FWCORE_IOSVC_H
#define FWCORE_IOSVC_H

#include "Gaudi/Property.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include "podio/ROOTFrameReader.h"
#include "podio/ROOTFrameWriter.h"
#include "podio/ROOTRNTupleReader.h"
#include "podio/ROOTRNTupleWriter.h"
#include "podio/IROOTFrameReader.h"
#include "podio/IROOTFrameWriter.h"

#include "k4FWCore/KeepDropSwitch.h"

#include "IIOSvc.h"

#include <string>
#include <type_traits>
#include <vector>

class IOSvc : public extends<Service, IIOSvc, IIncidentListener> {
  using extends::extends;
public:
  // Gaudi doesn't run the destructor of the Services so we have to
  // manually ask for the writer to be deleted so it will call finish()
  ~IOSvc() override = default;

  StatusCode initialize() override;
  StatusCode finalize() override;

  std::tuple<std::vector<std::shared_ptr<podio::CollectionBase>>, std::vector<std::string>, podio::Frame> next() override;

  std::shared_ptr<std::vector<std::string>> getCollectionNames() const override {
    return std::make_shared<std::vector<std::string>>(m_collectionNames);
  }

  void setReadingCollectionNames(const std::vector<std::string>& names);
  void setReadingFileNames(const std::vector<std::string>& names);

protected:

  Gaudi::Property<unsigned int> m_bufferNbEvents{
      this, "BufferNbEvents", 20000,
      "approximate size of the buffer used to prefetch rawbanks in terms of number of events. Default is 20000"};
  Gaudi::Property<unsigned int> m_nbSkippedEvents{this, "NSkip", 0, "First event to process"};

  Gaudi::Property<std::vector<std::string>> m_collectionNames{this, "CollectionNames", {}, "List of files to read"};
  Gaudi::Property<std::vector<std::string>> m_readingFileNames{this, "input", {}, "List of files to read"};
  Gaudi::Property<std::string> m_writingFileName{this, "output", {}, "List of files to read"};
  Gaudi::Property<std::vector<std::string>> m_outputCommands{
      this, "outputCommands", {"keep *"}, "A set of commands to declare which collections to keep or drop."};

  /// lock for handling the change of buffer
  std::mutex m_changeBufferLock;

  KeepDropSwitch m_switch;

  std::unique_ptr<podio::IROOTFrameReader> m_reader{nullptr};
  std::shared_ptr<podio::ROOTFrameWriter> m_writer{nullptr};

  std::shared_ptr<podio::ROOTFrameWriter> getWriter() override {
    if (!m_writer) {
      m_writer = std::shared_ptr<podio::ROOTFrameWriter>(new podio::ROOTFrameWriter(m_writingFileName.value()));
    }
    return m_writer;
  }

  void deleteWriter() override {
    if (m_writer) {
      m_writer = nullptr;
    }
  }
  void deleteReader() override {
    if (m_reader) {
      m_reader = nullptr;
    }
  }

  SmartIF<IDataProviderSvc> m_dataSvc;
  SmartIF<IIncidentSvc> m_incidentSvc;
  void handle(const Incident& incident) override;
};

#endif
