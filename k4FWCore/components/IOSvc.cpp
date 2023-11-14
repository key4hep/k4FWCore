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

#include <mutex>
#include <thread>
#include <tuple>

StatusCode IOSvc::initialize() {
  m_reader = std::make_unique<podio::ROOTFrameReader>();
  m_reader->openFiles(m_fileNames);

  m_switch = KeepDropSwitch(m_outputCommands);

  return Service::initialize();
}

StatusCode IOSvc::finalize() {

  return Service::finalize();
}

std::vector<std::shared_ptr<podio::CollectionBase>> IOSvc::next() {
  info() << "IOSvc::next()" << endmsg;
  podio::Frame* frame;
  {
    std::scoped_lock<std::mutex> lock(m_changeBufferLock);
    frame = new podio::Frame(std::move(m_reader->readNextEntry(podio::Category::Event)));
  }

  std::vector<std::shared_ptr<podio::CollectionBase>> collections;

  for (const auto& name : m_collectionNames) {
    info() << "Collection name: " << name << endmsg;
    auto ptr = const_cast<podio::CollectionBase*>(frame->get(name));
    collections.push_back(std::shared_ptr<podio::CollectionBase>(ptr));
  }

  return collections;
}

DECLARE_COMPONENT(IOSvc)
