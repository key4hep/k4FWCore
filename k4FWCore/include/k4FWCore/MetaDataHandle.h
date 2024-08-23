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

#include <GaudiKernel/DataHandle.h>

#include "k4FWCore/MetadataUtils.h"
#include "k4FWCore/PodioDataSvc.h"

template <typename T> class MetaDataHandle {
public:
  MetaDataHandle();
  MetaDataHandle(const std::string& descriptor, Gaudi::DataHandle::Mode a);
  MetaDataHandle(const Gaudi::DataHandle& handle, const std::string& descriptor, Gaudi::DataHandle::Mode a);
  ~MetaDataHandle();

  /// Get the value that is stored in this MetaDataHandle
  ///
  /// @returns The value for this MetaDataHandle
  ///
  /// @throws GaudiException in case the value is not (yet) available
  const T get() const;

  /// Get the (optional) value that is stored in this MetaDataHandle
  ///
  /// @returns An optional that contains the value if it was available from the
  ///          data store and is not engaged otherwise
  std::optional<T> get_optional() const;

  /// Get the value that is stored in the MetaDataHandle or the provided default
  /// value in case that is not available
  ///
  /// @returns The value stored in the Handle or the default value
  const T get(const T& defaultValue) const;

  /// Set the value for this MetaDataHandle
  ///
  /// @note This can only be called during initialize and/or finalize but not
  /// during execute for algorithms that use it
  void put(T);

private:
  std::string fullDescriptor() const;

  void checkPodioDataSvc();

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
  m_eds.retrieve().ignore();
  m_podio_data_service = dynamic_cast<PodioDataSvc*>(m_eds.get());
  checkPodioDataSvc();
}

//---------------------------------------------------------------------------
template <typename T>
MetaDataHandle<T>::MetaDataHandle(const Gaudi::DataHandle& handle, const std::string& descriptor,
                                  Gaudi::DataHandle::Mode a)
    : m_eds("EventDataSvc", "DataHandle"), m_descriptor(descriptor), m_dataHandle(&handle), m_mode(a) {
  m_eds.retrieve().ignore();
  m_podio_data_service = dynamic_cast<PodioDataSvc*>(m_eds.get());
  checkPodioDataSvc();
}

//---------------------------------------------------------------------------
template <typename T> std::optional<T> MetaDataHandle<T>::get_optional() const {
  const auto& frame = m_podio_data_service->getMetaDataFrame();
  return frame.getParameter<T>(fullDescriptor());
}

//---------------------------------------------------------------------------
template <typename T> const T MetaDataHandle<T>::get() const {
  std::optional<T> maybeVal;
  // DataHandle based algorithms
  if (m_podio_data_service) {
    maybeVal = get_optional();
    if (!maybeVal.has_value()) {
      throw GaudiException("MetaDataHandle empty handle access",
                           "MetaDataHandle " + fullDescriptor() + " not (yet?) available", StatusCode::FAILURE);
    }
    // Functional algorithms
  } else {
    maybeVal = k4FWCore::getParameter<T>(fullDescriptor());
  }
  return maybeVal.value();
}

//---------------------------------------------------------------------------
template <typename T> const T MetaDataHandle<T>::get(const T& defaultValue) const {
  return get_optional().value_or(defaultValue);
}

//---------------------------------------------------------------------------
template <typename T> void MetaDataHandle<T>::put(T value) {
  if (m_mode != Gaudi::DataHandle::Writer)
    throw GaudiException("MetaDataHandle policy violation", "Put for non-writing MetaDataHandle not allowed",
                         StatusCode::FAILURE);
  // check whether we are in the proper State
  // put is only allowed in the initialization

  std::string full_descriptor = fullDescriptor();
  // DataHandle based algorithms
  if (m_podio_data_service) {
    if (m_podio_data_service->targetFSMState() == Gaudi::StateMachine::RUNNING) {
      throw GaudiException("MetaDataHandle policy violation", "Put cannot be used during the event loop",
                           StatusCode::FAILURE);
    }
    podio::Frame& frame = m_podio_data_service->getMetaDataFrame();
    frame.putParameter(full_descriptor, value);
    // Functional algorithms
  } else {
    k4FWCore::putParameter(full_descriptor, value);
  }
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

  // The proper check would be the following:
  // if (!m_podio_data_service && !Gaudi::svcLocator()->service<IMetadataSvc>("MetadataSvc")) {
  // However, it seems there is always a service called "MetadataSvc" from Gaudi,
  // so the check will always pass
  if (!m_podio_data_service) {
    std::cout << "Warning: MetaDataHandles require the PodioDataSvc (ignore if using IOSvc)" << std::endl;
  }
}

#endif
