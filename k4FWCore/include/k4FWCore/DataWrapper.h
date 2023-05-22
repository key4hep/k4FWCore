#ifndef K4FWCORE_DATAWRAPPER_H
#define K4FWCORE_DATAWRAPPER_H

#include <type_traits>

// Include files
#include "GaudiKernel/DataObject.h"
#include "podio/CollectionBase.h"

// forward declaration
template<typename T>
class DataHandle;

class GAUDI_API DataWrapperBase : public DataObject {
public:
  // ugly hack to circumvent the usage of boost::any yet
  // DataSvc would need a templated register method
  virtual podio::CollectionBase* collectionBase() = 0;
  virtual ~DataWrapperBase(){};
  virtual void resetData() = 0;
};

template <class T> class GAUDI_API DataWrapper : public DataWrapperBase {
public:
  template<class T2>
  friend class DataHandle;
public:
  DataWrapper() : DataWrapperBase(), m_data(nullptr){};
  virtual ~DataWrapper();

  const T* getData() { return m_data; }
  void     setData(const T* data) { m_data = data; }
  virtual void     resetData() { m_data = nullptr; }

private:
  /// try to cast to collectionBase; may return nullptr;
  virtual podio::CollectionBase* collectionBase();

private:
  const T* m_data;

};

template <class T> DataWrapper<T>::~DataWrapper<T>() {
  if (m_data != nullptr)
    delete m_data;
}

template <class T> podio::CollectionBase* DataWrapper<T>::collectionBase() {
  if constexpr (std::is_base_of<podio::CollectionBase, T>::value) {
    return const_cast<T*>(m_data);
  }
  return nullptr;
}

#endif
