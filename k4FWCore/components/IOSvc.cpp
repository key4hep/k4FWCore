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
  if (!m_readingFileNames.empty()) {
    m_reader = std::make_unique<podio::ROOTReader>();
    try {
      m_reader->openFiles(m_readingFileNames);
    } catch (std::runtime_error& e) {
      error() << "Error when opening files: " << e.what() << endmsg;
      return StatusCode::FAILURE;
    }
    m_entries = m_reader->getEntries(podio::Category::Event);
  }

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

  m_hiveWhiteBoard = service("EventDataSvc");

  return StatusCode::SUCCESS;
}

StatusCode IOSvc::finalize() { return Service::finalize(); }

std::tuple<std::vector<std::shared_ptr<podio::CollectionBase>>, std::vector<std::string>, podio::Frame> IOSvc::next() {
  podio::Frame frame;
  {
    std::scoped_lock<std::mutex> lock(m_changeBufferLock);
    info() << "m_nextEntry = " << m_nextEntry << " m_entries = " << m_entries << endmsg;
    if (m_nextEntry < m_entries) {
      frame = podio::Frame(std::move(m_reader->readEntry(podio::Category::Event, m_nextEntry)));
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
    // if (true) {
    auto       ep = serviceLocator()->as<IEventProcessor>();
    StatusCode sc = ep->stopRun();
    if (sc.isFailure()) {
      error() << "Error when stopping run" << endmsg;
      throw GaudiException("Error when stopping run", name(), StatusCode::FAILURE);
    }
    info() << "m_nextEntry = " << m_nextEntry << " m_entries = " << m_entries << endmsg;
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
    }
    // else {
    //   info() << "Collection not found: " << coll << endmsg;
    // }
  }
}

void IOSvc::setReadingCollectionNames(const std::vector<std::string>& names) { m_collectionNames = names; }

void IOSvc::setReadingFileNames(const std::vector<std::string>& names) { m_readingFileNames = names; }

bool IOSvc::checkIfWriteCollection(const std::string& collName) { return m_switch.isOn(collName); }

DECLARE_COMPONENT(IOSvc)
