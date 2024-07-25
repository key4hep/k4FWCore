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

#include "MetadataSvc.h"

#include "podio/Frame.h"

#include <GaudiKernel/AnyDataWrapper.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/Service.h>

#include "podio/Frame.h"

StatusCode MetadataSvc::initialize() {
  StatusCode sc = Service::initialize();
  if (sc.isFailure()) {
    error() << "Unable to initialize base class Service." << endmsg;
    return sc;
  }
  m_dataSvc = service("EventDataSvc");
  if (!m_dataSvc) {
    error() << "Unable to locate the EventDataSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  m_frame.reset(new podio::Frame());

  return StatusCode::SUCCESS;
}

StatusCode MetadataSvc::finalize() { return Service::finalize(); }

void MetadataSvc::setFrame(podio::Frame&& fr) {
  m_frame.reset(new podio::Frame(std::move(fr)));
}

DECLARE_COMPONENT(MetadataSvc)
