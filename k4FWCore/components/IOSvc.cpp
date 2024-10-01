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

#include "IOSvc.h"

#include "podio/Frame.h"
#include "podio/FrameCategories.h"
#include "podio/Reader.h"

#include "k4FWCore/FunctionalUtils.h"
#include "k4FWCore/KeepDropSwitch.h"

#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/IEventProcessor.h"

#include <mutex>
#include <tuple>

StatusCode IOSvc::initialize() {
  StatusCode sc = Service::initialize();
  if (sc.isFailure()) {
    error() << "Unable to initialize base class Service." << endmsg;
    return sc;
  }

  if (!m_importedFromk4FWCore) {
    error() << "Use 'from k4FWCore import IOSvc' instead of 'from Configurables import IOSvc' to access the service"
            << endmsg;
  }
  if (m_outputType != "default" && m_outputType != "ROOT" && m_outputType != "RNTuple") {
    error() << "Unknown input type: " << m_outputType << ", expected ROOT, RNTuple or default" << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_readingFileNamesDeprecated.empty()) {
    warning() << ".input is deprecated, use .Input instead in the steering file" << endmsg;
    m_readingFileNames = m_readingFileNamesDeprecated;
  }
  if (!m_readingFileNames.empty()) {
    try {
      m_reader = podio::makeReader(m_readingFileNames.value());
    } catch (std::runtime_error& e) {
      error() << "Error when opening files: " << e.what() << endmsg;
      return StatusCode::FAILURE;
    }
    m_entries = m_reader->getEvents();
  }

  if ((m_entries && m_firstEventEntry >= m_entries) || m_firstEventEntry < 0) {
    error() << "First event entry is larger than the number of entries in the file or negative" << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_writingFileNameDeprecated.empty()) {
    warning() << ".output is deprecated, use .Output instead in the steering file" << endmsg;
    m_writingFileName = m_writingFileNameDeprecated;
  }
  m_nextEntry = m_firstEventEntry;

  m_switch = KeepDropSwitch(m_outputCommands);

  m_incidentSvc = service("IncidentSvc");
  if (!m_incidentSvc) {
    error() << "Unable to locate IIncidentSvc interface" << endmsg;
    return StatusCode::FAILURE;
  }
  m_incidentSvc->addListener(this, IncidentType::EndEvent);

  m_dataSvc = service("EventDataSvc");
  if (!m_dataSvc) {
    error() << "Unable to locate the EventDataSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  m_metadataSvc = service("MetadataSvc");
  if (!m_metadataSvc) {
    error() << "Unable to locate the MetadataSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_reader) {
    auto categories = m_reader->getAvailableCategories();
    if (std::find(categories.begin(), categories.end(), podio::Category::Metadata) != categories.end() &&
        m_reader->getEntries(podio::Category::Metadata) > 0) {
      info() << "Setting metadata frame" << endmsg;
      m_metadataSvc->setFrame(m_reader->readFrame(podio::Category::Metadata, 0));
    }
  }

  m_hiveWhiteBoard = service("EventDataSvc");

  return StatusCode::SUCCESS;
}

StatusCode IOSvc::finalize() { return Service::finalize(); }

std::tuple<std::vector<std::shared_ptr<podio::CollectionBase>>, std::vector<std::string>, podio::Frame> IOSvc::next() {
  podio::Frame frame;
  {
    std::scoped_lock<std::mutex> lock(m_changeBufferLock);
    if (m_nextEntry < m_entries) {
      frame = podio::Frame(m_reader->readEvent(m_nextEntry));
    } else {
      return std::make_tuple(std::vector<std::shared_ptr<podio::CollectionBase>>(), std::vector<std::string>(),
                             std::move(frame));
    }
    m_nextEntry++;
    if (m_collectionNames.empty()) {
      m_collectionNames = frame.getAvailableCollections();
    }
  }

  if (m_nextEntry >= m_entries) {
    auto       ep = serviceLocator()->as<IEventProcessor>();
    StatusCode sc = ep->stopRun();
    if (sc.isFailure()) {
      error() << "Error when stopping run" << endmsg;
      throw GaudiException("Error when stopping run", name(), StatusCode::FAILURE);
    }
  }

  std::vector<std::shared_ptr<podio::CollectionBase>> collections;

  for (const auto& name : m_collectionNames) {
    auto ptr = const_cast<podio::CollectionBase*>(frame.get(name));
    collections.push_back(std::shared_ptr<podio::CollectionBase>(ptr));
  }

  return std::make_tuple(collections, m_collectionNames, std::move(frame));
}

// After every event if there is still a frame in the TES
// that means it hasn't been written so the collections inside the Frame
// should be removed so that they are deleted when the Frame is deleted
// and not deleted when clearing the store
void IOSvc::handle(const Incident& incident) {
  StatusCode code;
  if (m_hiveWhiteBoard) {
    if (!incident.context().valid()) {
      info() << "No context found in IOSvc" << endmsg;
      return;
    }
    debug() << "Setting store to " << incident.context().slot() << endmsg;
    code = m_hiveWhiteBoard->selectStore(incident.context().slot());
    if (code.isFailure()) {
      error() << "Error when setting store" << endmsg;
      throw GaudiException("Error when setting store", name(), StatusCode::FAILURE);
    }
  }
  DataObject* p;
  code = m_dataSvc->retrieveObject("/Event" + k4FWCore::frameLocation, p);
  if (code.isFailure()) {
    return;
  }

  auto frame = dynamic_cast<AnyDataWrapper<podio::Frame>*>(p);
  if (!frame) {
    error() << "Expected Frame in " << k4FWCore::frameLocation << " but there was something else" << endmsg;
    return;
  }
  for (const auto& coll : frame->getData().getAvailableCollections()) {
    DataObject* collPtr;
    code = m_dataSvc->retrieveObject("/Event/" + coll, collPtr);
    if (code.isSuccess()) {
      debug() << "Removing the collection: " << coll << " from the store" << endmsg;
      code = m_dataSvc->unregisterObject(collPtr);
    } else {
      error() << "Expected collection " << coll << " in the store but it was not found" << endmsg;
    }
  }
}

void IOSvc::setReadingCollectionNames(const std::vector<std::string>& names) { m_collectionNames = names; }

void IOSvc::setReadingFileNames(const std::vector<std::string>& names) { m_readingFileNames = names; }

bool IOSvc::checkIfWriteCollection(const std::string& collName) { return m_switch.isOn(collName); }

DECLARE_COMPONENT(IOSvc)
