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

#include "IOSvc.h"

#include "edm4hep/MCParticleCollection.h"
#include "podio/Frame.h"
#include "podio/FrameCategories.h"

#include "k4FWCore/KeepDropSwitch.h"

#include "GaudiKernel/AnyDataWrapper.h"

#include <mutex>
#include <thread>
#include <tuple>

StatusCode IOSvc::initialize() {
  if (!m_readingFileNames.empty()) {
    m_reader = std::make_unique<podio::ROOTFrameReader>();
    m_reader->openFiles(m_readingFileNames);
  }

  m_switch = KeepDropSwitch(m_outputCommands);

  m_incidentSvc = service("IncidentSvc");
  m_incidentSvc->addListener(this, IncidentType::EndEvent);

  m_dataSvc = service("EventDataSvc");

  return Service::initialize();
}

StatusCode IOSvc::finalize() { return Service::finalize(); }

std::tuple<std::vector<std::shared_ptr<podio::CollectionBase>>, std::vector<std::string>, podio::Frame> IOSvc::next() {
  info() << "IOSvc::next()" << endmsg;
  podio::Frame frame;
  {
    std::scoped_lock<std::mutex> lock(m_changeBufferLock);
    frame = podio::Frame(std::move(m_reader->readNextEntry(podio::Category::Event)));
    if (m_collectionNames.empty()) {
        m_collectionNames = frame.getAvailableCollections();
    }
  }

  std::vector<std::shared_ptr<podio::CollectionBase>> collections;

  for (const auto& name : m_collectionNames) {
    info() << "Collection name: " << name << endmsg;
    auto ptr = const_cast<podio::CollectionBase*>(frame.get(name));
    collections.push_back(std::shared_ptr<podio::CollectionBase>(ptr));
  }

  return std::make_tuple(collections, m_collectionNames, std::move(frame));
}

// After every event if there is still a frame in the TES
// that means it hasn't been written so the collections inside the Frame
// should be removed so that they are deleted when the Frame is deleted
void IOSvc::handle( const Incident& incident ) {
  DataObject *p;
  auto code = m_dataSvc->retrieveObject("/Event/_Frame", p);
  if (code.isFailure()) {
    return;
  }

  auto frame = dynamic_cast<AnyDataWrapper<podio::Frame>*>(p);
  for (const auto& coll : frame->getData().getAvailableCollections()) {
    DataObject *collPtr;
    code = m_dataSvc->retrieveObject("/Event/" + coll, collPtr);
    if (code.isSuccess()) {
      code = m_dataSvc->unregisterObject(collPtr);
    }
    // else {
    //   info() << "Collection not found: " << coll << endmsg;
    // }
  }
}

void IOSvc::setReadingCollectionNames(const std::vector<std::string>& names) {
  m_collectionNames = names;
}

void IOSvc::setReadingFileNames(const std::vector<std::string>& names) {
  m_readingFileNames = names;
}

DECLARE_COMPONENT(IOSvc)
