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
#include "CollectionFromObjectSvc.h"

#include "k4FWCore/FunctionalUtils.h"

#include <podio/CollectionBase.h>
#include <podio/ObjectID.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

template <>
struct fmt::formatter<podio::ObjectID> : ostream_formatter {};

StatusCode CollectionFromObjectSvc::initialize() {
  auto sc = Service::initialize();
  if (sc.isFailure()) {
    error() << "Unable to initialize base class Serivce." << endmsg;
    return sc;
  }
  m_dataSvc = service("EventDataSvc");
  if (!m_dataSvc) {
    error() << "Unable to locate the EventDataSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

const std::optional<std::string> CollectionFromObjectSvc::getCollectionNameFor(const podio::ObjectID id) const {
  debug() << "Trying to retrieve collection name for object " << id << endmsg;
  const auto& idTable = k4FWCore::details::getTESCollectionIDTable(m_dataSvc, this);
  auto name = idTable.name(id.collectionID);
  if (!name.has_value()) {
    error() << "Could not get a collection name for object " << id << endmsg;
    return std::nullopt;
  }
  return name.value();
}

const podio::CollectionBase* CollectionFromObjectSvc::getCollectionFor(const podio::ObjectID id) const {
  debug() << "Trying to retrieve collection for object " << id << endmsg;
  const auto& idTable = k4FWCore::details::getTESCollectionIDTable(m_dataSvc, this);
  auto name = idTable.name(id.collectionID);
  if (!name.has_value()) {
    error() << "Could not get a collection name for object " << id << endmsg;
    return nullptr;
  }

  DataObject* p{nullptr};
  if (m_dataSvc->retrieveObject("/Event/" + name.value(), p).isFailure()) {
    error() << "Could not get the collection '" << name.value() << "' from the Event store" << endmsg;
    return nullptr;
  }
  const auto collWrapper = dynamic_cast<AnyDataWrapper<std::unique_ptr<podio::CollectionBase>>*>(p);
  if (!collWrapper) {
    error() << "Could not cast data object to the necessary type for collection " << name.value() << endmsg;
    return nullptr;
  }

  return collWrapper->getData().get();
}

DECLARE_COMPONENT(CollectionFromObjectSvc)
