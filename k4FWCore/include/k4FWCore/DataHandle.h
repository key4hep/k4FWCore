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

#include "Gaudi/Algorithm.h"
#include "GaudiKernel/DataObjectHandle.h"

#include "edm4hep/Constants.h"

#include "TTree.h"

#include <GaudiKernel/AnyDataWrapper.h>
#include <type_traits>

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
  bool                            m_isGoodType{false};
  bool                            m_isCollection{false};
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
 * Try to retrieve from the transient store. If the retrieval succeded and
 * this is the first time we retrieve, perform a dynamic cast to the desired
 * object. Then finally set the handle as Read.
 * If this is not the first time we cast and the cast worked, just use the
 * static cast: we do not need the checks of the dynamic cast for every access!
 */
template <typename T> const T* DataHandle<T>::get() {
  DataObject* dataObjectp = nullptr;
  auto        sc          = m_eds->retrieveObject(DataObjectHandle<DataWrapper<T>>::fullKey().key(), dataObjectp);

  if (sc.isSuccess()) {
    if (!m_isGoodType && !m_isCollection) {
      // only do this once (if both are false after this, we throw exception)
      m_isGoodType = nullptr != dynamic_cast<DataWrapper<T>*>(dataObjectp);
      if (!m_isGoodType) {
        auto tmp = dynamic_cast<DataWrapper<podio::CollectionBase>*>(dataObjectp);
        if (tmp != nullptr) {
          m_isCollection = nullptr != dynamic_cast<T*>(tmp->collectionBase());
        }
      }
    }
    if (m_isGoodType) {
      return static_cast<DataWrapper<T>*>(dataObjectp)->getData();
    } else if (m_isCollection) {
      // The reader does not know the specific type of the collection. So we need a reinterpret_cast if the handle was
      // created by the reader.
      DataWrapper<podio::CollectionBase>* tmp = static_cast<DataWrapper<podio::CollectionBase>*>(dataObjectp);
      return reinterpret_cast<const T*>(tmp->collectionBase());
    } else {
      // When a functional has pushed an std::shared_ptr<podio::CollectionBase> into the store
      auto ptr = static_cast<AnyDataWrapper<std::shared_ptr<podio::CollectionBase>>*>(dataObjectp)->getData();
      if (ptr) {
        return reinterpret_cast<const T*>(ptr.get());
      }
      std::string errorMsg("The type provided for " + DataObjectHandle<DataWrapper<T>>::pythonRepr() +
                           " is different from the one of the object in the store.");
      throw GaudiException(errorMsg, "wrong product type", StatusCode::FAILURE);
    }
  }
  std::string msg("Could not retrieve product " + DataObjectHandle<DataWrapper<T>>::pythonRepr());
  throw GaudiException(msg, "wrong product name", StatusCode::FAILURE);
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

    /// virtual Destructor
    virtual ~Property() {}
  };
}  // namespace Gaudi

#endif
