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

#include "k4FWCore/k4DataSvc.h"
#include "k4FWCore/DataWrapper.h"

#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IEventProcessor.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/StatusCode.h>

/// Standard Constructor
k4DataSvc::k4DataSvc(const std::string& name, ISvcLocator* svc) : DataSvc(name, svc) {
  declareProperty("inputs", m_filenames = {}, "Names of the files to read");
  declareProperty("input", m_filename = "", "Name of the file to read");
  declareProperty("FirstEventEntry", m_1stEvtEntry = 0, "First event to read");
}

/// Service initialisation
StatusCode k4DataSvc::initialize() {
  // Nothing to do: just call base class initialisation
  StatusCode   status  = DataSvc::initialize();
  ISvcLocator* svc_loc = serviceLocator();

  // Attach data loader facility
  m_cnvSvc = svc_loc->service("EventPersistencySvc");
  status   = setDataLoader(m_cnvSvc);

  if (m_filename != "") {
    m_filenames.push_back(m_filename);
  }

  if (m_filenames.size() > 0) {
    if (m_filenames[0] != "") {
      m_reading_from_file = true;
      m_reader.openFiles(m_filenames);
      m_numAvailableEvents = m_reader.getEntries("events");
      m_numAvailableEvents -= m_1stEvtEntry;
    }
  }

  if (m_reading_from_file) {
    if (auto metadata = m_reader.readEntry("metadata", 0)) {
      m_metadataframe = std::move(metadata);
    } else {
      warning() << "Reading file without a 'metadata' category." << endmsg;
      m_metadataframe = podio::Frame();
    }
  } else {
    m_metadataframe = podio::Frame();
  }

  IProperty* property;
  auto       sc = service("ApplicationMgr", property);
  if (sc == StatusCode::FAILURE) {
    error() << "Could not get ApplicationMgr properties" << std::endl;
  }
  Gaudi::Property<int> evtMax;
  evtMax.assign(property->getProperty("EvtMax"));
  m_requestedEventMax = evtMax;
  m_requestedEventMax -= m_1stEvtEntry;

  // if run with a fixed number of requested events and we have enough
  // in the file we don't need to check if we run out of events
  if (m_requestedEventMax > 0 && m_requestedEventMax <= m_numAvailableEvents) {
    m_bounds_check_needed = false;
  }

  return status;
}

/// Service reinitialisation
StatusCode k4DataSvc::reinitialize() {
  // Do nothing for this service
  return StatusCode::SUCCESS;
}

/// Service finalization
StatusCode k4DataSvc::finalize() {
  m_cnvSvc = 0;  // release
  DataSvc::finalize().ignore();
  return StatusCode::SUCCESS;
}

StatusCode k4DataSvc::clearStore() {
  // as the frame takes care of the ownership of the podio::Collections,
  // make sure the DataWrappers don't cause a double delete
  for (auto wrapper : m_podio_datawrappers) {
    wrapper->resetData();
  }
  m_podio_datawrappers.clear();

  DataSvc::clearStore().ignore();
  return StatusCode::SUCCESS;
}

StatusCode k4DataSvc::i_setRoot(std::string root_path, IOpaqueAddress* pRootAddr) {
  // create a new frame
  if (m_reading_from_file) {
    m_eventframe = podio::Frame(m_reader.readEntry("events", m_eventNum + m_1stEvtEntry));
  } else {
    m_eventframe = podio::Frame();
  }
  return DataSvc::i_setRoot(root_path, pRootAddr);
}

StatusCode k4DataSvc::i_setRoot(std::string root_path, DataObject* pRootObj) {
  // create a new frame
  if (m_reading_from_file) {
    m_eventframe = podio::Frame(m_reader.readEntry("events", m_eventNum + m_1stEvtEntry));
  } else {
    m_eventframe = podio::Frame();
  }
  return DataSvc::i_setRoot(root_path, pRootObj);
}

void k4DataSvc::endOfRead() {
  m_eventNum++;

  if (!m_bounds_check_needed) {
    return;
  }

  StatusCode sc;
  // m_eventNum already points to the next event here so check if it is available
  if (m_eventNum >= m_numAvailableEvents) {
    info() << "Reached end of file with event " << m_eventNum << " (" << m_requestedEventMax << " events requested)"
           << endmsg;
    IEventProcessor* eventProcessor;
    sc = service("ApplicationMgr", eventProcessor);
    sc = eventProcessor->stopRun();
  }
  // todo: figure out sthg to do with sc (added to silence -Wunused-result)
}

const std::string_view k4DataSvc::getCollectionType(const std::string& collName) {
  const auto coll = m_eventframe.get(collName);
  if (coll == nullptr) {
    error() << "Collection " << collName << " does not exist." << endmsg;
    return "";
  }
  return coll->getTypeName();
}

StatusCode k4DataSvc::registerObject(std::string_view parentPath, std::string_view fullPath, DataObject* pObject) {
  DataWrapperBase* wrapper = dynamic_cast<DataWrapperBase*>(pObject);
  if (wrapper != nullptr) {
    podio::CollectionBase* coll = wrapper->collectionBase();
    if (coll != nullptr) {
      size_t      pos = fullPath.find_last_of("/");
      std::string shortPath(fullPath.substr(pos + 1, fullPath.length()));
      // Attention: this passes the ownership of the data to the frame
      m_eventframe.put(std::unique_ptr<podio::CollectionBase>(coll), shortPath);
      m_podio_datawrappers.push_back(wrapper);
    }
  }
  return DataSvc::registerObject(parentPath, fullPath, pObject);
}
