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

#include "podio/ROOTFrameReader.h"
#include "podio/ROOTFrameWriter.h"
#include "podio/ROOTNTupleReader.h"
#include "podio/ROOTNTupleWriter.h"

#include "k4FWCore/KeepDropSwitch.h"

#include "IIOSvc.h"

#include <string>
#include <type_traits>
#include <vector>

class IOSvc : public extends<Service, IIOSvc> {
  using extends::extends;
public:
  // Gaudi doesn't run the destructor of the Services so we have to
  // manually ask for the writer to be deleted so it will call finish()
  ~IOSvc() override = default;

  StatusCode initialize() override;
  StatusCode finalize() override;

  std::vector<std::shared_ptr<podio::CollectionBase>> next() override;

  std::shared_ptr<std::vector<std::string>> getCollectionNames() const override {
    return std::make_shared<std::vector<std::string>>(m_collectionNames);
  }

protected:
  Gaudi::Property<std::vector<std::string>> m_collectionNames{this, "CollectionNames", {}, "List of files to read"};
  Gaudi::Property<std::vector<std::string>> m_fileNames{this, "FileNames", {}, "List of files to read"};
  Gaudi::Property<unsigned int> m_bufferNbEvents{
      this, "BufferNbEvents", 20000,
      "approximate size of the buffer used to prefetch rawbanks in terms of number of events. Default is 20000"};
  Gaudi::Property<std::vector<std::string>> m_input{this, "Input", {}, "List of inputs"};
  Gaudi::Property<unsigned int> m_nbSkippedEvents{this, "NSkip", 0, "First event to process"};
  Gaudi::Property<std::string> m_output{this, "Output", "output.root", "Output file name"};
  Gaudi::Property<std::vector<std::string>> m_outputCommands{
      this, "outputCommands", {"keep *"}, "A set of commands to declare which collections to keep or drop."};

  /// lock for handling the change of buffer
  std::mutex m_changeBufferLock;

  KeepDropSwitch m_switch;

  // To be changed by a base class to allow to use different readers
  std::unique_ptr<podio::ROOTFrameReader> m_reader{nullptr};
  std::shared_ptr<podio::ROOTFrameWriter> m_writer{nullptr};

  std::shared_ptr<podio::ROOTFrameWriter> getWriter() override {
    if (!m_writer) {
      m_writer = std::shared_ptr<podio::ROOTFrameWriter>(new podio::ROOTFrameWriter(m_output));
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
};

#endif
