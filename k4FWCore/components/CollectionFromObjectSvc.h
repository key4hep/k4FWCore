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
#ifndef FWCORE_COLLECTIONFROMOBJECTSVC_H
#define FWCORE_COLLECTIONFROMOBJECTSVC_H

#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/Service.h>

#include <podio/CollectionBase.h>
#include <podio/ObjectID.h>

#include "k4FWCore/ICollectionFromObjectSvc.h"

class CollectionFromObjectSvc : public extends<Service, ICollectionFromObjectSvc> {
  using extends::extends;

public:
  StatusCode initialize() override;

protected:
  const podio::CollectionBase* getCollectionFor(const podio::ObjectID id) const override;
  const std::optional<std::string> getCollectionNameFor(const podio::ObjectID id) const override;

private:
  SmartIF<IDataProviderSvc> m_dataSvc;
};

#endif
