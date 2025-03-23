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
#ifndef K4FWCORE_DATAWRAPPER_H
#define K4FWCORE_DATAWRAPPER_H

#include <type_traits>

#include "GaudiKernel/DataObject.h"
#include "podio/CollectionBase.h"

// forward declaration
template <typename T>
class DataHandle;

class GAUDI_API DataWrapperBase : public DataObject {
public:
  // ugly hack to circumvent the usage of boost::any yet
  // DataSvc would need a templated register method
  virtual podio::CollectionBase* collectionBase() = 0;
  virtual void resetData() = 0;
};

template <class T>
class GAUDI_API DataWrapper : public DataWrapperBase {
public:
  template <class T2>
  friend class DataHandle;

public:
  DataWrapper() : m_data(nullptr) {}
  DataWrapper(T&& coll) {
    m_data = new T(std::move(coll));
    is_owner = true;
  }
  DataWrapper(std::unique_ptr<T> uptr) : m_data(uptr.get()) {
    uptr.release();
    is_owner = false;
  }
  ~DataWrapper() override;

  const T* getData() const { return m_data; }
  void setData(const T* data) { m_data = data; }
  void resetData() override { m_data = nullptr; }

  operator const T&() const& { return *m_data; }

private:
  /// try to cast to collectionBase; may return nullptr;
  podio::CollectionBase* collectionBase() override;

private:
  const T* m_data;
  bool is_owner{true};
};

template <class T>
DataWrapper<T>::~DataWrapper() {
  if (is_owner) {
    delete m_data;
  }
}

template <class T>
podio::CollectionBase* DataWrapper<T>::collectionBase() {
  if constexpr (std::is_base_of<podio::CollectionBase, T>::value) {
    return const_cast<T*>(m_data);
  }
  return nullptr;
}

#endif
