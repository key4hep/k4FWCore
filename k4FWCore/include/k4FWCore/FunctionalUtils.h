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

#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/DataObject.h"
#include "podio/CollectionBase.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"

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
      requires std::same_as<P, std::shared_ptr<podio::CollectionBase>>
    const auto& maybeTransformToEDM4hep(const P& arg) {
      return static_cast<const T&>(*arg);
    }

    // This function will be used to modify std::shared_ptr<podio::CollectionBase> to the actual collection type
    template <typename T, typename P> const auto& maybeTransformToEDM4hep2(const P& arg) { return arg; }
    template <typename T, typename P>
      requires std::is_base_of_v<podio::CollectionBase, P>
    const auto& maybeTransformToEDM4hep2(P* arg) {
      return *arg;
    }

    template <typename T, typename P>
      requires std::same_as<P, std::shared_ptr<podio::CollectionBase>>
    const auto& maybeTransformToEDM4hep2(P arg) {
      return static_cast<const T&>(*arg);
    }

    template <typename T, bool addPtr = std::is_base_of_v<podio::CollectionBase, T>>
    using SelectType = std::conditional_t<addPtr, std::add_pointer_t<T>, T>;

    template <typename Tuple, typename F, std::size_t... I>
    decltype(auto) apply_helper(F&& f, Tuple&& t, std::index_sequence<I...>) {
      return (std::forward<F>(f)(std::get<I...>(std::forward<Tuple>(t))), 0);
    }

    template <typename Tuple, typename F> decltype(auto) apply_index_sequence(F&& f, Tuple&& t) {
      constexpr std::size_t tuple_size = std::tuple_size<std::remove_reference_t<Tuple>>::value;
      return apply_helper(std::forward<F>(f), std::forward<Tuple>(t), std::make_index_sequence<tuple_size>{});
    }

    template <typename Tuple, typename F> decltype(auto) apply(F&& f, auto ctx, Tuple&& t) {
      return apply_index_sequence(std::forward<F>(f), ctx, std::forward<Tuple>(t));
    }

    template <typename T> const auto& transformtoEDM4hep(const std::shared_ptr<podio::CollectionBase>& arg) {
      return static_cast<const T>(*arg);
    }

    template <typename T> struct is_map_like : std::false_type {};

    template <typename Value> struct is_map_like<std::map<std::string, Value>> : std::true_type {};

    inline std::string hash_string(const std::string& str) {
      std::string result = "_";
      for (auto& c : std::to_string(std::hash<std::string>{}(str))) {
        result += 'a' + (c - '0');
      }
      return result;
    }

    template <typename T, typename K> std::string getName(const K& first, bool pair = false) {
      if constexpr (is_map_like<T>::value) {
        if (pair) {
          return hash_string(first.first);
        }
        return first.first;
      }
      if (pair) {
        return first.first;
      }
      return hash_string(first.first);
    }

    // transformType function to transform the types from the ones that the user wants
    // like edm4hep::MCParticleCollection, to the ones that are actually stored in the
    // event store, like std::shared_ptr<podio::CollectionBase>
    // For std::map<std::string, Coll>, th

    template <typename T> struct transformType {
      using type = T;
    };

    template <typename T>
      requires std::is_base_of_v<podio::CollectionBase, T> || is_map_like<T>::value
    struct transformType<T> {
      using type = std::shared_ptr<podio::CollectionBase>;
    };

    template <typename T, std::enable_if_t<!std::is_same_v<std::shared_ptr<podio::CollectionBase>, T>, int> = 0>
    auto ptrOrCast(T&& arg) {
      // return arg;
      return std::shared_ptr<podio::CollectionBase>(std::make_shared<T>(std::move(arg)));
    }
    template <typename T, std::enable_if_t<std::is_same_v<std::shared_ptr<podio::CollectionBase>, T>, int> = 0>
    auto ptrOrCast(T&& arg) {
      // return arg;
      std::cout << "Calling static_cast<const T&>(*arg) (ptrOrCast)" << std::endl;
      return static_cast<const T&>(*arg);
    }

    template <typename... In> struct filter_evtcontext_tt {
      static_assert(!std::disjunction_v<std::is_same<EventContext, In>...>,
                    "EventContext can only appear as first argument");

      template <typename Algorithm, typename Handles> static auto apply(const Algorithm& algo, Handles& handles) {
        return std::apply(
            [&](const auto&... handle) { return algo(get(handle, algo, Gaudi::Hive::currentContext())...); }, handles);
      }

      template <typename Algorithm, typename Handles>
      static auto apply(const Algorithm& algo, const EventContext& ctx, Handles& handles) {
        return std::apply(
            [&](const auto&... handle) { return algo(maybeTransformToEDM4hep<In>(get(handle, algo, ctx))...); },
            handles);
      }
    };

    template <size_t Index, typename... In, typename... Handles>
    void readMapInputs(const std::tuple<Handles...>& handles, const auto& m_inputLocations, auto& m_inputLocationsMap,
                       auto thisClass) {
      if constexpr (Index < sizeof...(Handles)) {
        if constexpr (is_map_like<std::tuple_element_t<Index, std::tuple<In...>>>::value) {
          // In case of map types like std::map<std::string, edm4hep::MCParticleCollection&>
          // we have to remove the reference to get the actual type
          using EDM4hepType =
              std::remove_reference_t<typename std::tuple_element_t<Index, std::tuple<In...>>::mapped_type>;
          auto inputMap = std::map<std::string, const EDM4hepType&>();

          // To be locked
          if (!m_inputLocationsMap.contains(std::get<Index>(handles).objKey())) {
            auto vec = std::vector<std::string>();
            vec.reserve(m_inputLocations[Index].value().size());
            for (auto& val : m_inputLocations[Index].value()) {
              vec.push_back(val.key());
            }
            m_inputLocationsMap[std::get<Index>(handles).objKey()] = vec;
          }

          for (auto& value : m_inputLocationsMap.at(std::get<Index>(handles).objKey())) {
            DataObject* p;
            auto        sc = thisClass->evtSvc()->retrieveObject(value, p);
            if (!sc.isSuccess()) {
              throw GaudiException("Failed to retrieve object " + value, "Consumer", StatusCode::FAILURE);
            }
            const auto collection = dynamic_cast<AnyDataWrapper<std::shared_ptr<podio::CollectionBase>>*>(p);
            auto       ptr        = std::dynamic_pointer_cast<EDM4hepType>(collection->getData());
            inputMap.emplace(value, *ptr);
          }
          // std::get<Index>(handles).put(std::move(inputMap));
        }

        // Recursive call for the next index
        readMapInputs<Index + 1, In...>(handles, m_inputLocations, m_inputLocationsMap, thisClass);
      }
    }

    template <size_t Index, typename... In, typename... Handles>
    void deleteMapInputs(const std::tuple<Handles...>& handles, auto thisClass) {
      if constexpr (Index < sizeof...(Handles)) {
        if constexpr (is_map_like<std::tuple_element_t<Index, std::tuple<In...>>>::value) {
          // In case of map types like std::map<std::string, edm4hep::MCParticleCollection&>
          // we have to remove the reference to get the actual type
          auto sc = thisClass->evtSvc()->unregisterObject(std::get<Index>(handles).objKey());
          if (!sc.isSuccess()) {
            throw GaudiException("Failed to retrieve object " + std::get<Index>(handles).objKey(), "Consumer",
                                 StatusCode::FAILURE);
          }
        }
        // Recursive call for the next index
        deleteMapInputs<Index + 1, In...>(handles, thisClass);
      }
    }

  }  // namespace details
}  // namespace k4FWCore

#endif  // CORE_FUNCTIONALUTILS_H
