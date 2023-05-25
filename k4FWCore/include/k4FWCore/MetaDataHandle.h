#ifndef K4FWCORE_METADATAHANDLE_H
#define K4FWCORE_METADATAHANDLE_H

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

#include "podio/GenericParameters.h"
#include "k4FWCore/PodioDataSvc.h"

#include "GaudiKernel/MsgStream.h"

#include <type_traits>

template <typename T> class MetaDataHandle {
public:
  MetaDataHandle();
  MetaDataHandle(const std::string& descriptor, Gaudi::DataHandle::Mode a);
  MetaDataHandle(const Gaudi::DataHandle& handle, const std::string& descriptor, Gaudi::DataHandle::Mode a);
  ~MetaDataHandle();

  const T get();
  void put(T);
  
private:
  std::string fullDescriptor();

private:
  ServiceHandle<IDataProviderSvc> m_eds;
  std::string                     m_descriptor;
  PodioDataSvc*                   m_podio_data_service{nullptr};
  const Gaudi::DataHandle*        m_dataHandle{nullptr}; // holds the identifier in case we do collection metadata
  Gaudi::DataHandle::Mode         m_mode;

};

template <typename T> MetaDataHandle<T>::~MetaDataHandle() {
}

//---------------------------------------------------------------------------
template <typename T>
MetaDataHandle<T>::MetaDataHandle(const std::string& descriptor, Gaudi::DataHandle::Mode a)
: m_eds("EventDataSvc", "DataHandle"), m_descriptor(descriptor), m_mode(a) {

  StatusCode    sc = m_eds.retrieve();
  m_podio_data_service = dynamic_cast<PodioDataSvc*>(m_eds.get()); 
  if (nullptr == m_podio_data_service) {
    std::cout << "ERROR: MetaDataHandles require the PodioDataSvc" << std::endl;
  }
}

//---------------------------------------------------------------------------
template <typename T>
MetaDataHandle<T>::MetaDataHandle(const Gaudi::DataHandle& handle, const std::string& descriptor, Gaudi::DataHandle::Mode a)
: m_eds("EventDataSvc", "DataHandle"), m_descriptor(descriptor), m_dataHandle(&handle), m_mode(a) {

  StatusCode    sc = m_eds.retrieve();
  m_podio_data_service = dynamic_cast<PodioDataSvc*>(m_eds.get());
  if (nullptr == m_podio_data_service) {
    std::cout << "ERROR: MetaDataHandles require the PodioDataSvc" << std::endl;
  }
}


//---------------------------------------------------------------------------
template <typename T> const T MetaDataHandle<T>::get() {
  podio::Frame& frame = m_podio_data_service->getMetaDataFrame();
  return frame.getParameter<T>(fullDescriptor());
}

//---------------------------------------------------------------------------
template <typename T> void MetaDataHandle<T>::put(T value) {

  if (m_mode != Gaudi::DataHandle::Writer)
    throw GaudiException("MetaDataHandle policy violation", "Put for non-writing MetaDataHandle not allowed", StatusCode::FAILURE);
  // check whether we are in the proper State
  // put is only allowed in the initalization
  if (m_podio_data_service->targetFSMState() == Gaudi::StateMachine::RUNNING) {
    throw GaudiException("MetaDataHandle policy violation", "Put cannot be used during the event loop", StatusCode::FAILURE);
  }
  std::string full_descriptor = fullDescriptor();
  podio::Frame& frame = m_podio_data_service->getMetaDataFrame();
  frame.putParameter(full_descriptor, value);
}

//---------------------------------------------------------------------------
template <typename T> std::string MetaDataHandle<T>::fullDescriptor() {

  std::string full_descriptor;
  if (nullptr != m_dataHandle) {
    full_descriptor = m_dataHandle->objKey() + "__" + m_descriptor;
    // remove the "/Event/" part of the collections' object key if in read mode
    if (m_mode == Gaudi::DataHandle::Reader) {
      full_descriptor.erase(0,7);
    }
  } else {
    full_descriptor = m_descriptor;
  }
  return full_descriptor;
}
#endif
