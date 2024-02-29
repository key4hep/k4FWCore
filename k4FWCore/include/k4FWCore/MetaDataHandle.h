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
#ifndef K4FWCORE_METADATAHANDLE_H
#define K4FWCORE_METADATAHANDLE_H

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

#include "k4FWCore/PodioDataSvc.h"
#include "podio/GenericParameters.h"

#include "GaudiKernel/MsgStream.h"

#include <type_traits>

template <typename T> class MetaDataHandle {
public:
  MetaDataHandle();
  MetaDataHandle(const std::string& descriptor, Gaudi::DataHandle::Mode a);
  MetaDataHandle(const Gaudi::DataHandle& handle, const std::string& descriptor, Gaudi::DataHandle::Mode a);
  ~MetaDataHandle();

  const T get() const;
  void    put(T);

private:
  std::string fullDescriptor() const;
  void        checkPodioDataSvc();

private:
  ServiceHandle<IDataProviderSvc> m_eds;
  std::string                     m_descriptor;
  PodioDataSvc*                   m_podio_data_service{nullptr};
  const Gaudi::DataHandle*        m_dataHandle{nullptr};  // holds the identifier in case we do collection metadata
  Gaudi::DataHandle::Mode         m_mode;
};

template <typename T> MetaDataHandle<T>::~MetaDataHandle() {}

//---------------------------------------------------------------------------
template <typename T>
MetaDataHandle<T>::MetaDataHandle(const std::string& descriptor, Gaudi::DataHandle::Mode a)
    : m_eds("EventDataSvc", "DataHandle"), m_descriptor(descriptor), m_mode(a) {
  StatusCode sc [[maybe_unused]] = m_eds.retrieve();
  m_podio_data_service           = dynamic_cast<PodioDataSvc*>(m_eds.get());
  checkPodioDataSvc();
}

//---------------------------------------------------------------------------
template <typename T>
MetaDataHandle<T>::MetaDataHandle(const Gaudi::DataHandle& handle, const std::string& descriptor,
                                  Gaudi::DataHandle::Mode a)
    : m_eds("EventDataSvc", "DataHandle"), m_descriptor(descriptor), m_dataHandle(&handle), m_mode(a) {
  StatusCode sc [[maybe_unused]] = m_eds.retrieve();
  m_podio_data_service           = dynamic_cast<PodioDataSvc*>(m_eds.get());
  checkPodioDataSvc();
}

//---------------------------------------------------------------------------
template <typename T> const T MetaDataHandle<T>::get() const {
  const auto& frame = m_podio_data_service->getMetaDataFrame();
  return frame.getParameter<T>(fullDescriptor());
}

//---------------------------------------------------------------------------
template <typename T> void MetaDataHandle<T>::put(T value) {
  if (m_mode != Gaudi::DataHandle::Writer)
    throw GaudiException("MetaDataHandle policy violation", "Put for non-writing MetaDataHandle not allowed",
                         StatusCode::FAILURE);
  // check whether we are in the proper State
  // put is only allowed in the initalization
  if (m_podio_data_service->targetFSMState() == Gaudi::StateMachine::RUNNING) {
    throw GaudiException("MetaDataHandle policy violation", "Put cannot be used during the event loop",
                         StatusCode::FAILURE);
  }
  std::string   full_descriptor = fullDescriptor();
  podio::Frame& frame           = m_podio_data_service->getMetaDataFrame();
  frame.putParameter(full_descriptor, value);
}

//---------------------------------------------------------------------------
template <typename T> std::string MetaDataHandle<T>::fullDescriptor() const {
  if (nullptr != m_dataHandle) {
    auto full_descriptor = podio::collMetadataParamName(m_dataHandle->objKey(), m_descriptor);
    // remove the "/Event/" part of the collections' object key if in read mode
    if (m_mode == Gaudi::DataHandle::Reader && full_descriptor.find("/Event/") == 0u) {
      full_descriptor.erase(0, 7);
    }
    return full_descriptor;
  }

  return m_descriptor;
}

//---------------------------------------------------------------------------
template <typename T> void MetaDataHandle<T>::checkPodioDataSvc() {
  // do not do this check during the genconf step
  const std::string cmd = System::cmdLineArgs()[0];
  if (cmd.find("genconf") != std::string::npos)
    return;

  if (nullptr == m_podio_data_service) {
    std::cout << "ERROR: MetaDataHandles require the PodioDataSvc" << std::endl;
  }
}

#endif
