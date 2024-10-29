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
#ifndef K4FWCORE_DATAHANDLE_H
#define K4FWCORE_DATAHANDLE_H

#include "k4FWCore/DataWrapper.h"
#include "k4FWCore/PodioDataSvc.h"

#include "GaudiKernel/DataObjectHandle.h"

#include <GaudiKernel/AnyDataWrapper.h>
#include <type_traits>
#include <stdexcept>

/**
 * Specialisation of the Gaudi DataHandle
 * for use with podio collections.
 */
template <typename T> class DataHandle : public DataObjectHandle<DataWrapper<T>> {
public:
  friend class Algorithm;
  friend class AlgTool;

public:
  DataHandle();
  ~DataHandle();

  /// Initialises mother class
  DataHandle(DataObjID& descriptor, Gaudi::DataHandle::Mode a, IDataHandleHolder* fatherAlg);

  DataHandle(const std::string& k, Gaudi::DataHandle::Mode a, IDataHandleHolder* fatherAlg);

  ///Retrieve object from transient data store
  const T* get();

  /**
   * Register object in transient store
   */
  void put(T* object);

  // Temporary workaround for k4MarlinWrapper
  const std::string getCollMetadataCellID(const unsigned int id);

  T* put(std::unique_ptr<T> object);

  /**
  * Create and register object in transient store
  */
  T* createAndPut();

private:
  ServiceHandle<IDataProviderSvc> m_eds;
  T*                              m_dataPtr;
};

template <typename T> DataHandle<T>::~DataHandle() {
  // release memory allocated for primitive types (see comments in ctor)
  if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
    delete m_dataPtr;
  }
}

//---------------------------------------------------------------------------
template <typename T>
DataHandle<T>::DataHandle(DataObjID& descriptor, Gaudi::DataHandle::Mode a, IDataHandleHolder* fatherAlg)
    : DataObjectHandle<DataWrapper<T>>(descriptor, a, fatherAlg), m_eds("EventDataSvc", "DataHandle") {}

template <typename T>
DataHandle<T>::DataHandle(const std::string& descriptor, Gaudi::DataHandle::Mode a, IDataHandleHolder* fatherAlg)
    : DataObjectHandle<DataWrapper<T>>(descriptor, a, fatherAlg), m_eds("EventDataSvc", "DataHandle") {
  if (a == Gaudi::DataHandle::Writer) {
    m_eds.retrieve().ignore();
    m_dataPtr                = nullptr;
    auto* podio_data_service = dynamic_cast<PodioDataSvc*>(m_eds.get());
    if (nullptr != podio_data_service) {
      if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
        m_dataPtr = new T();
      }
    }
  }
}

/**
 * Try to retrieve from the transient store. If the retrieval succeeded and
 * this is the first time we retrieve, perform a dynamic cast to the desired
 * object. Then finally set the handle as Read.
 * If this is not the first time we cast and the cast worked, just use the
 * static cast: we do not need the checks of the dynamic cast for every access!
 */
template <typename T> const T* DataHandle<T>::get() {
  DataObject* dataObjectp;
  auto        sc = m_eds->retrieveObject(DataObjectHandle<DataWrapper<T>>::fullKey().key(), dataObjectp);

  if (sc.isFailure()) {
    std::string msg("Could not retrieve product " + DataObjectHandle<DataWrapper<T>>::pythonRepr());
    throw GaudiException(msg, "wrong product name", StatusCode::FAILURE);
  }
  bool isGoodType = nullptr != dynamic_cast<DataWrapper<T>*>(dataObjectp);
  if (isGoodType) {
    return static_cast<DataWrapper<T>*>(dataObjectp)->getData();
  }

  // When a functional has pushed a std::unique_ptr<podio::CollectionBase> into the store
  // We wrap it inside constexpr because if the handle has a type that is not a collection
  // then the static_cast will fail at compile time
  if constexpr (std::is_base_of_v<podio::CollectionBase, T>) {
    auto ptr = static_cast<AnyDataWrapper<std::unique_ptr<podio::CollectionBase>>*>(dataObjectp);
    return static_cast<const T*>(ptr->getData().get());
  }
  std::string errorMsg("The type provided for " + DataObjectHandle<DataWrapper<T>>::pythonRepr() +
                       " is different from the one of the object in the store " + typeid(*dataObjectp).name());
  throw std::runtime_error(errorMsg);
}

//---------------------------------------------------------------------------
template <typename T> void DataHandle<T>::put(T* objectp) {
  std::unique_ptr<DataWrapper<T>> dw = std::make_unique<DataWrapper<T>>();
  // in case T is of primitive type, we must not change the pointer address
  // (see comments in ctor) instead copy the value of T into allocated memory
  if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
    *m_dataPtr = *objectp;
  } else {
    m_dataPtr = objectp;
  }
  dw->setData(objectp);
  DataObjectHandle<DataWrapper<T>>::put(std::move(dw));
}

//---------------------------------------------------------------------------
template <typename T> T* DataHandle<T>::put(std::unique_ptr<T> objectp) {
  put(objectp.get());
  return objectp.release();
}

//---------------------------------------------------------------------------
/**
 * Create the collection, put it in the DataObjectHandle and return the
 * pointer to the data. Call this function if you create a collection and
 * want to save it.
 */
template <typename T> T* DataHandle<T>::createAndPut() {
  T* objectp = new T();
  this->put(objectp);
  return objectp;
}

// temporary to allow property declaration
namespace Gaudi {
  template <class T> class Property<::DataHandle<T>&> : public ::DataHandleProperty {
  public:
    Property(const std::string& name, ::DataHandle<T>& value) : ::DataHandleProperty(name, value) {}
  };
}  // namespace Gaudi

#endif
