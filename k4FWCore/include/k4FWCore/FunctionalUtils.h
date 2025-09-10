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
#ifndef FWCORE_FUNCTIONALUTILS_H
#define FWCORE_FUNCTIONALUTILS_H

#include "Gaudi/Functional/details.h"
#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <GaudiKernel/GaudiException.h>

#include "podio/CollectionBase.h"

#include "k4FWCore/DataWrapper.h"

// #include "GaudiKernel/CommonMessaging.h"

#include <fmt/format.h>

#include <memory>
#include <tuple>
#include <type_traits>

namespace k4FWCore {

static const std::string frameLocation = "/_Frame";

namespace details {

  // It doesn't need to be a template but this allows parameter pack expansion
  template <typename T>
  struct EventStoreType {
    using type = std::unique_ptr<podio::CollectionBase>;
  };
  using EventStoreType_t = typename EventStoreType<void>::type;

  // This is used when there is an arbitrary number of collections as input/output
  template <typename T, typename P>
    requires(!std::is_same_v<P, podio::CollectionBase*>)
  const auto& maybeTransformToEDM4hep(const P& arg) {
    return arg;
  }

  // This is used by the FilterPredicate
  template <typename T, typename P>
    requires std::same_as<P, podio::CollectionBase*>
  const auto& maybeTransformToEDM4hep(P&& arg) {
    return static_cast<const T&>(*arg);
  }

  // This is used by the EfficiencyFilter
  template <typename T, typename P>
    requires std::same_as<P, podio::CollectionBase* const>
  const auto& maybeTransformToEDM4hep(P& arg) {
    return static_cast<const podio::CollectionBase&>(*arg);
  }

  // This is used in all the remaining cases
  template <typename T, typename P>
    requires(std::is_base_of_v<podio::CollectionBase, P> && !std::same_as<podio::CollectionBase, P>)
  const auto& maybeTransformToEDM4hep(P* arg) {
    return *arg;
  }

  template <typename T, bool addPtr = std::is_base_of_v<podio::CollectionBase, T>>
  using addPtrIfColl = std::conditional_t<addPtr, std::add_pointer_t<T>, T>;

  // Check if the type is a vector like type, where vector is the special
  // type to have an arbitrary number of collections as input or output:
  // std::vector<Coll> where Coll is the collection type for output
  // and const std::vector<const Coll*>& for input
  template <typename T>
  struct isVectorLike : std::false_type {};

  template <typename Value>
    requires std::is_base_of_v<podio::CollectionBase, std::remove_cv_t<Value>> ||
             std::is_same_v<podio::CollectionBase*, std::remove_cv_t<Value>>
  struct isVectorLike<std::vector<Value*>> : std::true_type {};

  template <typename Value>
    requires std::is_base_of_v<podio::CollectionBase, std::remove_cv_t<Value>>
  struct isVectorLike<std::vector<Value>> : std::true_type {};

  template <class T>
  inline constexpr bool isVectorLike_v = isVectorLike<T>::value;

  template <typename T>
  auto convertToUniquePtr(T&& arg) {
    // This is the case for CollectionMerger.cpp, where a raw pointer is
    // returned from the algorithm
    if constexpr (std::same_as<T, podio::CollectionBase*>) {
      return std::unique_ptr<podio::CollectionBase>(std::forward<T>(arg));
    } else {
      // Most common case, when an algorithm returns a collection and
      // we want to store a unique_ptr
      return std::make_unique<T>(std::forward<T>(arg));
    }
  }

  template <typename... In>
  struct filter_evtcontext_tt {
    static_assert(!std::disjunction_v<std::is_same<EventContext, In>...>,
                  "EventContext can only appear as first argument");

    template <typename Algorithm, typename Handles>
    static auto apply(const Algorithm& algo, Handles& handles) {
      return std::apply(
          [&](const auto&... handle) { return algo(get(handle, algo, Gaudi::Hive::currentContext())...); }, handles);
    }

    template <typename Algorithm, typename Handles>
    static auto apply(const Algorithm& algo, const EventContext&, Handles& handles) {
      auto inputTuple = std::tuple<addPtrIfColl<In>...>();

      // Build the input tuple by picking up either std::vector with an arbitrary
      // number of collections or single collections
      readVectorInputs<0, In...>(handles, &algo, inputTuple);

      return std::apply([&](const auto&... input) { return algo(maybeTransformToEDM4hep<decltype(input)>(input)...); },
                        inputTuple);
    }
  };

  template <size_t Index, typename... In, typename... Handles, typename InputTuple>
  void readVectorInputs(const std::tuple<Handles...>& handles, auto thisClass, InputTuple& inputTuple) {
    if constexpr (Index < sizeof...(Handles)) {
      using TupleType = std::tuple_element_t<Index, std::tuple<In...>>;
      if constexpr (isVectorLike_v<TupleType>) {
        // Bare EDM4hep type, without pointers or const
        using EDM4hepType = std::remove_cv_t<std::remove_pointer_t<typename TupleType::value_type>>;
        auto inputVector = std::vector<const EDM4hepType*>();
        inputVector.reserve(std::get<Index>(handles).size());
        for (const auto& handle : std::get<Index>(handles)) {
          podio::CollectionBase* collection = handle.get()->get();
          auto* typedCollection = dynamic_cast<const EDM4hepType*>(collection);
          if (typedCollection) {
            inputVector.push_back(typedCollection);
          } else {
            throw GaudiException(
                fmt::format("Failed to cast collection {} to the required type {}, the type of the collection is {}",
                            handle.objKey(), typeid(EDM4hepType).name(),
                            collection ? collection->getTypeName() : "[undetermined]"),
                thisClass->name(),
                StatusCode::FAILURE);
          }
        }
        std::get<Index>(inputTuple) = std::move(inputVector);
      } else {
        // Bare EDM4hep type, without pointers or const
        using EDM4hepType = std::remove_cv_t<std::remove_pointer_t<TupleType>>;
        try {
          podio::CollectionBase* collection = std::get<Index>(handles)[0].get()->get();
          auto* typedCollection = dynamic_cast<EDM4hepType*>(collection);
            throw GaudiException(
                fmt::format("Failed to cast collection {} to the required type {}, the type of the collection is {}",
                            std::get<Index>(handles)[0].objKey(), typeid(EDM4hepType).name(),
                            collection ? collection->getTypeName() : "[undetermined]"),
                thisClass->name(),
                StatusCode::FAILURE);
          if (typedCollection) {
            std::get<Index>(inputTuple) = typedCollection;
          } else {
            throw GaudiException(
                fmt::format("Failed to cast collection {} to the required type {}, the type of the collection is {}",
                            std::get<Index>(handles)[0].objKey(), typeid(EDM4hepType).name(),
                            collection ? collection->getTypeName() : "[undetermined]"),
                thisClass->name(),
                StatusCode::FAILURE);
          }
        } catch (GaudiException& e) {
          // When the type of the collection is different from the one requested, this can happen because
          // 1. a mistake was made in the input types of a functional algorithm
          // 2. the data was produced using the old DataHandle, which is never going to be in the input type
          if (e.message().find("different from") != std::string::npos) {
            thisClass->debug() << "Trying to cast the collection " << std::get<Index>(handles)[0].objKey()
                               << " to the requested type didn't work " << endmsg;
            DataObject* dataObject;
            IDataProviderSvc* eventDataSvc = thisClass->evtSvc();
            eventDataSvc->retrieveObject("/Event/" + std::get<Index>(handles)[0].objKey(), dataObject).ignore();
            // This is how Gaudi::Algorithms saves collections through the DataHandle
            const auto* wrapper = dynamic_cast<const DataWrapper<EDM4hepType>*>(dataObject);
            // This is how the Marlin wrapper saves collections when converting from LCIO to EDM4hep
            const auto* marlinWrapper = dynamic_cast<const DataWrapper<podio::CollectionBase>*>(dataObject);
            if (!wrapper && !marlinWrapper) {
              throw GaudiException(fmt::format("Failed to cast collection {} to the required type {}",
                                               std::get<Index>(handles)[0].objKey(), typeid(EDM4hepType).name()),
                                   thisClass->name(),
                                   StatusCode::FAILURE);
            }
            if (wrapper) {
              std::get<Index>(inputTuple) = const_cast<EDM4hepType*>(wrapper->getData());
            } else {
              std::get<Index>(inputTuple) =
                  dynamic_cast<EDM4hepType*>(const_cast<podio::CollectionBase*>(marlinWrapper->getData()));
            }
          } else {
            throw e;
          }
        }
      }

      // Recursive call for the next index
      readVectorInputs<Index + 1, In...>(handles, thisClass, inputTuple);
    }
  }

  template <size_t Index, typename... Out, typename... Handles>
  void putVectorOutputs(std::tuple<Handles...>&& handles, const auto& outputs, auto thisClass) {
    if constexpr (Index < sizeof...(Handles)) {
      auto& outputHandles = std::get<Index>(handles); // Can not be const to allow std::move(value) below
      if constexpr (isVectorLike_v<std::tuple_element_t<Index, std::tuple<Out...>>>) {
        const auto& outputVector = std::get<Index>(outputs);
        if (outputHandles.size() != outputVector.size()) {
          std::string msg = "Size of the output vector " + std::to_string(outputHandles.size()) + " with type " +
                            typeid(outputHandles).name() + " does not match the expected size from the steering file " +
                            std::to_string(outputVector.size());
          throw GaudiException(msg, thisClass->name(), StatusCode::FAILURE);
        }
        size_t index = 0;
        for (auto& value : outputHandles) {
          Gaudi::Functional::details::put(outputVector[index], convertToUniquePtr(std::move(value)));
          ++index;
        }
      } else {
        Gaudi::Functional::details::put(std::get<Index>(outputs)[0], convertToUniquePtr(std::move(outputHandles)));
      }

      // Recursive call for the next index
      putVectorOutputs<Index + 1, Out...>(std::move(handles), outputs, thisClass);
    }
  }

  inline std::vector<DataObjID> to_DataObjID(const std::vector<std::string>& inputStrings) {
    std::vector<DataObjID> outputIds;
    outputIds.reserve(inputStrings.size());
    std::transform(inputStrings.begin(), inputStrings.end(), std::back_inserter(outputIds),
                   [](const std::string& str) { return DataObjID{str}; });
    return outputIds;
  }

  // Functional handles
  // This is currently used so that the FilterPredicate can be used together with the
  // consumer/producer/transformer
  template <typename T>
  class FunctionalDataObjectReadHandle : public ::details::ReadHandle<T> {
    template <typename... Args, std::size_t... Is>
    FunctionalDataObjectReadHandle(std::tuple<Args...>&& args, std::index_sequence<Is...>)
        : FunctionalDataObjectReadHandle(std::get<Is>(std::move(args))...) {}

  public:
    /// Autodeclaring constructor with property name, mode, key and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <typename OWNER, typename K, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
    FunctionalDataObjectReadHandle(OWNER* owner, std::string propertyName, K key = {}, std::string doc = "")
        : ::details::ReadHandle<T>(owner, Gaudi::DataHandle::Reader, std::move(propertyName), std::move(key),
                                   std::move(doc)) {}

    template <typename... Args>
    FunctionalDataObjectReadHandle(std::tuple<Args...>&& args)
        : FunctionalDataObjectReadHandle(std::move(args), std::index_sequence_for<Args...>{}) {}

    const T& get() const;
  };

  template <typename T>
  const T& FunctionalDataObjectReadHandle<T>::get() const {
    const auto dataObj = this->fetch();
    if (!dataObj) {
      throw GaudiException(fmt::format("Cannot retrieve '{}' from transient store [{}]", this->objKey(),
                                       this->m_owner ? this->owner()->name() : "no owner"),
                           "FunctionalDataObjectReadHandle", StatusCode::FAILURE);
    }
    const auto ptr = dynamic_cast<AnyDataWrapper<std::unique_ptr<podio::CollectionBase>>*>(dataObj);
    return maybeTransformToEDM4hep<T>(ptr->getData().get());
  }

  struct BaseClass_t {
    template <typename T>
    using InputHandle = FunctionalDataObjectReadHandle<T>;
    // template <typename T> using OutputHandle = DataObjectWriteHandle<T>;

    using BaseClass = Gaudi::Algorithm;
  };

} // namespace details
} // namespace k4FWCore

#endif // CORE_FUNCTIONALUTILS_H
