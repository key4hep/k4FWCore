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
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/StateMachine.h>

#include <memory>

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
  bool hasParameters = !m_intParameters.empty() || !m_floatParameters.empty() || !m_doubleParameters.empty() ||
                       !m_stringParameters.value().empty();
  if (hasParameters && !m_setAtFinalize) {
    // If there is a reader, then the metadata frame will be set from the IOSvc
    // to what is read from the Reader
    if (!m_frame) {
      m_frame = std::make_unique<podio::Frame>();
    }
    applyPropertyParameters();
  }
  return StatusCode::SUCCESS;
}

StatusCode MetadataSvc::finalize() { return Service::finalize(); }

void MetadataSvc::throwIfRunning() const {
  if (targetFSMState() == Gaudi::StateMachine::RUNNING) {
    throw GaudiException("putParameter cannot be called during the event loop", name(), StatusCode::FAILURE);
  }
}

const podio::Frame* MetadataSvc::getFrame() const { return m_frame.get(); }

podio::Frame* MetadataSvc::getFrame() {
  if (m_setAtFinalize && !m_paramsApplied) {
    if (!m_frame) {
      m_frame = std::make_unique<podio::Frame>();
    }
    applyPropertyParameters();
    m_paramsApplied = true;
  }
  return m_frame.get();
}

void MetadataSvc::setFrame(podio::Frame frame) {
  m_frame = std::make_unique<podio::Frame>(std::move(frame));
  m_paramsApplied = false;
  if (!m_setAtFinalize) {
    applyPropertyParameters();
  }
}

void MetadataSvc::applyPropertyParameters() {
  for (const auto& [key, val] : m_intParameters) {
    m_frame->putParameter(key, val);
  }
  for (const auto& [key, val] : m_floatParameters) {
    m_frame->putParameter(key, val);
  }
  for (const auto& [key, val] : m_doubleParameters) {
    m_frame->putParameter(key, val);
  }
  for (const auto& [key, val] : m_stringParameters) {
    m_frame->putParameter(key, val);
  }
}

DECLARE_COMPONENT(MetadataSvc)
