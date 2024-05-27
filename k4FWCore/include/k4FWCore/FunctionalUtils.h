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

#include <GaudiKernel/AnyDataWrapper.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include "Gaudi/Functional/details.h"
#include "GaudiKernel/DataObjID.h"
#include "k4FWCore/DataWrapper.h"
#include "podio/CollectionBase.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"

// #include "GaudiKernel/CommonMessaging.h"

#include <map>
#include <memory>
#include <tuple>
#include <type_traits>

namespace k4FWCore {

  static const std::string frameLocation = "/_Frame";

  namespace details {

    // This function will be used to modify std::shared_ptr<podio::CollectionBase> to the actual collection type
    template <typename T, typename P> const auto& maybeTransformToEDM4hep(const P& arg) { return arg; }
    template <typename T, typename P>
      requires std::is_base_of_v<podio::CollectionBase, P>
    const auto& maybeTransformToEDM4hep(P* arg) {
      return *arg;
    }

    template <typename T, typename P>
      requires std::same_as<P, std::shared_ptr<podio::CollectionBase>>
    const auto& maybeTransformToEDM4hep(P arg) {
      return static_cast<const T&>(*arg);
    }

    template <typename T, bool addPtr = std::is_base_of_v<podio::CollectionBase, T>>
    using addPtrIfColl = std::conditional_t<addPtr, std::add_pointer_t<T>, T>;

    template <typename T> const auto& transformtoEDM4hep(const std::shared_ptr<podio::CollectionBase>& arg) {
      return static_cast<const T>(*arg);
    }

    // Check if the type is a map like type, where map type is the special map
    // type to have an arbitrary number of collections as input or output:
    // std::map<std::string, Coll> where Coll is the collection type
    template <typename T> struct isMapToCollLike : std::false_type {};

    template <typename Value>
      requires std::is_base_of_v<podio::CollectionBase, std::remove_cvref_t<Value>>
    struct isMapToCollLike<std::map<std::string, Value>> : std::true_type {};

    template <class T> inline constexpr bool isMapToCollLike_v = isMapToCollLike<T>::value;

    // transformType function to transform the types from the ones that the user wants
    // like edm4hep::MCParticleCollection, to the ones that are actually stored in the
    // event store, like std::shared_ptr<podio::CollectionBase>
    // For std::map<std::string, Coll>, th
    template <typename T> struct transformType {
      using type = T;
    };

    template <typename T>
      requires std::is_base_of_v<podio::CollectionBase, T> || isMapToCollLike_v<T>
    struct transformType<T> {
      using type = std::shared_ptr<podio::CollectionBase>;
    };

    template <typename T> auto convertToSharedPtr(T&& arg) {
      return std::shared_ptr<podio::CollectionBase>(std::make_shared<T>(std::move(arg)));
    }

    template <typename... In> struct filter_evtcontext_tt {
      static_assert(!std::disjunction_v<std::is_same<EventContext, In>...>,
                    "EventContext can only appear as first argument");

      template <typename Algorithm, typename Handles> static auto apply(const Algorithm& algo, Handles& handles) {
        return std::apply(
            [&](const auto&... handle) { return algo(get(handle, algo, Gaudi::Hive::currentContext())...); }, handles);
      }

      template <typename Algorithm, typename Handles>
      static auto apply(const Algorithm& algo, const EventContext&, Handles& handles) {
        auto inputTuple = std::tuple<addPtrIfColl<In>...>();

        // Build the input tuple by picking up either std::map with an arbitrary
        // number of collections or single collections
        readMapInputs<0, In...>(handles, &algo, inputTuple);

        return std::apply(
            [&](const auto&... input) { return algo(maybeTransformToEDM4hep<decltype(input)>(input)...); }, inputTuple);
      }
    };

    template <size_t Index, typename... In, typename... Handles, typename InputTuple>
    void readMapInputs(const std::tuple<Handles...>& handles, auto thisClass, InputTuple& inputTuple) {
      if constexpr (Index < sizeof...(Handles)) {
        if constexpr (isMapToCollLike_v<std::tuple_element_t<Index, std::tuple<In...>>>) {
          // In case of map types like std::map<std::string, edm4hep::MCParticleCollection&>
          // we have to remove the reference to get the actual type
          using EDM4hepType =
              std::remove_reference_t<typename std::tuple_element_t<Index, std::tuple<In...>>::mapped_type>;
          auto inputMap = std::map<std::string, const EDM4hepType&>();
          for (auto& handle : std::get<Index>(handles)) {
            auto in = get(handle, thisClass, Gaudi::Hive::currentContext());
            inputMap.emplace(handle.objKey(), *static_cast<EDM4hepType*>(in.get()));
          }
          std::get<Index>(inputTuple) = std::move(inputMap);

        } else {
          try {
            auto in                     = get(std::get<Index>(handles)[0], thisClass, Gaudi::Hive::currentContext());
            std::get<Index>(inputTuple) = static_cast<std::tuple_element_t<Index, std::tuple<In...>>*>(in.get());
          } catch (GaudiException& e) {
            // When the type of the collection is different from the one requested, this can happen because
            // 1. a mistake was made in the input types of a functional algorithm
            // 2. the data was produced using the old DataHandle, which is never going to be in the input type
            if (e.message().find("different from") != std::string::npos) {
              thisClass->debug() << "Trying to cast the collection " << std::get<Index>(handles)[0].objKey()
                                 << " to the requested type didn't work " << endmsg;
              DataObject*       p;
              IDataProviderSvc* svc = thisClass->serviceLocator()->template service<IDataProviderSvc>("EventDataSvc");
              svc->retrieveObject("/Event/" + std::get<Index>(handles)[0].objKey(), p).ignore();
              const auto wrp = dynamic_cast<const DataWrapper<std::tuple_element_t<Index, std::tuple<In...>>>*>(p);
              if (!wrp) {
                throw GaudiException(thisClass->name(),
                                     "Failed to cast collection " + std::get<Index>(handles)[0].objKey() +
                                         " to the requested type " +
                                         typeid(std::tuple_element_t<Index, std::tuple<In...>>).name(),
                                     StatusCode::FAILURE);
              }
              std::get<Index>(inputTuple) = const_cast<std::tuple_element_t<Index, std::tuple<In...>>*>(wrp->getData());
            } else {
              throw e;
            }
          }
        }

        // Recursive call for the next index
        readMapInputs<Index + 1, In...>(handles, thisClass, inputTuple);
      }
    }

    template <size_t Index, typename... Out, typename... Handles>
    void putMapOutputs(std::tuple<Handles...>&& handles, const auto& m_outputs, auto thisClass) {
      if constexpr (Index < sizeof...(Handles)) {
        if constexpr (isMapToCollLike_v<std::tuple_element_t<Index, std::tuple<Out...>>>) {
          int i = 0;
          if (std::get<Index>(handles).size() != std::get<Index>(m_outputs).size()) {
            std::string msg = "Size of the output map " + std::to_string(std::get<Index>(handles).size()) +
                              " does not match the expected size from the steering file " +
                              std::to_string(std::get<Index>(m_outputs).size()) + ". Expected the collections: ";
            for (auto& out : std::get<Index>(m_outputs)) {
              msg += out.objKey() + " ";
            }
            msg += " but got the collections: ";
            for (auto& out : std::get<Index>(handles)) {
              msg += out.first + " ";
            }
            throw GaudiException(thisClass->name(), msg, StatusCode::FAILURE);
          }
          for (auto& [key, val] : std::get<Index>(handles)) {
            if (key != std::get<Index>(m_outputs)[i].objKey()) {
              throw GaudiException(thisClass->name(),
                                   "Output key in the map \"" + key +
                                       "\" does not match the expected key from the steering file \"" +
                                       std::get<Index>(m_outputs)[i].objKey() + "\"",
                                   StatusCode::FAILURE);
            }
            Gaudi::Functional::details::put(std::get<Index>(m_outputs)[i], convertToSharedPtr(std::move(val)));
            i++;
          }
        } else {
          Gaudi::Functional::details::put(std::get<Index>(m_outputs)[0],
                                          convertToSharedPtr(std::move(std::get<Index>(handles))));
        }

        // Recursive call for the next index
        putMapOutputs<Index + 1, Out...>(std::move(handles), m_outputs, thisClass);
      }
    }

    inline std::vector<DataObjID> to_DataObjID(const std::vector<std::string>& in) {
      std::vector<DataObjID> out;
      out.reserve(in.size());
      std::transform(in.begin(), in.end(), std::back_inserter(out), [](const std::string& i) { return DataObjID{i}; });
      return out;
    }

  }  // namespace details
}  // namespace k4FWCore

#endif  // CORE_FUNCTIONALUTILS_H
